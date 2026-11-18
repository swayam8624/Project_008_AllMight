#!/usr/bin/env python3
"""Emit source-audited print-only diagram override choices; source Markdown/SVG stay unchanged."""
import hashlib,json
from pathlib import Path
HERE=Path(__file__).resolve().parent; LATEX=HERE.parent
r=json.loads((HERE/'compact-print-selection-audit.json').read_text())
prov=json.loads((HERE/'compact-print-provenance.json').read_text())
pmap={x['source_asset_id']:x for x in prov['assets']}
entries=[]
override_path=LATEX/'Assets'/'Diagrams'/'Print-v1'/'overrides.json'
previous=json.loads(override_path.read_text()) if override_path.is_file() else {}
previous_by_id={x['source_asset_id']:x for x in previous.get('overrides',[])}
recomposed=[]
for a in r['assets']:
 p=pmap[a['source_asset_id']]
 chosen=next(c for c in a['candidates'] if c['id']==a['selected'])
 entry={'source_asset_id':a['source_asset_id'],'source_code_sha256':a['source_code_sha256'],
  'canonical_svg':p['source_svg'],'source_markdown':p['source_markdown'],'source_kind':p['source_asset_type'],
  'print_svg':chosen['svg'],'print_pdf':chosen['pdf'],'variant':chosen['direction'],
  'projected_minimum_label_pt':chosen['fontpt'],'meets_8pt':chosen['pass'],
  'source_semantics_verified':a['source_semantics_match']}
 retained=previous_by_id.get(a['source_asset_id'],{})
 panels=retained.get('panels',[])
 if (not entry['meets_8pt'] and retained.get('source_code_sha256')==a['source_code_sha256']
     and retained.get('source_graph_sha256')==a['source_code_sha256']
     and retained.get('page_proof_sha256') and panels):
  valid=True
  for number,panel in enumerate(panels,1):
   pdf=Path(panel.get('print_pdf',''))
   svg=Path(panel.get('print_svg',''))
   if (panel.get('panel')!=number or not pdf.is_file()
       or hashlib.sha256(pdf.read_bytes()).hexdigest()!=panel.get('pdf_sha256')
       or not svg.is_file() or hashlib.sha256(svg.read_bytes()).hexdigest()!=panel.get('svg_sha256')
       or float(panel.get('projected_minimum_label_pt',0))<8):
    valid=False; break
  if valid:
   entry.update({'source_graph_sha256':retained['source_graph_sha256'],
    'page_proof_sha256':retained['page_proof_sha256'],'panels':panels,
    'print_pdf':panels[0]['print_pdf'],'print_svg':panels[0]['print_svg'],'variant':'semantic-panels',
    'projected_minimum_label_pt':min(x['projected_minimum_label_pt'] for x in panels),
    'meets_8pt':True})
   recomposed.append(a['source_asset_id'])
 entries.append(entry)
held=[x['source_asset_id'] for x in entries if not x['meets_8pt']]
out={'schema':'allmight-print-diagram-overrides-v2-panels' if recomposed else 'allmight-print-diagram-overrides-v1',
 'candidate_source_asset_count':len(entries),
 'selected_meeting_minimum':sum(x['meets_8pt'] for x in entries),'held_for_panel_recomposition':held,
 'panel_recomposed_source_asset_ids':recomposed,
 'selection_audit_sha256':hashlib.sha256((HERE/'compact-print-selection-audit.json').read_bytes()).hexdigest(),
 'overrides':entries}
override_path.write_text(json.dumps(out,indent=2)+'\n')
print(json.dumps({k:out[k] for k in ('candidate_source_asset_count','selected_meeting_minimum','held_for_panel_recomposition')},indent=2))
