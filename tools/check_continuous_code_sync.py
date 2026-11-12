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
MATH = CONTINUOUS / "Computing Foundations - From Bits to C++ and Systems APIs.md"
GRAPHICS = CONTINUOUS / "University - Worlds, Images and Inference - Graphics, Vision and Photography.md"
ENGINE = CONTINUOUS / "Game Engine - From First Frame to Living Worlds.md"

# Every entry is a complete standalone witness embedded in the reader-facing book.
EMBEDDED = {
    MATH: ("math_contact_toi_lab.cpp", "temporary_lifetime_lab.cpp", "se3_exp_log_lab.cpp"),
    GRAPHICS: (
        "disk_ccd_lab.cpp",
        "swept_broadphase_lab.cpp",
    ),
    ENGINE: (
        "disk_event_pipeline_lab.cpp",
        "persistent_contact_lab.cpp",
        "two_point_contact_block_lab.cpp",
        "contact_cache_lab.cpp",
        "disk_event_broadphase_differential_lab.cpp",
        "spring_generator_lab.cpp",
        "contact_penetration_refresh_lab.cpp",
        "render_graph_contract_lab.cpp",
        "engine_world_contract_lab.cpp",
        "animation_controller_contract_lab.cpp",
        "threaded_asset_publication_lab.cpp",
        "resource_state_plan_lab.cpp",
    ),
}


def load(path: Path) -> str:
    if not path.is_file():
        raise ValueError(f"missing required file: {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8")


def embedded_cpp(volume: str, source: str, path: Path) -> str:
    # Accept the two Markdown fence styles used by the canonical notes.
    first_line = source.splitlines()[0]
    matches = []
    for fence in ("```", "~~~"):
        start_marker = fence + "cpp\n" + first_line
        pos = 0
        while (pos := volume.find(start_marker, pos)) != -1:
            begin = pos + len(fence + "cpp\n")
            end = volume.find("\n" + fence, begin)
            if end == -1:
                raise ValueError(f"{path.name}: unclosed embedded cpp block")
            matches.append(volume[begin:end].rstrip("\n"))
            pos = end + len(fence)
    if len(matches) != 1:
        raise ValueError(
            f"{path.name}: expected exactly one full cpp code block starting "
            f"{first_line!r}, found {len(matches)}"
        )
    return matches[0]


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
        python_witnesses = []
        if volume_path == GRAPHICS:
            python_witnesses.append("xr_planar_vio_ekf_lab.py")
        if volume_path == MATH:
            python_witnesses.append("research_inference_lab.py")
        for filename in python_witnesses:
            python_path = ROOT / "tools" / "labs" / filename
            python_source = load(python_path).rstrip("\n")
            expected_backtick = chr(96) * 3 + "python\n" + python_source + "\n" + chr(96) * 3
            expected_tilde = "~~~python\n" + python_source + "\n~~~"
            count = text.count(expected_backtick) + text.count(expected_tilde)
            if count != 1:
                print(f"FAIL {filename}: full Python source differs or is duplicated")
                failed = True
            else:
                print(f"SYNC {filename}: exact source agreement")
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
