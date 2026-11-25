#!/usr/bin/env python3
"""Book 03: verify original masters, selected span hashes and derived file bytes.

Run from a checkout; uses the existing Chapter Edition manifest, never writes sources.
The verified span is the original master slice, not a claim of reader integration.
"""
import argparse
import hashlib
import json
from pathlib import Path

SELECT = {
    '01 - Computing Foundations': set(range(32, 39)),
    '02 - Graphics Vision and XR': {1, 2, 3, 4, 13, 15, 19, 23, 24, 27, 30, 32, 33, 41},
}


def sha(data):
    return hashlib.sha256(data).hexdigest()


def verify_pinned(root):
    baseline = root / 'Notes/Book Production/Seven-Book Curriculum/Book03/SOURCE_CUSTODY_BASELINE.json'
    receipt = json.loads(baseline.read_text(encoding='utf-8'))
    if receipt['schema'] != 1 or len(receipt['files']) != 25:
        raise ValueError('invalid baseline receipt')
    for name, expected in receipt['files'].items():
        data = (root / name).read_bytes()
        header = f'blob {len(data)}\0'.encode('ascii')
        actual = hashlib.sha1(header + data).hexdigest()
        if actual != expected:
            raise ValueError(f'pinned Git blob mismatch: {name}: {actual} != {expected}')
    print(f"P37 PASS 25 exact Git blob identities from {receipt['baseline_commit']}")


def verify(root, report_historical_drift=False):
    verify_pinned(root)
    notes = root / 'Notes'
    manifest = json.loads((notes / 'Chapter Edition/MANIFEST.json').read_text(encoding='utf-8'))
    if manifest['schema'] != 1:
        raise ValueError('unsupported manifest schema')
    failures, masters, spans, seen = [], 0, 0, set()
    for book in manifest['books']:
        path = notes / book['source']
        # Match build_chapter_edition.py: universal newline text decoding,
        # then UTF-8 encoding for the historical manifest hashes.
        source = path.read_text(encoding='utf-8')
        data = source.encode('utf-8')
        masters += 1
        if len(data) != book['bytes'] or sha(data) != book['sha256']:
            failures.append(f'master byte/hash mismatch: {path}')
        # Keep line terminators. Manifest ranges are one-based, end-exclusive.
        lines = source.splitlines(keepends=True)
        for chapter in book['chapters']:
            p = Path(chapter['path'])
            family = p.parent.name
            if family not in SELECT:
                continue
            number = int(p.name.split(' - ', 1)[0])
            if number not in SELECT[family]:
                continue
            key = (family, number)
            if key in seen:
                failures.append(f'duplicate mapped chapter: {p}')
            seen.add(key)
            start, end = chapter['start_line'], chapter['end_line_exclusive']
            if not 1 <= start < end <= len(lines) + 1:
                failures.append(f'invalid master span: {p}')
                continue
            span = ''.join(lines[start-1:end-1]).encode('utf-8')
            if sha(span) != chapter['source_span_sha256']:
                failures.append(f'original span mismatch: {p} ({start}:{end})')
            if sha((notes / p).read_text(encoding='utf-8').encode('utf-8')) != chapter['file_sha256']:
                failures.append(f'derived file mismatch: {p}')
            spans += 1
    expected = {(family, number) for family, numbers in SELECT.items() for number in numbers}
    if seen != expected:
        failures.append(f'missing mapped chapters: {sorted(expected-seen)}')
    if masters != 3 or spans != len(expected):
        failures.append(f'expected 3 masters and {len(expected)} spans; got {masters}, {spans}')
    if failures:
        if not report_historical_drift:
            raise ValueError('\n'.join(failures))
        print(f'HISTORICAL MANIFEST NOT RECONCILED: {len(failures)} discrepancies')
        for failure in failures:
            print(f'::warning::{failure}')
        return masters, spans
    print(f'P37 PASS {masters} master hashes; {spans} original span and derived-file hashes')
    return masters, spans


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[5])
    parser.add_argument('--report-historical-drift', action='store_true',
                        help='report old manifest discrepancies without failing the pinned-byte gate')
    args = parser.parse_args()
    verify(args.root.resolve(), args.report_historical_drift)
