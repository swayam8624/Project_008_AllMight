#!/usr/bin/env python3
"""Audit compact print diagram candidates against canonical SVG semantics and fit caps."""
import base64,hashlib,json,re
from collections import defaultdict
from pathlib import Path
from xml.etree import ElementTree as ET
HERE=Path(__file__).resolve().parent; LATEX=HERE.parent; PROD=LATEX.parent
prov=json.loads((HERE/'compact-print-provenance.json').read_text())
jobs=json.loads((HERE/'compact-print-variants.jobs.json').read_text())['assets']
jobs_by_id={j['id']:j for j in jobs}
source=json.loads((PROD/'jobs.json').read_text())
source_by_hash={hashlib.sha256(j['code'].encode()).hexdigest():j for j in source['jobs'] if j.get('kind')=='mermaid'}
def source_record(a):
 sh=a['source_code_sha256']
 if sh in source_by_hash: return source_by_hash[sh]['code'],Path(source_by_hash[sh]['outfile'])
 for row in a.get('source_rows',[]):
  p=Path(row['source'])
  if p.exists():
   for m in re.finditer(r'```mermaid\s*\n(.*?)\n```',p.read_text(),re.S|re.I):
    code=m.group(1)
    if hashlib.sha256(code.encode()).hexdigest()==sh: return code,Path(a['source_svg'])
 raise KeyError(sh)

def sig(path):
 r=ET.parse(path).getroot(); nodes=[]; edges=[]; labels=[]
 for e in r.iter():
  c=e.attrib.get('class','').split(); tag=e.tag.rsplit('}',1)[-1]
  if 'node' in c and tag=='g':
   raw=e.attrib.get('id',''); m=re.search(r'-(?:flowchart|state)-(.+)$',raw)
   if m: nodes.append(re.sub(r'-\d+$','',m.group(1)))
  if 'flowchart-link' in c: edges.append(e.attrib.get('data-id') or re.sub(r'^.*?-L_','L_',e.attrib.get('id','')))
  if tag=='g' and 'label' in c:
   rows=[''.join(x.itertext()).strip() for x in e.iter() if 'row' in x.attrib.get('class','').split()]
   v=' '.join(x for x in rows if x) if any(rows) else ''.join(e.itertext()).strip()
   if v: labels.append(v)
 return sorted(set(nodes)),sorted(set(edges)),sorted(set(labels))
reflow=json.loads((HERE/'reflow-simple-paths-report.json').read_text())
reflow_by_source={x['source_asset_id']:x for x in reflow['variants']}
results=[]; remaining=[]
for a in prov['assets']:
 sh=a['source_code_sha256']; original,canonical_path=source_record(a); canonical=sig(canonical_path); cand=[]
 for v in a['variants']:
  p=Path(v['svg']); assert p.exists() and p.with_suffix('.pdf').exists(),p
  S=ET.parse(p).getroot(); w=float(S.attrib['width'].removesuffix('px'))*.75; h=float(S.attrib['height'].removesuffix('px'))*.75
  fit=min(1,.98*447.84/w,.62*606.24/h); font=(a.get('source_font_px') or 12)*.75*fit
  same=sig(p)==canonical
  if not same: raise SystemExit(f"semantic SVG mismatch {v['variant_id']}")
  cand.append({'id':v['variant_id'],'direction':v['direction'],'svg':str(p),'pdf':v['pdf'],'wpt':round(w,2),'hpt':round(h,2),'fontpt':round(font,2),'pass':font>=8,'sha':hashlib.sha256(p.read_bytes()).hexdigest()})
  if v['direction'] in ('LR','TB'):
   clean=re.sub(r"%%\{init: \{'themeVariables': \{'fontSize': '12px', 'fontFamily': 'Arial'\}, 'flowchart': \{'nodeSpacing': \d+, 'rankSpacing': \d+, 'padding': 4, 'wrappingWidth': \d+, 'htmlLabels': false\}\}\}%%\n",'',jobs_by_id[v['variant_id']]['code'],count=1)
   d0=re.search(r'^\s*(?:flowchart|graph)\s+(LR|RL|TD|TB|BT)\b',original,re.M)
   d1=re.search(r'^\s*(?:flowchart|graph)\s+(LR|TB)\b',clean,re.M)
   clean=re.sub(r'(^\s*(?:flowchart|graph)\s+)(LR|TB)\b',lambda m:m.group(1)+d0.group(1),clean,count=1,flags=re.M)
   if clean!=original: raise SystemExit(f"source code mismatch {v['variant_id']}")
 if a['source_asset_id'] in reflow_by_source:
  rr=reflow_by_source[a['source_asset_id']]; p=Path(rr['svg']); S=ET.parse(p).getroot(); w=float(S.attrib['width'].removesuffix('px'))*.75; h=float(S.attrib['height'].removesuffix('px'))*.75; fit=min(1,.98*447.84/w,.62*606.24/h); font=9*fit
  if sig(p)!=canonical: raise SystemExit(f"reflow semantics changed for {rr['variant']}")
  cand.append({'id':rr['variant'],'direction':'wrap2','svg':str(p),'pdf':rr['pdf'],'wpt':round(w,2),'hpt':round(h,2),'fontpt':round(font,2),'pass':font>=8,'sha':hashlib.sha256(p.read_bytes()).hexdigest()})
 cand.sort(key=lambda x:(x['pass'],-x['hpt'],x['fontpt'],x['direction']=='LR'),reverse=True)
 selected=cand[0]
 if not selected['pass']: remaining.append(a['source_asset_id'])
 results.append({'source_asset_id':a['source_asset_id'],'source_code_sha256':sh,'source_semantics_match':True,'candidates':cand,'selected':selected['id'],'projected_label_pt':selected['fontpt'],'meets_8pt':selected['pass']})
report={'schema':'allmight-print-diagram-batch-audit-v1','source_assets':len(results),'candidate_variants':sum(len(x['candidates']) for x in results),'all_candidate_semantics_match':True,'all_source_codes_reconstructed':True,'fit_caps':{'width_factor':.98,'height_factor':.62,'minimum_pt':8,'text_width_pt':447.84,'text_height_pt':606.24},'promoted_count':len(results)-len(remaining),'remaining_below_8pt':remaining,'assets':results}
(HERE/'compact-print-selection-audit.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps({k:report[k] for k in ('source_assets','candidate_variants','promoted_count','remaining_below_8pt')},indent=2))
