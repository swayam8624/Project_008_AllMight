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

    def test_math_protection_preserves_boxed_cuda_gpu_and_cpp_expressions(self):
        source = (
            "A scheduling rule.\n\n$\n"
            "\\boxed{\\text{run this kernel over a logical N-dimensional grid}.}\n"
            "$\n\n"
            + f"{TICK}cpp\n// literal $ and \\boxed{{x}} in code\n{TICK}\n\n"
            + "$\n\\texttt{simd_sum} = 256\n$\n"
        )
        stats = book.math_preflight(source, "Chapter 9")
        prepared, protected = book.plan_math(source, 9)
        self.assertEqual(stats["display_math"], 2)
        self.assertEqual(len(protected), 2)
        self.assertEqual(stats["boxed_math"], 1)
        self.assertIn("ALLMIGHTMATHCH09INDEX0001", prepared)
        self.assertIn(f"{TICK}cpp\n// literal $", prepared)
        rendered = book.replace_math_tokens(prepared, protected)
        self.assertIn(
            "\\boxed{\\text{run this kernel over a logical N-dimensional grid}.}",
            rendered,
        )
        self.assertIn("\\texttt{simd_sum} = 256", rendered)
        book.check_math_rendered(stats, rendered, 9)
        with self.assertRaises(ValueError):
            book.replace_math_tokens("missing marker", protected)

    def test_empty_display_and_empty_box_are_rejected(self):
        for source in ("$$\n\n$$", "$$\n\\boxed{\n}\n$$"):
            with self.assertRaises(ValueError):
                book.math_preflight(source, "broken")

    def test_pandoc_math_marker_roundtrip_when_available(self):
        import shutil
        import subprocess
        if not shutil.which("pandoc"):
            self.skipTest("Pandoc is optional for this unit test")
        sample = (
            "GPU compute uses a logical grid.\n\n"
            "$\n\\boxed{\\text{run this kernel over a logical N-dimensional grid}.}\n$\n\n"
            "$\n\\texttt{simd_sum} = 256\n$\n"
        )
        stats = book.math_preflight(sample, "Chapter 9")
        markdown, formulas = book.plan_math(sample, 9)
        with TemporaryDirectory() as temp:
            work = Path(temp)
            source = work / "input.md"
            source.write_text(markdown, encoding="utf-8")
            result = subprocess.run(
                ["pandoc", "-f", "markdown+tex_math_single_backslash+tex_math_double_backslash",
                 "-t", "latex", "--wrap=none", str(source)],
                text=True, capture_output=True, check=True,
            )
            tex = book.replace_math_tokens(result.stdout, formulas)
            book.check_math_rendered(stats, tex, 9)
            self.assertIn("run this kernel over a logical N-dimensional grid", tex)
            self.assertEqual(len(formulas), 2)

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
