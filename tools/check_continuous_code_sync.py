#!/usr/bin/env python3
"""Mechanical source-to-continuous-volume contract, not a semantic teaching audit.

The three canonical volumes embed selected COMPLETE runnable C++20
witnesses. An edited source must also be synchronized into the teaching chapter.
This check intentionally avoids treating short source links as complete instruction.

Run: python3 tools/check_continuous_code_sync.py
"""
from __future__ import annotations

import difflib
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CONTINUOUS = ROOT / "Continuous Notes"
MATH = CONTINUOUS / "00 - From Vectors to Images - Integrated Mathematics for Visual Computing.md"
GRAPHICS = CONTINUOUS / "University - Worlds, Images and Inference - Graphics, Vision and Photography.md"
ENGINE = CONTINUOUS / "Game Engine - From First Frame to Living Worlds.md"

# Every entry is a complete standalone witness embedded in the reader-facing book.
EMBEDDED = {
    MATH: ("math_contact_toi_lab.cpp",),
    GRAPHICS: (
        "disk_ccd_lab.cpp",
        "swept_broadphase_lab.cpp",
    ),
    ENGINE: (
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
    ),
}


def load(path: Path) -> str:
    if not path.is_file():
        raise ValueError(f"missing required file: {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8")


def embedded_cpp(volume: str, source: str, path: Path) -> str:
    # Use a unique, complete first line as an identity check. Repeated or absent
    # programs are errors: silently matching the wrong copy would be misleading.
    first_line = source.splitlines()[0]
    start_marker = "```cpp\n" + first_line
    locations = []
    pos = 0
    while (pos := volume.find(start_marker, pos)) != -1:
        locations.append(pos)
        pos += len(start_marker)
    if len(locations) != 1:
        raise ValueError(
            f"{path.name}: expected exactly one full cpp code block starting "
            f"{first_line!r}, found {len(locations)}"
        )
    begin = locations[0] + len("```cpp\n")
    end = volume.find("\n```", begin)
    if end == -1:
        raise ValueError(f"{path.name}: unclosed embedded cpp block")
    return volume[begin:end].rstrip("\n")


def verify() -> int:
    failed = False
    for volume_path in (MATH, GRAPHICS, ENGINE):
        text = load(volume_path)
        fences = sum(line.startswith("```") for line in text.splitlines())
        math = sum(line.strip() == "$$" for line in text.splitlines())
        if fences % 2 or math % 2:
            print(f"FAIL {volume_path.name}: unbalanced fences={fences} math={math}")
            failed = True
        else:
            print(f"FORMAT {volume_path.name}: {fences} fenced boundaries, {math} display boundaries")
        for filename in EMBEDDED.get(volume_path, ()):
            source_path = ROOT / "tools" / "labs" / filename
            source = load(source_path).rstrip("\n")
            try:
                embedded = embedded_cpp(text, source, source_path)
            except ValueError as error:
                print(f"FAIL {error}")
                failed = True
                continue
            if embedded == source:
                print(f"SYNC {filename}: exact source == continuous narrative")
            else:
                failed = True
                diff = list(difflib.unified_diff(
                    embedded.splitlines(), source.splitlines(),
                    fromfile=f"book/{filename}", tofile=f"source/{filename}",
                    lineterm=""
                ))
                print(f"FAIL {filename}: teaching is stale; first diff follows")
                print("\n".join(diff[:24]))
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
