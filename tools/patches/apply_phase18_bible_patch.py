#!/usr/bin/env python3
"""Idempotently integrate the bounded Phase 18 reconstruction into Engine.

This patch owns only the Engine note. It does not normalize unrelated source
blocks or edit Graphics/checker files whose source blocks may be absent.
"""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
ENGINE = ROOT / "Continuous Notes" / "Game Engine - From First Frame to Living Worlds.md"
FRAGMENT = ROOT / "tools" / "patches" / "phase18_coupled_contact_fragment.md"
LAB = ROOT / "tools" / "labs" / "two_point_contact_block_lab.cpp"
OLD = "That closes the **entire 18-pass run**."
HEADING = "## From one collision impulse to a coupled contact problem — reconstructed Phase 18 bridge"


def main() -> None:
    engine = ENGINE.read_text(encoding="utf-8")
    fragment = FRAGMENT.read_text(encoding="utf-8").rstrip()
    lab = LAB.read_text(encoding="utf-8").rstrip()
    if engine.count(OLD) > 1:
        raise RuntimeError("historical over-closure sentence is not unique")
    if fragment.count("{{TWO_POINT_CONTACT_BLOCK_LAB}}") != 1:
        raise RuntimeError("Phase-18 fragment must contain exactly one lab placeholder")
    replacement = fragment.replace("{{TWO_POINT_CONTACT_BLOCK_LAB}}", lab)
    if HEADING not in engine:
        if engine.count(OLD) != 1:
            raise RuntimeError("need the unique historical closure marker for insertion")
        engine = engine.replace(OLD, replacement, 1)
    else:
        # Refresh only this owned reconstruction span so source comments stay in sync.
        start = engine.rfind("That closes the original rigid-body mechanics reconstruction", 0, engine.index(HEADING))
        end = engine.index("### An animation library is one part of an authoring workflow", engine.index(HEADING))
        if start < 0:
            raise RuntimeError("cannot locate the Phase-18 reconstruction span")
        engine = engine[:start] + replacement + "\n\n" + engine[end:]
    if OLD in engine:
        engine = engine.replace(OLD, "That closes the original rigid-body mechanics reconstruction through Part 1700.", 1)
    if engine.count(HEADING) != 1 or lab not in engine or OLD in engine:
        raise RuntimeError("Phase-18 Engine reconstruction did not validate")
    ENGINE.write_text(engine, encoding="utf-8")
    print("Phase-18 Engine reconstruction: PASS")


if __name__ == "__main__":
    main()
