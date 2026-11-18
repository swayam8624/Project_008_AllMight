#!/usr/bin/env python3
"""Prepare source-faithful semantic panel candidates for the 11 held diagrams.

This writes only to Assets/Diagrams/Panels-Review-next and its renderer job file.
It does not modify source Markdown, canonical diagrams, Print-v1 assets, or the
active override map.
"""
from __future__ import annotations

import hashlib
import json
import re
from pathlib import Path

LATEX = Path(__file__).resolve().parents[1]
PRODUCTION = LATEX.parent
OUT = LATEX / 'Assets' / 'Diagrams' / 'Panels-Review-next'
JOBS = OUT / 'jobs.json'
REPORT = OUT / 'panel-provenance.json'
OVERRIDES = LATEX / 'Assets' / 'Diagrams' / 'Print-v1' / 'overrides.json'
AUDIT = LATEX / 'QA' / 'figure-layout-audit.json'

# Each range names zero-based source-code lines to retain in each panel. Flow
# and state panels contain complete, unchanged edge/transition declarations.
# Sequence panels repeat participant declarations verbatim, then partition
# message/control lines; alt/else/end groups always stay together.
PLAN = {
    '0afe9d09c4fc80ae1bff5d38': [[1, 2, 3], [4, 5, 6], [7, 8]],
    '0b0e76bfeca07fcd2864142a': [[4, 5, 6], [7, 8, 9]],
    '2ce7218d89da9386836a7652': [[1, 2, 3], [4, 5, 6], [7, 8, 9]],
    'ae23cbd6aa6c6b535c2ab7a2': [[1, 2, 3], [4, 5, 6, 7], [8, 9, 10]],
    '4587c34d178f477c59c8cb55': [[4, 5, 6], [7, 8, 9, 10, 11, 12]],
    'cf7ccfbe05510be35f986d5b': [[1, 2, 3], [4, 5, 6], [7, 8, 9]],
    'fb9dffc7a9c985e55f55150e': [[1, 2, 3], [4, 5, 6, 7]],
    '8e675126df1a1b1854a18b23': [[6, 7], [8, 9, 10], [11, 12, 13, 14, 15, 16]],
    '12f60b7ab1e672dd10535591': [[6, 7, 8], [9, 10, 11, 16],
                                 [9, 12, 13, 16], [9, 14, 15, 16]],
    'cc5b9ccaf7c8fa11a4b0ea16': [[4, 5, 6, 7], [8, 9, 10]],
    '3c3f8693bed9a50626badd2c': [[1, 2, 3, 4], [5, 6, 7]],
}


def sha(text: str) -> str:
    return hashlib.sha256(text.encode('utf-8')).hexdigest()


def find_source(row: dict) -> tuple[str, str]:
    expected = set(row['source_code_sha256'])
    for source_row in row['source_rows']:
        path = Path(source_row['source'])
        if not path.exists():
            continue
        text = path.read_text(encoding='utf-8', errors='replace')
        for match in re.finditer(r'```mermaid\s*\n(.*?)\n```', text, re.S):
            code = match.group(1)
            if sha(code) in expected:
                return str(path), code
    raise RuntimeError(f"canonical Mermaid source not found for {row['source_id']}")


def init_directive(kind: str, extra_x_margin: bool = False) -> str:
    if kind == 'sequenceDiagram':
        config = ("{'themeVariables': {'fontSize': '12px', 'fontFamily': 'Arial'}, "
                  "'sequence': {'actorFontSize': 12, 'messageFontSize': 12, "
                  "'noteFontSize': 12, 'actorMargin': 4, 'messageMargin': 5, "
                  "'diagramMarginX': 2, 'diagramMarginY': 2}" +
                  (", 'diagramMarginX': 24, 'diagramMarginY': 8" if extra_x_margin else "") + "}")
    else:
        config = ("{'themeVariables': {'fontSize': '12px', 'fontFamily': 'Arial'}, "
                  "'flowchart': {'nodeSpacing': 14, 'rankSpacing': 18, "
                  "'padding': 4, 'wrappingWidth': 132, 'htmlLabels': false}}")
    return f'%%{{init: {config}}}%%'


def wrap_sequence_label(line: str, limit: int = 28) -> tuple[str, str | None]:
    """Insert display-only breaks while retaining exact concatenated label text."""
    match = re.match(r'^(\s*(?:[^:]+?:\s*))(.*)$', line)
    if not match:
        return line, None
    prefix, label = match.groups()
    words = label.split()
    if not words:
        return line, None
    chunks, current = [], ''
    for word in words:
        candidate = (current + ' ' + word).strip()
        if current and len(candidate) > limit:
            chunks.append(current)
            current = word
        else:
            current = candidate
    if current:
        chunks.append(current)
    if len(chunks) == 1:
        return line, None
    rendered = prefix + '<br/>'.join(chunks)
    return rendered, label


