#!/usr/bin/env python3
"""Prepare source-audited compact candidates for all below-threshold print Mermaid assets."""
from __future__ import annotations
import hashlib, json, re
from collections import defaultdict
from pathlib import Path
from prepare_layout_variants import inject_compact_config, replace_direction

HERE=Path(__file__).resolve().parent
LATEX=HERE.parent
PRODUCTION=LATEX.parent
AUDIT=LATEX/'QA'/'figure-layout-audit.json'
JOBS_SOURCE=PRODUCTION/'jobs.json'
OUT=LATEX/'Assets'/'Diagrams'/'Print-v1'
JOBS_OUT=HERE/'compact-print-variants.jobs.json'
PROV_OUT=HERE/'compact-print-provenance.json'


def sha(value): return hashlib.sha256(value.encode()).hexdigest()

def extract_source_hash(source: Path, expected: str):
    text=source.read_text()
    for match in re.finditer(r'```mermaid\s*\n(.*?)\n```',text,re.S|re.I):
        code=match.group(1)
        if sha(code)==expected: return code
    return None

def source_code(record, jobs_by_hash):
    failures=[]
    for row in record.get('source_rows',[]):
        h=row['code_sha256']
        if h in jobs_by_hash: return jobs_by_hash[h]['code'],row
        source=Path(row['source'])
        if source.exists():
            code=extract_source_hash(source,h)
            if code is not None: return code,row
        failures.append({'source':row.get('source'),'sha256':h})
    raise ValueError(f"No available exact source block for {record['source_id']}: {failures}")

def diagram_type(code):
    for line in code.splitlines():
        m=re.match(r'^\s*(flowchart|graph|sequenceDiagram|stateDiagram-v2)\b',line)
        if m:return m.group(1)
    raise ValueError('Unsupported Mermaid type or missing diagram declaration')

def main():
    OUT.mkdir(parents=True,exist_ok=True)
    loaded=json.loads(JOBS_SOURCE.read_text())
    jobs_by_hash={sha(j['code']):j for j in loaded['jobs'] if j.get('kind')=='mermaid'}
    audit=json.loads(AUDIT.read_text())
    jobs=[]; provenance=[]; seen=set()
    for record in audit['assets']:
        proj=record['current_print_style_projection']
        if not proj.get('below_8pt'): continue
        code,row=source_code(record,jobs_by_hash); code_hash=sha(code)
        if code_hash not in {r['code_sha256'] for r in record['source_rows']}:
            raise ValueError(f"source hash mismatch for {record['source_id']}")
        base=diagram_type(code)
        directions=('LR','TB') if base in ('flowchart','graph') else ('source',)
        if record['source_id'] in seen: raise ValueError(f"duplicate print asset {record['source_id']}")
        seen.add(record['source_id'])
        variants=[]
        for index,direction in enumerate(directions):
            nodes=(12,18)[index%2]; ranks=(12,16)[index%2]; wrap=104
            if direction=='source':
                transformed=code
                config=("%%%%{init: {'themeVariables': {'fontSize': '12px', 'fontFamily': 'Arial'}, "
                        "'state': {'nodeSpacing': 12, 'rankSpacing': 12, 'padding': 4}}}%%%%\n") if base=='stateDiagram-v2' else (
                        "%%%%{init: {'themeVariables': {'fontSize': '12px', 'fontFamily': 'Arial'}, "
                        "'sequence': {'actorFontSize': 12, 'messageFontSize': 12, 'noteFontSize': 12, "
                        "'actorMargin': 12, 'messageMargin': 18, 'diagramMarginX': 12, 'diagramMarginY': 12}}}%%%%\n")
                lines=transformed.splitlines(keepends=True); at=0
                while at<len(lines) and lines[at].lstrip().startswith('%%{init:'): at+=1
                lines.insert(at,config.replace('%%%%','%%'))
                transformed=''.join(lines)
            else:
                transformed=replace_direction(code,direction)
                transformed=inject_compact_config(transformed,nodes,ranks,wrap)
            variant_id=f"{record['source_id']}-{direction.lower()}"
            outfile=OUT/f'{variant_id}.svg'
            jobs.append({'id':variant_id,'kind':'mermaid','code':transformed,'outfile':str(outfile),
                         'source':row['source'],'volume':row.get('volume','unknown')})
            variants.append({'variant_id':variant_id,'direction':direction,'renderer_code_sha256':sha(transformed),
                             'svg':str(outfile),'pdf':str(outfile.with_suffix('.pdf'))})
        source_pdf=Path(record['print_pdf'])
        provenance.append({'source_asset_id':record['source_id'],'source_asset_type':row['kind'],
            'source_markdown':row['source'],'volume':row.get('volume'),'source_code_sha256':code_hash,
            'source_svg':record['source_svg'],'source_svg_sha256':record.get('source_svg_sha256_actual'),
            'reference_pdf':str(source_pdf),'reference_pdf_sha256':record.get('print_pdf_sha256_actual'),
            'original_min_font_pt':proj.get('effective_minimum_explicit_font_pt'),
            'source_font_px':((record.get('source_font_pt') or {}).get('root_base') or 12),
            'diagram_type':base,'variants':variants,
            'source_rows':[{'kind':r['kind'],'source':r['source'],'code_sha256':r['code_sha256']} for r in record['source_rows']]})
    JOBS_OUT.write_text(json.dumps({'schema':'allmight-compact-print-jobs-v1','assets':jobs},indent=2)+'\n')
    PROV_OUT.write_text(json.dumps({'schema':'allmight-compact-print-provenance-v1','audit_sha256':hashlib.sha256(AUDIT.read_bytes()).hexdigest(),
        'source_jobs_sha256':hashlib.sha256(JOBS_SOURCE.read_bytes()).hexdigest(),
        'source_asset_count':len(provenance),'candidate_count':len(jobs),'assets':provenance},indent=2)+'\n')
    print(f"prepared {len(jobs)} candidates for {len(provenance)} below-threshold source assets")

if __name__=='__main__': main()
