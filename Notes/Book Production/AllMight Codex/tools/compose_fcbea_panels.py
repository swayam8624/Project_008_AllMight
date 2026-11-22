"""Build a private, self-contained SVG from the two reviewed fcbea panels.

The original Mermaid chapter diagram and production asset remain unchanged.
"""
from __future__ import annotations
import argparse, json, pathlib, re, hashlib, xml.etree.ElementTree as ET

ROOT = pathlib.Path(__file__).resolve().parent.parent / 'Design' / 'Diagram Sources'
parser = argparse.ArgumentParser(description='Compose the approved packaged fcbea Mermaid panels as one self-contained SVG.')
parser.add_argument('--panel-a', type=pathlib.Path, default=ROOT/'fcbea-panel-a-TB.svg')
parser.add_argument('--panel-b', type=pathlib.Path, default=ROOT/'fcbea-panel-b-TB.svg')
parser.add_argument('--output', type=pathlib.Path, default=ROOT/'fcbea-composite.svg')
parser.add_argument('--report', type=pathlib.Path, default=ROOT/'fcbea-composite-report.json')
args = parser.parse_args()
INPUTS = [args.panel_a, args.panel_b]
OUT = args.output
REPORT = args.report
NS='http://www.w3.org/2000/svg'

def read_svg(path: pathlib.Path, prefix: str):
    source = path.read_text()
    root = ET.fromstring(source)
    width = float(re.match(r'([0-9.]+)', root.attrib['width']).group(1))
    height = float(re.match(r'([0-9.]+)', root.attrib['height']).group(1))
    # Prefix every actual SVG ID and all local fragment references, including CSS selectors.
    ids = re.findall(r'(?:^|\s)id="([^"]+)"', source)
    if len(ids) != len(set(ids)):
        raise ValueError(f'duplicate source IDs in {path}')
    mapping = {old: f'{prefix}-{old}' for old in ids}
    for old, new in mapping.items():
        source = re.sub(r'(?<![\w-])id=\"'+re.escape(old)+r'\"', 'id=\"'+new+'\"', source)
        source = source.replace('#'+old, '#'+new)
        # IDREFS such as aria-labelledby contain bare, space-separated IDs.
        for attr in ['aria-labelledby', 'aria-describedby']:
            source = re.sub(r'('+attr+r'="[^"]*)\b'+re.escape(old)+r'\b', lambda m: m.group(1)+new, source)
    root_match = re.search(r'<svg\b[^>]*>', source)
    if not root_match:
        raise ValueError(f'missing SVG root in {path}')
    tag = root_match.group(0)
    tag = re.sub(r'\s+x="[^"]*"', '', tag)
    tag = re.sub(r'\s+y="[^"]*"', '', tag)
    tag = tag[:-1] + f' x="0" y="0">'
    source = source[:root_match.start()] + tag + source[root_match.end():]
    # Strip the inner root's XML declaration if one appears; Mermaid normally emits none.
    source = re.sub(r'^\s*<\?xml[^>]*\?>', '', source)
    return source, width, height, mapping

pA, wA, hA, idsA = read_svg(INPUTS[0], 'panelA')
pB, wB, hB, idsB = read_svg(INPUTS[1], 'panelB')
# Natural intrinsic geometry in CSS px. Keep both panels and captions within 396 x 500 pt.
margin=8
outer_w=max(wA,wB)+2*margin
caption_a_y=22
panel_a_y=30
caption_b_y=panel_a_y+hA+26
panel_b_y=caption_b_y+8
note_y=panel_b_y+hB+24
outer_h=note_y+80

def sized_nested(svg: str, x: float, y: float, w: float, h: float) -> str:
    tag=re.search(r'<svg\b[^>]*>',svg).group(0)
    tag=re.sub(r'\s+x="[^"]*"','',tag)
    tag=re.sub(r'\s+y="[^"]*"','',tag)
    tag=re.sub(r'\s+width="[^"]*"','',tag)
    tag=re.sub(r'\s+height="[^"]*"','',tag)
    tag=tag[:-1]+f' x="{x:.3f}" y="{y:.3f}" width="{w:.3f}px" height="{h:.3f}px">'
    return svg.replace(re.search(r'<svg\b[^>]*>',svg).group(0),tag,1)

