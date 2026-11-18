#!/usr/bin/env python3
"""Self-contained tests for preservation and idempotence of project inserts.

Run from repository root:
  python3 -m unittest discover -s tools/tests -p 'test_cpp23_project_inserts.py'

All tests operate in temporary fixture repositories. None edits the real book.
"""
from __future__ import annotations

import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
SCRIPT = REPO / "tools/apply_cpp23_project_inserts.py"
MASTER_NAME = "Computing Foundations - From Bits to C++ and Systems APIs.md"


class ProjectInsertionTests(unittest.TestCase):
    def setUp(self) -> None:
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        (self.root / "tools").mkdir()
        (self.root / "Continuous Notes").mkdir()
        (self.root / "Supplementary/C++23 Project Labs").mkdir(parents=True)
        shutil.copy2(SCRIPT, self.root / "tools/apply_cpp23_project_inserts.py")
        self.master = self.root / "Continuous Notes" / MASTER_NAME
        self.original = (
            "# Original complete book\n\n"
            "## Bits become decisions and compact fields\n"
            "Keep the untouched old lesson.\n"
            "## Arithmetic becomes a policy\n"
            "Keep the untouched original arithmetic lesson.\n"
            "## A handle remembers which occupant was intended\n"
            "Keep the untouched original identity lesson.\n"
            "## Choose a type for its promise, not its familiar size\n"
            "Keep original construction and failure guarantee content.\n"
            "## Let the compiler compose the operations\n"
            "Keep original laboratory and ownership content.\n"
            "## From one owned resource to many requests and a replaceable backend\n"
            "Keep all the original subsequent material.\n"
        ).encode("utf-8")
        self.master.write_bytes(self.original)
        self.lesson1 = self.root / "Supplementary/C++23 Project Labs/NES-01 - Cartridge Bytes and Trusted Layout.md"
        self.lesson2 = self.root / "Supplementary/C++23 Project Labs/NES-02 - Owned NROM and Memory Bus.md"
        self.lesson3 = self.root / "Supplementary/C++23 Project Labs/NES-02T - Transactional Cartridge Replacement.md"
        self.lesson4 = self.root / "Supplementary/C++23 Project Labs/NES-02P - Ownership Proofs for an Emulator.md"
        self.lesson1.write_text("### NES-01 — A tested cartridge\n\nOriginal NES-01 addition.\n")
        self.lesson2.write_text("### NES-02 — A tested bus\n\nOriginal NES-02 addition.\n")
        self.lesson3.write_text("### NES-02T — A tested transaction\n\nOriginal NES-02T addition.\n")
        self.lesson4.write_text("### NES-02P — A tested ownership proof\n\nOriginal NES-02P addition.\n")

    def run_script(self, mode: str, success: bool = True) -> str:
        result = subprocess.run(
            [sys.executable, str(self.root / "tools/apply_cpp23_project_inserts.py"), mode],
            cwd=self.root,
            text=True,
            capture_output=True,
            check=False,
        )
        if success:
            self.assertEqual(result.returncode, 0, result.stderr)
        else:
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("Refused", result.stderr)
        return result.stdout

    def test_dry_run_preserves_every_original_byte(self) -> None:
        self.run_script("--check")
        self.assertEqual(self.master.read_bytes(), self.original)

    def test_apply_once_and_idempotent_second_apply(self) -> None:
        self.run_script("--apply")
        once = self.master.read_bytes()
        self.assertTrue(once.startswith(b"# Original complete book"))
        self.assertEqual(once.count(b"<!-- ALLMIGHT-CPP23-NES01-ADDITIVE-START -->"), 1)
        self.assertEqual(once.count(b"<!-- ALLMIGHT-CPP23-NES02-ADDITIVE-START -->"), 1)
        self.assertEqual(once.count(b"<!-- ALLMIGHT-CPP23-NES02T-ADDITIVE-START -->"), 1)
        self.assertEqual(once.count(b"<!-- ALLMIGHT-CPP23-NES02P-ADDITIVE-START -->"), 1)
        self.assertEqual(once.count(b"Keep the untouched old lesson."), 1)
        self.assertEqual(once.count(b"Keep all the original subsequent material."), 1)

        # Stronger preservation oracle: surgically delete only the four
        # newly inserted byte ranges, then compare the entire old book
        # byte-for-byte, not just a few snippets.
        stripped = once
        for name in (b"NES01", b"NES02", b"NES02T", b"NES02P"):
            start = b"\n<!-- ALLMIGHT-CPP23-" + name + b"-ADDITIVE-START -->\n"
            end = b"<!-- ALLMIGHT-CPP23-" + name + b"-ADDITIVE-END -->\n"
            begin_index = stripped.index(start)
            end_index = stripped.index(end, begin_index) + len(end)
            stripped = stripped[:begin_index] + stripped[end_index:]
        self.assertEqual(stripped, self.original)

        self.run_script("--check")
        self.run_script("--apply")
        self.assertEqual(self.master.read_bytes(), once)

    def test_previously_applied_nes01_is_not_duplicated(self) -> None:
        # Emulate an existing NES-01 insert made by the earlier one-section tool.
        beginning = b"<!-- ALLMIGHT-CPP23-NES01-ADDITIVE-START -->\n"
        ending = b"<!-- ALLMIGHT-CPP23-NES01-ADDITIVE-END -->\n"
        block = b"\n" + beginning + self.lesson1.read_bytes().rstrip(b"\n") + b"\n" + ending
        self.master.write_bytes(self.original.replace(
            b"\n## Arithmetic becomes a policy\n",
            block + b"\n## Arithmetic becomes a policy\n",
        ))
        self.run_script("--apply")
        final = self.master.read_bytes()
        self.assertEqual(final.count(beginning), 1)
        self.assertEqual(final.count(b"<!-- ALLMIGHT-CPP23-NES02-ADDITIVE-START -->"), 1)
        self.assertEqual(final.count(b"<!-- ALLMIGHT-CPP23-NES02T-ADDITIVE-START -->"), 1)
        self.assertEqual(final.count(b"<!-- ALLMIGHT-CPP23-NES02P-ADDITIVE-START -->"), 1)

    def test_modified_insert_is_a_hard_conflict(self) -> None:
        self.run_script("--apply")
        modified = self.master.read_bytes().replace(
            b"Original NES-02P addition.", b"Locally edited NES-02P addition."
        )
        self.master.write_bytes(modified)
        self.run_script("--apply", success=False)
        self.assertEqual(self.master.read_bytes(), modified)

    def test_duplicate_anchor_refuses_to_edit(self) -> None:
        anchor = b"\n## Let the compiler compose the operations\n"
        ambiguous = self.original.replace(anchor, anchor + anchor)
        self.master.write_bytes(ambiguous)
        self.run_script("--apply", success=False)
        self.assertEqual(self.master.read_bytes(), ambiguous)

    def test_missing_or_ambiguous_anchor_refuses_to_change_file(self) -> None:
        other = self.original.replace(
            b"\n## Choose a type for its promise, not its familiar size\n",
            b"\n## No recognizable heading here\n"
        )
        self.master.write_bytes(other)
        self.run_script("--apply", success=False)
        self.assertEqual(self.master.read_bytes(), other)


if __name__ == "__main__":
    unittest.main()
