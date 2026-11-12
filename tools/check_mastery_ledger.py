#!/usr/bin/env python3
"""Validate the PROJECTALLMIGHT M001-M180 mastery ledger."""
from __future__ import annotations
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
LEDGER = ROOT / "mastery" / "mastery_ledger.json"
ALLOWED = {"developed", "developing", "reference_implemented", "native_verified", "production_verified"}

def main() -> None:
    data = json.loads(LEDGER.read_text(encoding="utf-8"))
    entries = data["entries"]
    expected = [f"M{i:03d}" for i in range(1, 181)]
    actual = [entry["chunk"] for entry in entries]
    if actual != expected:
        raise SystemExit("mastery ledger must contain M001-M180 exactly once and in order")

    for i, entry in enumerate(entries, start=1):
        expected_phase = (i - 1) // 6 + 1
        if entry["phase"] != expected_phase:
            raise SystemExit(f"{entry['chunk']}: wrong phase")
        if entry["state"] not in ALLOWED:
            raise SystemExit(f"{entry['chunk']}: unsupported state {entry['state']}")
        evidence = entry.get("evidence", {})
        for required in ("canonical_teaching", "executable_reference", "native_backend", "production_repository", "source_verification"):
            if required not in evidence:
                raise SystemExit(f"{entry['chunk']}: missing evidence tier {required}")

    print("mastery_ledger: PASS — M001-M180 present with explicit evidence tiers")
    print("LIMIT: the checker validates accounting, not truth of evidence claims")

if __name__ == "__main__":
    main()
