#!/usr/bin/env python3
"""Check complete, commented program bodies in the three reader-facing books.

This checks exact source agreement and Markdown structure, not semantic mastery
or native device behavior. Both valid Markdown fence styles are supported.
"""
from pathlib import Path
import sys
from note_integrity import code_blocks, prose_lines

ROOT = Path(__file__).resolve().parents[1]
CONTINUOUS = ROOT / "Continuous Notes"
EMBEDDED = {
    "Computing Foundations - From Bits to C++ and Systems APIs.md": (
        "basis_measurement_lab.cpp", "math_contact_toi_lab.cpp",
        "temporary_lifetime_lab.cpp", "se3_exp_log_lab.cpp",
        "research_inference_lab.py",
    ),
    "University - Worlds, Images and Inference - Graphics, Vision and Photography.md": (
        "disk_ccd_lab.cpp", "swept_broadphase_lab.cpp",
        "xr_planar_vio_ekf_lab.py", "xr_3d_imu_preintegration_lab.py",
        "nerf_volume_rendering_lab.py", "gaussian_training_step_lab.py",
    ),
    "Game Engine - From First Frame to Living Worlds.md": (
        "disk_event_pipeline_lab.cpp", "persistent_contact_lab.cpp",
        "two_point_contact_block_lab.cpp", "contact_cache_lab.cpp",
        "disk_event_broadphase_differential_lab.cpp", "spring_generator_lab.cpp",
        "contact_penetration_refresh_lab.cpp", "render_graph_contract_lab.cpp",
        "engine_world_contract_lab.cpp", "animation_controller_contract_lab.cpp",
        "threaded_asset_publication_lab.cpp", "resource_state_plan_lab.cpp",
        "rollback_netcode_lab.cpp", "audio_voice_mixer_lab.cpp",
        "ai_decision_models_lab.cpp", "scene_persistence_lab.cpp",
    ),
}


def exact_program_count(blocks, source, language):
    # Compare the whole program, including teaching comments. Common first lines
    # such as #include <algorithm> cannot uniquely identify a laboratory.
    return sum(block.language == language and block.body == source.rstrip("\r\n")
               for block in blocks)


def verify():
    failed = False
    checked = 0
    for name, filenames in EMBEDDED.items():
        path = CONTINUOUS / name
        text = path.read_text(encoding="utf-8")
        try:
            blocks = code_blocks(text)
            displays = sum(line.strip() == "$$" for _, line in prose_lines(text))
        except ValueError as problem:
            print(f"FAIL {name}: {problem}")
            failed = True
            continue
        if displays % 2:
            print(f"FAIL {name}: odd display-math boundary count {displays}")
            failed = True
        else:
            print(f"FORMAT {name}: {len(blocks)} complete code blocks, {displays} display boundaries")
        for filename in filenames:
            source = (ROOT / "tools" / "labs" / filename).read_text(encoding="utf-8")
            language = "python" if filename.endswith(".py") else "cpp"
            count = exact_program_count(blocks, source, language)
            checked += 1
            if count != 1:
                print(f"FAIL {filename}: expected one exact full {language} program, found {count}")
                failed = True
            else:
                print(f"SYNC {filename}: exact full source and comments match")
    print(f"Continuous volume source sync: {'FAIL' if failed else 'PASS'} ({checked} declared programs; mechanical only)")
    return 1 if failed else 0


if __name__ == "__main__":
    try:
        sys.exit(verify())
    except (OSError, UnicodeError, ValueError) as problem:
        print(f"Continuous volume verification error: {problem}", file=sys.stderr)
        sys.exit(2)
