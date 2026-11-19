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
        math_delim = chr(36) * 2
        source = (
            "A scheduling rule.\n\n"
            + math_delim + "\n"
            + "\\boxed{\\text{run this kernel over a logical N-dimensional grid}.}\n"
            + math_delim + "\n\n"
            + f"{TICK}cpp\n// literal display delimiter {math_delim} "
              f"and \\boxed{{x}} in code\n{TICK}\n\n"
            + math_delim + "\n\\texttt{simd_sum} = 256\n"
            + math_delim + "\n"
        )
        stats = book.math_preflight(source, "Chapter 9")
        prepared, protected = book.plan_math(source, 9)
        self.assertEqual(stats["display_math"], 2)
        self.assertEqual(len(protected), 2)
        self.assertEqual(stats["boxed_math"], 1)
        self.assertIn("ALLMIGHTMATHCH09INDEX0001", prepared)
        self.assertIn(f"{TICK}cpp\n// literal display delimiter", prepared)
        rendered = book.replace_math_tokens(prepared, protected)
        self.assertIn(
            "\\boxed{\\text{run this kernel over a logical N-dimensional grid}.}",
            rendered,
        )
        self.assertIn("\\texttt{simd_sum} = 256", rendered)
        book.check_math_rendered(stats, rendered, 9)
        with self.assertRaises(ValueError):
            book.replace_math_tokens("missing marker", protected)

    def test_percent_in_math_is_not_latex_comment(self):
        source = (
            chr(36) * 2 + "\n"
            + "\\boxed{\\text{parallelizing 99% does not make "
              "the remaining serial 1% disappear}.}\n"
            + chr(36) * 2 + "\n"
            + chr(36) * 2 + "\n"
            + "\\text{do identical work 5% faster}.\n"
            + chr(36) * 2 + "\n"
        )
        stats = book.math_preflight(source, "Ch09 and Ch12")
        protected, entries = book.plan_math(source, 9)
        converted = book.replace_math_tokens(protected, entries)
        self.assertIn("99\\%", converted)
        self.assertIn("1\\%", converted)
        self.assertIn("5\\%", converted)
        self.assertNotIn("99\\\\%", converted)
        book.check_math_rendered(stats, converted, 9)
        self.assertEqual(
            book.sanitize_source_math_for_tex(r"99\% and 5%"),
            r"99\% and 5\%",
        )

    def test_unsupported_code_fence_metadata_is_normalized_for_pandoc(self):
        source = (
            "Serial:\n\n"
            + f'{TICK}cpp id="x1rcxl"\n'
              "for each item:\n    if useful:\n        output.push_back(item);\n"
            + f"{TICK}\n\n"
            + f'{TICK}text id="0aznm5"\n'
              "predicate/count\n    ↓\nparallel scan\n"
            + f"{TICK}\n"
        )
        result, count = book.normalize_code_fence_metadata(source)
        self.assertEqual(count, 2)
        self.assertIn(f"{TICK}cpp\nfor each item:", result)
        self.assertIn(f"{TICK}text\npredicate/count", result)
        self.assertIn("output.push_back(item)", result)
        self.assertNotIn('id="', result)
        import shutil
        import subprocess
        if shutil.which("pandoc"):
            pandoc = subprocess.run(
                ["pandoc", "-f", "markdown", "-t", "latex", "--no-highlight"],
                input=result, capture_output=True, text=True, check=True,
            )
            self.assertEqual(pandoc.stdout.count(r"\begin{verbatim}"), 2)
            self.assertIn("output.push_back(item)", pandoc.stdout)

    def test_empty_display_and_empty_box_are_rejected(self):
        for source in ("$$\n\n$$", "$$\n\\boxed{\n}\n$$"):
            with self.assertRaises(ValueError):
                book.math_preflight(source, "broken")

    def test_pandoc_math_marker_roundtrip_when_available(self):
        import shutil
        import subprocess
        if not shutil.which("pandoc"):
            self.skipTest("Pandoc is optional for this unit test")
        math_delim = chr(36) * 2
        sample = (
            "GPU compute uses a logical grid.\n\n"
            + math_delim + "\n"
            + "\\boxed{\\text{run this kernel over a logical N-dimensional grid}.}\n"
            + math_delim + "\n\n"
            + math_delim + "\n\\texttt{simd_sum} = 256\n"
            + math_delim + "\n"
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
