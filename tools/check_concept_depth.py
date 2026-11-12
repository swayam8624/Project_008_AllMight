#!/usr/bin/env python3
"""Guard against concept-card regression.

This is a structural floor, not a semantic mastery certificate. A note can be
long and still poor; the real mastery ledger/evidence tiers remain authoritative.
"""
from __future__ import annotations

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CONCEPTS = ROOT / "Concepts"
MIN_BYTES = 900

def main() -> None:
    failures: list[str] = []
    notes = sorted(CONCEPTS.glob("*.md"))
    if not notes:
        raise SystemExit("no concept notes found")

    for path in notes:
        raw = path.read_bytes()
        text = raw.decode("utf-8")
        if len(raw) < MIN_BYTES and "depth_floor: exempt" not in text:
            failures.append(f"{path.name}: only {len(raw)} bytes (< {MIN_BYTES})")
        lines = text.splitlines()
        index = 0
        if lines and lines[0].strip() == "---":
            index = 1
            while index < len(lines) and lines[index].strip() != "---":
                index += 1
            index += 1
        while index < len(lines) and not lines[index].strip():
            index += 1
        if index >= len(lines) or not lines[index].startswith("# "):
            failures.append(f"{path.name}: missing top-level concept heading after optional frontmatter")
        for ch in text:
            code = ord(ch)
            if code < 32 and ch not in "\n\t":
                failures.append(f"{path.name}: contains control character U+{code:04X}")
                break

    if failures:
        print("Concept depth guard: FAIL")
        for failure in failures:
            print(" -", failure)
        raise SystemExit(1)

    print(f"Concept depth guard: PASS — {len(notes)} concept notes, none below {MIN_BYTES} bytes")
    print("LIMIT: byte count is only a regression floor; semantic depth is tracked separately")

if __name__ == "__main__":
    main()
