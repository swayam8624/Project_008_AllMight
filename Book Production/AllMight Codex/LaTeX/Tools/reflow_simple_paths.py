#!/usr/bin/env python3
"""Reflow unlabelled directed paths into a compact two-row print SVG, without changing graph identity."""
import base64,json,re,shutil,subprocess
from pathlib import Path
from xml.etree import ElementTree as ET
HERE=Path(__file__).resolve().parent; LATEX=HERE.parent
NS={'s':'http://www.w3.org/2000/svg'}
ET.register_namespace('',NS['s']); ET.register_namespace('xlink','http://www.w3.org/1999/xlink')
prov=json.loads((HERE/'compact-print-provenance.json').read_text())
selection=json.loads((HERE/'compact-print-selection-audit.json').read_text())
sel={x['source_asset_id']:x for x in selection['assets']}

def edge_nodes(data_id,names):
 raw=data_id.removeprefix('L_')
 if '_' not in raw:return None
 raw=raw.rsplit('_',1)[0]
 for src in sorted(names,key=len,reverse=True):
  prefix=src+'_'
  if raw.startswith(prefix):
   dst=raw[len(prefix):]
   if dst in names:return src,dst
 return None

def reflow(svg_path,out_path):
 tree=ET.parse(svg_path); root=tree.getroot(); nodes={}
 for el in root.iter():
  if el.tag.rsplit('}',1)[-1]=='g' and 'node' in el.attrib.get('class','').split():
   m=re.search(r'-(?:flowchart|state)-(.+)$',el.attrib.get('id',''))
   if not m:continue
   name=re.sub(r'-\d+$','',m.group(1)); rect=next((x for x in el.iter() if x.tag.rsplit('}',1)[-1]=='rect' and 'label-container' in x.attrib.get('class','').split()),None)
   if rect is None:return None
   w=float(rect.attrib['width']); h=float(rect.attrib['height']); xy=re.search(r'translate\(([-\d.]+)[ ,]+([-\d.]+)\)',el.attrib.get('transform',''))
   if not xy:return None
   nodes[name]={'el':el,'w':w,'h':h,'x':float(xy.group(1)),'y':float(xy.group(2))}
 if len(nodes)<5:return None
 edges=[]
 for el in root.iter():
  if el.tag.rsplit('}',1)[-1]=='path' and 'flowchart-link' in el.attrib.get('class','').split():
   pair=edge_nodes(el.attrib.get('data-id',''),set(nodes))
   if pair is None:return None
   edges.append((el,pair[0],pair[1]))
 if len(edges) not in (len(nodes)-1,len(nodes)):return None
 # Refuse edge-label groups: their placement needs layout-specific geometry.
 if any('edgeLabel' in e.attrib.get('class','').split() and ''.join(e.itertext()).strip() for e in root.iter()):return None
 outs={n:[] for n in nodes}; ins={n:[] for n in nodes}
 for _,a,b in edges:outs[a].append(b);ins[b].append(a)
 if any(len(outs[n])>1 or len(ins[n])>1 for n in nodes):return None
 starts=[n for n in nodes if not ins[n]]; cycle=not starts
 if cycle:
  if any(not ins[n] or not outs[n] for n in nodes):return None
  start=min(nodes)
 else:
  if len(starts)!=1:return None
  start=starts[0]
 order=[]; cur=start
 while cur not in order:
  order.append(cur)
  if not outs[cur]:break
  cur=outs[cur][0]
 if len(order)!=len(nodes) or (cycle and cur!=start) or (not cycle and cur in order[:-1]):return None
 path_edges={(a,b):e for e,a,b in edges}
 # Balanced rows use source order on the top and reverse order on the bottom,
 # so the directed path remains visually continuous across the page.
 cut=(len(order)+1)//2; top=order[:cut]; bottom=list(reversed(order[cut:]))
 if not bottom:return None
 gap=24; row_gap=42; margin=22
 rowwidths=[sum(nodes[n]['w'] for n in row)+gap*(len(row)-1) for row in (top,bottom)]
 width=max(rowwidths)+2*margin
 maxh=max(v['h'] for v in nodes.values()); y1=margin+maxh/2; y2=y1+maxh+row_gap
 centers={}
 for row,y,row_width in ((top,y1,rowwidths[0]),(bottom,y2,rowwidths[1])):
  x=margin+(max(rowwidths)-row_width)
  for name in row:
   v=nodes[name]; centers[name]=(x+v['w']/2,y); x+=v['w']+gap
 height=y2+maxh/2+margin
 for name,(x,y) in centers.items():
  nodes[name]['el'].set('transform',f'translate({x:.3f}, {y:.3f})')
 for el,a,b in edges:
  ax,ay=centers[a]; bx,by=centers[b]; ah=nodes[a]['h']/2; bh=nodes[b]['h']/2; aw=nodes[a]['w']/2; bw=nodes[b]['w']/2
  if abs(ay-by)<1:
   sign=1 if bx>ax else -1; p1=(ax+sign*aw,ay); p2=(bx-sign*bw,by); d=f'M{p1[0]:.3f},{p1[1]:.3f} L{p2[0]:.3f},{p2[1]:.3f}'
  elif abs(ax-bx)<max(aw,bw)+2:
   sign=1 if by>ay else -1; p1=(ax,ay+sign*ah); p2=(bx,by-sign*bh); d=f'M{p1[0]:.3f},{p1[1]:.3f} L{p2[0]:.3f},{p2[1]:.3f}'
  else:
   sx=1 if bx>ax else -1; sy=1 if by>ay else -1
   p1=(ax+sx*aw,ay); p2=(bx,by-sy*bh); bend=(p1[0]+p2[0])/2
   d=f'M{p1[0]:.3f},{p1[1]:.3f} L{bend:.3f},{p1[1]:.3f} L{bend:.3f},{p2[1]:.3f} L{p2[0]:.3f},{p2[1]:.3f}'
  el.set('d',d)
  # Refresh renderer point data so SVG remains useful to downstream inspectors.
  el.set('data-points',base64.b64encode(json.dumps([{'x':centers[a][0],'y':centers[a][1]},{'x':centers[b][0],'y':centers[b][1]}],separators=(',',':')).encode()).decode())
 if cycle:
  a=order[-1]; b=order[0]; el=path_edges[(a,b)]; ax,ay=centers[a];bx,by=centers[b]; aw=nodes[a]['w']/2;bw=nodes[b]['w']/2
  left=margin/2; d=f'M{ax-aw:.3f},{ay:.3f} L{left:.3f},{ay:.3f} L{left:.3f},{by:.3f} L{bx-bw:.3f},{by:.3f}';el.set('d',d)
 root.set('width',f'{width:.3f}px');root.set('height',f'{height:.3f}px');root.set('viewBox',f'0 0 {width:.3f} {height:.3f}');root.set('style',f'max-width: {width:.3f}px;')
 tree.write(out_path,encoding='unicode',xml_declaration=True)
 return {'nodes':order,'edges':len(edges),'cycle':cycle,'width_px':round(width,2),'height_px':round(height,2)}

