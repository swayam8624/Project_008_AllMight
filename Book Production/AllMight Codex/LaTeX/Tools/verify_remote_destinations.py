#!/usr/bin/env python3
"""Confirm a filtered heading ID is a real named destination in a second PDF."""
import json
from pathlib import Path
from pypdf import PdfReader
HERE=Path(__file__).resolve().parent; QA=HERE.parent/'QA'/'remote-destination-proof'
r1=PdfReader(QA/'stage1.pdf');r2=PdfReader(QA/'stage2.pdf')
name='geometry-basics';dest=r2.named_destinations.get(name)
if dest is None:raise SystemExit(f'missing named destination: {name}')
page_index=next((i for i,p in enumerate(r2.pages) if p.indirect_reference and p.indirect_reference.idnum==dest.page.idnum),None)
if page_index is None:raise SystemExit('named destination does not resolve to a page')
remote=[]
for i,page in enumerate(r1.pages):
 annots=page.get('/Annots');annots=annots.get_object() if hasattr(annots,'get_object') else annots
 for ref in annots or []:
  a=ref.get_object();action=a.get('/A');action=action.get_object() if hasattr(action,'get_object') else action
  if action and action.get('/S')=='/GoToR':
   remote.append({'page':i,'file':str(action.get('/F')),'destination':str(action.get('/D'))})
match=next((x for x in remote if x['file']=='stage2.pdf' and name in x['destination']),None)
if match is None:raise SystemExit(f'no remote link to {name}: {remote}')
report={'schema':'allmight-remote-named-destination-proof-v1','remote_pdf_count':2,
 'destination_name':name,'destination_page_index':page_index,
 'destination_action':match,'pypdf_named_destination_present':True,
 'cross_pdf_link_present':True,'note':'TeX compiled the LaTeX Header-filter output; PyPDF inspected actual PDF name trees, page references, and remote GoToR annotations.'}
(QA/'verification.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps(report,indent=2))
