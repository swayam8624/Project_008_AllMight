#!/usr/bin/env python3
"""Preserve Mermaid word spaces through librsvg's vector PDF conversion."""
import argparse, json, re
from pathlib import Path
root=Path(__file__).resolve().parents[1]
ap=argparse.ArgumentParser(); ap.add_argument('jobs',nargs='?',default=str(root/'Tools'/'compact-layout-variants.jobs.json')); args=ap.parse_args()
jobs=json.loads(Path(args.jobs).read_text())['assets']
for job in jobs:
    path=Path(job['outfile']); svg=path.read_text()
    if not re.search(r'\bxml:space\s*=\s*["\']preserve["\']',svg):
        svg,n=re.subn(r'(<svg\b[^>]*)(>)',r'\1 xml:space="preserve"\2',svg,count=1)
        if n!=1: raise SystemExit(f'no SVG root in {path}')
    svg=re.sub(r'<tspan\b[^>]*>',lambda m:m.group(0)[:-1]+' xml:space="preserve">' if 'xml:space=' not in m.group(0) else m.group(0),svg)
    path.write_text(svg)
print(f'preserved SVG root and tspan whitespace in {len(jobs)} candidates')
