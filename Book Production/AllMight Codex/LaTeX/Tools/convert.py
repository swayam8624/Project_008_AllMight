#!/usr/bin/env python3
"""Convert verified reader AST and original companion notes to LaTeX fragments."""
import base64, collections, hashlib, json, os, re, subprocess, sys, urllib.parse, shutil, tempfile
from pathlib import Path
V=Path('/Users/swayamsingal/Library/Mobile Documents/iCloud~md~obsidian/Documents/Master')
R=V/'Book Production/AllMight Codex'; O=R/'LaTeX'; C=Path('/Users/swayamsingal/.codex/source-cache/allmight-book-production-2026-11-15')
sys.path.insert(0,str(C)); import build_books as b

def sha(x): return hashlib.sha256(x).hexdigest()
def read(p): return json.loads(Path(p).read_text())
def pandoc(data,fr,to,division='chapter'):
 args=['pandoc','-f',fr,'-t',to,'--wrap=none','--no-highlight']
 if to=='latex': args+=['--top-level-division='+division,'--lua-filter='+str(O/'Tools'/'semantic-callouts.lua'),
                        '--lua-filter='+str(O/'Tools'/'diagram-panels.lua')]
 p=subprocess.run(args,input=data,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
 if p.returncode: raise RuntimeError(p.stderr.decode(errors='replace'))
 return p.stdout

def normalize_layout_formula(value):
 value=' '.join(value.split())
 value=re.sub(r'\s*\\(?:rightarrow|to)\s*',lambda match: r'\'+match.group(0).strip().lstrip('\\'),value)
 value=re.sub(r'\{\s+','{',value)
 return re.sub(r'\s+\}', '}',value)

def add_channel_membership_breakpoints(tex):
 """Add invisible line-break points to long JSON values in one source table."""
 marker='Channel / membership'
 header=tex.find(marker)
 if header<0: raise RuntimeError('Expected source table header was not emitted')
 start=tex.rfind(r'\begin{longtable}',0,header)
 end=tex.find(r'\end{longtable}',header)
 if start<0 or end<0: raise RuntimeError('Could not isolate the source metadata longtable')
 lines=tex[start:end].splitlines()
 changed=0
 for position,line in enumerate(lines):
  if r'\{``feed' not in line or ' & ' not in line: continue
  first,separator,rest=line.partition(' & ')
  first=re.sub(r'([,:])',lambda match:match.group(1)+r'\allowbreak{}',first)
  first=first.replace(r'\_',r'\_\allowbreak{}')
  lines[position]=first+separator+rest
  changed+=1
 if changed==0: raise RuntimeError('No channel-membership metadata rows received breakpoints')
 return tex[:start]+'\n'.join(lines)+tex[end:],changed

def set_binary_format_table_widths(table):
 """Give the exact three-column binary-format table fixed wrapping widths."""
 if table.get('t')!='Table' or len(table.get('c',[]))!=6: return False
 c=table['c']
 try:
  cells=c[3][1][0][1]
  header=[' '.join(b.textin(block.get('c',[])) for block in cell[4]) for cell in cells]
 except (IndexError,TypeError,KeyError): return False
 if header!=['Property','binary32','binary64']: return False
 if len(c[2])!=3: return False
 c[2]=[[{'t':'AlignDefault'},{'t':'ColWidth','c':width}] for width in (0.42,0.27,0.31)]
 return True

def walk(x):
 if isinstance(x,dict):
  yield x
  for v in x.values(): yield from walk(v)
 elif isinstance(x,list):
  for v in x: yield from walk(v)
def transform(x,fn):
 if isinstance(x,dict):
  x=fn(x)
  return {k:transform(v,fn) for k,v in x.items()}
 if isinstance(x,list): return [transform(v,fn) for v in x]
 return x

def chapter_page_marker(tex, ident):
 prefix='\\hypertarget{'+ident+'}{}\n'
 if tex.startswith(prefix): tex=tex[len(prefix):]
 start=tex.find('\\chapter{')
 if start<0: raise RuntimeError('No numbered chapter command for '+ident)
 depth=0; end=None
 for i in range(start+len('\\chapter'),len(tex)):
  if tex[i]=='{': depth+=1
  elif tex[i]=='}':
   depth-=1
   if depth==0: end=i+1; break
 if end is None: raise RuntimeError('Unclosed chapter title for '+ident)
 tail=tex[end:]
 generated_label=re.match(r'\\label\{[^}]+\}',tail)
 if generated_label: end+=generated_label.end()
 marker='\\phantomsection\\hypertarget{'+ident+'}{}\\label{stagechapter:'+ident+'}'
 return tex[:end]+marker+tex[end:]

def generated_navigation(block):
 if block.get('t')!='Para' or not isinstance(block.get('c'),list): return None
 links=[]
 for node in block['c']:
  if node.get('t')=='Link': links.append(node)
  elif node.get('t')=='Space': continue
  elif node.get('t')=='Str' and node.get('c')=='·': continue
  else: return None
 labels=[b.textin(link.get('c',[None,[],[]])[1]).strip().lower() for link in links]
 if not links or labels[0]!='reading map' or any(x not in ('reading map','previous chapter','next chapter') for x in labels): return None
 if labels.count('reading map')!=1 or labels.count('previous chapter')>1 or labels.count('next chapter')>1: return None
 return links

def remap_generated_navigation(doc, source, chapter_id, stage_id, neighbors, chapters_by_id, vault_root, source_hash):
 blocks=doc.get('blocks',[])
 if len(blocks)<2 or blocks[0].get('t')!='Header' or blocks[0].get('c',[None])[0]!=1:
  raise RuntimeError('Expected generated chapter title at first AST block: '+chapter_id)
 links=generated_navigation(blocks[1])
 if links is None: raise RuntimeError('Expected generated first-paragraph navigation for '+chapter_id)
 old=[{'label':b.textin(link['c'][1]).strip(),'target':link['c'][2][0]} for link in links]
 map_target=next(item['target'] for item in old if item['label'].lower()=='reading map')
 previous=neighbors[chapter_id]['previous_chapter_id']; following=neighbors[chapter_id]['next_chapter_id']
 def source_link(label,target_id=None):
  if target_id is None: target=map_target
  else:
   target_path=Path(chapters_by_id[target_id]['source']).resolve().relative_to(vault_root).with_suffix('').as_posix()
   target=target_path
  words=label.split()
  content=[]
  for index,word in enumerate(words):
   if index: content.append({'t':'Space'})
   content.append({'t':'Str','c':word})
  return {'t':'Link','c':[['',['wikilink'],[]],content,[target,'']]}
 new=[source_link('Reading map')]
 if previous: new += [{'t':'Space'},{'t':'Str','c':'·'},{'t':'Space'},source_link('Previous chapter',previous)]
 if following: new += [{'t':'Space'},{'t':'Str','c':'·'},{'t':'Space'},source_link('Next chapter',following)]
 blocks[1]['c']=new
 neighbors[chapter_id]['remapped']=True
 neighbors[chapter_id]['source_sha256']=source_hash
 neighbors[chapter_id]['source']=str(source)
 neighbors[chapter_id]['stage_id']=stage_id
 neighbors[chapter_id]['global_reading_order']=neighbors[chapter_id]['global_reading_order']
 neighbors[chapter_id]['chapter_id']=chapter_id
 neighbors[chapter_id]['previous_target']=str(Path(chapters_by_id[previous]['source']).resolve().relative_to(vault_root).with_suffix('')) if previous else None
 neighbors[chapter_id]['next_target']=str(Path(chapters_by_id[following]['source']).resolve().relative_to(vault_root).with_suffix('')) if following else None
 neighbors[chapter_id]['old_generated_links']=old
 neighbors[chapter_id]['reading_map_target']=map_target
 return neighbors[chapter_id]

def restore_blank_padded_display_math(doc, source, exceptions):
 blocks=doc.get('blocks',[]); output=[]; i=0; restored=0
 def is_delimiter(block):
  return block.get('t') in ('Para','Plain') and b.textin(block.get('c',[])).strip()=='$$'
 while i<len(blocks):
  if not is_delimiter(blocks[i]):
   output.append(blocks[i]); i+=1; continue
  end=next((j for j in range(i+1,len(blocks)) if is_delimiter(blocks[j])),None)
  if end is None:
   output.append(blocks[i]); i+=1; continue
  between=blocks[i+1:end]
  allowed={'Str','Space','SoftBreak','LineBreak','RawInline','Math','Emph','Strong','Subscript','Superscript'}
  nodes=[n for block in between for n in walk(block.get('c',[])) if isinstance(n,dict) and 't' in n]
  safe=all(n['t'] in allowed and (n['t']!='RawInline' or n.get('c',[None])[0]=='tex') for n in nodes)
  formula=' '.join(b.textin(block.get('c',[])).strip() for block in between if block.get('t') in ('Para','Plain')).strip()
  if not between or not safe or not formula:
   output.extend(blocks[i:end+1]); i=end+1; continue
  output.append({'t':'Para','c':[{'t':'Math','c':[{'t':'DisplayMath'},formula]}]})
  exceptions.append({'source':str(source),'kind':'blank-padded-display-math-restored','original_delimiter':'$$','rendered':formula})
  restored+=1; i=end+1
 doc['blocks']=output
 return restored

def main():
 m=read(R/'production-manifest.json'); plan=read(C/'candidate3-print-reference-dedup-plan-v2.json')
 catalog=read(O/'Tools/stage-catalog.json'); stages=catalog['stages']
 chapters_by_id={ch['id']:ch for vol in m['volumes'] for ch in vol['chapters']}
 source_to_chapter={str(Path(ch['source']).resolve()):ch['id'] for ch in chapters_by_id.values()}
 stage_by_chapter={}; chapter_order={}; chapter_position={}
 for stage in stages:
  for pos,cid in enumerate(stage['chapters'],1):
   if cid not in chapters_by_id or cid in stage_by_chapter: raise RuntimeError('Invalid or duplicate stage chapter: '+cid)
   stage_by_chapter[cid]=stage['id']; chapter_order[cid]=pos
 if set(stage_by_chapter)!=set(chapters_by_id): raise RuntimeError('Stage catalog must assign every source chapter exactly once')
 stage_by_id={s['id']:s for s in stages}; stage_number={s['id']:s['number'] for s in stages}
 for cid,ch in chapters_by_id.items(): chapter_position[cid]=chapter_order[cid]
 chapter_navigation={}
 global_chapter_order=[cid for learning_stage in stages for cid in learning_stage['chapters']]
 for global_index,cid in enumerate(global_chapter_order):
  chapter_navigation[cid]={'logical_chapter_number':chapter_order[cid],
    'global_reading_order':global_index+1,
    'previous_chapter_id':global_chapter_order[global_index-1] if global_index else None,
    'next_chapter_id':global_chapter_order[global_index+1] if global_index+1<len(global_chapter_order) else None}
 # Companion notes follow their first inbound source-chapter use in the approved stage order.
 assignment_by_target={}
 for i,a in enumerate(plan['assignments']):
  for t in a.get('link_targets',[]):
   for tid in (t.get('target_id'),t.get('base_target_id')):
    if tid: assignment_by_target[tid]=i
 first_use={}
 old_records=read(R/'link-targets.json')['records']
 stage_rank={s['id']:i for i,s in enumerate(stages)}
 for rec in old_records:
  cid=source_to_chapter.get(str(Path(rec.get('source','')).resolve()))
  ai=assignment_by_target.get(rec.get('target_id'))
  if cid is None or ai is None: continue
  stage_id=stage_by_chapter[cid]
  rank=(stage_rank[stage_id],chapter_order[cid])
  if ai not in first_use or rank<first_use[ai][0]: first_use[ai]=(rank,cid)
 direct_first_use=set(first_use)
 direct_first_use_chapter={i:first_use[i][1] for i in direct_first_use}
 # Use direct chapter citations. Companion indexes can cite hundreds of notes at once;
 # those are not a pedagogically useful first-use signal for this assignment.
 first_use_reason={i:'direct-chapter-citation' for i in direct_first_use}
 if len(first_use)!=len(plan['assignments']):
  # A small set of notes is cited only by companion indexes, so retain its recorded source volume.
  legacy_chapters={v['id']:[c['id'] for c in v['chapters']] for v in m['volumes']}
  for i,a in enumerate(plan['assignments']):
   if i in first_use: continue
   candidates=[cid for cid in legacy_chapters.get(a['assigned_volume'],[]) if cid in stage_by_chapter]
   if candidates:
    cid=min(candidates,key=lambda x:(stage_rank[stage_by_chapter[x]],chapter_order[x]))
    first_use[i]=((stage_rank[stage_by_chapter[cid]],chapter_order[cid]),cid)
    first_use_reason[i]='legacy-semantic-volume-fallback'
 if len(first_use)!=len(plan['assignments']): raise RuntimeError('Could not place every companion note by first source use')
 atlas_chapter='04-visual-computing-ii-ch036'
 for i,a in enumerate(plan['assignments']):
  if Path(a['source_path']).name in ('Keyword Index.md','Dependency Map.md'):
   first_use[i]=((stage_rank[stage_by_chapter[atlas_chapter]],chapter_order[atlas_chapter]),atlas_chapter)
   first_use_reason[i]='stage7-reference-atlas'
 parts_path=O/'physical-parts.json'
 part_data=read(parts_path) if parts_path.exists() else None
 part_by_chapter={}; part_meta={}
 if part_data:
  for stage_parts in part_data['stages']:
   for part in stage_parts['parts']:
    part_meta[part['id']]=part
    for cid in part['chapter_ids']:
     if cid in part_by_chapter: raise RuntimeError('Chapter assigned to multiple physical parts: '+cid)
     part_by_chapter[cid]=part['id']
  if set(part_by_chapter)!=set(chapters_by_id): raise RuntimeError('Physical-part map must assign every source chapter exactly once')
 else:
  part_by_chapter={cid:stage_by_chapter[cid] for cid in chapters_by_id}
 part_by_reference={}; stage_by_label={}
 for cid,pid in part_by_chapter.items(): stage_by_label[cid]=stage_by_chapter[cid]
 for i,a in enumerate(plan['assignments']):
  ident='appendix-'+sha(a['source_path'].encode())[:12]
  home=first_use[i][1]
  part_by_reference[ident]=part_by_chapter[home]
  stage_by_label[ident]=stage_by_chapter[home]
 assets=read(R/'Tools/Production/merged-print-asset-manifest.json')['assets']+read(R/'QA/print-reference-dedup/supplemental-mermaid-print-assets.json')['assets']
 mermaid={j:a['pdf'] for a in assets for j in a.get('job_ids',[]) if 'mermaid' in a.get('kinds',[])}
 # Source-aware print overrides are keyed by the exact Mermaid block hash. Only
 # audited variants with readable labels and verified source semantics are used.
 # The canonical source SVGs and source Markdown remain untouched.
 override_path=O/'Assets'/'Diagrams'/'Print-v1'/'overrides.json'
 diagram_overrides={}
 if override_path.is_file():
  override_data=read(override_path)
  if override_data.get('schema') not in ('allmight-print-diagram-overrides-v1','allmight-print-diagram-overrides-v2-panels'):
   raise RuntimeError('Unexpected print diagram override schema: '+str(override_path))
  for entry in override_data.get('overrides',[]):
   panels=entry.get('panels')
   if not entry.get('meets_8pt') and not panels:
    continue
   if panels:
    if (not entry.get('source_semantics_verified') or not entry.get('source_graph_sha256')
        or not entry.get('page_proof_sha256') or not panels):
     raise RuntimeError('Unverified panel diagram override: '+str(entry.get('source_asset_id')))
    for number,panel in enumerate(panels,1):
     pdf=Path(panel.get('print_pdf',''))
     svg=Path(panel.get('print_svg',''))
     if (panel.get('panel')!=number or not pdf.is_file() or sha(pdf.read_bytes())!=panel.get('pdf_sha256')
         or not svg.is_file() or sha(svg.read_bytes())!=panel.get('svg_sha256')):
      raise RuntimeError('Invalid or stale panel asset in override: '+str(entry.get('source_asset_id')))
     if float(panel.get('projected_minimum_label_pt',0))<8:
      raise RuntimeError('Below-threshold panel diagram override: '+str(entry.get('source_asset_id')))
   elif not entry.get('source_semantics_verified') or float(entry.get('projected_minimum_label_pt',0))<8:
    raise RuntimeError('Unverified or below-threshold diagram override: '+str(entry.get('source_asset_id')))
   code_hash=entry.get('source_code_sha256')
   canonical=Path(entry.get('canonical_svg',''))
   print_pdf=Path(entry.get('print_pdf',''))
   if not code_hash or not canonical.is_file() or canonical.stem!=entry.get('source_asset_id'):
    raise RuntimeError('Invalid canonical diagram identity in override: '+str(entry.get('source_asset_id')))
   if not panels and not print_pdf.is_file():
    raise RuntimeError('Missing selected print diagram PDF: '+str(print_pdf))
   if code_hash in diagram_overrides:
    raise RuntimeError('Duplicate source code hash in print diagram overrides: '+code_hash)
   diagram_overrides[code_hash]=entry
 bypath={str(Path(a['source_svg']).resolve()):a['pdf'] for a in assets}
 byhash={a.get('source_svg_sha256'):a['pdf'] for a in assets}
 index=b.all_markdown_index(); routes={}; headmaps={}; docs={}; unresolved=[]; globalheads=collections.defaultdict(list); attachments=collections.defaultdict(dict); exceptions=[]
 stage_reflows=collections.defaultdict(list)
 proposal_files=(('stage6-reflow-proposals.json','stage6-gpu-source-reflow-proposals-v1'),
                 ('stage5-7-reflow-proposals.json','stage5-7-source-reflow-proposals-v1'))
 for proposal_name,proposal_schema in proposal_files:
  proposal_data=read(O/'QA'/proposal_name)
  if proposal_data.get('schema')!=proposal_schema:
   raise RuntimeError('Unexpected source reflow proposal schema: '+proposal_name)
  for proposal in proposal_data.get('cases',[]):
   proposal_source=Path(proposal['source_path']).resolve()
   if sha(proposal_source.read_bytes())!=proposal['source_sha256']:
    raise RuntimeError('Source reflow source hash mismatch: '+str(proposal_source))
   source_text=proposal_source.read_text()
   if normalize_layout_formula(proposal['before']) not in normalize_layout_formula(source_text):
    raise RuntimeError('Source reflow source fragment mismatch: '+str(proposal_source))
   # The transient-state box is already covered by its earlier exact SHA guard.
   # Keep one conversion record for this same formula instead of applying it twice.
   if (proposal['source_id']=='06-engine-ii-ch038' and
       proposal['before'].startswith(r'\boxed{\text{Any transient value capable of changing future evolution')):
    continue
   proposal['_proposal_file']=proposal_name
   stage_reflows[str(proposal_source)].append(proposal)
 records=collections.defaultdict(list); navigation_remaps=[]
 for rec in read(R/'link-targets.json')['records']: records[(rec.get('source'),rec.get('target'))].append(rec)
 for cid,ch in chapters_by_id.items(): routes[str(Path(ch['source']).resolve())]=(stage_by_chapter[cid],cid)
 chapter_sources={str(Path(ch['source']).resolve()) for vol in m['volumes'] for ch in vol['chapters']}
 reference_routes={}
 for i,a in enumerate(plan['assignments']):
  home=first_use[i][1]; stage_id=stage_by_chapter[home]; ident='appendix-'+sha(a['source_path'].encode())[:12]
  source=str((V/a['source_path']).resolve()); routes[source]=(stage_id,ident); reference_routes[source]={'stage_id':stage_id,'id':ident,'first_use_chapter':home,'direct_first_use_chapter':direct_first_use_chapter.get(i),'first_use_reason':first_use_reason[i],'assignment':a}
 display_names={s['id']:s['title'] for s in stages}
 locators={}
 for cid,ch in chapters_by_id.items():
  locators[cid]='Ch. '+str(chapter_order[cid])
 for source,ref in reference_routes.items(): locators[ref['id']]='Reference notes: '+ref['assignment']['title']
 if len(plan['assignments'])!=439 or len(routes)!=568: raise RuntimeError('Unexpected source count')
 O.mkdir(parents=True,exist_ok=True); (O/'Tools').mkdir(exist_ok=True)
 # Parse once and establish stable, source-qualified targets before resolving links.
 for vol in m['volumes']:
  inputs=[(c['source'],c['normalized_reader_ast'],c['source_sha256']) for c in vol['chapters']]
  inputs += [(str(V/a['source_path']),None,a['source_sha256']) for a in plan['assignments'] if a['assigned_volume']==vol['id']]
  for source,astpath,expected in inputs:
   src=Path(source).resolve(); raw=src.read_bytes()
   if sha(raw)!=expected: raise RuntimeError('source hash mismatch: '+str(src))
   doc=read(astpath) if astpath else json.loads(pandoc(raw,'markdown+wikilinks_title_after_pipe+tex_math_single_backslash+tex_math_double_backslash','json'))
   chapter_id=source_to_chapter.get(str(src))
   if chapter_id:
    navigation_remaps.append(remap_generated_navigation(doc,src,chapter_id,stage_by_chapter[chapter_id],chapter_navigation,chapters_by_id,V,expected))
   ident=routes[str(src)][1]; mapping={}
   for node in walk(doc.get('blocks',[])):
    if node.get('t')=='Header':
     old=node['c'][1][0]; text=b.textin(node['c'][2]); new=ident+'-'+(old or sha(text.encode())[:12]); node['c'][1][0]=new
     mapping.setdefault(b.nheading(old),new); mapping.setdefault(b.nheading(text),new)
   if str(src) in chapter_sources:
    chapter_started=False; nested_started=False
    for node in doc.get('blocks',[]):
     if node.get('t')!='Header': continue
     c=node['c']
     if c[0]==1 and not chapter_started:
      chapter_started=True; ins=c[2]
      if len(ins)>=4 and ins[0].get('t')=='Str' and ins[0]['c'].isdigit() and ins[1].get('t')=='Space' and ins[2].get('t')=='Str' and ins[2]['c'] in ('—','–','-') and ins[3].get('t')=='Space':
       exceptions.append({'source':str(src),'kind':'edition-heading-number-removed','original':ins[0]['c']}); c[2]=ins[4:]
      continue
     if c[0]==1: nested_started=True
     if nested_started:
      before=c[0]; c[0]=min(6,before+1)
      exceptions.append({'source':str(src),'kind':'chapter-heading-demoted','original_level':before,'rendered_level':c[0],'target':c[1][0]})
   headmaps[str(src)]=mapping; docs[str(src)]=doc
   for key,label in mapping.items():
    if key: globalheads[key].append((routes[str(src)][0],label,str(src)))
 alllabels={label:(volume,source) for key,entries in globalheads.items() for volume,label,source in entries}
 # Deduplicate source attachments by exact content SHA and assign each body to
 # the first logical chapter that cites any of its path aliases.
 attachment_candidates=collections.defaultdict(list)
 attachment_occurrences=[]
 for source,doc in docs.items():
  owner=source_to_chapter.get(source)
  if owner is None and source in reference_routes: owner=reference_routes[source]['first_use_chapter']
  if owner is None: continue
  owner_stage=stage_by_chapter[owner]
  owner_rank=(stage_rank[owner_stage],chapter_order[owner])
  for node in walk(doc.get('blocks',[])):
   if node.get('t')!='Link': continue
   target=node['c'][2][0]; parsed=urllib.parse.urlsplit(target)
   if (parsed.scheme and parsed.scheme!='file') or target.startswith('//'): continue
   rawpath=urllib.parse.unquote(parsed.path)
   if '/.codex/' in rawpath: continue
   resolved=Path(source) if not rawpath else b.resolve(target,Path(source),index)
   if rawpath and Path(rawpath).is_absolute() and Path(rawpath).is_file(): resolved=Path(rawpath)
   if not resolved and rawpath:
    candidate=(V/(rawpath if rawpath.endswith('.md') else rawpath+'.md')).resolve()
    if candidate.is_file(): resolved=candidate
   if not resolved or not resolved.is_file() or resolved.suffix.lower() in ('.md','.png','.jpg','.jpeg','.svg','.pdf'): continue
   rawasset=resolved.read_bytes()
   try: rawasset.decode('utf-8'); assert b'\0' not in rawasset
   except (UnicodeError,AssertionError): continue
   digest=sha(rawasset); aid='source-file-'+digest[:24]
   attachment_candidates[aid].append({'source':source,'target':target,'path':str(resolved.resolve()),
                                      'owner':owner,'stage_id':owner_stage,'part_id':part_by_chapter[owner],
                                      'rank':owner_rank,'sha256':digest})
 attachment_inventory={}
 for aid,uses in attachment_candidates.items():
  ordered=sorted(uses,key=lambda u:(u['rank'],u['source'],u['target'],u['path']))
  first=ordered[0]; aliases=sorted({u['path'] for u in uses})
  attachment_inventory[aid]={'id':aid,'source':first['path'],'source_sha256':first['sha256'],
    'source_paths':aliases,'source_names':sorted({Path(x).name for x in aliases}),
    'first_use_chapter':first['owner'],'stage_id':first['stage_id'],'part_id':first['part_id'],
    'link_uses':sorted([{'source':u['source'],'target':u['target'],'citing_chapter':u['owner'],
                         'citing_part_id':u['part_id']} for u in uses],key=lambda u:(u['source'],u['target']))}
 stage=Path(tempfile.mkdtemp(prefix='.generation-',dir=O))
 (stage/'Assets/Print').mkdir(parents=True); (stage/'Assets/Source').mkdir(parents=True)
 def bundle(path,category):
  path=Path(path); data=path.read_bytes(); name=sha(data)+path.suffix.lower(); output=stage/'Assets'/category/name
  if not output.exists(): output.write_bytes(data)
  return '../current/Assets/'+category+'/'+name
 rows=[]; missing=[]; warnings=[]; image_caption_normalizations=[]; links=collections.Counter(); total=collections.Counter()
 for logical_stage in stages:
  vid=logical_stage['id']
  planned_parts=[p for s in part_data['stages'] if s['id']==vid for p in s['parts']] if part_data else [{'id':vid,'index':1,'title':logical_stage['title'],'chapter_ids':logical_stage['chapters']}]
  for physical in planned_parts:
   partid=physical['id']; dest=stage/partid; dest.mkdir(exist_ok=True); (dest/'content').mkdir(exist_ok=True); (dest/'AST').mkdir(exist_ok=True)
   inputs=[('chapter',chapters_by_id[cid]['source'],chapters_by_id[cid]['normalized_reader_ast'],chapters_by_id[cid]['source_sha256'],cid,chapters_by_id[cid]['title']) for cid in physical['chapter_ids']]
   assignments=[(i,a) for i,a in enumerate(plan['assignments']) if part_by_reference['appendix-'+sha(a['source_path'].encode())[:12]]==partid]
   inputs += [('reference',str(V/a['source_path']),None,a['source_sha256'],'appendix-'+sha(a['source_path'].encode())[:12],a['title']) for i,a in assignments]
   for kind,src,astpath,expected,ident,title in inputs:
    src=Path(src); raw=src.read_bytes()
    if sha(raw)!=expected: raise RuntimeError('source hash mismatch: '+str(src))
    doc=docs[str(src.resolve())]
    restore_blank_padded_display_math(doc,src,exceptions)
    doc,html_report=b.presentation_html_clone(doc,str(src))
    counts=collections.Counter(n.get('t') for n in walk(doc) if 't' in n); total.update(counts)
    current_part=partid
    current_owner=ident if kind=='chapter' else reference_routes[str(src.resolve())]['first_use_chapter']
    codehash=[sha(n['c'][1].encode()) for n in walk(doc) if n.get('t')=='CodeBlock' and 'mermaid' not in n['c'][0][1]]
    diagrams=[]; caption_changes=[]
    def change(n):
     t=n.get('t'); c=n.get('c')
     if t=='RawBlock' and c[0]=='tex' and c[1].strip().startswith(r'\begin{aligned}') and c[1].strip().endswith(r'\end{aligned}'):
      exceptions.append({'source':str(src),'kind':'raw-aligned-display-wrapped','original':c[1],'rendered':r'\['+c[1]+r'\]'})
      return {'t':'RawBlock','c':['tex',r'\['+c[1]+r'\]']}
     if t in ('Para','Plain') and isinstance(c,list):
      output=[]; i=0
      while i<len(c):
       if c[i].get('t')=='Str' and c[i].get('c')==r'\(':
        end=next((j for j in range(i+1,len(c)) if c[j].get('t')=='Str' and c[j].get('c')==r'\)'),None)
        if end is not None:
         formula=b.textin(c[i+1:end]).strip()
         exceptions.append({'source':str(src),'kind':'escaped-math-delimiters-restored','original':r'\('+formula+r'\)','rendered':formula})
         output.append({'t':'Math','c':[{'t':'InlineMath'},formula]}); i=end+1; continue
       if c[i].get('t')=='Str' and c[i].get('c')=='$':
        end=next((j for j in range(i+1,len(c)) if c[j].get('t')=='Str' and c[j].get('c')=='$'),None)
        if end is not None and any(x.get('t')=='RawInline' and x.get('c',[None])[0]=='tex' for x in c[i+1:end]):
         formula=b.textin(c[i+1:end]).strip()
         exceptions.append({'source':str(src),'kind':'spaced-dollar-math-restored','original':'$ '+formula+' $','rendered':formula})
         output.append({'t':'Math','c':[{'t':'InlineMath'},formula]}); i=end+1; continue
       output.append(c[i]); i+=1
      c=output; n['c']=c
     if t=='Para' and c and c[0].get('t')=='Str' and c[0].get('c')=='$' and c[-1].get('t')=='Str' and c[-1].get('c')=='$' and any(x.get('t')=='RawInline' and x.get('c',[None])[0]=='tex' for x in c):
      formula=b.textin(c)[1:-1].strip()
      if r'\mathrm{charged}' in formula and r'\mathrm{limit}' in formula:
       exceptions.append({'source':str(src),'kind':'spaced-dollar-math-restored','original':b.textin(c),'rendered':formula})
       return {'t':'Para','c':[{'t':'Math','c':[{'t':'DisplayMath'},formula]}]}
     if (t=='Header' and
         src.as_posix().endswith('/Chapter Edition/03 - Game Engine/32 - Terrain materials and image composition.md') and
         expected=='f6d9af03096d302ef3c40e99f0056d7c5bfd90a9865b3af131592c9f3bd09fd8' and
         b.textin(c[2])=='Use framebuffer attachments for editor interaction,#### Selection and picking, and#### Camera and coordinate spaces`.'):
      original_title=b.textin(c[2])
      repaired_title='Use framebuffer attachments for editor interaction, Selection and picking, and Camera and coordinate spaces.'
      words=repaired_title.split(' '); c[2]=[]
      for word_index,word in enumerate(words):
       if word_index: c[2].append({'t':'Space'})
       c[2].append({'t':'Str','c':word})
      exceptions.append({'source':str(src),'source_sha256':expected,
       'kind':'malformed-inline-markdown-heading-normalized','original':original_title,
       'rendered':repaired_title,
       'reason':'The exact source-hash-bound heading had unmatched backticks and literal heading markers; export removes those markup artifacts while preserving every title word and its order.'})
     if (t=='Table' and
         src.as_posix().endswith('/Chapter Edition/03 - Game Engine/01 - A game promise becomes bytes and running state.md') and
         expected=='fc461d5123a78ed27badcc6591db160ebd5e74b490260822566b5cfb40782392' and
         set_binary_format_table_widths(n)):
      exceptions.append({'source':str(src),'source_sha256':expected,
       'kind':'binary32-binary64-reference-table-wrapped','headers':['Property','binary32','binary64'],
       'column_width_fractions':[0.42,0.27,0.31],
       'reason':'The exact authored three-column table is assigned fixed paragraph widths so long property labels wrap and both format columns remain inside the page text block; cell text and order are unchanged.'})
     if t=='Math':
      before=c[1]
      compact=' '.join(c[1].split())
      process_chain=(r'\text{scene} \rightarrow \text{resource upload} \rightarrow \text{command recording} '
        r'\rightarrow \text{submission} \rightarrow \text{vertex stage} \rightarrow \text{clipping} '
        r'\rightarrow \text{rasterization} \rightarrow \text{fragment stage} \rightarrow \text{depth/stencil} '
        r'\rightarrow \text{attachment} \rightarrow \text{presentation}.')
      representation_chain=(r'\text{dense explicit field} \leftrightarrow \text{factorized explicit field} '
        r'\leftrightarrow \text{hybrid encoded field} \leftrightarrow \text{implicit neural field} '
        r'\leftrightarrow \text{explicit learned primitives},')
      verbal_normalization=(r'\boxed{ 4N_LN_V = '
        r'\text{geometry/Jacobian normalization required to express microscopic mirror area as a macroscopic BRDF}. }')
      process_source=src.as_posix().endswith('/Chapter Edition/02 - Graphics Vision and XR/14 - Transparency, Blending and Visibility.md')
      representation_source=src.as_posix().endswith('/Chapter Edition/02 - Graphics Vision and XR/35 - A Complete Renderer as an Executable Forward Model.md')
      verbal_source=src.as_posix().endswith('/Chapter Edition/02 - Graphics Vision and XR/05 - Reflectance, Materials and Light Transport.md')
      visibility_hypothesis=(r'H: \text{GPU-driven culling overtakes CPU submission only after a particular object-count/visibility threshold.}')
      visibility_hypothesis_source=src.as_posix().endswith('/Chapter Edition/02 - Graphics Vision and XR/14 - Transparency, Blending and Visibility.md')
      one_material_boxed=r'\boxed{ \text{one material may contain multiple physically meaningful reflection lobes with different roughness distributions}. }'
      random_variables_boxed=r'\boxed{ \text{a carefully constructed sequence of random variables whose expectation equals the rendering equation.} }'
      roulette_boxed=r'\boxed{ \text{roulette probability should estimate future contribution, not blindly react to representation artifacts}. }'
      transient_state_boxed=r'\boxed{ \text{Any transient value capable of changing future evolution belongs to deterministic state at the instant it is captured.} }'
      source_layout_hashes={
       'asset_pipeline':'f4e53547fd7da05ff8e36bbb838e960ba6ba2b74811711fb40691b58aa645917',
       'async_resources':'b4a7e05400e17da004e4df9a1ef76af3866ae0dce5c176b9a223e08a3229c7c6',
       'continuous_models':'0c1ad9e2f3166502def17f2afefbb56e035ababfeef53eb08adadc9cc644550e',
       'reliable_optimization':'d105fc7597a5a6cfa965a35be8459c06830c45dcce20bce1ef635ab223e86db5',
       'monte_carlo':'16b91688be2184495d0c09583c0d30461de59caa5bd5caeca145aa9357c56123',
       'rasterization':'aa0745a555084978be24bc51d412535f40d764ad1c6b87cc8c42904383633ea3',
       'stencil':'203b5cebe45551ce74a4a56a2d6a1a4eb67b0acb2962b0a4950c21400f553893'}
      layout_reflow_sources={
       'reflectance':('/Chapter Edition/02 - Graphics Vision and XR/05 - Reflectance, Materials and Light Transport.md',
        '1412240b54c98a60468567b575d9ae2778545aa1f958acaa211e44ed7f91afc0'),
       'mis':('/Chapter Edition/02 - Graphics Vision and XR/09 - Point Lights, MIS and Environment Lighting.md',
        '9094d7057c0c986ddeaab13c7e1615807ef0707c092edd7f2c3d7452873a8ee2'),
       'roulette':('/Chapter Edition/02 - Graphics Vision and XR/10 - A Production Path Tracer Media, Randomness and Validation.md',
        '741cd67cd6109ef16b7b54414b86e328410593f361923c24aa51d555befd6e01'),
       'transient_state':('/Chapter Edition/03 - Game Engine/38 - Integrator choices and energy drift.md',
        'ce2291ff7681c465d79de4458376a8eafb55c5dcca655bda71c042ea69544ed6')}
      if process_source and compact==process_chain:
       c[1]=(r'\begin{aligned}'
        r'\text{scene} &\rightarrow \text{resource upload} \rightarrow \text{command recording} \\'
        r'\text{command recording} &\rightarrow \text{submission} \rightarrow \text{vertex stage} \\'
        r'\text{vertex stage} &\rightarrow \text{clipping} \rightarrow \text{rasterization} \\'
        r'\text{rasterization} &\rightarrow \text{fragment stage} \rightarrow \text{depth/stencil} \\'
        r'\text{depth/stencil} &\rightarrow \text{attachment} \rightarrow \text{presentation}.'
        r'\end{aligned}')
       exceptions.append({'source':str(src),'source_sha256':expected,'kind':'display-process-chain-reflowed',
        'original':before,'rendered':c[1],'reason':'The exact ordered stage chain is reflowed into five short aligned rows with repeated boundary labels; every stage and directed edge remains in order.'})
       before=c[1]
      elif representation_source and compact==representation_chain:
       c[1]=(r'\begin{aligned}'
        r'\text{dense explicit field} &\leftrightarrow \text{factorized explicit field} \leftrightarrow \text{hybrid encoded field} \\'
        r'\text{hybrid encoded field} &\leftrightarrow \text{implicit neural field} \leftrightarrow \text{explicit learned primitives},'
        r'\end{aligned}')
       exceptions.append({'source':str(src),'source_sha256':expected,'kind':'representation-family-chain-reflowed',
        'original':before,'rendered':c[1],'reason':'The exact ordered representation family is reflowed into two short aligned rows with the boundary label repeated; every bidirectional edge remains in order.'})
       before=c[1]
      elif verbal_source and compact==verbal_normalization:
       c[1]=(r'\boxed{4N_LN_V=\begin{gathered}'
        r'\text{geometry/Jacobian normalization required to express}\\'
        r'\text{microscopic mirror area as a macroscopic BRDF}.'
        r'\end{gathered}}')
       exceptions.append({'source':str(src),'source_sha256':expected,'kind':'boxed-normalization-description-reflowed',
        'original':before,'rendered':c[1],'reason':'Only the exact boxed verbal description is wrapped at a word boundary; all formula symbols and English words remain in order.'})
       before=c[1]
      elif (visibility_hypothesis_source and
            expected=='9330ee45adf5ab1a95866e9e7b9a6c584dd542a77cd443ffe504dd6a9ef43c96' and
            compact==visibility_hypothesis):
       c[1]=(r'\begin{aligned}'
        r'H: &\text{GPU-driven culling overtakes CPU submission only after a particular}\\'
        r'&\text{object-count/visibility threshold.}'
        r'\end{aligned}')
       exceptions.append({'source':str(src),'source_sha256':expected,'kind':'gpu-culling-hypothesis-reflowed',
        'original':before,'rendered':c[1],
        'reason':'The exact hypothesis is reflowed at a word boundary into two aligned display rows; the H: prefix and every word remain in their original order.'})
       before=c[1]
      elif (src.as_posix().endswith(layout_reflow_sources['reflectance'][0]) and
            expected==layout_reflow_sources['reflectance'][1] and compact==one_material_boxed):
       c[1]=(r'\boxed{\begin{gathered}'
        r'\text{one material may contain multiple}\\'
        r'\text{physically meaningful reflection lobes}\\'
        r'\text{with different roughness distributions}.'
        r'\end{gathered}}')
       exceptions.append({'source':str(src),'source_sha256':expected,'kind':'boxed-material-lobes-reflowed',
        'original':before,'rendered':c[1],
        'reason':'The exact boxed sentence is split at word boundaries into three short lines; every word and its final period remain in order.'})
       before=c[1]
      elif (src.as_posix().endswith(layout_reflow_sources['mis'][0]) and
            expected==layout_reflow_sources['mis'][1] and compact==random_variables_boxed):
       c[1]=(r'\boxed{\begin{gathered}'
        r'\text{a carefully constructed sequence of random variables}\\'
        r'\text{whose expectation equals the rendering equation.}'
        r'\end{gathered}}')
       exceptions.append({'source':str(src),'source_sha256':expected,'kind':'boxed-rendering-equation-reflowed',
        'original':before,'rendered':c[1],
        'reason':'The exact boxed sentence is split at a word boundary into two short lines; every word and punctuation mark remain in order.'})
       before=c[1]
      elif (src.as_posix().endswith(layout_reflow_sources['roulette'][0]) and
            expected==layout_reflow_sources['roulette'][1] and compact==roulette_boxed):
       c[1]=(r'\boxed{\begin{gathered}'
        r'\text{roulette probability should estimate future contribution,}\\'
        r'\text{not blindly react to representation artifacts.}'
        r'\end{gathered}}')
       exceptions.append({'source':str(src),'source_sha256':expected,'kind':'boxed-roulette-principle-reflowed',
        'original':before,'rendered':c[1],
        'reason':'The exact boxed sentence is split at a word boundary into two short lines; every word and punctuation mark remain in order.'})
       before=c[1]
      elif (src.as_posix().endswith(layout_reflow_sources['transient_state'][0]) and
            expected==layout_reflow_sources['transient_state'][1] and compact==transient_state_boxed):
       c[1]=(r'\boxed{\begin{gathered}'
        r'\text{Any transient value capable of changing future evolution}\\'
        r'\text{belongs to deterministic state at the instant it is}\\'
        r'\text{captured.}'
        r'\end{gathered}}')
       exceptions.append({'source':str(src),'source_sha256':expected,'kind':'boxed-transient-state-principle-reflowed',
        'original':before,'rendered':c[1],
        'reason':'The exact boxed sentence is split at word boundaries into three short lines; every word and punctuation mark remain in order.'})
       before=c[1]
      elif (src.as_posix().endswith('/Chapter Edition/01 - Computing Foundations/05 - Generic Asset Pipelines without Hidden Contracts.md') and
            expected==source_layout_hashes['asset_pipeline'] and
            compact==r'\text{language semantics} \rightarrow \text{small example} \rightarrow \text{engine ownership/lifetime consequence} \rightarrow \text{measured production behavior}'):
       c[1]=(r'\begin{aligned}'
        r'\text{language semantics} &\rightarrow \text{small example}\\'
        r'\text{small example} &\rightarrow \text{engine ownership/lifetime consequence}\\'
        r'\text{engine ownership/lifetime consequence} &\rightarrow \text{measured production behavior}'
        r'\end{aligned}')
       exceptions.append({'source':str(src),'source_sha256':expected,'kind':'asset-pipeline-evidence-chain-reflowed','original':before,'rendered':c[1],
        'reason':'The exact four-node ordered chain is reflowed into short aligned rows with repeated boundary labels.'})
       before=c[1]
      elif (src.as_posix().endswith('/Chapter Edition/01 - Computing Foundations/05 - Generic Asset Pipelines without Hidden Contracts.md') and
            expected==source_layout_hashes['asset_pipeline'] and
            compact==r'\boxed{ \text{make ownership explicit} + \text{make access patterns measurable} + \text{make async lifetime nested} + \text{make performance claims reproducible} }'):
       c[1]=(r'\boxed{\begin{gathered}'
        r'\text{make ownership explicit}\\'
        r'+\;\text{make access patterns measurable}\\'
        r'+\;\text{make async lifetime nested}\\'
        r'+\;\text{make performance claims reproducible}'
        r'\end{gathered}}')
       exceptions.append({'source':str(src),'source_sha256':expected,'kind':'cpp-contract-summary-reflowed','original':before,'rendered':c[1],
        'reason':'The exact boxed four-part summary is wrapped at its plus operators; every phrase and operator remains in order.'})
       before=c[1]
      elif (src.as_posix().endswith('/Chapter Edition/01 - Computing Foundations/12 - Commands, Async Work, and Resource Lifetimes.md') and
            expected==source_layout_hashes['async_resources'] and compact==r'\text{projection} \rightarrow \text{tile counts} \rightarrow \text{scan} \rightarrow \text{key generation} \rightarrow \text{radix histogram} \rightarrow \text{prefix} \rightarrow \text{scatter} \rightarrow \text{tile ranges} \rightarrow \text{composite}.'):
       c[1]=(r'\begin{aligned}'
        r'\text{projection} &\rightarrow \text{tile counts} \rightarrow \text{scan}\\'
        r'\text{scan} &\rightarrow \text{key generation} \rightarrow \text{radix histogram}\\'
        r'\text{radix histogram} &\rightarrow \text{prefix} \rightarrow \text{scatter}\\'
        r'\text{scatter} &\rightarrow \text{tile ranges} \rightarrow \text{composite}.'
        r'\end{aligned}')
       exceptions.append({'source':str(src),'source_sha256':expected,'kind':'gaussian-pipeline-chain-reflowed','original':before,'rendered':c[1],
        'reason':'The exact ordered pipeline is split into four aligned rows with repeated boundary stages.'})
       before=c[1]
      elif (src.as_posix().endswith('/Chapter Edition/01 - Computing Foundations/12 - Commands, Async Work, and Resource Lifetimes.md') and
            expected==source_layout_hashes['async_resources'] and compact==r'\text{project} \rightarrow \text{count tile references} \rightarrow \text{parallel scan} \rightarrow \text{GPU-generated indirect count} \rightarrow \text{generate key/value records} \rightarrow \text{subgroup histogram} \rightarrow \text{prefix} \rightarrow \text{stable radix scatter} \rightarrow \text{tile ranges} \rightarrow \text{composite}.'):
       c[1]=(r'\begin{aligned}'
        r'\text{project} &\rightarrow \text{count tile references}\\'
        r'\text{count tile references} &\rightarrow \text{parallel scan} \rightarrow \text{GPU-generated indirect count}\\'
        r'\text{GPU-generated indirect count} &\rightarrow \text{generate key/value records}\\'
        r'\text{generate key/value records} &\rightarrow \text{subgroup histogram} \rightarrow \text{prefix}\\'
        r'\text{prefix} &\rightarrow \text{stable radix scatter} \rightarrow \text{tile ranges} \rightarrow \text{composite}.'
        r'\end{aligned}')
       exceptions.append({'source':str(src),'source_sha256':expected,'kind':'gaussian-hardware-chain-reflowed','original':before,'rendered':c[1],
        'reason':'The exact ordered hardware chain is split into short aligned rows with repeated boundary stages.'})
       before=c[1]
      elif (src.as_posix().endswith('/Chapter Edition/01 - Computing Foundations/25 - Continuous Models and Numerical Error.md') and
            expected==source_layout_hashes['continuous_models'] and compact==r'\boxed{ \text{action} \rightarrow \text{Euler--Lagrange equations} \rightarrow \text{conjugate momentum} \rightarrow \text{Hamiltonian phase-space flow} \rightarrow \text{symplectic discrete update} }'):
       c[1]=(r'\boxed{\begin{aligned}'
        r'\text{action} &\rightarrow \text{Euler--Lagrange equations}\\'
        r'\text{Euler--Lagrange equations} &\rightarrow \text{conjugate momentum}\\'
        r'\text{conjugate momentum} &\rightarrow \text{Hamiltonian phase-space flow}\\'
        r'\text{Hamiltonian phase-space flow} &\rightarrow \text{symplectic discrete update}'
        r'\end{aligned}}')
       exceptions.append({'source':str(src),'source_sha256':expected,'kind':'dynamics-hierarchy-chain-reflowed','original':before,'rendered':c[1],
        'reason':'The exact ordered hierarchy is reflowed into four aligned rows with repeated boundary labels.'})
       before=c[1]
      elif (src.as_posix().endswith('/Chapter Edition/01 - Computing Foundations/28 - Reliable Optimization and Root Finding.md') and
            expected==source_layout_hashes['reliable_optimization'] and compact==r'\boxed{ \hat Q-Q = \text{modeling contribution} + \text{discretization contribution} + \text{solver contribution} + \text{arithmetic contribution} }'):
       c[1]=(r'\boxed{\begin{aligned}'
        r'\hat Q-Q ={}&\text{modeling contribution} + \text{discretization contribution}\\'
        r'&+\text{solver contribution} + \text{arithmetic contribution}'
        r'\end{aligned}}')
       exceptions.append({'source':str(src),'source_sha256':expected,'kind':'error-contribution-equation-reflowed','original':before,'rendered':c[1],
        'reason':'The exact boxed four-term additive equation is wrapped across two aligned rows; variables, terms, and operators are unchanged.'})
       before=c[1]
      elif (src.as_posix().endswith('/Chapter Edition/01 - Computing Foundations/30 - Monte Carlo Estimates and Uncertainty Metrics.md') and
            expected==source_layout_hashes['monte_carlo'] and
            compact==r'\text{estimate distribution or parameter} \rightarrow \text{plug in unknown quantities} \rightarrow \text{linearize the requested transformation} \rightarrow \text{propagate uncertainty}.'):
       c[1]=(r'\begin{aligned}'
        r'\text{estimate distribution or parameter} &\rightarrow \text{plug in unknown quantities}\\'
        r'\text{plug in unknown quantities} &\rightarrow \text{linearize the requested transformation}\\'
        r'\text{linearize the requested transformation} &\rightarrow \text{propagate uncertainty}.'
        r'\end{aligned}')
       exceptions.append({'source':str(src),'source_sha256':expected,'kind':'delta-method-process-chain-reflowed','original':before,'rendered':c[1],
        'reason':'The exact four-step chain is reflowed into three aligned rows with repeated boundary labels; every phrase and arrow remains.'})
       before=c[1]
      elif (src.as_posix().endswith('/Chapter Edition/02 - Graphics Vision and XR/13 - From View Space to Clip Space.md') and
            expected==source_layout_hashes['stencil'] and compact==r'\text{geometry generates samples} \rightarrow \text{stencil classifies existing sample state} \rightarrow \text{pipeline conditionally permits/modifies fragments}.'):
       c[1]=(r'\begin{aligned}'
        r'\text{geometry generates samples} &\rightarrow \text{stencil classifies existing sample state}\\'
        r'\text{stencil classifies existing sample state} &\rightarrow \text{pipeline conditionally permits/modifies fragments}.'
        r'\end{aligned}')
       exceptions.append({'source':str(src),'source_sha256':expected,'kind':'stencil-process-chain-reflowed','original':before,'rendered':c[1],
        'reason':'The exact three-stage stencil sequence is reflowed into two aligned rows with its intermediate state repeated.'})
       before=c[1]
      elif (src.as_posix().endswith('/Chapter Edition/02 - Graphics Vision and XR/12 - Probability, Statistics and Uncertainty.md') and
            expected==source_layout_hashes['rasterization'] and
            compact==r'\text{vertex/index memory} \rightarrow \text{primitive assembly} \rightarrow \text{vertex processing} \rightarrow \text{clip coordinates} \rightarrow \text{clipping} \rightarrow \text{perspective divide} \rightarrow \text{NDC} \rightarrow \text{viewport} \rightarrow \text{triangle setup} \rightarrow \text{coverage} \rightarrow \text{interpolation} \rightarrow \text{fragment invocation} \rightarrow \text{depth test}.'):
       c[1]=(r'\begin{aligned}'
        r'\text{vertex/index memory} &\rightarrow \text{primitive assembly} \rightarrow \text{vertex processing}\\'
        r'\text{vertex processing} &\rightarrow \text{clip coordinates} \rightarrow \text{clipping}\\'
        r'\text{clipping} &\rightarrow \text{perspective divide} \rightarrow \text{NDC} \rightarrow \text{viewport}\\'
        r'\text{viewport} &\rightarrow \text{triangle setup} \rightarrow \text{coverage} \rightarrow \text{interpolation}\\'
        r'\text{interpolation} &\rightarrow \text{fragment invocation} \rightarrow \text{depth test}.'
        r'\end{aligned}')
       exceptions.append({'source':str(src),'source_sha256':expected,'kind':'rasterization-pipeline-chain-reflowed','original':before,'rendered':c[1],
        'reason':'The exact ordered pipeline is split into five aligned rows with repeated boundary stages; stage names and arrows remain in order.'})
       before=c[1]
      for proposal in stage_reflows.get(str(src.resolve()),[]):
       current=normalize_layout_formula(c[1]); before_guard=normalize_layout_formula(proposal['before'])
       after=proposal['after']
       # A verified long EOS question is an exact interior of a boxed source
       # expression; retain the preceding generational-identity stage and box.
       if (proposal['source_id']=='05-engine-i-ch015' and
           proposal['before'].startswith(r'\text{slot reuse}') and
           expected==proposal['source_sha256'] and
           current==normalize_layout_formula(r'\boxed{\text{generational identity}\rightarrow\text{slot reuse}\rightarrow\text{typed pools}\rightarrow\text{sparse indirection}\rightarrow\text{dense storage}\rightarrow\text{constant-time membership}\rightarrow\text{smallest-pool joins}\rightarrow\text{invalidation rules}\rightarrow\text{deferred structural mutation}.}')):
        after=(r'\boxed{\begin{gathered}'
         r'\text{generational identity}\rightarrow\text{slot reuse}\rightarrow\text{typed pools}\\'
         r'\rightarrow\text{sparse indirection}\rightarrow\text{dense storage}\rightarrow\text{constant-time membership}\\'
         r'\rightarrow\text{smallest-pool joins}\rightarrow\text{invalidation rules}\\'
         r'\rightarrow\text{deferred structural mutation}.'
         r'\end{gathered}}')
       elif (proposal['before'].startswith(r'\text{3D Gaussians}') and
             current.startswith(r'\boxed{') and not after.startswith(r'\boxed{')):
        after=r'\boxed{'+after+r'}'
       elif not ('\\text{' in proposal['before']) and not ('\\boxed{' in proposal['before']):
        current=re.sub(r'\s+','',current)
        before_guard=re.sub(r'\s+','',before_guard)
       if (expected==proposal['source_sha256'] and current==before_guard):
        c[1]=after
        exceptions.append({'source':str(src),'source_id':proposal['source_id'],'source_sha256':expected,
         'proposal_file':proposal['_proposal_file'],'kind':'source_verified_trim_reflow','original':proposal['before'],'rendered':after,
         'affected_pdf_pages':proposal.get('affected_pdf_pages',[]),
         'reason':'Applied the exact source-hash-bound display reflow with source words and operators retained.'})
        break
      bad_matrix=r'NX= -\begin{pmatrix} d_1\\\d_2\\\d_3 \end{pmatrix}'
      good_matrix=r'NX= -\begin{pmatrix} d_1\\ d_2\\ d_3 \end{pmatrix}'
      if src.as_posix().endswith('/Chapter Edition/02 - Graphics Vision and XR/21 - Collision Implementations and Repository Comparison.md') and bad_matrix in c[1]:
       c[1]=c[1].replace(bad_matrix,good_matrix)
       exceptions.append({'source':str(src),'source_sha256':expected,'kind':'matrix-row-variable-d-escape-normalized','original':before,'rendered':c[1],'reason':'The source used \\d after matrix row separators, which invokes a combining-dot accent instead of the variable d.'})
       before=c[1]
      c[1]=c[1].replace(r'$p=\alpha a+\beta b+\gamma c$',r'p=\alpha a+\beta b+\gamma c').replace(r'\text{do identical work 5% faster}',r'\text{do identical work 5\% faster}').replace(r'\begin{bmatrix}a\b\c\d\end{bmatrix}',r'\begin{bmatrix}a\\b\\c\\d\end{bmatrix}')
      c[1]=c[1].replace(r'v\\\w\end{pmatrix}',r'v\\w\end{pmatrix}').replace(r'-c\\\f_0\end{pmatrix}',r'-c\\f_0\end{pmatrix}').replace(r's\\\t\end{pmatrix}',r's\\t\end{pmatrix}').replace(r'd_{20}\\\d_{21}\end{pmatrix}',r'd_{20}\\d_{21}\end{pmatrix}')
      if (src.as_posix().endswith('/Chapter Edition/03 - Game Engine/12 - Transactions, recovery, and safe project boundaries.md')
          and c[1].strip()=='é'):
       c[1]=r'\text{é}'
       exceptions.append({'source':str(src),'source_sha256':expected,'kind':'unicode-normalization-example-text-mode','original':'é','rendered':c[1],'reason':'This display-math node is a Unicode character example, not a mathematical variable; text mode preserves the NFC code point without selecting a math font glyph.'})
      c[1]=re.sub(r'\\(texttt|text)\{([^{}]*)\}',lambda m: '\\'+m.group(1)+'{'+re.sub(r'(?<!\\)_',r'\\_',m.group(2))+'}',c[1])
      if before!=c[1]: exceptions.append({'source':str(src),'kind':'math-text-and-matrix-syntax-normalized','original':before,'rendered':c[1]})
     if t=='Math' and ('parallelizing 99% does not make the remaining serial 1% disappear' in c[1] or r'\TargetEntity' in c[1]):
      before=c[1]; c[1]=c[1].replace('parallelizing 99% does not make the remaining serial 1% disappear',r'parallelizing 99\% does not make the remaining serial 1\% disappear').replace(r'\TargetEntity',r'\text{TargetEntity}'); exceptions.append({'source':str(src),'kind':'math-source-syntax-normalized','original':before,'rendered':c[1]})
     if t=='Math' and '\\<' in c[1]:
      before=c[1]; c[1]=c[1].replace('\\<','<'); exceptions.append({'source':str(src),'kind':'math-comparison-escape-normalized','original':before,'rendered':c[1]})
     if t=='Figure' and isinstance(c,list) and len(c)>=3:
      caption_blocks=c[1][1] if isinstance(c[1],list) and len(c[1])>1 else []
      caption_text=' '.join(b.textin(block.get('c',[])) for block in caption_blocks if isinstance(block,dict))
      image=next((x for x in walk(c[2]) if x.get('t')=='Image'),None)
      target=image.get('c',[None,None,['']])[2][0] if image else ''
      source_alt=b.textin(image.get('c',[None,[],[]])[1]) if image else ''
      source_uri=source_alt if source_alt and source_alt==caption_text else target
      if (caption_text==source_uri and ('/' in source_uri or '\\' in source_uri)
          and not urllib.parse.urlsplit(source_uri).scheme
          and Path(urllib.parse.urlsplit(source_uri).path).suffix.lower() in ('.png','.jpg','.jpeg','.svg')):
       filename=Path(urllib.parse.unquote(urllib.parse.urlsplit(source_uri).path)).name
       stem=re.sub(r'[_-]+',' ',Path(filename).stem)
       human=stem[:1].upper()+stem[1:]
       c[0][1].append('allmight-auto-path-caption')
       c[0][2].append(['data-auto-path-caption-uri',source_uri])
       image['c'][0][1].append('allmight-auto-path-caption')
       image['c'][0][2].append(['data-auto-path-caption-uri',source_uri])
       record={'source':str(src),'source_sha256':expected,'source_uri':source_uri,'original_alt':source_alt,'original_caption':caption_text,'rendered_caption':human}
       caption_changes.append(record); image_caption_normalizations.append(record)
     if t=='CodeBlock' and 'mermaid' in c[0][1]:
      code=c[1]; code_hash=sha(code.encode()); jid=sha(json.dumps({'code':code,'kind':'mermaid','version':1},sort_keys=True,ensure_ascii=False).encode())[:24]
      override=diagram_overrides.get(code_hash)
      panel_paths=[p['print_pdf'] for p in override['panels']] if override and override.get('panels') else None
      path=override['print_pdf'] if override and not panel_paths else (panel_paths[0] if panel_paths else mermaid.get(jid))
      if not path or not Path(path).is_file(): missing.append({'kind':'mermaid','id':jid,'source':str(src)});return n
      diagram_record={'id':jid,'code_sha256':code_hash,'pdf':path}
      if override:
       diagram_record.update({'source_asset_id':override['source_asset_id'],'print_override_variant':override['variant'],
        'projected_minimum_label_pt':override['projected_minimum_label_pt'],'print_profile':'Panel','source_semantics_verified':True,
        'bundled_pdf_sha256':sha(Path(path).read_bytes())})
       if panel_paths:
        panel_assets=[]
        for number,panel in enumerate(override['panels'],1):
         panel_assets.append({'panel':number,'print_pdf':bundle(panel['print_pdf'],'Print'),
          'pdf_sha256':panel['pdf_sha256'],'print_svg':bundle(panel['print_svg'],'Print'),
          'svg_sha256':panel['svg_sha256']})
        diagram_record.update({'panel_assets':panel_assets,'panel_count':len(panel_assets),
          'source_graph_sha256':override['source_graph_sha256']})
      diagrams.append(diagram_record)
      image_classes=['allmight-diagram-panel'] if override else []
      if panel_paths:
       images=[]
       for number,panel_path in enumerate(panel_paths,1):
        attrs=[['data-panel-index',str(number)],['data-panel-count',str(len(panel_paths))]]
        images.append({'t':'Image','c':[['',image_classes,attrs],
          [{'t':'Str','c':'Diagram'}] if number==1 else [],[bundle(panel_path,'Print'),'']]})
       return {'t':'Para','c':images}
      return {'t':'Para','c':[{'t':'Image','c':[['',image_classes,[]],[{'t':'Str','c':'Diagram'}],[bundle(path,'Print'),'']]}]}
     if t=='Link':
      target=c[2][0]; parsed=urllib.parse.urlsplit(target)
      if parsed.scheme and parsed.scheme!='file' or target.startswith('//'): return n
      if target in {'4,1.2],[1.2,1','2,0],[0.6,0.8','1/2,1/2],[1/2,1/2'}:
       exceptions.append({'source':str(src),'target':target,'kind':'restored-matrix-syntax'}); return {'t':'Code','c':[['',[],[]],'[['+target+']]']}
      if target=='i' and not b.textin(c[1]) and src.name=='02 - C++ - Objects, Ownership and Lifetime - Visual Recall.md' and raw.decode().count('operator[](i)')==1:
       exceptions.append({'source':str(src),'target':target,'kind':'restored-operator-syntax'}); return {'t':'Code','c':[['',[],[]],'[](i)']}
      rawpath=urllib.parse.unquote(parsed.path); fragment=urllib.parse.unquote(parsed.fragment)
      if '/.codex/' in rawpath:
       exceptions.append({'source':str(src),'target':target,'kind':'excluded-private-provenance'}); links['excluded-private-provenance']+=1
       return {'t':'Span','c':[['',[],[]],c[1]]}
      resolved=src if not rawpath else b.resolve(target,src,index)
      if rawpath and Path(rawpath).is_absolute() and Path(rawpath).is_file(): resolved=Path(rawpath)
      if not resolved and rawpath:
       candidate=(V/(rawpath if rawpath.endswith('.md') else rawpath+'.md')).resolve()
       if candidate.is_file(): resolved=candidate
      route=routes.get(str(resolved.resolve())) if resolved else None
      direct_heading=None
      matches=records.get((str(src),target),[])
      matches=sorted(matches,key=lambda x: x.get('volume')!=vid)
      for rec in matches:
       label=rec.get('target_id')
       if label and label in alllabels:
        route=(alllabels[label][0],label); direct_heading=label; break
      if resolved and not route and resolved.suffix.lower() not in ('.md','.png','.jpg','.jpeg','.svg','.pdf'):
       rawasset=resolved.read_bytes()
       try: rawasset.decode('utf-8'); assert b'\0' not in rawasset
       except (UnicodeError,AssertionError): unresolved.append({'source':str(src),'target':target,'reason':'attachment is not UTF-8 text'}); return n
       digest=sha(rawasset); aid='source-file-'+digest[:24]; attachment=attachment_inventory.get(aid)
       if attachment is None or attachment['source_sha256']!=digest:
        unresolved.append({'source':str(src),'target':target,'reason':'source attachment missing from unique inventory or hash collision'}); return n
       owner_part=attachment['part_id']; owner_chapter=attachment['first_use_chapter']
       rel=bundle(resolved,'Source')
       attachments[owner_part][aid]={'id':aid,'source':attachment['source'],'sha256':digest,'bundle':rel,
          'name':Path(attachment['source']).name,'first_use_chapter':owner_chapter,
          'source_paths':attachment['source_paths'],'link_uses':attachment['link_uses']}
       if owner_part==current_part: c[2][0]='#'+aid
       else:
        c[2][0]='../'+owner_part+'/main.pdf#nameddest='+aid
        loc=display_names[attachment['stage_id']]
        if owner_part in part_meta: loc+=', Part '+str(part_meta[owner_part].get('index',1))
        loc+=', Ch. '+str(chapter_order[owner_chapter])
        c[1].extend([{'t':'Space'},{'t':'Str','c':'('+loc+')'}])
       attachment_occurrences.append({'id':aid,'source':str(src),'target':target,'citing_chapter':current_owner,
          'citing_part_id':current_part,'assigned_part_id':owner_part,'remote':owner_part!=current_part})
       links['attachment']+=1; return n
      if resolved and resolved.name=='00 - Reading Map.md':
       route=(vid,'volume-reading-map'); fragment=''
      if route:
       destvol,label=route
       if fragment and not direct_heading:
        heading=headmaps.get(str(resolved.resolve()),{}).get(b.nheading(fragment))
        if heading: label=heading
        else:
         exceptions.append({'source':str(src),'target':target,'kind':'heading-unavailable'}); return {'t':'Span','c':[['',[],[]],c[1]]}
       target_chapter=next((cid for cid in chapters_by_id if label==cid or label.startswith(cid+'-')),None)
       destpart=current_part if label=='volume-reading-map' else (part_by_chapter.get(target_chapter) or part_by_reference.get(label) or current_part)
       if destpart==current_part: c[2][0]='#'+label; links['same_part']+=1
       else:
        c[2][0]='../'+destpart+'/main.pdf#nameddest='+label
        base=next((key for key in locators if label==key or label.startswith(key+'-')),None)
        locator=display_names.get(destvol,destvol)
        if destpart in part_meta: locator+=', Part '+str(part_meta[destpart].get('index',1))
        if base: locator+=', '+locators[base]
        elif label in locators: locator+=', '+locators[label]
        c[1].extend([{'t':'Space'},{'t':'Str','c':'('+locator+')'}]); links['cross_part']+=1
      elif rawpath:
       if resolved and resolved.suffix.lower() in ('.svg','.png','.jpg','.jpeg'):
        asset=resolved
        if resolved.suffix.lower()=='.svg':
         pdf=byhash.get(sha(resolved.read_bytes()))
         if pdf: asset=Path(pdf)
        c[2][0]=bundle(asset,'Print'); links['linked_asset']+=1; return n
       category='excluded-private-provenance' if '/.codex/' in rawpath else ('local-book-citation' if rawpath.lower().endswith('.pdf') else 'source-unavailable')
       exceptions.append({'source':str(src),'target':target,'kind':category}); links[category]+=1
       suffix=Path(rawpath).name if category=='local-book-citation' else 'referenced source unavailable'
       return {'t':'Span','c':[['',[],[]],c[1]]}
     if t=='Image':
      target=c[2][0]
      if target.startswith('../current/Assets/'):
       bundled=stage/target.removeprefix('../current/')
       if not bundled.is_file() or sha(bundled.read_bytes())!=bundled.stem: missing.append({'kind':'invalid-bundled-asset','source':str(src),'target':target})
       return n
      if target.startswith('data:'):
       header,data=target.split(',',1); content=base64.b64decode(data) if ';base64' in header else urllib.parse.unquote_to_bytes(data); pdf=byhash.get(sha(content))
       if pdf: c[2][0]=bundle(pdf,'Print')
       else: missing.append({'kind':'data-image','source':str(src),'sha256':sha(content)})
       return n
      name=urllib.parse.unquote(target); p=(src.parent/name).resolve()
      if not p.is_file(): p=(V/name).resolve()
      if not p.is_file(): p=(V/'Visual Recall'/name).resolve()
      if p.suffix.lower()=='.svg':
       pdf=bypath.get(str(p)) or (byhash.get(sha(p.read_bytes())) if p.is_file() else None);
       if pdf: c[2][0]=bundle(pdf,'Print')
       else: missing.append({'kind':'image','source':str(src),'target':target})
      elif p.suffix.lower() in ('.png','.jpg','.jpeg','.pdf'):
       if p.is_file(): c[2][0]=bundle(p,'Print')
       else: missing.append({'kind':'raster-image','source':str(src),'target':target})
     return n
    doc=transform(doc,change)
    transformed=json.dumps(doc,ensure_ascii=False).encode(); (dest/'AST'/(ident+'.json')).write_bytes(transformed)
    tex=pandoc(transformed,'json','latex', 'chapter' if kind=='chapter' else 'section').decode()
    long_commit='9226f19e867fc89cef869f209408d434daaaf27f'
    if (kind=='reference' and ident=='appendix-a103e7846d21' and
        src.as_posix().endswith('/Supplementary/Sources and Code Anchors.md') and
        expected=='f3c7d99a1bad185322e24fe63d8a971e78ecf7686138696461e51d6db28b4525' and
        tex.count(long_commit)==1):
     inline_chars=''.join('{'+ch+'}' for ch in long_commit)
     tex=tex.replace(long_commit,r'\BookInlineCode{'+inline_chars+'}{'+long_commit+'}')
     exceptions.append({'source':str(src),'source_sha256':expected,'kind':'hosted-verification-commit-hash-wrapped',
      'value':long_commit,
      'reason':'Only the exact long commit identifier is passed through the existing breakable inline-code macro in print TeX; its characters, source text, and semantic value are unchanged.'})
    if (kind=='reference' and
        src.as_posix().endswith('/Supplementary/Game Engine - Sources and Coverage.md') and
        expected=='83bd2d031c9151c9e16ac081659315fb8fcae88305963e6604aafdfa3c51e7db'):
     tex,breakpoint_rows=add_channel_membership_breakpoints(tex)
     exceptions.append({'source':str(src),'source_sha256':expected,
      'kind':'longtable-channel-membership-zero-width-breakpoints','row_count':breakpoint_rows,
      'reason':'Inserted invisible TeX break opportunities after JSON punctuation in the first column so long playlist-membership metadata wraps without entering the Video column; source characters and order are unchanged.'})
    if kind=='reference':
     has_header=any(n.get('t')=='Header' for n in walk(doc.get('blocks',[])))
     tex='\\hypertarget{'+ident+'}{}\\label{referencestart:'+ident+'}\n'+('' if has_header else '\\section*{'+title.replace('&',r'\&').replace('_',r'\_')+'}\n')+tex
    else: tex=chapter_page_marker('\\hypertarget{'+ident+'}{}\n'+tex,ident)
    name=ident+'.tex'; (dest/'content'/name).write_text(tex)
    home=reference_routes[str(src.resolve())]['first_use_chapter'] if kind=='reference' else None
    reason=reference_routes[str(src.resolve())]['first_use_reason'] if kind=='reference' else None
    direct_home=reference_routes[str(src.resolve())].get('direct_first_use_chapter') if kind=='reference' else None
    rows.append({'volume':partid,'stage_id':vid,'part_id':partid,'kind':kind,'id':ident,'first_use_chapter':home,'direct_first_use_chapter':direct_home,'first_use_reason':reason,'source':str(src),'source_sha256':expected,'reader_ast':astpath,'transformed_ast':str(O/'current'/partid/'AST'/(ident+'.json')),'transformed_ast_sha256':sha(transformed),'tex':str(O/'current'/partid/'content'/name),'tex_sha256':sha(tex.encode()),'node_counts':dict(counts),'code_sha256':codehash,'diagrams':diagrams,'image_caption_normalizations':caption_changes,'escaped_cpp_singletons':html_report['count']})
   print(partid,'fragments',len(inputs),flush=True)
 # Generate one entry point per physical book. Initial proof generations use one part per learning stage.
 all_parts=[]
 for logical_stage in stages:
  planned_parts=[p for s in part_data['stages'] if s['id']==logical_stage['id'] for p in s['parts']] if part_data else [{'id':logical_stage['id'],'index':1,'title':logical_stage['title'],'chapter_ids':logical_stage['chapters']}]
  for p in planned_parts: all_parts.append((logical_stage,p))
 for logical_stage,physical in all_parts:
  vid=logical_stage['id']; partid=physical['id']; here=stage/partid
  chapterrows=[r for r in rows if r['part_id']==partid and r['kind']=='chapter']
  refs=[r for r in rows if r['part_id']==partid and r['kind']=='reference']
  title=pandoc(logical_stage['title'].encode(),'markdown','latex').decode().strip()
  part_title=pandoc(physical.get('title',logical_stage['title']).encode(),'markdown','latex').decode().strip()
  main=['\\documentclass[11pt,twoside,openany]{book}','\\usepackage[print]{allmight-book}',
        '\\providecommand{\\SetBookStage}[2]{}',
        '\\SetBookStage{'+str(logical_stage['number'])+'}{'+title+'}',
        '\\title{'+title+'}', '\\author{Swayam Singal}', '\\begin{document}', '\\maketitle',
        '\\begin{center}\\AllMightStageMark{'+str(logical_stage['number'])+'}{'+title+'}\\end{center}',
        '\\chapter*{Reading Map}', '\\phantomsection\\hypertarget{volume-reading-map}{}',
        '\\noindent This is '+part_title+'. The seven learning stages are:']
  for s in stages:
   parts_in_stage=[p for st,p in all_parts if st['id']==s['id']]
   if parts_in_stage:
    first=parts_in_stage[0]
    main.append('\\par\\noindent Stage '+str(s['number'])+': '+s['title']+' --- \\href{../'+first['id']+'/main.pdf}{'+first.get('title',s['title'])+'}.')
  part_idx=int(physical.get('index',1)); parts_in_stage=[p for s,p in all_parts if s['id']==vid]
  if len(parts_in_stage)>1:
   prev=next((p for p in parts_in_stage if int(p.get('index',1))==part_idx-1),None)
   nxt=next((p for p in parts_in_stage if int(p.get('index',1))==part_idx+1),None)
   main.append('\\par\\noindent Physical part '+str(part_idx)+' of '+str(len(parts_in_stage))+'.')
   if prev: main.append('\\par\\noindent Previous part: \\href{../'+prev['id']+'/main.pdf}{'+prev.get('title',prev['id'])+'}.')
   if nxt: main.append('\\par\\noindent Next part: \\href{../'+nxt['id']+'/main.pdf}{'+nxt.get('title',nxt['id'])+'}.')
  main += ['\\tableofcontents','\\mainmatter']
  offset=sum(len(p['chapter_ids']) for p in parts_in_stage if int(p.get('index',1))<part_idx)
  main += ['\\setcounter{chapter}{'+str(offset)+'}']+['\\input{../current/'+partid+'/content/'+Path(r['tex']).name+'}' for r in chapterrows]
  main += ['\\label{stagechaptersend:'+vid+'}']
  if refs: main += ['\\appendix','\\chapter{Reference Notes}']+['\\input{../current/'+partid+'/content/'+Path(r['tex']).name+'}' for r in refs]
  main += ['\\label{stagereferencesend:'+vid+'}']
  if attachments[partid]:
   main += ['\\chapter{Source Attachments}']
   for a in sorted(attachments[partid].values(),key=lambda x:x['name']):
    name=pandoc(a['name'].encode(),'markdown','latex').decode().strip()
    main += ['\\hypertarget{'+a['id']+'}{}','\\label{attachmentstart:'+a['id']+'}','\\section*{'+name+'}','\\VerbatimInput[fontsize=\\small]{'+a['bundle']+'}']
  main += ['\\end{document}']; (here/'main.tex').write_text('\n'.join(main)+'\n')
 # Keep a full logical-stage entry point for chapter/reference page-weight proofs.
 # Physical books remain chapter-boundary slices; this proof file includes every
 # stage chapter and its first-use notes in logical order, independent of the map.
 for logical_stage in stages:
  vid=logical_stage['id']; proof=stage/'_stage-proofs'/vid; proof.mkdir(parents=True,exist_ok=True)
  stage_parts=[p for st,p in all_parts if st['id']==vid]
  chapter_rows={r['id']:r for r in rows if r['stage_id']==vid and r['kind']=='chapter'}
  stage_chapters=[chapter_rows[cid] for cid in logical_stage['chapters']]
  stage_refs=[r for r in rows if r['stage_id']==vid and r['kind']=='reference']
  stage_title=pandoc(logical_stage['title'].encode(),'markdown','latex').decode().strip()
  proof_main=['\\documentclass[11pt,twoside,openany]{book}','\\usepackage[print]{allmight-book}',
              '\\providecommand{\\SetBookStage}[2]{}',
              '\\SetBookStage{'+str(logical_stage['number'])+'}{'+stage_title+'}',
              '\\title{'+stage_title+'}','\\author{Swayam Singal}','\\begin{document}','\\maketitle',
              '\\begin{center}\\AllMightStageMark{'+str(logical_stage['number'])+'}{'+stage_title+'}\\end{center}',
              '\\chapter*{Reading Map}','\\phantomsection\\hypertarget{volume-reading-map}{}',
              '\\noindent This proof covers the complete learning stage.']
  for other in stages:
   other_parts=[p for st,p in all_parts if st['id']==other['id']]
   if other_parts:
    first=other_parts[0]
    proof_main.append('\\par\\noindent Stage '+str(other['number'])+': '+other['title']+' --- \\href{../'+first['id']+'/main.pdf}{'+first.get('title',other['title'])+'}.')
  proof_main += ['\\tableofcontents','\\mainmatter','\\setcounter{chapter}{0}']
  proof_main += ['\\input{../current/'+r['part_id']+'/content/'+Path(r['tex']).name+'}' for r in stage_chapters]
  proof_main += ['\\label{stagechaptersend:'+vid+'}']
  if stage_refs:
   proof_main += ['\\appendix','\\chapter{Reference Notes}']
   proof_main += ['\\input{../current/'+r['part_id']+'/content/'+Path(r['tex']).name+'}' for r in stage_refs]
  proof_main += ['\\label{stagereferencesend:'+vid+'}']
  stage_attachments=[]
  for part in stage_parts:
   stage_attachments.extend(attachments.get(part['id'],{}).values())
  if stage_attachments:
   proof_main += ['\\chapter{Source Attachments}']
   for attachment in sorted(stage_attachments,key=lambda x:x['name']):
    name=pandoc(attachment['name'].encode(),'markdown','latex').decode().strip()
    proof_main += ['\\hypertarget{'+attachment['id']+'}{}','\\label{attachmentstart:'+attachment['id']+'}',
                   '\\section*{'+name+'}','\\VerbatimInput[fontsize=\\small]{'+attachment['bundle']+'}']
  proof_main += ['\\end{document}']; (proof/'main.tex').write_text('\n'.join(proof_main)+'\n')
 report={'schema':'allmight-latex-conversion-v2','rows':rows,'stages':stages,'physical_parts':part_data,'companion_assignment':{'rule':'earliest direct chapter citation in approved stage order; legacy semantic-volume fallback only for notes with no direct chapter use; global indexes go to stage 7 reference atlas','direct_chapter_use_count':len(direct_first_use),'legacy_semantic_volume_fallback_count':len(plan['assignments'])-len(direct_first_use),'stage7_reference_atlas_count':sum(1 for x in first_use_reason.values() if x=='stage7-reference-atlas'),'assigned_once':len(reference_routes)==439,'missing_first_use':0,'duplicate_assignment_ids':0},'node_counts':dict(total),'links':dict(links),'missing_assets':missing,'unresolved_links':unresolved,'exceptions':exceptions,'warnings':warnings,'generation_id':stage.name,'attachments':dict(attachments),'image_caption_normalizations':image_caption_normalizations,'chapter_navigation':{'remapped_count':len(navigation_remaps),'records':navigation_remaps},
  'source_attachment_inventory':{'unique_count':len(attachment_inventory),
  'unique_ids':len(attachment_inventory)==len(set(attachment_inventory)),
  'records':list(attachment_inventory.values()),'link_occurrence_count':len(attachment_occurrences),
  'link_occurrences':attachment_occurrences}}
 (stage/'conversion-manifest.json').write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n')
 print('missing assets',len(missing),'links',dict(links),'nodes',dict(total))
 if missing or unresolved:
  raise RuntimeError('Unresolved asset or link targets; see '+str(stage/'conversion-manifest.json'))
 if os.environ.get('ALLMIGHT_STAGE_ONLY')=='1':
  print('STAGED',stage,flush=True); return
 # A single symlink switch publishes all stages and physical books together.
 generations=O/'Generations'; generations.mkdir(exist_ok=True)
 generation=generations/stage.name; stage.rename(generation)
 wrapper_ids=[p['id'] for _,p in all_parts]
 for vid in wrapper_ids:
  out=O/vid; out.mkdir(exist_ok=True)
  wrapper='\\input{../current/'+vid+'/main.tex}\n'
  temp=out/'.main.tex.tmp'; temp.write_text(wrapper); temp.replace(out/'main.tex')
 marker=O/'conversion-manifest.json'
 if not marker.is_symlink():
  if marker.exists(): marker.replace(O/'conversion-manifest.previous.json')
  marker.symlink_to('current/conversion-manifest.json')
 nextlink=O/'.current-next'
 if nextlink.is_symlink() or nextlink.exists(): nextlink.unlink()
 nextlink.symlink_to(Path('Generations')/generation.name, target_is_directory=True)
 nextlink.replace(O/'current')
 state={'generation_id':generation.name,'generation_path':str(Path('Generations')/generation.name),'stages':[{'id':s['id'],'title':s['title'],'number':s['number']} for s in stages],'parts':wrapper_ids}
 temp=O/'.CURRENT.json.tmp'; temp.write_text(json.dumps(state,indent=2)+'\n'); temp.replace(O/'CURRENT.json')
 print('PROMOTED',generation,flush=True)
if __name__=='__main__': main()
