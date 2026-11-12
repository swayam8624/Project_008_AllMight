#!/usr/bin/env python3
"""Preservation audit for the three canonical PROJECTALLMIGHT volumes.

Retired source volumes are read from Git history rather than required to
remain in the working tree. This proves textual preservation only.
"""
from __future__ import annotations

from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
VOL = ROOT / "Continuous Notes"
FOUNDATIONS = VOL / "Computing Foundations - From Bits to C++ and Systems APIs.md"
GRAPHICS = VOL / "University - Worlds, Images and Inference - Graphics, Vision and Photography.md"
ENGINE = VOL / "Game Engine - From First Frame to Living Worlds.md"
LAB = ROOT / "tools" / "labs" / "basis_measurement_lab.cpp"
BASE = "89c386f25b361685051bb035799014c8875487cd"

RETIRED = {
    "Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing.md": FOUNDATIONS,
    "Continuous Notes/XR - From Light to Perception - Seeing and Representing Reality.md": GRAPHICS,
    "Continuous Notes/Rigid Bodies - From Extended Motion to Contact Constraints.md": ENGINE,
    "Continuous Notes/Systems - From Cartridge Bytes to Reproducible State.md": ENGINE,
}
BASE_CANONICAL = {
    "Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography.md": GRAPHICS,
    "Continuous Notes/Game Engine - From First Frame to Living Worlds.md": ENGINE,
}

def read(path: Path) -> str:
    if not path.is_file():
        raise RuntimeError(f"missing canonical file: {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8")

def git_show(ref: str, path: str) -> str:
    result = subprocess.run(["git", "show", f"{ref}:{path}"], cwd=ROOT, capture_output=True, check=False)
    if result.returncode:
        raise RuntimeError(f"cannot read historical source {ref}:{path}")
    return result.stdout.decode("utf-8")

def fence_count(content: str) -> int:
    return sum(line.startswith("```") for line in content.splitlines())

def line_subsequence(earlier: str, current: str) -> bool:
    stream = iter(current.splitlines(keepends=True))
    for old_line in earlier.splitlines(keepends=True):
        for new_line in stream:
            if new_line == old_line:
                break
        else:
            return False
    return True

def prove_preserved(source: str, destination: str, label: str) -> None:
    if source in destination:
        print(f"EXACT: {label}")
    elif line_subsequence(source, destination):
        print(f"ADDITIVE: {label}")
    else:
        raise AssertionError(f"deleted/reordered historical lines: {label}")

def main() -> None:
    foundations = read(FOUNDATIONS)
    graphics = read(GRAPHICS)
    engine = read(ENGINE)
    for name, text in (("foundations", foundations), ("graphics", graphics), ("engine", engine)):
        fences = fence_count(text)
        if fences % 2:
            raise AssertionError(f"unbalanced Markdown fences in {name}: {fences}")

    checked = 0
    for historical_path, destination_path in {**RETIRED, **BASE_CANONICAL}.items():
        prove_preserved(git_show(BASE, historical_path), read(destination_path), historical_path)
        checked += 1

    basis = read(LAB).rstrip()
    if basis not in foundations:
        raise AssertionError("basis_measurement_lab.cpp is not embedded in Computing Foundations")

    print(f"PASS: preserved historical source bodies checked: {checked}")
    print("PASS: current canonical volumes have balanced code fences")
    print("PASS: basis witness is embedded in current Foundations")
    print("LIMIT: mechanical preservation is not semantic mastery proof")

if __name__ == "__main__":
    try:
        main()
    except (OSError, UnicodeError, RuntimeError, AssertionError) as exc:
        print(f"three-volume preservation error: {exc}")
        raise SystemExit(1)
