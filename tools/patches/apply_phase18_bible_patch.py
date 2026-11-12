#!/usr/bin/env python3
"""Apply the Phase-18 Bible repair to the canonical narratives.

The canonical Markdown files are several megabytes and can exceed API body limits.
This patch is deterministic and idempotent. It also repairs two earlier PR-31
insertions whose Markdown fences were accidentally stored as literal escaped
backticks, preventing the exact-source checker from recognizing their code.
"""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
ENGINE = ROOT / "Continuous Notes" / "Game Engine - From First Frame to Living Worlds.md"
GRAPHICS = ROOT / "Continuous Notes" / "University - Worlds, Images and Inference - Graphics, Vision and Photography.md"
FRAGMENT = ROOT / "tools" / "patches" / "phase18_coupled_contact_fragment.md"
LAB = ROOT / "tools" / "labs" / "two_point_contact_block_lab.cpp"
XR_LAB = ROOT / "tools" / "labs" / "xr_planar_vio_ekf_lab.py"
RESOURCE_LAB = ROOT / "tools" / "labs" / "resource_state_plan_lab.cpp"
CHECKER = ROOT / "tools" / "check_continuous_code_sync.py"

OLD = "That closes the **entire 18-pass run**."
HEADING = "## From one collision impulse to a coupled contact problem — reconstructed Phase 18 bridge"
PLACEHOLDER = "{{TWO_POINT_CONTACT_BLOCK_LAB}}"

def normalize_escaped_exact_block(document: Path, source: Path, language: str) -> None:
    """Normalize one exact source block whose fence was saved as \\`\\`\\`."""
    text = document.read_text(encoding="utf-8")
    program = source.read_text(encoding="utf-8").rstrip()
    escaped_fence = "\\`\\`\\`"
    escaped = escaped_fence + language + "\n" + program + "\n" + escaped_fence
    actual = "```" + language + "\n" + program + "\n```"
    if actual in text:
        print(f"{source.name}: executable fence already normalized.")
        return
    count = text.count(escaped)
    if count != 1:
        raise RuntimeError(f"{source.name}: expected one escaped exact block, found {count}")
    document.write_text(text.replace(escaped, actual), encoding="utf-8")
    print(f"{source.name}: normalized escaped Markdown fence around exact source.")

def main() -> None:
    normalize_escaped_exact_block(GRAPHICS, XR_LAB, "python")
    normalize_escaped_exact_block(ENGINE, RESOURCE_LAB, "cpp")

    engine = ENGINE.read_text(encoding="utf-8")
    fragment = FRAGMENT.read_text(encoding="utf-8").rstrip()
    lab = LAB.read_text(encoding="utf-8").rstrip()

    if HEADING in engine:
        print("Phase-18 narrative already applied; leaving Engine insertion unchanged.")
    else:
        if engine.count(OLD) != 1:
            raise RuntimeError(f"expected exactly one historical closure marker, found {engine.count(OLD)}")
        if fragment.count(PLACEHOLDER) != 1:
            raise RuntimeError("Phase-18 fragment must contain exactly one lab placeholder")
        replacement = fragment.replace(PLACEHOLDER, lab)
        engine = engine.replace(OLD, replacement)
        ENGINE.write_text(engine, encoding="utf-8")
        print("Applied Phase-18 coupled-contact narrative and exact lab source.")

    checker = CHECKER.read_text(encoding="utf-8")
    entry = '        "two_point_contact_block_lab.cpp",\\n'
    if entry not in checker:
        anchor = '        "persistent_contact_lab.cpp",\\n'
        if checker.count(anchor) != 1:
            raise RuntimeError("could not find unique persistent-contact checker anchor")
        checker = checker.replace(anchor, anchor + entry)
        CHECKER.write_text(checker, encoding="utf-8")
        print("Added two-point contact witness to continuous-source sync checker.")
    else:
        print("Source-sync checker already contains the two-point witness.")

    final = ENGINE.read_text(encoding="utf-8")
    if final.count(HEADING) != 1:
        raise RuntimeError("Phase-18 narrative heading is not unique after patch")
    if lab not in final:
        raise RuntimeError("canonical Engine volume does not contain exact two-point lab source")
    if OLD in final:
        raise RuntimeError("historical over-closure sentence still exists")
    print("Phase-18 Bible patch verification: PASS")

if __name__ == "__main__":
    main()
