#!/usr/bin/env python3
"""Offline regression tests for source-preserving Book 1 production guards."""
from pathlib import Path
from tempfile import TemporaryDirectory
import unittest
import book01_render_support as book


TICK = chr(96) * 3


class Book01ProductionTests(unittest.TestCase):
    def test_mermaid_is_never_passed_as_raw_code(self):
        source = (
            "## CPU to GPU transfer\n\n"
            f"{TICK}mermaid\nflowchart LR\n  A[CPU] --> B[GPU]\n{TICK}\n\n"
            "## Synchronization\n\n"
            f"{TICK}mermaid\nsequenceDiagram\nA->>B: Work\n{TICK}\n"
        )
        with TemporaryDirectory() as temp:
            root = Path(temp)
            part = root / "book01-expanded-p01"
            transformed, entries = book.plan_figures(source, 1, 1, part, root)
            self.assertEqual(len(entries), 2)
            self.assertNotIn(TICK + "mermaid", transformed)
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
                                 "\\[\\boxed{\\text{run this kernel over a logical N-dimensional grid}.}\\]", 9)
        with self.assertRaises(ValueError):
            book.check_math_rendered(stat, "\\[x+y\\]", 9)

    def test_empty_display_and_empty_box_are_rejected(self):
        for source in ("$$\n\n$$", "$$\n\\boxed{\n}\n$$"):
            with self.assertRaises(ValueError):
                book.math_preflight(source, "broken")

    def test_academic_spacing_preserves_fenced_code_and_math(self):
        prose = (
            "### GENERAL COMPUTE & PARALLEL ALGORITHMS\n\n"
            "$\n\\boxed{\\text{run this kernel}}\n$\n\n"
            "---\n\n**Workgroup size is an algorithmic choice**\n\n"
            "A workgroup is not a fixed performance recipe.\n\n"
            + f"{TICK}text\n---\nliteral code line\n{TICK}\n\n"
            + "---\n\nThe next section continues.\n"
        )
        result, report = book.prepare_reading_structure(prose)
        self.assertIn("#### Workgroup size is an algorithmic choice", result)
        self.assertIn("\\boxed{\\text{run this kernel}}", result)
        self.assertIn(f"{TICK}text\n---\nliteral code line\n{TICK}", result)
        self.assertNotIn("GENERAL COMPUTE & PARALLEL ALGORITHMS", result)
        self.assertGreaterEqual(report["promoted_concept_headings"], 1)

    def test_mermaid_source_hash_checks(self):
        with TemporaryDirectory() as temp:
            root = Path(temp)
            part = root / "book01-expanded-p02"
            text, entries = book.plan_figures(
                f"{TICK}mermaid\nflowchart LR\n A-->B\n{TICK}",
                5, 2, part, root
            )
            self.assertEqual(book.digest((root / entries[0]["source"]).read_bytes()),
                             entries[0]["diagram_sha256"])


if __name__ == "__main__":
    unittest.main()
