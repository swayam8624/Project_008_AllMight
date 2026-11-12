#!/usr/bin/env python3
"""Mechanical source-to-continuous-volume contract for current PROJECTALLMIGHT.

Every declared standalone C++ witness must occur as a complete fenced block in
at least one of the three canonical reader-facing volumes. If consolidation
duplicates a witness, every detected copy must match the standalone source.

This deliberately avoids hard-coding deleted historical volume names.
"""
from __future__ import annotations

import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CONTINUOUS = ROOT / "Continuous Notes"
CANONICAL = (
    CONTINUOUS / "Computing Foundations - From Bits to C++ and Systems APIs.md",
    CONTINUOUS / "University - Worlds, Images and Inference - Graphics, Vision and Photography.md",
    CONTINUOUS / "Game Engine - From First Frame to Living Worlds.md",
)

# These are complete standalone witnesses that the teaching corpus promises to
# carry inline. Add new *_lab.cpp entries here only when the full source is
# intentionally embedded in a canonical narrative.
EMBEDDED = (
    "basis_measurement_lab.cpp",
    "math_contact_toi_lab.cpp",
    "disk_ccd_lab.cpp",
    "swept_broadphase_lab.cpp",
    "disk_event_pipeline_lab.cpp",
    "persistent_contact_lab.cpp",
    "contact_cache_lab.cpp",
    "disk_event_broadphase_differential_lab.cpp",
    "spring_generator_lab.cpp",
    "contact_penetration_refresh_lab.cpp",
    "render_graph_contract_lab.cpp",
    "engine_world_contract_lab.cpp",
    "animation_controller_contract_lab.cpp",
    "threaded_asset_publication_lab.cpp",
)


def load(path: Path) -> str:
    if not path.is_file():
        raise ValueError(f"missing required file: {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8")


def all_cpp_blocks(volume: str) -> list[str]:
    """Extract every complete cpp fenced block without guessing its identity."""
    marker = "```cpp\n"
    blocks: list[str] = []
    pos = 0
    while True:
        pos = volume.find(marker, pos)
        if pos == -1:
            break
        begin = pos + len(marker)
        end = volume.find("\n```", begin)
        if end == -1:
            raise ValueError("unclosed cpp fenced block")
        blocks.append(volume[begin:end].rstrip("\n"))
        pos = end + 4
    return blocks


def verify() -> int:
    failed = False
    volumes = {path: load(path) for path in CANONICAL}

    for path, text in volumes.items():
        fences = sum(line.startswith("```") for line in text.splitlines())
        math = sum(line.strip() == "$$" for line in text.splitlines())
        if fences % 2 or math % 2:
            print(f"FAIL {path.name}: unbalanced fences={fences} math={math}")
            failed = True
        else:
            print(f"FORMAT {path.name}: fences={fences}, display-math boundaries={math}")

    parsed: dict[Path, list[str]] = {}
    for volume_path, volume in volumes.items():
        try:
            parsed[volume_path] = all_cpp_blocks(volume)
        except ValueError as error:
            print(f"FAIL {volume_path.name}: {error}")
            failed = True
            parsed[volume_path] = []

    for filename in EMBEDDED:
        source_path = ROOT / "tools" / "labs" / filename
        source = load(source_path).rstrip("\n")
        owners = [
            volume_path
            for volume_path, blocks in parsed.items()
            if source in blocks
        ]
        if not owners:
            print(f"FAIL {filename}: exact complete witness not embedded in any canonical volume")
            failed = True
            continue
        print(f"SYNC {filename}: exact source in {', '.join(path.name for path in owners)}")

    if failed:
        print("Continuous volume source sync: FAIL")
        return 1

    print("Continuous volume source sync: PASS (mechanical only)")
    return 0


if __name__ == "__main__":
    try:
        sys.exit(verify())
    except (OSError, UnicodeError, ValueError) as exc:
        print(f"Continuous volume verification error: {exc}", file=sys.stderr)
        sys.exit(2)
