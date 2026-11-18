#!/usr/bin/env python3
"""Prepare ten source-faithful Mermaid layout candidates for LaTeX review."""
from __future__ import annotations
import hashlib, json, re
from pathlib import Path

HERE = Path(__file__).resolve().parent
LATEX = HERE.parent
PRODUCTION = LATEX.parent
SOURCE_JOBS = PRODUCTION / 'jobs.json'
OUT = LATEX / 'Assets' / 'Diagrams' / 'Review-v1'
JOB_FILE = HERE / 'compact-layout-variants.jobs.json'
REPORT_FILE = OUT / 'variant-provenance.json'
# Deliberately covers straight, branching, feedback-loop, and wide conceptual flows.
SOURCES = [
    'd21d21b04281361cfda13e69',  # seven-step resource lifecycle
    '843c03a92a2bd701feb3265d',  # ownership and failure paths
    'd1a7a180dbe8b9fa761dcd70',  # fixed-step loop and decision branch
    '8ffa21a425f320ac21b9143b',  # stiffness/mass generalized modes
    '68fe71157effee5a538f53db',  # world-to-image transport chain
]


def sha(text: str) -> str:
    return hashlib.sha256(text.encode()).hexdigest()


def replace_direction(code: str, direction: str) -> str:
    pattern = re.compile(r'^(\s*(?:flowchart|graph)\s+)(LR|RL|TD|TB|BT)\b', re.M)
    result, n = pattern.subn(lambda m: m.group(1) + direction, code, count=1)
    if n != 1:
        raise ValueError(f'expected one flowchart/graph direction, found {n}')
    return result


def inject_compact_config(code: str, node: int, rank: int, wrap: int) -> str:
    # Keep all original directives and definitions. A later init directive only
    # overrides spacing/font/wrapping for the candidate's print layout.
    directive = ("%%%%{init: {'themeVariables': {'fontSize': '12px', 'fontFamily': 'Arial'}, "
                 "'flowchart': {'nodeSpacing': %d, 'rankSpacing': %d, "
                 "'padding': 4, 'wrappingWidth': %d, 'htmlLabels': false}}}%%%%\n"
                 % (node, rank, wrap))
    lines = code.splitlines(keepends=True)
    insert_at = 0
    while insert_at < len(lines) and lines[insert_at].lstrip().startswith('%%{init:'):
        insert_at += 1
    lines.insert(insert_at, directive)
    return ''.join(lines)


def main() -> None:
    OUT.mkdir(parents=True, exist_ok=True)
    loaded = json.loads(SOURCE_JOBS.read_text())
    by_id = {j['id']: j for j in loaded['jobs'] if j.get('kind') == 'mermaid'}
    jobs, entries = [], []
    for source_id in SOURCES:
        src = by_id[source_id]
        code = src['code']
        if src.get('kind') != 'mermaid':
            raise ValueError(f'not a Mermaid job: {source_id}')
        for i, direction in enumerate(('LR', 'TB')):
            node_spacing = (12, 18, 12, 18, 12)[SOURCES.index(source_id)]
            rank_spacing = (12, 16, 16, 12, 16)[SOURCES.index(source_id)]
            wrapping = (104, 112, 104, 112, 104)[SOURCES.index(source_id)]
            variant_id = f'{source_id}-{direction.lower()}'
            transformed = replace_direction(code, direction)
            transformed = inject_compact_config(transformed, node_spacing, rank_spacing, wrapping)
            outfile = OUT / f'{variant_id}.svg'
            jobs.append({'id': variant_id, 'kind': 'mermaid', 'code': transformed,
                         'outfile': str(outfile), 'source': src['source'], 'volume': src['volume']})
            entries.append({
                'variant_id': variant_id, 'source_id': source_id,
                'source_markdown': src['source'], 'volume': src['volume'],
                'source_code_sha256': sha(code), 'source_code_sha256_jobs': src.get('code_sha256'),
                'rendered_code_sha256': sha(transformed), 'layout': direction,
                'base_font_px': 12, 'node_spacing': node_spacing,
                'rank_spacing': rank_spacing, 'label_wrap_px': wrapping,
                'source_svg': src['outfile'], 'variant_svg': str(outfile),
                'semantic_inputs_unchanged': True,
                'preservation_note': 'Only graph direction and a later Mermaid print-layout init directive are added; source node declarations, edge declarations, IDs, and label characters are kept byte-for-byte.'
            })
    JOB_FILE.write_text(json.dumps({'schema':'allmight-compact-layout-jobs-v1','assets':jobs}, indent=2) + '\n')
    REPORT_FILE.write_text(json.dumps({'schema':'allmight-compact-layout-review-v1',
        'renderer_jobs':str(JOB_FILE),'source_jobs':str(SOURCE_JOBS),
        'source_jobs_sha256':sha(SOURCE_JOBS.read_text()),'variants':entries}, indent=2)+'\n')
    print(f'prepared {len(jobs)} candidates from {len(SOURCES)} canonical Mermaid jobs')

if __name__ == '__main__': main()
