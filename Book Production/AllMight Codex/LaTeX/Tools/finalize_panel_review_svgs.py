#!/usr/bin/env python3
"""Finalize tentative panel SVG text spacing and small state labels."""
from __future__ import annotations
import hashlib, json, re
from pathlib import Path

LATEX = Path(__file__).resolve().parents[1]
OUT = LATEX / 'Assets' / 'Diagrams' / 'Panels-Review-next'

def sha(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()

def main() -> None:
    report_path = OUT / 'panel-provenance.json'
    report = json.loads(report_path.read_text())
    changed = []
    for graph in report['source_panels']:
        for panel in graph['panels']:
            path = Path(panel['svg'])
            before = path.read_bytes()
            text = before.decode('utf-8')
            after = text
            root = re.search(r'<svg\b[^>]*>', after)
            if not root:
                raise RuntimeError(f'SVG root not found in {path.name}')
            if 'xml:space=' not in root.group(0):
                after = after[:root.end()-1] + ' xml:space="preserve">' + after[root.end():]
            if graph['source_kind'].startswith('stateDiagram'):
                after = after.replace('font-size:10px', 'font-size:12px')
            if after == text:
                panel['svg_sha256'] = sha(before)
                continue
            path.write_text(after, encoding='utf-8')
            panel['svg_pre_finalize_sha256'] = sha(before)
            panel['svg_sha256'] = sha(path.read_bytes())
            adjustments = []
            if 'xml:space="preserve"' in after and 'xml:space="preserve"' not in text:
                adjustments.append('Set xml:space="preserve" on SVG root so SVG renderers retain authored spaces across adjacent tspans.')
            if graph['source_kind'].startswith('stateDiagram') and 'font-size:10px' in text:
                adjustments.append('State diagram CSS font-size 10px to 12px; node IDs and label text unchanged.')
            panel['tentative_svg_adjustments'] = adjustments
            changed.append(path.name)
    report['state_label_style_adjustments'] = changed
    report_path.write_text(json.dumps(report, indent=2) + '\n')
    print(f'adjusted {len(changed)} tentative state panel SVGs: {", ".join(changed)}')

if __name__ == '__main__':
    main()