made=[]
for a in prov['assets']:
 # Read the compact LR candidate even if the fit selector initially chose TB.
 base=LATEX/'Assets'/'Diagrams'/'Print-v1'/f"{a['source_asset_id']}-lr.svg"
 if not base.exists():continue
 out=base.with_name(base.stem+'-wrap2.svg')
 data=reflow(base,out)
 if data:
  pdf=out.with_suffix('.pdf')
  r=subprocess.run(['rsvg-convert','-f','pdf','-o',str(pdf),str(out)],capture_output=True,text=True)
  if r.returncode:out.unlink(missing_ok=True);raise SystemExit(r.stderr)
  fit=min(1,.98*447.84/(data['width_px']*.75),.62*606.24/(data['height_px']*.75));pt=9*fit
  made.append({'source_asset_id':a['source_asset_id'],'variant':out.stem,'svg':str(out),'pdf':str(pdf),'projected_label_pt':round(pt,3),'meets_8pt':pt>=8,**data})
(HERE/'reflow-simple-paths-report.json').write_text(json.dumps({'schema':'allmight-simple-path-reflow-v1','candidate_count':len(made),'variants':made},indent=2)+'\n')
print(json.dumps({'candidate_count':len(made),'passes_8pt':sum(x['meets_8pt'] for x in made),'below_8pt':[x['source_asset_id'] for x in made if not x['meets_8pt']]},indent=2))