pA=sized_nested(pA,margin,panel_a_y,wA,hA)
pB=sized_nested(pB,margin,panel_b_y,wB,hB)
svg=(f'<svg xmlns="{NS}" width="{outer_w:.3f}px" height="{outer_h:.3f}px" '
     f'viewBox="0 0 {outer_w:.3f} {outer_h:.3f}" xml:space="preserve" role="img" '
     'aria-labelledby="fcbea-title fcbea-description">'
     '<title id="fcbea-title">GPU update paths and command buffer retirement</title>'
     '<desc id="fcbea-description">Two connected panels preserve the complete source graph. Panel A shows asset, descriptor and batch inputs joining at the command buffer. Panel B shows picking and render-target branches joining at that same command buffer before GPU completion and safe retirement.</desc>'
     '<style>.panel-caption{font-family:Palatino,Georgia,serif;font-size:18px;font-weight:600;fill:#111}.shared-note{font-family:Palatino,Georgia,serif;font-size:13px;fill:#222}</style>'
     f'<text class="panel-caption" x="{margin}" y="{caption_a_y}">A. Asset, shader and batch inputs</text>'
     +pA+
     f'<text class="panel-caption" x="{margin}" y="{caption_b_y}">B. Scene, target and completion path</text>'
     +pB+
     f'<text class="shared-note" x="{margin}" y="{note_y}"><tspan x="{margin}" dy="0">The command buffer in panel B is the same submission</tspan>'
     f'<tspan x="{margin}" dy="16">shown in panel A.</tspan><tspan x="{margin}" dy="16">Together, the panels follow updates to the final buffer</tspan>'
     f'<tspan x="{margin}" dy="16">through GPU completion and safe retirement.</tspan></text>'
     '</svg>')
ET.fromstring(svg) # Fail closed on malformed composed XML.
OUT.write_text(svg)
# Validate global ID uniqueness and local reference resolution across nested panels.
root=ET.fromstring(svg); all_ids=[e.attrib['id'] for e in root.iter() if 'id' in e.attrib]
if len(all_ids)!=len(set(all_ids)): raise ValueError('composite contains duplicate ids')
idset=set(all_ids); unresolved=[]
for e in root.iter():
    for k,v in e.attrib.items():
        for ref in re.findall(r'url\(#([^\)]+)\)',v):
            if ref not in idset: unresolved.append((k,ref))
        if k in ('href','{http://www.w3.org/1999/xlink}href') and v.startswith('#') and v[1:] not in idset:
            unresolved.append((k,v[1:]))
for style in [e.text or '' for e in root.iter() if e.tag.endswith('style')]:
    for ref in re.findall(r'url\(#([^\)]+)\)',style):
        if ref not in idset: unresolved.append(('style-url',ref))
    for selector in re.findall(r'(?<![\w-])#([A-Za-z][\w.-]*)',style):
        if selector not in idset and selector.startswith(('diagram-','flowchart-')): unresolved.append(('style-selector',selector))
if unresolved: raise ValueError(f'unresolved local references: {unresolved[:20]}')
report={'schema':'fcbea-single-svg-composite-v1','renderer_version':'allmight-fcbea-composite-v2','svg':str(OUT),'svg_sha256':hashlib.sha256(svg.encode()).hexdigest(),'source_panels':[str(x) for x in INPUTS],'source_panel_svg_sha256':[hashlib.sha256(x.read_bytes()).hexdigest() for x in INPUTS],'outer_dimensions_css_px':[outer_w,outer_h],'outer_dimensions_pt':[outer_w*.75,outer_h*.75],'fits_396x500pt':outer_w*.75<=396 and outer_h*.75<=500,'panel_positions_css_px':{'A':[margin,panel_a_y,wA,hA],'B':[margin,panel_b_y,wB,hB]},'prefixed_id_counts':{'panelA':len(idsA),'panelB':len(idsB),'total_unique':len(all_ids)},'unresolved_local_references':unresolved,'source_file_untouched':True,'source_job_untouched':True,'captions':['A. Asset, shader and batch inputs','B. Scene, target and completion path','The command buffer in panel B is the same submission shown in panel A.','Together, the panels follow updates to the final buffer through GPU completion and safe retirement.']}
REPORT.parent.mkdir(parents=True, exist_ok=True)
REPORT.write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps(report,indent=2))
