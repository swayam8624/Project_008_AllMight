"""Book Two content gate: the proof, numerical oracle and both reader formats agree.

The full frozen manuscript remains byte-for-byte unchanged.  This test
certifies one newly reconstructed cross-chapter proof, *not* a theorem-by-
theorem proof of the prior 225k-word source text.
"""
from __future__ import annotations

from fractions import Fraction
from pathlib import Path
import sys
import unittest

from book02_coverage_oracle import EXAMPLE, exact_coverage, clipped_triangle

ROOT = Path(__file__).resolve().parents[3]
TOOLS = ROOT / "Notes/Book Production" / "AllMight Codex" / "LaTeX" / "Tools"
ASSETS = ROOT / "Notes/Book Production" / "AllMight Codex" / "LaTeX" / "Assets" / "Book02"
sys.path.insert(0, str(TOOLS))

from book02_visuals import RICH_FIGURES  # noqa: E402
from book02_epub_supplements import (  # noqa: E402
    COVERAGE, ALTERNATIVES, all_source_filenames
)
from book02_source_preflight import audit  # noqa: E402


class CoverageLessonSourceContracts(unittest.TestCase):
    def test_exact_mathematical_example_matches_manuscript(self):
        lesson = (ASSETS / "36-from-probability-to-pixels.tex").read_text()
        self.assertEqual(exact_coverage(EXAMPLE), Fraction(119, 192))
        self.assertIn(r"\tfrac{119}{192}", lesson)
        # This formula is the exact Bernoulli numerator p(1-p).
        self.assertIn("8687/36864", lesson)
        self.assertIn(r"\operatorname{Var}(\widehat c_N)", lesson)
        self.assertIn("top-left", lesson)
        self.assertIn("shared", lesson.lower())
        self.assertIn(r"\begin{figure}", lesson)
        self.assertIn(r"\caption{", lesson)
        self.assertIn(r"\label{fig:book02-coverage-bridge}", lesson)
        self.assertIn("rendered/coverage/triangle-pixel-coverage.pdf", lesson)
        self.assertEqual(len(clipped_triangle(EXAMPLE)), 6)

    def test_exactly_one_reader_boundary_and_epub_alt(self):
        suffix=RICH_FIGURES["02-foundations-ii-ch031"][-1]
        self.assertEqual(suffix,"36-from-probability-to-pixels.tex")
        self.assertEqual(sum("36-from-probability-to-pixels.tex" in group
                             for group in RICH_FIGURES.values()),1)
        self.assertEqual(len(all_source_filenames()),36)
        self.assertEqual(COVERAGE[suffix],"triangle-pixel-coverage")
        self.assertIn("rational",ALTERNATIVES["triangle-pixel-coverage"])
        self.assertEqual(len(ALTERNATIVES),13)

    def test_canonical_manuscript_chapters_not_modified(self):
        result=audit()
        self.assertEqual(result["original_chapters"],16)
        self.assertEqual(result["additive_sources"],36)
        self.assertTrue(result["unchanged_github_sha1"])


if __name__ == "__main__":
    unittest.main()
