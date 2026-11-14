#!/usr/bin/env python3
"""Protect the live three books and the approved November14 repair boundary.

Preservation is distinct from source exhaustion, semantic correctness and device
validation. The pinned input is the actual clean book state before this repair.
"""
from pathlib import Path
import subprocess
import sys
from note_integrity import (code_blocks, migrate_legacy_links,
                            normalized_baseline, ordered_lines_preserved)
from check_continuous_code_sync import EMBEDDED

ROOT = Path(__file__).resolve().parents[1]
NOTES = ROOT / "Continuous Notes"
BASE = "c142869a391c32d55068ddfbf4d4ac11ce52762c"
XR_BASE = "5be7d79c337c3d359569c9531621dd386fa0b7f0"
GRAPHICS = "University - Worlds, Images and Inference - Graphics, Vision and Photography.md"
ENGINE = "Game Engine - From First Frame to Living Worlds.md"
XR_START = "## A correct image must survive the viewer — perceptual rendering and XR"
MATH_START = "## Mathematical language: points, vectors, and derivatives"
BRIDGE = "## From one collision impulse to a coupled contact problem — reconstructed Phase 18 bridge"


def require(condition, message):
    if not condition:
        raise RuntimeError(message)


def historical(name, revision):
    command = ["git", "show", f"{revision}:Continuous Notes/{name}"]
    result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True)
    require(result.returncode == 0, f"Historical baseline unavailable: {revision}:{name}")
    return result.stdout


def main():
    require({p.name for p in NOTES.glob("*.md")} == set(EMBEDDED),
            "Physical canonical inventory differs from the three teaching books")
    for name in EMBEDDED:
        current = (NOTES / name).read_text(encoding="utf-8")
        code_blocks(current)  # Actual paired fence syntax, not prefix parity.
        previous = normalized_baseline(historical(name, BASE), name)
        if name == GRAPHICS:
            # The approved repair moves this one intact source packet after its
            # prerequisites. Compare the surrounding text independently so no
            # other reordering or deletion can hide behind that exception.
            begin, end = previous.index(XR_START), previous.index(MATH_START)
            require(begin < end, "Pinned input no longer has the original early XR packet")
            relocated = previous[begin:end]
            require(current.count(relocated) == 1, "The approved XR packet lost bytes or was duplicated")
            prior_rest = previous.replace(relocated, "", 1)
            current_rest = current.replace(relocated, "", 1)
            require(ordered_lines_preserved(prior_rest, current_rest),
                    "Graphics text outside the approved XR relocation was removed/reordered")
        else:
            require(ordered_lines_preserved(previous, current),
                    f"Original text removed/reordered beyond approved corrections: {name}")
        print(f"PRESERVED {name}: approved-route baseline lines retained")
        if name == ENGINE:
            require("That closes the **entire 18-pass run**." not in current,
                    "False Phase-18 source-closure claim remains")
            require(current.count(BRIDGE) == 1, "Phase-18 reconstruction bridge missing/duplicated")
        if name == GRAPHICS:
            old = historical(name, XR_BASE)
            begin, end = old.index(XR_START), old.index(MATH_START)
            require(begin < end, "Historical XR seam changed")
            xr = migrate_legacy_links(old[begin:end])
            require(current.count(XR_START) == 1, "XR reader route missing/duplicated")
            require(current.index(MATH_START) < current.index(XR_START),
                    "XR source relocated ahead of its mathematics prerequisites")
            require(ordered_lines_preserved(xr, current),
                    "Original XR teaching lost after its historical relocation")
            print("PRESERVED historical XR: original lines retained after prerequisite route")
    print("PASS: three canonical books; prior teaching preserved with documented link/fence/closure corrections")
    print("LIMIT: not source exhaustion, all-part mastery, native driver proof or production integration")


if __name__ == "__main__":
    try:
        main()
    except (OSError, UnicodeError, ValueError, RuntimeError) as problem:
        print(f"FAIL: {problem}", file=sys.stderr)
        sys.exit(1)
