#!/usr/bin/env python3
"""Verify live three-volume inventory, inline witnesses and lossless XR relocation.

Structural and source-preservation check only. It does NOT prove source coverage,
semantics, device correctness, or reconstruction of the original 3,000 parts.
"""
from __future__ import annotations
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
NOTES = ROOT / "Continuous Notes"
FOUNDATIONS = NOTES / "Computing Foundations - From Bits to C++ and Systems APIs.md"
GRAPHICS = NOTES / "University - Worlds, Images and Inference - Graphics, Vision and Photography.md"
ENGINE = NOTES / "Game Engine - From First Frame to Living Worlds.md"
BASE = "5be7d79c337c3d359569c9531621dd386fa0b7f0"
START = "## A correct image must survive the viewer — perceptual rendering and XR"
END = "## Mathematical language: points, vectors, and derivatives"
LABS = {
    FOUNDATIONS: ("basis_measurement_lab.cpp", "math_contact_toi_lab.cpp"),
    GRAPHICS: ("xr_planar_vio_ekf_lab.py",),
    ENGINE: ("render_graph_contract_lab.cpp", "resource_state_plan_lab.cpp"),
}


def verify(condition: bool, message: str) -> None:
    if not condition:
        raise RuntimeError(message)


def subsequence(old: str, new: str) -> bool:
    """Verify all old lines still exist in the same order after insertions."""
    cursor = iter(new.splitlines(keepends=True))
    for wanted in old.splitlines(keepends=True):
        for candidate in cursor:
            if candidate == wanted:
                break
        else:
            return False
    return True


def baseline_text(path: Path) -> str:
    relative = path.relative_to(ROOT).as_posix()
    proc = subprocess.run(
        ["git", "show", f"{BASE}:{relative}"],
        cwd=ROOT, capture_output=True, check=False
    )
    verify(proc.returncode == 0,
           f"Cannot read pinned baseline {BASE}:{relative}; fetch full history")
    return proc.stdout.decode("utf-8")


def display_check(path: Path, content: str) -> None:
    lines = content.splitlines()
    backtick = chr(96) * 3
    fences = sum(line.startswith(backtick) or line.startswith("~~~") for line in lines)
    maths = sum(line.strip() == "$$" for line in lines)
    verify(fences % 2 == 0, f"{path.name}: odd code fence count {fences}")
    verify(maths % 2 == 0, f"{path.name}: odd display-math fence count {maths}")
    print(f"FORMAT {path.name}: {fences} code boundaries, {maths} math boundaries")


def main() -> None:
    current = {p: p.read_text(encoding="utf-8") for p in LABS}
    discovered = set(NOTES.glob("*.md"))
    verify(discovered == set(LABS),
           "Live physical three-volume inventory changed; adjust routing/checker")
    for path, body in current.items():
        display_check(path, body)
        previous = baseline_text(path)
        if path == GRAPHICS:
            verify(previous.count(START) == 1 and previous.count(END) == 1,
                   "Baseline XR source anchors are not unique")
            start, end = previous.index(START), previous.index(END)
            verify(start < end, "Old XR source seam order changed")
            original_xr = previous[start:end]
            verify(body.count(original_xr) == 1,
                   "Losslessly relocated XR foundation is missing or changed")
            verify(body.index(END) < body.index(START),
                   "XR block moved back before mathematics prerequisites")
            verify(subsequence(previous.replace(original_xr, "", 1),
                               body.replace(original_xr, "", 1)),
                   "Graphics lost or reordered baseline text outside relocated XR")
            print(f"RELOCATED XR: {len(original_xr)} original characters preserved")
        else:
            verify(subsequence(previous, body),
                   f"{path.name}: original source lines removed or reordered")
        print(f"BASELINE PRESERVED: {path.name}")
        for filename in LABS[path]:
            source = (ROOT / "tools" / "labs" / filename).read_text("utf-8").rstrip()
            verify(source in body, f"{filename} differs from inline complete code")
            print(f"INLINE SOURCE: {filename} matches")

    print("PASS: actual three-volume inventory, inline labs, formatting, and "
          "lossless source preservation with XR relocation")
    print("LIMIT: this does not prove source exhaustion, S4 for every part, "
          "GPU driver validation, or current Kairo/Maveb integration")


if __name__ == "__main__":
    try:
        main()
    except (OSError, UnicodeError, RuntimeError) as problem:
        print(f"FAIL: {problem}", file=sys.stderr)
        raise SystemExit(1)
