#!/usr/bin/env python3
"""Mechanical preservation audit for three canonical PROJECTALLMIGHT volumes."""
from __future__ import annotations
from pathlib import Path
import os
import subprocess

ROOT = Path(__file__).resolve().parents[1]
VOL = ROOT / "Continuous Notes"
MATH = VOL / "00 - From Vectors to Images - Integrated Mathematics for Visual Computing.md"
GRAPHICS = VOL / "University - Worlds, Images and Inference - Graphics, Vision and Photography.md"
ENGINE = VOL / "Game Engine - From First Frame to Living Worlds.md"
FOUNDATIONS = VOL / "Computing Foundations - From Bits to C++ and Systems APIs.md"
XR = VOL / "XR - From Light to Perception - Seeing and Representing Reality.md"
RIGID = VOL / "Rigid Bodies - From Extended Motion to Contact Constraints.md"
SYSTEMS = VOL / "Systems - From Cartridge Bytes to Reproducible State.md"
LAB = ROOT / "tools" / "labs" / "basis_measurement_lab.cpp"
BASE = "89c386f25b361685051bb035799014c8875487cd"

def read(path: Path) -> str:
    return path.read_text(encoding="utf-8")

def fence_count(content: str) -> int:
    return sum(line.startswith(chr(96) * 3) for line in content.splitlines())

def line_subsequence(earlier: str, current: str) -> bool:
    """All old lines remain in order even if new paragraphs are inserted."""
    stream = iter(current.splitlines(keepends=True))
    for old_line in earlier.splitlines(keepends=True):
        for new_line in stream:
            if new_line == old_line:
                break
        else:
            return False
    return True

def main() -> None:
    math, graphics, engine = map(read, (MATH, GRAPHICS, ENGINE))
    foundations, xr, rigid, systems = map(read, (FOUNDATIONS, XR, RIGID, SYSTEMS))
    a = foundations.index("## Make a shader record an explicit binary contract")
    b = foundations.index("## From generated GPU work to safe explicit execution")
    assert 0 < a < b
    front, shader, backend = foundations[:a], foundations[a:b], foundations[b:]
    assert front + shader + backend == foundations
    assert all(fence_count(x) % 2 == 0 for x in (front, shader, backend))
    for source, destination, name in [
        (front, engine, "C++/CPU engine foundations"),
        (backend, engine, "Vulkan/Metal engine backend"),
        (shader, graphics, "GPU shader and execution"),
        (xr, graphics, "XR and human perception"),
        (rigid, engine, "rigid-body bridge"),
        (systems, engine, "emulator and low-level systems"),
    ]:
        # First check exact inclusion for the initial migration; future in-place
        # explanatory paragraphs are allowed between original lines, provided
        # no original content or order is lost.
        if source in destination:
            print(f"EXACT: {name}")
        else:
            assert line_subsequence(source, destination), (
                f"Deleted or rewritten original source lines: {name}"
            )
            print(f"ADDITIVE: original lines preserved in order for {name}")
    assert read(LAB).rstrip() in math, "Standalone basis lab differs from inline math code"
    assert all(fence_count(x) % 2 == 0 for x in (math, graphics, engine))

    earlier_checked = 0
    for path, current in ((MATH, math), (GRAPHICS, graphics), (ENGINE, engine)):
        relative = path.relative_to(ROOT).as_posix()
        result = subprocess.run(["git", "show", f"{BASE}:{relative}"],
                                cwd=ROOT, capture_output=True, check=False)
        if result.returncode:
            print(f"NOT CHECKED: baseline history unavailable for {relative}")
            continue
        original = result.stdout.decode("utf-8")
        assert line_subsequence(original, current), f"Original lines removed: {relative}"
        earlier_checked += 1

    print("PASS: all former source bodies in three canonical volumes")
    print("PASS: inline C++ laboratory, split boundaries, Markdown fence parity")
    print(f"Original canonical books preserved by ordered-line audit: {earlier_checked}/3")
    if earlier_checked != 3:
        if os.environ.get("CI"):
            raise RuntimeError("CI must fetch full baseline history for the ordered-line audit")
        print("WARNING: historical audit needs complete git history")
    print("LIMIT: conceptual coverage, hardware behavior, sources and all 3,000 parts need more review")

if __name__ == "__main__":
    main()