def main() -> None:
    OUT.mkdir(parents=True, exist_ok=True)
    overrides = json.loads(OVERRIDES.read_text())
    if set(PLAN) != set(overrides['held_for_panel_recomposition']):
        raise RuntimeError('panel plan does not exactly match current held IDs')
    audit_rows = {row['source_id']: row for row in json.loads(AUDIT.read_text())['assets']}
    jobs, entries = [], []
    for source_id in overrides['held_for_panel_recomposition']:
        row = audit_rows[source_id]
        source_path, original = find_source(row)
        source_lines = original.splitlines()
        kind = source_lines[0].strip()
        if not (kind.startswith('flowchart ') or kind.startswith('graph ')
                or kind == 'sequenceDiagram' or kind.startswith('stateDiagram')):
            raise RuntimeError(f'unsupported source kind: {kind}')
        plan = PLAN[source_id]
        body_lines = [i for group in plan for i in group]
        participants = [line for line in range(1, len(source_lines))
                        if source_lines[line].lstrip().startswith('participant ')]
        is_sequence = kind == 'sequenceDiagram'
        available = set(range(1, len(source_lines))) - (set(participants) if is_sequence else set())
        duplicates = sorted(i for i in set(body_lines) if body_lines.count(i) > 1)
        permitted_duplicates = ([9, 16] if source_id == '12f60b7ab1e672dd10535591' else [])
        if set(body_lines) != available or duplicates != permitted_duplicates:
            raise RuntimeError(f'panel plan fails exact source-line partition for {source_id}: '
                               f'missing={sorted(available-set(body_lines))}, '
                               f'extra={sorted(set(body_lines)-available)}, duplicates={duplicates}')

        panel_records = []
        for panel_index, selected in enumerate(plan, 1):
            rendered_header = source_lines[0]
            if kind.startswith(('flowchart LR', 'graph LR')):
                rendered_header = re.sub(r'\bLR\b', 'TB', rendered_header, count=1)
            lines = [rendered_header]
            repeated = []
            if is_sequence:
                referenced = set()
                for i in selected:
                    message = re.match(r'^\s*([A-Za-z][A-Za-z0-9_]*)\s*(?:[-=]+>>?|-->>|<<--|-->|->>).*?:', source_lines[i])
                    if message:
                        referenced.add(message.group(1))
                        rhs = re.search(r'(?:[-=]+>>?|-->>|<<--|-->|->>)\s*([A-Za-z][A-Za-z0-9_]*)\s*:', source_lines[i])
                        if rhs:
                            referenced.add(rhs.group(1))
                    note = re.search(r'Note over\s+([^:]+):', source_lines[i])
                    if note:
                        referenced.update(re.findall(r'[A-Za-z][A-Za-z0-9_]*', note.group(1)))
                declared = {re.match(r'^\s*participant\s+([A-Za-z][A-Za-z0-9_]*)', source_lines[i]).group(1): i
                            for i in participants}
                missing_participants = sorted(referenced - set(declared))
                if missing_participants:
                    raise RuntimeError(f'undeclared sequence participants {missing_participants} in {source_id}')
                repeated = [declared[name] for name in declared if name in referenced]
                lines += [source_lines[i] for i in repeated]
            rendered_labels = []
            synthetic_content = []
            for i in selected:
                line = source_lines[i]
                if is_sequence:
                    line, original_label = wrap_sequence_label(line)
                    if original_label is not None:
                        rendered_labels.append({'source_line_index_zero_based': i,
                                                'exact_source_label': original_label,
                                                'rendered_label_with_display_breaks': line.split(':', 1)[1].strip(),
                                                'visible_text_after_break_removal': ' '.join(line.split(':', 1)[1].replace('<br/>', ' ').split()),
                                                'word_sequence_validation': ('PASS' if ' '.join(original_label.split()) == ' '.join(line.split(':', 1)[1].replace('<br/>', ' ').split()) else 'FAIL')})
                lines.append(line)
            rendered_lines = []
            for line in lines:
                rendered_lines.append(line)
            code = init_directive(kind, extra_x_margin=(source_id == '0b0e76bfeca07fcd2864142a' and panel_index == 1)) + '\n' + '\n'.join(rendered_lines)
            variant = f'{source_id}-panel-{panel_index:02d}'
            svg = OUT / f'{variant}.svg'
            jobs.append({'id': variant, 'kind': 'mermaid', 'code': code,
                         'outfile': str(svg), 'source': source_path,
                         'volume': row['source_rows'][0]['volume']})
            panel_records.append({
                'panel': panel_index, 'variant_id': variant,
                'source_line_indices_zero_based': selected,
                'source_lines_exact': [source_lines[i] for i in selected],
                'repeated_participant_line_indices_zero_based': repeated,
                'repeated_participant_lines_exact': [source_lines[i] for i in repeated],
                'omitted_unreferenced_source_participants': [source_lines[i] for i in participants if i not in repeated],
                'display_only_label_wraps': rendered_labels,
                'synthetic_content': synthetic_content, 'rendered_code_sha256': sha(code),
                'svg': str(svg), 'pdf': str(svg.with_suffix('.pdf')),
                'png': str(svg.with_suffix('.png')),
            })

        # Explicitly identify graph nodes shared by multiple panels. The
        # Mermaid edge lines remain exact, so these aliases are the source IDs
        # that connect neighboring panels in the source graph.
        node_to_panels: dict[str, set[int]] = {}
        node_label: dict[str, str] = {}
        for pi, group in enumerate(plan, 1):
            for line_index in group:
                line = source_lines[line_index]
                for node_id, label in re.findall(r'\b([A-Za-z][A-Za-z0-9_]*)\s*\["([^"\n]*)"\]', line):
                    node_to_panels.setdefault(node_id, set()).add(pi)
                    node_label[node_id] = label
                for node_id, label in re.findall(r'\b([A-Za-z][A-Za-z0-9_]*)\s*\{"([^"\n]*)"\}', line):
                    node_to_panels.setdefault(node_id, set()).add(pi)
                    node_label[node_id] = label
                for node_id in re.findall(r'\b([A-Za-z][A-Za-z0-9_]*)\s*(?:-->|--\|)', line):
                    node_to_panels.setdefault(node_id, set()).add(pi)
                for node_id in re.findall(r'(?:-->|\|[^|]*\|)\s*([A-Za-z][A-Za-z0-9_]*)\b', line):
                    node_to_panels.setdefault(node_id, set()).add(pi)
        if is_sequence:
            participant_labels = {}
            for line_index in participants:
                match = re.match(r'^\s*participant\s+([A-Za-z][A-Za-z0-9_]*)(?:\s+as\s+(.*))?$', source_lines[line_index])
                if match:
                    participant_labels[match.group(1)] = match.group(2) or match.group(1)
            for pi, record in enumerate(panel_records, 1):
                for line_index in record['repeated_participant_line_indices_zero_based']:
                    match = re.match(r'^\s*participant\s+([A-Za-z][A-Za-z0-9_]*)', source_lines[line_index])
                    if match:
                        node_to_panels.setdefault(match.group(1), set()).add(pi)
                        node_label[match.group(1)] = participant_labels[match.group(1)]
        aliases = [{'source_node_id': node_id, 'label': node_label.get(node_id),
                    'panels': sorted(panels),
                    'relationship': 'same source node ID reused across panels'}
                   for node_id, panels in sorted(node_to_panels.items()) if len(panels) > 1]
        source_edges = [source_lines[i] for group in plan for i in group]
        entries.append({
            'source_asset_id': source_id,
            'source_graph_sha256': sha(original),
            'expected_source_graph_sha256': row['source_code_sha256'][0],
            'source_markdown': source_path, 'source_code_exact': original,
            'source_kind': kind, 'panel_count': len(panel_records),
            'layout_transform': ({'source_direction': 'LR', 'panel_direction': 'TB',
                                  'source_nodes_edges_and_label_text_unchanged': True}
                                 if kind.startswith(('flowchart LR', 'graph LR')) else None),
            'partition_validation': ('PASS: every non-header source line appears across panels; all non-control source lines occur once and any control duplication is listed'
                                     if duplicates else 'PASS: every non-header source line appears exactly once across panels'),
            'source_control_line_duplicates': [{'zero_based_line_index': i, 'exact_source_line': source_lines[i],
                                                'panels': [pi for pi, group in enumerate(plan, 1) if i in group]}
                                               for i in duplicates],
            'source_edge_transition_or_message_lines_exact': source_edges,
            'cross_panel_node_aliases': aliases,
            'cross_panel_sequence_relations': ([
                {'from_panel': 1, 'to_panel': 2, 'source_message_order': 'new pipeline creation precedes successful publication'},
                {'from_panel': 2, 'to_panel': 3, 'shared_source_condition': source_lines[9], 'source_message_order': 'publication and command submission precede in-flight completion and retirement'},
                {'from_panel': 3, 'to_panel': 4, 'shared_source_condition': source_lines[9], 'branch_relation': source_lines[14], 'source_message_order': 'success path and failure path are alternative outcomes of the same condition'},
            ] if kind == 'sequenceDiagram' and source_id == '12f60b7ab1e672dd10535591' else []),
            'sequence_participant_declarations': [source_lines[i] for i in participants],
            'sequence_control_blocks_preserved_whole': kind == 'sequenceDiagram',
            'source_min_projected_font_pt_current_held': row['current_print_style_projection']['effective_minimum_explicit_font_pt'],
            'panels': panel_records,
        })

    JOBS.write_text(json.dumps({'schema': 'allmight-held-panels-review-jobs-v1',
                                'assets': jobs}, indent=2) + '\n')
    REPORT.write_text(json.dumps({'schema': 'allmight-held-panels-review-v1',
                                  'purpose': 'Tentative semantic panel recomposition review; not active print mapping.',
                                  'source_override_manifest_sha256': sha(OVERRIDES.read_text()),
                                  'renderer_jobs': str(JOBS),
                                  'source_panels': entries}, indent=2) + '\n')
    print(f'prepared {len(jobs)} tentative panels from {len(entries)} held source graphs')


if __name__ == '__main__':
    main()
