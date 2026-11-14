#!/usr/bin/env python3
"""Validate evidence for the book programme; discovery never implies completion."""
import argparse
import hashlib
import json
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
CONTENT_GATES = ('prerequisites', 'definitions_notation_units', 'derivation_or_algorithm',
                 'worked_trace', 'representation_or_model', 'failure_cases',
                 'alternatives_tradeoffs', 'exercise_with_solution', 'narrative_review')


def audit(root, curriculum, ledger):
    errors = []
    expected = {p['part']: p for p in curriculum['parts']}
    if sorted(expected) != list(range(1, 3001)) or len(curriculum['parts']) != 3000:
        errors.append('Canonical part set is not exactly 1–3000')
    rows = ledger['parts']
    if sorted(r['part'] for r in rows) != list(range(1, 3001)):
        errors.append('Coverage ledger must contain every original part exactly once')
    reviewed = complete = 0
    content = {}
    for row in rows:
        original = expected.get(row['part'])
        if original is None or row['title'] != original['title']:
            errors.append(f"Part {row['part']}: canonical title differs")
        if row['review_status'] == 'not_audited':
            if row.get('content_complete'):
                errors.append(f"Part {row['part']}: unaudited marked complete")
            continue
        reviewed += 1
        for destination in row.get('destinations', []):
            path = destination['file']
            if path not in content:
                file = root / path
                content[path] = file.read_text() if file.is_file() else ''
            quote = destination['exact_quote']
            if not quote or quote not in content[path]:
                errors.append(f"Part {row['part']}: destination quotation absent")
            if hashlib.sha256(quote.encode()).hexdigest() != destination['sha256']:
                errors.append(f"Part {row['part']}: destination hash differs")
        if row.get('content_complete'):
            complete += 1
            if not row.get('destinations'):
                errors.append(f"Part {row['part']}: no first-use destination")
            for gate in CONTENT_GATES:
                evidence = row.get('gates', {}).get(gate, {})
                if evidence.get('status') not in ('pass', 'not_applicable') or not evidence.get('reason'):
                    errors.append(f"Part {row['part']}: content gate {gate} has no signed reason")
            if not row.get('supervisor_review', {}).get('accepted'):
                errors.append(f"Part {row['part']}: no supervisor acceptance")
    return {'canonical_parts': len(expected), 'reviewed_parts': reviewed,
            'content_complete_parts': complete, 'parts_without_final_signoff': 3000-complete,
            'source_exhaustion_certified': False, 'errors': errors,
            'content_complete': complete == 3000 and not errors}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, default=ROOT)
    parser.add_argument('--require-complete', action='store_true')
    args = parser.parse_args()
    base = args.root / 'tools/book_completion'
    curriculum = json.loads((base / 'canonical_curriculum.json').read_text())
    ledger = json.loads((base / 'part_coverage.json').read_text())
    result = audit(args.root, curriculum, ledger)
    print(json.dumps(result, indent=2))
    return bool(result['errors']) or (args.require_complete and not result['content_complete'])


if __name__ == '__main__':
    sys.exit(main())
