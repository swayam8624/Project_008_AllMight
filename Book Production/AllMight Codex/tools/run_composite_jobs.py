#!/usr/bin/env python3
"""Run versioned post-render SVG composites, then atomically publish the asset.

Invoke after canonical asset rendering and before any font, print-PDF, native,
EPUB, or production-manifest validation. A clean canonical rerender overwrites
composited paths, so this stage must run again before downstream validation.
"""
from __future__ import annotations
import argparse, hashlib, json, os, pathlib, re, shutil, subprocess, sys, tempfile, xml.etree.ElementTree as ET


def sha(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()

def json_sha(value) -> str:
    return sha(json.dumps(value, ensure_ascii=False, sort_keys=True, separators=(',', ':')).encode())

def node_edges(code: str):
    nodes = {m.group(1): (m.group(2) or m.group(3)) for m in re.finditer(r'(?<![A-Z])([A-Z])\[(?:"([^"]*)"|([^\]]*))\]', code)}
    edges = {tuple(m.groups()) for m in re.finditer(r'([A-Z])(?:\[[^\]]+\])?\s*--?>\s*([A-Z])\b', code)}
    return nodes, edges

def block_from_source(source_path: pathlib.Path, expected_code: str):
    blocks = re.findall(r'```mermaid\s*\n(.*?)\n```', source_path.read_text(), re.S)
    matches = [b for b in blocks if b == expected_code]
    if len(matches) != 1:
        raise RuntimeError(f'Expected one exact Mermaid source block in {source_path}; found {len(matches)}')
    return matches[0]

def svg_content(path: pathlib.Path):
    root = ET.parse(path).getroot()
    nodes, edges, labels = set(), set(), {}
    for e in root.iter():
        ident=e.attrib.get('id','')
        m=re.search(r'-flowchart-([A-Z])-\d+$', ident)
        if m: nodes.add(m.group(1))
        data_id=e.attrib.get('data-id','')
        m=re.fullmatch(r'L_([A-Z])_([A-Z])_\d+',data_id)
        if m: edges.add((m.group(1),m.group(2)))
    for e in root.iter():
        ident=e.attrib.get('id','')
        m=re.search(r'-flowchart-([A-Z])-\d+$',ident)
        if not m: continue
        node=m.group(1)
        text=next((x for x in e.iter() if x.tag.endswith('text')),None)
        if text is None: continue
        rows=[]
        for outer in list(text):
            line=''.join((x.text or '') for x in outer.iter() if x is not outer)
            if line: rows.append(line)
        labels.setdefault(node,[]).append(' '.join(rows))
    return nodes,edges,labels

def atomic_write(path: pathlib.Path, data: bytes):
    path.parent.mkdir(parents=True,exist_ok=True)
    tmp=path.with_name(path.name+'.postrender-tmp')
    tmp.write_bytes(data)
    os.replace(tmp,path)

def verify_packaged_sources(manifest: dict, root: pathlib.Path, job: dict):
    rows = manifest.get('recovery_sources')
    if not isinstance(rows, list) or not rows:
        raise RuntimeError('Production-local recovery_sources are required')
    checked = {}
    for row in rows:
        rel = pathlib.Path(row.get('path', ''))
        if not row.get('path') or rel.is_absolute() or '..' in rel.parts:
            raise RuntimeError('Recovery SVG paths must be safe production-relative paths')
        path = (root / rel).resolve()
        if root not in path.parents or not path.is_file():
            raise RuntimeError(f'Packaged recovery SVG is missing or outside production root: {rel}')
        digest = sha(path.read_bytes())
        if digest != row.get('sha256'):
            raise RuntimeError(f'Packaged recovery SVG hash mismatch: {rel}')
        checked[row['path']] = digest
    recovery = job.get('recovery_copy')
    if recovery not in checked or checked[recovery] != job.get('recovery_copy_sha256'):
        raise RuntimeError('Job recovery copy is not covered by the packaged source ledger')
    for component in job.get('components', []):
        rel = component.get('packaged_source_svg')
        if rel not in checked or checked[rel] != component.get('source_svg_sha256'):
            raise RuntimeError(f"Packaged source panel hash mismatch for {component.get('id')}")
    def scan(value):
        if isinstance(value, dict):
            for child in value.values(): scan(child)
        elif isinstance(value, list):
            for child in value: scan(child)
        elif isinstance(value, str) and '.codex/source-cache/' in value:
            raise RuntimeError('Production composite manifest still references a private source-cache path')
    scan(manifest)


def run_one(manifest_path: pathlib.Path, job: dict, publish: bool):
    manifest=json.loads(manifest_path.read_text())
    root=manifest_path.parent.resolve()
    verify_packaged_sources(manifest, root, job)
    renderer=root/job.get('renderer_script','tools/render_assets.cjs')
    compositor=root/job.get('compositor_script','tools/compose_fcbea_panels.py')
    if not renderer.is_file() or not compositor.is_file(): raise RuntimeError('Manifest renderer/compositor tool is missing')
    source_job_path=root/'jobs.json'
    source_doc=json.loads(source_job_path.read_text())
    canonical=next((j for j in source_doc['jobs'] if j['id']==job['source_job_id']),None)
    if canonical is None: raise RuntimeError('Canonical source job not found')
    canonical_sha=sha(canonical['code'].encode())
    if canonical_sha!=job['source_code_sha256']: raise RuntimeError('Canonical source-job code hash changed')
    if block_from_source(pathlib.Path(canonical['source']),canonical['code'])!=canonical['code']:
        raise RuntimeError('Canonical source Markdown block no longer matches source job')
    source_nodes,source_edges=node_edges(canonical['code'])
    semantics=job['source_semantics']
    if source_nodes!=semantics['node_labels'] or sorted(map(list,source_edges))!=sorted(semantics['edges']):
        raise RuntimeError('Source code nodes/edges do not match the composite semantic ledger')
    if not semantics['combined_checks']['all_nodes_cover_source'] or not semantics['combined_checks']['all_edges_match_source'] or not semantics['combined_checks']['all_source_label_chars_match']:
        raise RuntimeError('Composite semantic ledger is not closed')
    for c in job['components']:
        if sha(c['code'].encode())!=c['code_sha256']:
            raise RuntimeError(f"Component code hash mismatch for {c['id']}")
        cn,ce=node_edges(c['code'])
        if sorted(cn)!=sorted(c['expected_node_ids']) or sorted(map(list,ce))!=sorted(c['expected_edges']):
            raise RuntimeError(f"Component node/edge ledger mismatch for {c['id']}")
    if set().union(*(set(c['expected_node_ids']) for c in job['components']))!=set(source_nodes):
        raise RuntimeError('Component node union differs from source nodes')
    if set(tuple(e) for c in job['components'] for e in c['expected_edges'])!=source_edges:
        raise RuntimeError('Component edge union differs from source edges')

    work_parent=root/'QA'/'composite-stage'
    work_parent.mkdir(parents=True,exist_ok=True)
    with tempfile.TemporaryDirectory(prefix=job['id']+'-',dir=work_parent) as t:
        work=pathlib.Path(t)
        component_jobs=[]
        for c in job['components']:
            component_jobs.append({'id':c['id'],'kind':'mermaid','display':True,'code':c['code'],'outfile':str(work/c['output_name'])})
        jobs_path=work/'component-jobs.json'; jobs_path.write_text(json.dumps(component_jobs,ensure_ascii=False,indent=2)+'\n')
        render_path=work/'component-render-report.json'
        proc=subprocess.run(['node',str(renderer),str(jobs_path),str(render_path)],capture_output=True,text=True)
        if proc.returncode: raise RuntimeError(f'Canonical component renderer failed: {proc.stderr[-2000:]}')
        render=json.loads(render_path.read_text())
        if render['failed'] or render['unique_asset_count']!=len(component_jobs):
            raise RuntimeError(f'Component render report not complete: {render.get("errors")}')
        render_by={x['id']:x for x in render['assets']}
        component_results=[]
        for c in job['components']:
            rr=render_by[c['id']]; svg_path=pathlib.Path(rr['outfile'])
            if rr['original_code_sha256']!=c['code_sha256'] or rr['svg_sha256']!=sha(svg_path.read_bytes()):
                raise RuntimeError(f"Rendered component proof failed for {c['id']}")
            n,e,l=svg_content(svg_path)
            if n!=set(c['expected_node_ids']) or e!={tuple(x) for x in c['expected_edges']}:
                raise RuntimeError(f"Rendered component graph IDs/edges failed for {c['id']}")
            expected=source_nodes
            for node in c['expected_node_ids']:
                if not l.get(node) or any(''.join(x.split())!=''.join(expected[node].split()) for x in l[node]):
                    raise RuntimeError(f"Rendered component label failed for {c['id']} node {node}")
            component_results.append({'id':c['id'],'code_sha256':c['code_sha256'],'renderer_version':rr['renderer_version'],'svg_path':str(svg_path),'svg_sha256':rr['svg_sha256'],'dimensions_pt':[rr['true_width_pt'],rr['true_height_pt']],'node_ids':sorted(n),'edges':[list(x) for x in sorted(e)]})
        composite_stage=work/'composite.svg'; composite_stage_report=work/'composite-report.json'
        proc=subprocess.run([sys.executable,str(compositor),'--panel-a',component_results[0]['svg_path'],'--panel-b',component_results[1]['svg_path'],'--output',str(composite_stage),'--report',str(composite_stage_report)],capture_output=True,text=True)
        if proc.returncode: raise RuntimeError(f'Composite renderer failed: {proc.stderr[-2000:]}')
        cr=json.loads(composite_stage_report.read_text())
        if not cr['fits_396x500pt'] or cr['unresolved_local_references'] or cr['prefixed_id_counts']['total_unique']<1:
            raise RuntimeError('Composite SVG fails dimensions, ID, or reference checks')
        final_nodes,final_edges,final_labels=svg_content(composite_stage)
        if final_nodes!=set(source_nodes) or final_edges!=source_edges:
            raise RuntimeError('Composed SVG node/edge set differs from source graph')
        for node,label in source_nodes.items():
            if not final_labels.get(node) or any(''.join(x.split())!=''.join(label.split()) for x in final_labels[node]):
                raise RuntimeError(f'Composed SVG label proof failed for source node {node}')
        if len({e.attrib['id'] for e in ET.parse(composite_stage).getroot().iter() if 'id' in e.attrib})!=cr['prefixed_id_counts']['total_unique']:
            raise RuntimeError('Composed SVG has duplicate or unaccounted IDs')
        out=pathlib.Path(job['output_path']); digest=sha(composite_stage.read_bytes())
        if publish:
            atomic_write(out,composite_stage.read_bytes())
        report={'schema':'allmight-postrender-composite-report-v1','status':'published' if publish else 'validated-private','job_id':job['id'],'manifest':str(manifest_path),'manifest_sha256':sha(manifest_path.read_bytes()),'manifest_version':manifest['manifest_version'],'composite_renderer_version':manifest['compositor_version'],'compositor_script_sha256':sha(compositor.read_bytes()),'component_renderer_script_sha256':sha(renderer.read_bytes()),'component_renderer_version':render['renderer_version'],'source_job_id':job['source_job_id'],'source_file':canonical['source'],'source_code_sha256':canonical_sha,'source_svg_before_composition_sha256':job.get('canonical_source_svg_sha256_before_composite'),'components':component_results,'transform_report':cr,'output_path':str(out),'output_sha256':digest,'output_dimensions_pt':cr['outer_dimensions_pt'],'shrink_only_fit_scale':min(1.0,396/cr['outer_dimensions_pt'][0],500/cr['outer_dimensions_pt'][1]),'node_ids':sorted(final_nodes),'edges':[list(x) for x in sorted(final_edges)],'source_node_labels':source_nodes,'all_nodes_edges_labels_pass':True,'source_markdown_changed':False,'source_job_code_changed':False,'reproducibility_command':f'python3 {manifest_path.parent}/tools/run_composite_jobs.py {manifest_path.name} --job-id {job["id"]} --publish'}
    report_path=pathlib.Path(job['output_report_path']) if publish else (manifest_path.parent/'QA'/'composite-stage'/f'{job["id"]}-private-run-report.json')
    atomic_write(report_path,json.dumps(report,ensure_ascii=False,indent=2).encode()+b'\n')
    return report


def main():
    ap=argparse.ArgumentParser(description='Run versioned post-render composite jobs.')
    ap.add_argument('manifest',type=pathlib.Path)
    ap.add_argument('--job-id')
    ap.add_argument('--publish',action='store_true',help='Atomically write the composite to its production asset path.')
    args=ap.parse_args(); manifest_path=args.manifest.resolve(); manifest=json.loads(manifest_path.read_text())
    if manifest.get('schema')!='allmight-postrender-composite-jobs-v1': raise SystemExit('Unknown composite job manifest schema')
    jobs=[j for j in manifest['jobs'] if not args.job_id or j['id']==args.job_id]
    if not jobs: raise SystemExit('No matching composite job')
    results=[run_one(manifest_path,j,args.publish) for j in jobs]
    print(json.dumps({'status':'published' if args.publish else 'validated-private','jobs':[{'id':x['job_id'],'output':x['output_path'],'sha256':x['output_sha256']} for x in results]}))
if __name__=='__main__': main()
