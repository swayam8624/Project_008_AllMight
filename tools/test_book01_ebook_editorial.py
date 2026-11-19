#!/usr/bin/env python3
"""Offline editorial regression gates for the Book 1 EPUB reader."""
from __future__ import annotations

import unittest
import book01_ebook_editorial as editorial

BACKTICKS = chr(96) * 3
TILDES = "~~~"


class EBookEditorialTests(unittest.TestCase):
    def test_additive_marker_removal_preserves_real_milestone_heading(self):
        source = (
            "A useful explanation.\n\n"
            "<!-- ALLMIGHT-CPP23-NES01-ADDITIVE-START -->\n"
            "### NES-01 — A cartridge is a contract\n\n"
            "The same byte can have different interpretations.\n\n"
            "<!-- ALLMIGHT-CPP23-NES01-ADDITIVE-END -->\n\n"
            "Continue learning about overflow.\n"
        )
        result, stats = editorial.clean_reader_markdown(source, 1)
        self.assertIn("### NES-01 — A cartridge is a contract", result)
        self.assertIn("The same byte can", result)
        self.assertIn("Continue learning about overflow", result)
        self.assertNotIn("ADDITIVE-", result)
        self.assertEqual(stats["authoring_markers_removed"], 2)

    def test_original_code_math_and_mermaid_are_unchanged(self):
        source = (
            "The existing chapter already explains ownership.\n\n"
            + BACKTICKS + "cpp\n"
            "const char* value = \"The user has reserved\";\n"
            "std::vector<int> ids{1,2,3};\n"
            + BACKTICKS + "\n\n"
            + TILDES + "mermaid\n"
            "flowchart LR\n  A --> B\n" + TILDES + "\n\n"
            "ALLMIGHTMATHCH03INDEX0001\n\n"
            "ALLMIGHTFIGURECH03INDEX001\n"
        )
        result, stats = editorial.clean_reader_markdown(source, 3)
        self.assertIn(BACKTICKS + "cpp\nconst char*", result)
        self.assertIn("flowchart LR\n  A --> B", result)
        self.assertIn("ALLMIGHTMATHCH03INDEX0001", result)
        self.assertIn("ALLMIGHTFIGURECH03INDEX001", result)

    def test_clears_editorial_only_assembly_notes_not_technical_caveats(self):
        source = (
            "The original Chapter 13 contains approximately 4,556 lines "
            "of past work, not teaching content.\n\n"
            "Continue the original Chapter 13. This section is staged "
            "before the next original heading.\n\n"
            "This pass adds a C++23 test source and a real validation rule.\n\n"
            "No native GPU run is asserted without a device trace.\n\n"
            "The user requested no intermediate commands or LaTeX "
            "regeneration, so both remain deferred.\n"
        )
        result, stats = editorial.clean_reader_markdown(source, 13)
        self.assertNotIn("approximately 4,556 lines", result)
        self.assertNotIn("Continue the original", result)
        self.assertNotIn("The user requested", result)
        self.assertIn("The companion laboratory includes", result)
        self.assertIn("No native GPU run is asserted", result)
        self.assertGreaterEqual(stats["authoring_paragraphs_removed"], 2)

    def test_publication_gate_rejects_unplanned_authoring_leak(self):
        # If an unknown marker survives, this is a release failure, not
        # something to silently delete from the original manuscript.
        source = "The user has reserved a future manuscript rewrite.\n"
        with self.assertRaisesRegex(ValueError, "authoring text"):
            editorial.clean_reader_markdown(source, 12)


if __name__ == "__main__":
    unittest.main()
