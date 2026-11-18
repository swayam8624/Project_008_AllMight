#!/usr/bin/env python3
"""Apply project-based lesson inserts without shortening canonical manuscripts.

The script does NOT regenerate chapter copies or compile/export LaTeX.
It makes *only* exact insertions; it never edits or deletes existing prose.

From repository root:
    python3 tools/apply_cpp23_project_inserts.py --check
    python3 tools/apply_cpp23_project_inserts.py --apply
    python3 tools/apply_cpp23_project_inserts.py --check

A changed already-inserted section is treated as a conflict and never silently
replaced: reconcile it manually against the canonical book's current text.
"""

from __future__ import annotations

import argparse
from dataclasses import dataclass
import hashlib
import os
from pathlib import Path
import stat
import sys

ROOT = Path(__file__).resolve().parent.parent
MASTER = ROOT / "Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md"


@dataclass(frozen=True)
class Insert:
    name: str
    source: str
    anchor: bytes


# The original 5,000+ page collection is never truncated, summarized or
# regenerated here. Each exact heading is an existing conceptual boundary.
INSERTS = (
    Insert(
        "NES-01",
        "Supplementary/C++23 Project Labs/NES-01 - Cartridge Bytes and Trusted Layout.md",
        b"\n## Arithmetic becomes a policy\n",
    ),
    Insert(
        "NES-02",
        "Supplementary/C++23 Project Labs/NES-02 - Owned NROM and Memory Bus.md",
        b"\n## Choose a type for its promise, not its familiar size\n",
    ),
)


def sha(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def get_block(spec: Insert) -> tuple[bytes, bytes, bytes]:
    path = ROOT / spec.source
    lesson = path.read_bytes()
    lesson.decode("utf-8")
    if not lesson.startswith(("### " + spec.name + " ").encode("utf-8")):
        raise ValueError(f"{spec.name}: lesson heading mismatch in {spec.source}")
    begin = ("<!-- ALLMIGHT-CPP23-" + spec.name + "-ADDITIVE-START -->\n").encode("ascii")
    end = ("<!-- ALLMIGHT-CPP23-" + spec.name + "-ADDITIVE-END -->\n").encode("ascii")
    if begin in lesson or end in lesson:
        raise ValueError(f"{spec.name}: unexpected insertion marker in lesson")
    return begin, end, b"\n" + begin + lesson.rstrip(b"\n") + b"\n" + end


def process(original: bytes) -> tuple[bytes, list[str]]:
    staged = original
    receipt: list[str] = []

    for spec in INSERTS:
        begin, end, block = get_block(spec)

        # Detect altered, duplicated, or partial prior insertions and refuse
        # to silently edit already-authorized book text.
        if begin in staged or end in staged:
            if staged.count(begin) != 1 or staged.count(end) != 1 or staged.count(block) != 1:
                raise ValueError(f"{spec.name}: previous insertion is altered or partial")
            receipt.append(f"{spec.name}: present, exact, untouched")
            continue

        if staged.count(spec.anchor) != 1:
            raise ValueError(
                f"{spec.name}: expected exactly one canonical anchor; "
                f"found {staged.count(spec.anchor)}"
            )

        prior = staged
        staged = staged.replace(spec.anchor, block + spec.anchor, 1)

        # Strict preservation: removing the one inserted block must recover
        # the entire prior manuscript byte for byte.
        if staged.replace(block, b"", 1) != prior:
            raise AssertionError(f"{spec.name}: byte-preservation invariant failed")
        receipt.append(f"{spec.name}: will insert {len(block):,} bytes")

    return staged, receipt


def run(apply: bool) -> int:
    if not MASTER.is_file():
        raise FileNotFoundError(f"Canonical master missing: {MASTER}")
    original = MASTER.read_bytes()
    original.decode("utf-8")
    revised, receipt = process(original)

    for line in receipt:
        print(line)
    print(f"Canonical master: {MASTER.relative_to(ROOT)}")
    print(f"Before: {len(original):,} bytes; SHA-256 {sha(original)}")
    print(f"After:  {len(revised):,} bytes; SHA-256 {sha(revised)}")

    if not apply:
        print("Dry run: no files changed. Use --apply for additive insertion.")
        return 0

    if original == revised:
        print("All revisions already present; no changes made.")
        return 0

    temporary = MASTER.with_name(MASTER.name + ".cpp23-insert.tmp")
    if temporary.exists():
        raise FileExistsError(f"Unexpected temporary file: {temporary}")
    try:
        with temporary.open("xb") as out:
            out.write(revised)
            out.flush()
            os.fsync(out.fileno())
        temporary.chmod(stat.S_IMODE(MASTER.stat().st_mode))
        os.replace(temporary, MASTER)
    finally:
        if temporary.exists():
            temporary.unlink()

    if MASTER.read_bytes() != revised:
        raise AssertionError("Post-write byte verification failed")
    print("Inserted only the missing project sections. No original bytes removed.")
    print("Review: git diff --check; git diff -- 'Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md'")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--check", action="store_true", help="Dry-run the insertions, no changes")
    mode.add_argument("--apply", action="store_true", help="Apply strictly additive insertions")
    options = parser.parse_args()
    try:
        return run(apply=options.apply)
    except (OSError, UnicodeError, ValueError, AssertionError) as exc:
        print(f"Refused to change canonical source: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
