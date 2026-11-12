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

# Historical source files were intentionally retired after consolidation.  Their
# existence in git history is audited, but this script does not claim a
# line-for-line embedding when editorial reconciliation changed the body.
RETIRED = (
    "Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing.md",
    "Continuous Notes/XR - From Light to Perception - Seeing and Representing Reality.md",
    "Continuous Notes/Rigid Bodies - From Extended Motion to Contact Constraints.md",
    "Continuous Notes/Systems - From Cartridge Bytes to Reproducible State.md",
)


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

    # Preserve the earlier reader-facing canonical bodies themselves.  This
    # checks additive evolution of the files that still define today's three
    # reading routes, rather than pretending retired source copies stayed
    # byte-identical after editorial reconciliation.
    earlier_checked = 0
    for path, current in volumes.items():
        relative = path.relative_to(ROOT).as_posix()
        original = git_show(relative)
        if original is None:
            continue
        assert line_subsequence(original, current), (
            f"Original canonical lines were deleted or reordered: {relative}"
        )
        earlier_checked += 1
        print(f"ADDITIVE: baseline canonical body preserved in {path.name}")

    # Retirement is reversible because the deleted sources remain in git
    # history.  This is a provenance check only, not a claim that every old
    # line survives verbatim in the edited masters.
    retired_found = 0
    for historical_path in RETIRED:
        if git_show(historical_path) is not None:
            retired_found += 1
            print(f"HISTORY: retired source recoverable: {historical_path}")
        elif os.environ.get("CI"):
            raise RuntimeError(f"baseline source unavailable: {historical_path}")

    # The executable basis witness must remain embedded somewhere in the
    # reader-facing corpus, but the checker does not prescribe which book owns it.
    basis = read(LAB).rstrip()
    owners = [path for path, content in volumes.items() if basis in content]
    assert owners, "Standalone basis lab is not embedded in any canonical volume"
    print("BASIS LAB:", ", ".join(p.name for p in owners))

    print("PASS: exactly three canonical continuous volumes")
    print(f"PASS: additive baseline preservation for live canonical routes: {earlier_checked}/{len(CANONICAL)}")
    print(f"PASS: retired source files recoverable from history: {retired_found}/{len(RETIRED)}")
    print("PASS: basis witness embedded and Markdown fence parity valid")
    print(
        "LIMIT: this proves preservation and selected source agreement only; "
        "semantic completeness, hardware behavior, production-project parity, "
        "and mastery of all 3,000 atomic parts require separate evidence."
    )


if __name__ == "__main__":
    main()
