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


def verify(root):
    notes = root / 'Notes'
    manifest = json.loads((notes / 'Chapter Edition/MANIFEST.json').read_text(encoding='utf-8'))
    if manifest['schema'] != 1:
        raise ValueError('unsupported manifest schema')
    failures, masters, spans, seen = [], 0, 0, set()
    for book in manifest['books']:
        path = notes / book['source']
        data = path.read_bytes()
        masters += 1
        if len(data) != book['bytes'] or sha(data) != book['sha256']:
            failures.append(f'master byte/hash mismatch: {path}')
        # Keep line terminators. Manifest ranges are one-based, end-exclusive.
        lines = data.splitlines(keepends=True)
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
            span = b''.join(lines[start-1:end-1])
            if sha(span) != chapter['source_span_sha256']:
                failures.append(f'original span mismatch: {p} ({start}:{end})')
            if sha((notes / p).read_bytes()) != chapter['file_sha256']:
                failures.append(f'derived file mismatch: {p}')
            spans += 1
    expected = {(family, number) for family, numbers in SELECT.items() for number in numbers}
    if seen != expected:
        failures.append(f'missing mapped chapters: {sorted(expected-seen)}')
    if masters != 3 or spans != len(expected):
        failures.append(f'expected 3 masters and {len(expected)} spans; got {masters}, {spans}')
    if failures:
        raise ValueError('\n'.join(failures))
    print(f'P36 PASS {masters} master hashes; {spans} original span and derived-file hashes')
    return masters, spans


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[5])
    args = parser.parse_args()
    verify(args.root.resolve())
