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

REPO = Path(__file__).resolve().parents[3]
SCRIPT = REPO / "Code Parts/tools/apply_cpp23_project_inserts.py"
MASTER_NAME = "Computing Foundations - From Bits to C++ and Systems APIs.md"


class ProjectInsertionTests(unittest.TestCase):
    def setUp(self) -> None:
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        (self.root / "Code Parts/tools").mkdir()
        (self.root / "Notes/Continuous Notes").mkdir()
        (self.root / "Notes/Supplementary/C++23 Project Labs").mkdir(parents=True)
        shutil.copy2(SCRIPT, self.root / "Code Parts/tools/apply_cpp23_project_inserts.py")
        self.master = self.root / "Notes/Continuous Notes" / MASTER_NAME
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
            "Keep original generic asset chapter content.\n"
            "### The real performance question is what gets instantiated, stored and moved\n"
            "Keep original typed asset pipeline material.\n"
            "## From a living asset registry to reliable queries and transformations\n"
            "Keep previous original query and CPU lessons.\n"
            "## The scene is now too large for the wrong container — Phase 5, M025–M027, Parts 401–445\n"
            "Keep original graph/algorithmic material.\n"
            "## An API is a promise about an operation\n"
            "Keep original API/device material.\n"
            "### GENERAL COMPUTE & PARALLEL ALGORITHMS\n"
            "Keep original GPU parallel algorithms and scan coverage.\n"
            "### REPRODUCIBILITY, CI, TESTING, DIAGNOSTICS, PROFILING AND THE COMPLETE ENGINE FLOW\n"
            "Keep original build and CI lessons.\n"
            "## From generated GPU work to safe explicit execution\n"
            "Keep original async command lessons.\n"
            "### PERFORMANCE MODELING & PRINCIPLED GPU REDESIGN\n"
            "Keep original GPU performance material.\n"
            "### BUFFERS, IMAGES, MEMORY & UPLOADS\n"
            "Keep original Vulkan buffer and image discussion.\n"
            "### VULKAN MEMORY MODEL, BARRIERS & MULTI-FRAME LIFETIMES\n"
            "Keep original Vulkan memory-model material.\n"
            "### STORAGE MODES, TILE MEMORY, DRAWABLES, FRAMES IN FLIGHT, HEAPS & SYNCHRONIZATION\n"
            "Keep the original Metal storage and tile discussion.\n"
            "### Distinguish host allocation callbacks from device-memory allocation\n"
            "Keep all the original subsequent material.\n"
        ).encode("utf-8")
        self.master.write_bytes(self.original)
        self.lesson1 = self.root / "Notes/Supplementary/C++23 Project Labs/NES-01 - Cartridge Bytes and Trusted Layout.md"
        self.lesson2 = self.root / "Notes/Supplementary/C++23 Project Labs/NES-02 - Owned NROM and Memory Bus.md"
        self.lesson3 = self.root / "Notes/Supplementary/C++23 Project Labs/NES-02T - Transactional Cartridge Replacement.md"
        self.lesson4 = self.root / "Notes/Supplementary/C++23 Project Labs/NES-02P - Ownership Proofs for an Emulator.md"
        self.lesson5 = self.root / "Notes/Supplementary/C++23 Project Labs/NES-05A - Type-Safe Asset Import and Publication.md"
        self.lesson6 = self.root / "Notes/Supplementary/C++23 Project Labs/NES-06G - Generational Identity and Safe Reuse.md"
        self.lesson7 = self.root / "Notes/Supplementary/C++23 Project Labs/NES-07C - CPU Instructions and Typed Debugger Structures.md"
        self.lesson8 = self.root / "Notes/Supplementary/C++23 Project Labs/NES-08G - Dependency Graphs and Explicit GPU Work Contracts.md"
        self.lesson9 = self.root / "Notes/Supplementary/C++23 Project Labs/NES-09M - GPU Memory and Ordering Contracts.md"
        self.lesson10 = self.root / "Notes/Supplementary/C++23 Project Labs/NES-10R - Reproducibility and Evidence-Driven Profiling.md"
        self.lesson11 = self.root / "Notes/Supplementary/C++23 Project Labs/NES-11B - Scoped Borrows and CPU-GPU Frame Buffering.md"
        self.lesson12 = self.root / "Notes/Supplementary/C++23 Project Labs/NES-12Q - Command Lifecycles and Uncertain Submission.md"
        self.lesson13 = self.root / "Notes/Supplementary/C++23 Project Labs/GPU-13D - Cross-Project Vulkan Devices and Resource Contracts.md"
        self.lesson14 = self.root / "Notes/Supplementary/C++23 Project Labs/GPU-14F - Frame Submission Synchronization and Retirement.md"
        self.lesson15 = self.root / "Notes/Supplementary/C++23 Project Labs/GPU-15M - Execute Verify and Measure a Metal Frame.md"
        self.lesson16 = self.root / "Notes/Supplementary/C++23 Project Labs/GPU-16P - GPU Profiling and Apple Callback Ownership.md"
        self.lesson17 = self.root / "Notes/Supplementary/C++23 Project Labs/CPP-18A - Filesystem Ranges Formatting and Coroutine Asset Import.md"
        self.lesson18 = self.root / "Notes/Supplementary/C++23 Project Labs/CPP-19E - A Complete Asset to Evidence Trace.md"
        self.lesson1.write_text("### NES-01 — A tested cartridge\n\nOriginal NES-01 addition.\n")
        self.lesson2.write_text("### NES-02 — A tested bus\n\nOriginal NES-02 addition.\n")
        self.lesson3.write_text("### NES-02T — A tested transaction\n\nOriginal NES-02T addition.\n")
        self.lesson4.write_text("### NES-02P — A tested ownership proof\n\nOriginal NES-02P addition.\n")
        self.lesson5.write_text("### NES-05A — A tested asset pipeline\n\nOriginal NES-05A addition.\n")
        self.lesson6.write_text("### NES-06G — A tested generational registry\n\nOriginal NES-06G addition.\n")
        self.lesson7.write_text("### NES-07C — A tested CPU/debugger\n\nOriginal NES-07C addition.\n")
        self.lesson8.write_text("### NES-08G — A tested graph planner\n\nOriginal NES-08G addition.\n")
        self.lesson9.write_text("### NES-09M — A tested memory model\n\nOriginal NES-09M addition.\n")
        self.lesson10.write_text("### NES-10R — A tested reproducibility protocol\n\nOriginal NES-10R addition.\n")
        self.lesson11.write_text("### NES-11B — A tested frame borrow ring\n\nOriginal NES-11B addition.\n")
        self.lesson12.write_text("### NES-12Q — A tested async journal\n\nOriginal NES-12Q addition.\n")
        self.lesson13.write_text("### GPU-13D — A tested cross-project device\n\nOriginal GPU-13D addition.\n")
        self.lesson14.write_text("### GPU-14F — A tested frame submission\n\nOriginal GPU-14F addition.\n")
        self.lesson15.write_text("### GPU-15M — A tested Metal frame\n\nOriginal GPU-15M addition.\n")
        self.lesson16.write_text("### GPU-16P — A tested GPU profile owner\n\nOriginal GPU-16P addition.\n")
        self.lesson17.write_text("### CPP-18A — A tested modern asset service\n\nOriginal CPP-18A addition.\n")
        self.lesson18.write_text("### CPP-19E — A tested end-to-end trace\n\nOriginal CPP-19E addition.\n")

    def run_script(self, mode: str, success: bool = True) -> str:
        result = subprocess.run(
            [sys.executable, str(self.root / "Code Parts/tools/apply_cpp23_project_inserts.py"), mode],
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
        self.assertEqual(once.count(b"<!-- ALLMIGHT-CPP23-NES05A-ADDITIVE-START -->"), 1)
        self.assertEqual(once.count(b"<!-- ALLMIGHT-CPP23-NES06G-ADDITIVE-START -->"), 1)
        self.assertEqual(once.count(b"<!-- ALLMIGHT-CPP23-NES07C-ADDITIVE-START -->"), 1)
        self.assertEqual(once.count(b"<!-- ALLMIGHT-CPP23-NES08G-ADDITIVE-START -->"), 1)
        self.assertEqual(once.count(b"<!-- ALLMIGHT-CPP23-NES09M-ADDITIVE-START -->"), 1)
        self.assertEqual(once.count(b"<!-- ALLMIGHT-CPP23-NES10R-ADDITIVE-START -->"), 1)
        self.assertEqual(once.count(b"<!-- ALLMIGHT-CPP23-NES11B-ADDITIVE-START -->"), 1)
        self.assertEqual(once.count(b"<!-- ALLMIGHT-CPP23-NES12Q-ADDITIVE-START -->"), 1)
        self.assertEqual(once.count(b"<!-- ALLMIGHT-CPP23-GPU13D-ADDITIVE-START -->"), 1)
        self.assertEqual(once.count(b"<!-- ALLMIGHT-CPP23-GPU14F-ADDITIVE-START -->"), 1)
        self.assertEqual(once.count(b"<!-- ALLMIGHT-CPP23-GPU15M-ADDITIVE-START -->"), 1)
        self.assertEqual(once.count(b"<!-- ALLMIGHT-CPP23-GPU16P-ADDITIVE-START -->"), 1)
        self.assertEqual(once.count(b"<!-- ALLMIGHT-CPP23-CPP18A-ADDITIVE-START -->"), 1)
        self.assertEqual(once.count(b"<!-- ALLMIGHT-CPP23-CPP19E-ADDITIVE-START -->"), 1)
        self.assertEqual(once.count(b"Keep the untouched old lesson."), 1)
        self.assertEqual(once.count(b"Keep all the original subsequent material."), 1)

        # Stronger preservation oracle: surgically delete only the eighteen
        # newly inserted byte ranges, then compare the entire old book
        # byte-for-byte, not just a few snippets.
        stripped = once
        for name in (b"NES01", b"NES02", b"NES02T", b"NES02P", b"NES05A", b"NES06G", b"NES07C", b"NES08G", b"NES09M", b"NES10R", b"NES11B", b"NES12Q", b"GPU13D", b"GPU14F", b"GPU15M", b"GPU16P", b"CPP18A", b"CPP19E"):
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
        self.assertEqual(final.count(b"<!-- ALLMIGHT-CPP23-NES05A-ADDITIVE-START -->"), 1)
        self.assertEqual(final.count(b"<!-- ALLMIGHT-CPP23-NES06G-ADDITIVE-START -->"), 1)
        self.assertEqual(final.count(b"<!-- ALLMIGHT-CPP23-NES07C-ADDITIVE-START -->"), 1)
        self.assertEqual(final.count(b"<!-- ALLMIGHT-CPP23-NES08G-ADDITIVE-START -->"), 1)
        self.assertEqual(final.count(b"<!-- ALLMIGHT-CPP23-NES09M-ADDITIVE-START -->"), 1)
        self.assertEqual(final.count(b"<!-- ALLMIGHT-CPP23-NES10R-ADDITIVE-START -->"), 1)
        self.assertEqual(final.count(b"<!-- ALLMIGHT-CPP23-NES11B-ADDITIVE-START -->"), 1)
        self.assertEqual(final.count(b"<!-- ALLMIGHT-CPP23-NES12Q-ADDITIVE-START -->"), 1)
        self.assertEqual(final.count(b"<!-- ALLMIGHT-CPP23-GPU13D-ADDITIVE-START -->"), 1)
        self.assertEqual(final.count(b"<!-- ALLMIGHT-CPP23-GPU14F-ADDITIVE-START -->"), 1)
        self.assertEqual(final.count(b"<!-- ALLMIGHT-CPP23-GPU15M-ADDITIVE-START -->"), 1)
        self.assertEqual(final.count(b"<!-- ALLMIGHT-CPP23-GPU16P-ADDITIVE-START -->"), 1)
        self.assertEqual(final.count(b"<!-- ALLMIGHT-CPP23-CPP18A-ADDITIVE-START -->"), 1)

    def test_modified_insert_is_a_hard_conflict(self) -> None:
        self.run_script("--apply")
        modified = self.master.read_bytes().replace(
            b"Original CPP-18A addition.", b"Locally edited CPP-18A addition."
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

    def test_adjacent_shared_newline_anchor_is_still_ambiguous(self) -> None:
        """An identical heading can share a newline with its neighbor.

        bytes.count() misses this overlapping occurrence; the strict source
        patcher must detect BOTH headings rather than applying the lesson
        to the first one and silently accepting an ambiguous manuscript.
        """
        anchor = b"\n## Let the compiler compose the operations\n"
        adjacent = b"\n## Let the compiler compose the operations\n## Let the compiler compose the operations\n"
        self.assertEqual(adjacent.count(anchor), 1)  # Non-overlapping miscount.
        ambiguous = self.original.replace(anchor, adjacent)
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
