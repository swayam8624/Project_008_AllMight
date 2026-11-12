#!/usr/bin/env python3
"""Mechanical preservation audit for the current three canonical PROJECTALLMIGHT volumes.

The repository deliberately keeps only three reader-facing continuous-note files.
Older source volumes are recovered from git history for preservation checks; they
are not required to remain live files. This script therefore verifies the
*current* architecture instead of assuming that deleted pre-consolidation files
still exist.

This remains a mechanical preservation check, not a semantic mastery certificate.
"""
from __future__ import annotations

from pathlib import Path
import os
import subprocess

ROOT = Path(__file__).resolve().parents[1]
VOL = ROOT / "Continuous Notes"

FOUNDATIONS = VOL / "Computing Foundations - From Bits to C++ and Systems APIs.md"
GRAPHICS = VOL / "University - Worlds, Images and Inference - Graphics, Vision and Photography.md"
ENGINE = VOL / "Game Engine - From First Frame to Living Worlds.md"
CANONICAL = (FOUNDATIONS, GRAPHICS, ENGINE)

LAB = ROOT / "tools" / "labs" / "basis_measurement_lab.cpp"
BASE = "89c386f25b361685051bb035799014c8875487cd"

# Historical sources that were intentionally absorbed.  A source may be
# accepted in more than one destination because the final consolidation made
# some foundations available where they are first used.  At least one expected
# destination must preserve every original line in order.
LEGACY = {
    "Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing.md":
        (FOUNDATIONS, GRAPHICS),
    "Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md":
        (FOUNDATIONS, ENGINE),
    "Continuous Notes/XR - From Light to Perception - Seeing and Representing Reality.md":
        (GRAPHICS,),
    "Continuous Notes/Rigid Bodies - From Extended Motion to Contact Constraints.md":
        (ENGINE,),
    "Continuous Notes/Systems - From Cartridge Bytes to Reproducible State.md":
        (ENGINE,),
}


def read(path: Path) -> str:
    if not path.is_file():
        raise FileNotFoundError(f"missing canonical file: {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8")


def fence_count(content: str) -> int:
    return sum(line.startswith(chr(96) * 3) for line in content.splitlines())


def line_subsequence(earlier: str, current: str) -> bool:
    """Return True when every old line occurs later in the same order."""
    stream = iter(current.splitlines(keepends=True))
    for old_line in earlier.splitlines(keepends=True):
        for new_line in stream:
            if new_line == old_line:
                break
        else:
            return False
    return True


def git_show(ref_path: str) -> str | None:
    result = subprocess.run(
        ["git", "show", f"{BASE}:{ref_path}"],
        cwd=ROOT,
        capture_output=True,
        check=False,
    )
    if result.returncode:
        return None
    return result.stdout.decode("utf-8")


def main() -> None:
    volumes = {path: read(path) for path in CANONICAL}

    # Repository architecture: exactly the intended three reader-facing books.
    live_md = sorted(VOL.glob("*.md"))
    assert live_md == sorted(CANONICAL), (
        "Continuous Notes must contain exactly the three canonical master files; "
        f"found {[p.name for p in live_md]}"
    )

    for path, content in volumes.items():
        assert fence_count(content) % 2 == 0, f"unbalanced Markdown fences: {path.name}"

    preserved = 0
    unavailable = []
    for historical_path, destinations in LEGACY.items():
        original = git_show(historical_path)
        if original is None:
            unavailable.append(historical_path)
            continue

        exact = [p for p in destinations if original in volumes[p]]
        additive = [p for p in destinations if line_subsequence(original, volumes[p])]
        assert exact or additive, (
            "Historical source body is no longer preserved in an expected canonical "
            f"destination: {historical_path}"
        )
        chosen = (exact or additive)[0]
        mode = "EXACT" if exact else "ADDITIVE"
        print(f"{mode}: {historical_path} -> {chosen.name}")
        preserved += 1

    if unavailable:
        message = "baseline history unavailable for: " + ", ".join(unavailable)
        if os.environ.get("CI"):
            raise RuntimeError(message)
        print("WARNING:", message)

    # The executable basis witness must remain embedded somewhere in the
    # reader-facing corpus, but the checker does not prescribe which book owns it.
    basis = read(LAB).rstrip()
    owners = [path for path, content in volumes.items() if basis in content]
    assert owners, "Standalone basis lab is not embedded in any canonical volume"
    print("BASIS LAB:", ", ".join(p.name for p in owners))

    print("PASS: exactly three canonical continuous volumes")
    print(f"PASS: historical source bodies preserved from baseline: {preserved}/{len(LEGACY)}")
    print("PASS: basis witness embedded and Markdown fence parity valid")
    print(
        "LIMIT: this proves preservation and selected source agreement only; "
        "semantic completeness, hardware behavior, production-project parity, "
        "and mastery of all 3,000 atomic parts require separate evidence."
    )


if __name__ == "__main__":
    main()
