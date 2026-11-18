#!/usr/bin/env python3
"""Check source-preserving semantics and print-size label projections."""
from __future__ import annotations
import collections, hashlib, json, re
from pathlib import Path
from xml.etree import ElementTree as ET

HERE=Path(__file__).resolve().parent
LATEX=HERE.parent
PRODUCTION=LATEX.parent
OUT=LATEX/'Assets'/'Diagrams'/'Review-v1'
source=json.loads((PRODUCTION/'jobs.json').read_text())
source_by_id={j['id']:j for j in source['jobs'] if j.get('kind')=='mermaid'}
jobs=json.loads((HERE/'compact-layout-variants.jobs.json').read_text())['assets']
render=json.loads((HERE/'compact-layout-variants.report.json').read_text())
render_by_id={a['id']:a for a in render['assets']}

def signature(path):
    root=ET.parse(path).getroot()
    nodes=[]; edges=[]; labels=[]
    for e in root.iter():
        cls=e.attrib.get('class','').split()
        if 'node' in cls:
            raw=e.attrib.get('id','')
            nodes.append(raw.split('-flowchart-',1)[-1])
        elif 'flowchart-link' in cls:
            raw=e.attrib.get('id','')
            edges.append(re.sub(r'^diagram-[^-]+-', '', raw))
        if 'label' in cls and e.tag.rsplit('}',1)[-1]=='g':
            rows=[]
            for child in e.iter():
                if child.attrib.get('class','').split() and 'row' in child.attrib.get('class','').split():
                    value=''.join(child.itertext()).strip()
                    if value: rows.append(value)
            value=' '.join(rows) if rows else ''.join(e.itertext()).strip()
            if value: labels.append(value)
    return {'node_ids':sorted(nodes),'edge_ids':sorted(edges),'labels':sorted(labels)}

records=[]; pairs=collections.defaultdict(dict)
for job in jobs:
    vid=job['id']; source_id=vid.rsplit('-',1)[0]; src=source_by_id[source_id]
    variant=Path(job['outfile']); code=job['code']
    # Strip only the injected print configuration and undo the direction override.
    code_without=re.sub(r"%%\{init: \{'themeVariables': \{'fontSize': '12px', 'fontFamily': 'Arial'\}, 'flowchart': \{'nodeSpacing': \d+, 'rankSpacing': \d+, 'padding': 4, 'wrappingWidth': \d+, 'htmlLabels': false\}\}\}%%\n", '', code, count=1)
    direction=re.search(r'^\s*(?:flowchart|graph)\s+(LR|TB)\b',code_without,re.M).group(1)
    original_direction=re.search(r'^\s*(?:flowchart|graph)\s+(LR|RL|TD|TB|BT)\b',src['code'],re.M).group(1)
    restored=re.sub(r'^(\s*(?:flowchart|graph)\s+)(LR|TB)\b',lambda m:m.group(1)+original_direction,code_without,count=1,flags=re.M)
    exact=restored==src['code']
    if not exact: raise SystemExit(f'{vid}: altered source text beyond the intended direction and configuration')
    sig=signature(variant)
    canonical_sig=signature(Path(src['outfile']))
    matches_canonical=sig==canonical_sig
    if not matches_canonical: raise SystemExit(f'{vid}: rendered node IDs, edge IDs, or labels differ from canonical SVG')
    svg=ET.parse(variant).getroot()
    width=float(svg.attrib['width'].removesuffix('px'))*.75
    height=float(svg.attrib['height'].removesuffix('px'))*.75
    scale=min(1.0,(.98*447.84)/width,(.62*606.24)/height)
    min_font=9.0*scale
    record={'variant_id':vid,'source_code_reconstructed_exactly':exact,'matches_canonical_svg_semantics':matches_canonical,
            'source_code_sha256':hashlib.sha256(src['code'].encode()).hexdigest(),
            **sig,'svg_sha256':hashlib.sha256(variant.read_bytes()).hexdigest(),
            'svg_width_pt':round(width,3),'svg_height_pt':round(height,3),
            'panel_fit_scale':round(scale,5),'projected_base_label_pt':round(min_font,3),
            'meets_8pt':min_font>=8.0,'print_asset_pdf':str(variant.with_suffix('.pdf'))}
    records.append(record); pairs[source_id][direction]=sig
for sid, pair in pairs.items():
    if pair.get('LR')!=pair.get('TB'):
        raise SystemExit(f'{sid}: node IDs, edges, or labels changed across orientation variants')
report={'schema':'allmight-layout-variant-verification-v1',
        'method':'Input code was reconstructed byte-for-byte after removing one inserted layout directive and reverting only graph direction. SVG node IDs, edge IDs and rendered label text were compared between paired layouts.',
        'print_fit':{'text_width_pt':447.84,'text_height_pt':606.24,'panel_width_factor':.98,'panel_height_factor':.62,'source_label_px':12,'source_label_pt':9,'minimum_label_pt':8},
        'all_semantic_invariants_pass':True,'canonical_node_edge_and_label_signatures_match':True,'variant_count':len(records),'variants':records}
(OUT/'layout-variant-verification.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps({'variant_count':len(records),'all_semantic_invariants_pass':True,'meets_8pt':sum(x['meets_8pt'] for x in records),'below_8pt':[x['variant_id'] for x in records if not x['meets_8pt']]},indent=2))
