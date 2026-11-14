"""Regression tests for the exact failures exposed by the live-book audit."""
from pathlib import Path
import sys
import unittest
import tempfile
import re

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from note_integrity import (BT, code_blocks, migrate_legacy_links,
                            normalized_baseline, ordered_lines_preserved)
from check_continuous_code_sync import exact_program_count
from check_vault_links import audit


class NoteIntegrityTests(unittest.TestCase):
    def test_mixed_fence_lengths_and_literal_short_fence(self):
        text = BT*4 + "cpp\n" + BT*3 + "\nint x;\n" + BT*4 + "\n~~~python\npass\n~~~\n"
        blocks = code_blocks(text)
        self.assertEqual([b.language for b in blocks], ["cpp", "python"])
        self.assertEqual(blocks[0].body, BT*3 + "\nint x;")

    def test_unclosed_fence_is_not_hidden_by_other_marker(self):
        with self.assertRaisesRegex(ValueError, "line 1"):
            code_blocks(BT*3 + "cpp\nint x;\n~~~\n")

    def test_whole_program_identity_not_common_include_line(self):
        a = "#include <algorithm>\n// owner A\nint main() { return 0; }"
        b = "#include <algorithm>\n// owner B\nint main() { return 1; }"
        text = BT*3 + "cpp\n" + a + "\n" + BT*3 + "\n~~~cpp\n" + b + "\n~~~\n"
        self.assertEqual(exact_program_count(code_blocks(text), a, "cpp"), 1)
        self.assertEqual(exact_program_count(code_blocks(text), a.replace("owner A", "new teaching"), "cpp"), 0)
        self.assertEqual(exact_program_count(code_blocks(text + text), a, "cpp"), 2)

    def test_route_migration_preserves_anchor_alias_and_literals(self):
        old = "Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing"
        new = "Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs"
        link = "[[" + old + "#Basis|Read basis]]"
        text = link + "\n" + BT + link + BT + "\n~~~text\n" + link + "\n~~~\n"
        result = migrate_legacy_links(text)
        self.assertTrue(result.startswith("[[" + new + "#Basis|Read basis]]\n"))
        self.assertEqual(result.count(link), 2)
        self.assertEqual(migrate_legacy_links(result), result)

    def test_preservation_rejects_removed_teaching(self):
        previous = "# Lesson\nExplain why the invariant holds.\n"
        self.assertTrue(ordered_lines_preserved(previous, "Intro\n" + previous + "Example\n"))
        self.assertFalse(ordered_lines_preserved(previous, "# Lesson\nExample\n"))

    def test_only_the_specific_tape_and_overclaim_corrections(self):
        bad = "#include <vector>\n" + BT*3 + "\n\nclass Tape {\n};\n"
        fixed = "#include <vector>\n\nclass Tape {\n};\n"
        self.assertEqual(normalized_baseline(bad, "Computing Foundations.md"), fixed)
        with self.assertRaises(ValueError):
            normalized_baseline(bad, "Other book.md")
        claim = "That closes the **entire 18-pass run**.\nOther teaching stays.\n"
        self.assertEqual(normalized_baseline(claim, "Game Engine.md"), "Other teaching stays.\n")


class WikiTargetTests(unittest.TestCase):
    def test_file_heading_asset_block_and_literal_attribute(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            (root / "picture.svg").write_text("<svg/>")
            (root / "Topic.md").write_text("# Topic\n## A " + BT + "typed" + BT + " heading\nA block. ^stable\n")
            (root / "README.md").write_text(
                "[[Topic#A typed heading|Read]] ![[picture.svg]] [[Topic#^stable]]\n"
                + BT + "[[nodiscard]]" + BT + "\n~~~text\n[[not-a-link]]\n~~~\n")
            result = audit(root)
            self.assertEqual(result["error_count"], 0)
            self.assertEqual(result["wiki_targets"], 3)
            self.assertEqual(result["asset_targets"], 1)

    def test_missing_heading_is_not_confused_with_missing_file(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            (root / "Topic.md").write_text("# Present\n[[Topic#Absent]]\n")
            result = audit(root)
            self.assertEqual(result["error_count"], 1)
            self.assertEqual(result["errors"][0]["error"], "missing heading")

    def test_ambiguous_basename_is_reported(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            for name in ("one", "two"):
                (root / name).mkdir()
                (root / name / "Topic.md").write_text("# Topic\n")
            (root / "README.md").write_text("[[Topic]]\n")
            self.assertEqual(audit(root)["error_count"], 1)


class WorkflowPolicyTests(unittest.TestCase):
    def test_final_pass_workflows_only_verify_the_checkout(self):
        root = Path(__file__).resolve().parents[2]
        workflows = sorted((root / ".github/workflows").glob("bible-*.yml"))
        self.assertEqual(len(workflows), 4)
        for path in workflows:
            text = path.read_text()
            with self.subTest(workflow=path.name):
                self.assertIn("permissions:\n  contents: read\n", text)
                self.assertNotIn("git push", text)
                self.assertNotIn("git commit", text)
                self.assertNotRegex(text, r"run:\s*python3\s+tools/patches/")
                self.assertNotRegex(text, re.compile(r"^\s+ref:", re.M))
                self.assertIn("tools/check_patch_idempotence.py", text)


if __name__ == "__main__":
    unittest.main()
