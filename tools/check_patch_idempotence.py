#!/usr/bin/env python3
"""Verify retained authoring patches in an isolated copy, never edit the vault.

A patch that still changes a book exposes an unintegrated or stale lesson. CI
must check the committed reading surface rather than silently fixing it first.
"""
from pathlib import Path
import hashlib
import os
import shutil
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
PATCHES = (
    "apply_foundations_final_pass.py", "apply_graphics_final_pass.py",
    "apply_engine_final_pass.py", "apply_phase18_bible_patch.py",
)


def hashes(root):
    return {str(p.relative_to(root)): hashlib.sha256(p.read_bytes()).hexdigest()
            for p in (root / "Continuous Notes").glob("*.md")}


def main():
    with tempfile.TemporaryDirectory(prefix="allmight-patch-check-") as folder:
        isolated = Path(folder)
        for relative in ("Continuous Notes", "tools/labs", "tools/patches"):
            shutil.copytree(ROOT / relative, isolated / relative,
                            ignore=shutil.ignore_patterns("__pycache__"))
        expected = hashes(isolated)
        environment = dict(os.environ, PYTHONDONTWRITEBYTECODE="1")
        for repeat in range(2):
            for filename in PATCHES:
                command = [sys.executable, str(isolated / "tools/patches" / filename)]
                result = subprocess.run(command, cwd=isolated, env=environment,
                                        capture_output=True, text=True)
                if result.returncode:
                    raise RuntimeError(f"{filename} failed in isolated copy:\n"
                                       + result.stdout[-1000:] + result.stderr[-1000:])
                actual = hashes(isolated)
                if actual != expected:
                    changed = [name for name in set(expected) | set(actual)
                               if expected.get(name) != actual.get(name)]
                    raise RuntimeError(f"{filename} still changes the checked-in teaching: {changed}")
            print(f"PASS: all four retained patches leave committed book content unchanged (pass {repeat + 1})")
    print("PASS: authoring-patch checks used a temporary copy; live books were not edited")


if __name__ == "__main__":
    try:
        main()
    except (OSError, RuntimeError) as problem:
        print(f"FAIL: {problem}", file=sys.stderr)
        sys.exit(1)
