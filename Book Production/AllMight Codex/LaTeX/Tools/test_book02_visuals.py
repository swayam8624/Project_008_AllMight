"""Non-destructive Book 02 figure integration tests.

Run with:
    python3 -m unittest Book\ Production/AllMight\ Codex/LaTeX/Tools/test_book02_visuals.py
or from the Tools directory:
    python3 -m unittest test_book02_visuals.py
"""
from __future__ import annotations

import tempfile
import unittest
from pathlib import Path

from book02_visuals import install_book02_visuals


class Book02VisualTests(unittest.TestCase):
    def create_tex(self, root: Path, chapter_ids: list[str], stage: int = 2) -> Path:
        parts = [
            r"\documentclass{book}",
            r"\usepackage[print]{allmight-book}",
            rf"\SetBookStage{{{stage}}}{{Mathematics for Visual Computing}}",
            r"\begin{document}",
        ]
        for chapter in chapter_ids:
            part = "P01" if chapter in (
                "01-foundations-i-ch019", "02-foundations-ii-ch024"
            ) else "P02"
            parts.append(
                rf"\input{{../current/02-Mathematics-for-Visual-Computing-{part}/content/{chapter}.tex}}"
            )
        parts.append(r"\end{document}")
        tex = root / "main.tex"
        tex.write_text("\n".join(parts) + "\n", encoding="utf-8")
        return tex

    def test_part_one_inserts_only_matched_figures(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            main = self.create_tex(Path(tmp), [
                "01-foundations-i-ch019", "02-foundations-ii-ch024"
            ])
            original = main.read_text()
            installed = install_book02_visuals(main)
            result = main.read_text()
            self.assertEqual(installed, [
                "01-linear-transformation.tex", "02-gaussian-uncertainty.tex"
            ])
            self.assertIn(r"\input{book02-figures/preamble.tex}", result)
            self.assertEqual(result.count("book02-figures/01-linear-transformation.tex"), 1)
            self.assertEqual(result.count("book02-figures/02-gaussian-uncertainty.tex"), 1)
            self.assertIn(original.splitlines()[3], result)  # existing body preserved
            self.assertTrue((Path(tmp)/"book02-figures"/"preamble.tex").is_file())

    def test_part_two_inserts_three_figures_in_chapter_order(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            main = self.create_tex(Path(tmp), [
                "02-foundations-ii-ch025",
                "02-foundations-ii-ch027",
                "02-foundations-ii-ch029",
            ])
            names = install_book02_visuals(main)
            self.assertEqual(names, [
                "04-heat-diffusion.tex",
                "03-optimization-landscape.tex",
                "05-convergence-study.tex",
            ])
            text = main.read_text()
            self.assertLess(text.index("04-heat-diffusion.tex"),
                            text.index("03-optimization-landscape.tex"))
            self.assertLess(text.index("03-optimization-landscape.tex"),
                            text.index("05-convergence-study.tex"))

    def test_repeated_injection_is_idempotent(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            main = self.create_tex(Path(tmp), ["02-foundations-ii-ch025"])
            install_book02_visuals(main)
            first = main.read_bytes()
            self.assertEqual(install_book02_visuals(main), [])
            self.assertEqual(main.read_bytes(), first)

    def test_other_stage_is_unchanged(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            main = self.create_tex(Path(tmp), ["02-foundations-ii-ch025"], stage=1)
            first = main.read_bytes()
            self.assertEqual(install_book02_visuals(main), [])
            self.assertEqual(main.read_bytes(), first)


if __name__ == "__main__":
    unittest.main()
