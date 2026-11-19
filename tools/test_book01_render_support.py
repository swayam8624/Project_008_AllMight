#!/usr/bin/env python3
"""Offline regression tests for source-preserving Book 1 production guards."""
from pathlib import Path
from tempfile import TemporaryDirectory
import unittest
import book01_render_support as book


class Book01ProductionTests(unittest.TestCase):
    def test_mermaid_is_never_passed_as_raw_code(self):
        source = (
            "## CPU to GPU transfer\n\n"
            "\`\`\`mermaid\nflowchart LR\n  A[CPU] --> B[GPU]\n\`\`\`\n\n"
            "## Synchronization\n\n"
            "\`\`\`mermaid\nsequenceDiagram\nA->>B: Work\n\`\`\`\n"
        )
        source = source.replace("\\\`", "\`")
        with TemporaryDirectory() as temp:
            root = Path(temp)
            part = root / "book01-expanded-p01"
            transformed, entries = book.plan_figures(source, 1, 1, part, root)
            self.assertEqual(len(entries), 2)
            self.assertNotIn("\`\`\`mermaid", transformed)
            self.assertIn("ALLMIGHTFIGURECH01INDEX001", transformed)
            tex = book.replace_figure_tokens(transformed, entries)
            self.assertEqual(tex.count("\\includegraphics"), 2)
            self.assertNotIn("ALLMIGHTFIGURECH", tex)
            self.assertIn("CPU to GPU transfer", tex)
            for entry in entries:
                self.assertTrue((root / entry["source"]).exists())

    def test_missing_or_duplicate_figure_fails(self):
        item = {"part": 1, "image": "asset.png",
                "token": "ALLMIGHTFIGURECH01INDEX001",
                "caption": "Figure 1.1: Dependency"}
        with self.assertRaises(ValueError):
            book.replace_figure_tokens("There was no image here", [item])

    def test_boxed_math_source_and_render_are_checked(self):
        src = (
            "It says:\n\n$$\n\\boxed{\n"
            "\\text{run this kernel over a logical N-dimensional grid}.\n"
            "}\n$$\n"
        )
        stat = book.math_preflight(src, "Ch09")
        self.assertEqual(stat["boxed_math"], 1)
        book.check_math_rendered(stat,
                                 "\\[\\boxed{\\text{run this kernel}}\\]", 9)
        with self.assertRaises(ValueError):
            book.check_math_rendered(stat, "\\[x+y\\]", 9)

    def test_empty_display_and_empty_box_are_rejected(self):
        for source in ("$$\n\n$$", "$$\n\\boxed{\n}\n$$"):
            with self.assertRaises(ValueError):
                book.math_preflight(source, "broken")

    def test_mermaid_source_hash_checks(self):
        with TemporaryDirectory() as temp:
            root = Path(temp)
            part = root / "book01-expanded-p02"
            text, entries = book.plan_figures(
                "\`\`\`mermaid\nflowchart LR\n A-->B\n\`\`\`".replace("\\\`", "\`"),
                5, 2, part, root
            )
            self.assertEqual(book.digest((root / entries[0]["source"]).read_bytes()),
                             entries[0]["diagram_sha256"])


if __name__ == "__main__":
    unittest.main()
