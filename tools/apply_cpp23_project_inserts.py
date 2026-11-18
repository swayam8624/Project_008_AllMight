#!/usr/bin/env python3
"""Surgically insert verified C++23 project prose into canonical manuscripts.

This tool never regenerates a book, deletes old paragraphs, compiles LaTeX,
or edits derived Chapter Edition files. The default action is a dry run.

Example from repository root:
    python3 tools/apply_cpp23_project_inserts.py --check
    python3 tools/apply_cpp23_project_inserts.py --apply
    python3 tools/apply_cpp23_project_inserts.py --check

Review the resulting diff before committing it.
"""

from __future__ import annotations

import argparse
import hashlib
import os
from pathlib import Path
import stat
import sys

ROOT = Path(__file__).resolve().parent.parent
MASTER = ROOT / "Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md"
ADDITION = ROOT / "Supplementary/C++23 Project Labs/NES-01 - Cartridge Bytes and Trusted Layout.md"

# Put the milestone immediately after the existing bit-field exercises.
# Do NOT replace the existing discussion of bit fields or arithmetic.
NEXT_HEADING = b"\n## Arithmetic becomes a policy\n"
START = b"<!-- ALLMIGHT-CPP23-NES01-ADDITIVE-START -->\n"
END = b"<!-- ALLMIGHT-CPP23-NES01-ADDITIVE-END -->\n"


def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def planned_insert() -> bytes:
    lesson = ADDITION.read_bytes()
    if not lesson.startswith(b"### NES-01 "):
        raise ValueError("Addendum is not the expected NES-01 section.")
    if START in lesson or END in lesson:
        raise ValueError("The lesson must not contain insertion sentinels.")
    try:
        lesson.decode("utf-8")
    except UnicodeDecodeError as exc:
        raise ValueError("Lesson is not UTF-8.") from exc
    return b"\n" + START + lesson.rstrip(b"\n") + b"\n" + END


def handle(apply: bool) -> int:
    if not MASTER.is_file():
        raise FileNotFoundError(f"Canonical book source is missing: {MASTER}")
    if not ADDITION.is_file():
        raise FileNotFoundError(f"NES-01 addendum is missing: {ADDITION}")
    original = MASTER.read_bytes()
    original.decode("utf-8")  # fail closed; never silently re-encode source
    insertion = planned_insert()

    # The same original text may contain a link to this heading. Anchor on a
    # full, actual Markdown heading line rather than a substring.
    count = original.count(NEXT_HEADING)
    if count != 1:
        raise ValueError(f"Expected one exact insertion anchor, found {count}. Stop and inspect the source.")

    has_start = original.count(START)
    has_end = original.count(END)
    if has_start or has_end:
        if has_start != 1 or has_end != 1 or original.count(insertion) != 1:
            raise ValueError("Partial or altered NES-01 insertion detected; refusing to overwrite.")
        print(f"NES-01 already present and exact. File SHA-256: {sha256(original)}")
        return 0

    if not apply:
        print("Dry run: NES-01 is absent, the original heading exists once, and no source changes were made.")
        print(f"Canonical source: {MASTER.relative_to(ROOT)}")
        print(f"Original bytes: {len(original):,}; additional bytes: {len(insertion):,}")
        print(f"Original SHA-256: {sha256(original)}")
        print("Run with --apply to make one strictly additive insertion.")
        return 0

    # Preserve every original byte and its order. The only mutation is this
    # single insertion before the next pre-existing heading.
    updated = original.replace(NEXT_HEADING, insertion + NEXT_HEADING, 1)
    if updated.replace(insertion, b"", 1) != original:
        raise AssertionError("Source-preservation invariant failed; refusing write.")

    # Atomic replacement avoids leaving a half-written multi-megabyte book.
    # The original source is under Git, so changes remain inspectable/revertible.
    temp = MASTER.with_name(MASTER.name + ".cpp23-insert.tmp")
    if temp.exists():
        raise FileExistsError(f"Unexpected temporary file: {temp}")
    try:
        with temp.open("xb") as f:
            f.write(updated)
            f.flush()
            os.fsync(f.fileno())
        temp.chmod(stat.S_IMODE(MASTER.stat().st_mode))
        os.replace(temp, MASTER)
    finally:
        if temp.exists():
            temp.unlink()

    confirmed = MASTER.read_bytes()
    if confirmed != updated or confirmed.replace(insertion, b"", 1) != original:
        raise AssertionError("Post-write preservation check failed: inspect git diff immediately.")

    print("NES-01 inserted into canonical Computing Foundations source. No old content removed.")
    print(f"Original bytes: {len(original):,}; updated bytes: {len(updated):,}")
    print(f"Original SHA-256: {sha256(original)}")
    print(f"Updated SHA-256:  {sha256(updated)}")
    print("Review: git diff -- 'Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md'")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--check", action="store_true", help="Check the insertion and its anchor without editing anything.")
    mode.add_argument("--apply", action="store_true", help="Insert NES-01 once; preserve all original bytes.")
    options = parser.parse_args()
    try:
        return handle(apply=options.apply)
    except (OSError, UnicodeError, ValueError, AssertionError) as exc:
        print(f"Refused to modify source: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
