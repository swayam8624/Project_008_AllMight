"""Regression suite for Book Two additive chapter-level figure and mathematics integration.

Checks the exact mapping of all 16 original Book Two chapter IDs, per-book
isolation, working-copy provenance and the absence of duplicated injections.
Run from the Tools directory: python3 -m unittest -v test_book02_visuals
"""
from __future__ import annotations

import tempfile
import unittest
from pathlib import Path

from book02_visuals import FIGURES, PRELUDE_CHAPTER, PRELUDE_FILE, install_book02_visuals


ALL_CHAPTERS = [
    "01-foundations-i-ch017",
    "01-foundations-i-ch018",
    "01-foundations-i-ch019",
    "01-foundations-i-ch020",
    "02-foundations-ii-ch021",
    "02-foundations-ii-ch022",
    "02-foundations-ii-ch023",
    "02-foundations-ii-ch024",
    "02-foundations-ii-ch025",
    "02-foundations-ii-ch026",
    "02-foundations-ii-ch027",
    "02-foundations-ii-ch028",
    "02-foundations-ii-ch029",
    "02-foundations-ii-ch030",
    "02-foundations-ii-ch031",
    "03-visual-computing-i-ch012",
]


def make_tex(directory: Path, chapter_ids: list[str], stage: int = 2) -> Path:
    parts = [
        r"\documentclass{book}",
        r"\usepackage[print]{allmight-book}",
        rf"\SetBookStage{{{stage}}}{{Mathematics for Visual Computing}}",
        r"\begin{document}",
    ]
    for id_ in chapter_ids:
        parts.append(
            rf"\input{{../current/02-Mathematics-for-Visual-Computing-P01/content/{id_}.tex}}"
        )
    parts.append(r"\end{document}")
    tex = directory / "main.tex"
    tex.write_text("\n".join(parts) + "\n", encoding="utf-8")
    return tex


class Book02VisualTests(unittest.TestCase):
    def test_all_chapters_have_explicit_supplemental_ownership(self) -> None:
        self.assertEqual(set(FIGURES), set(ALL_CHAPTERS))
        self.assertEqual(len(ALL_CHAPTERS), len(set(ALL_CHAPTERS)))
        all_names = [n for names in FIGURES.values() for n in names]
        self.assertEqual(len(all_names), len(set(all_names)))
        source = Path(__file__).resolve().parents[1] / "Assets" / "Book02"
        for name in (PRELUDE_FILE, "preamble.tex", *all_names):
            self.assertTrue((source / name).is_file(), name)

    def test_complete_stage_preserves_every_source_input(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            main = make_tex(Path(tmp), ALL_CHAPTERS)
            before = main.read_text()
            added = install_book02_visuals(main)
            after = main.read_text()
            expected = [PRELUDE_FILE] + [n for chapter in ALL_CHAPTERS for n in FIGURES[chapter]]
            self.assertEqual(added, expected)
            self.assertEqual(after.count(r"\input{book02-figures/preamble.tex}"), 1)
            for chapter in ALL_CHAPTERS:
                original = rf"\input{{../current/02-Mathematics-for-Visual-Computing-P01/content/{chapter}.tex}}"
                self.assertEqual(before.count(original), 1)
                self.assertEqual(after.count(original), 1)
                self.assertLess(after.index(original), after.index("book02-figures/"+FIGURES[chapter][0]))
                self.assertTrue(all((Path(tmp)/"book02-figures"/n).is_file() for n in FIGURES[chapter]))
            self.assertLess(after.index("book02-figures/"+PRELUDE_FILE),
                            after.index("01-foundations-i-ch017.tex"))

    def test_physical_part_distribution(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            first = make_tex(Path(tmp), ALL_CHAPTERS[:8])
            got = install_book02_visuals(first)
            expected = [PRELUDE_FILE]+[n for c in ALL_CHAPTERS[:8] for n in FIGURES[c]]
            self.assertEqual(got, expected)
        with tempfile.TemporaryDirectory() as tmp:
            second = make_tex(Path(tmp), ALL_CHAPTERS[8:])
            got = install_book02_visuals(second)
            expected = [n for c in ALL_CHAPTERS[8:] for n in FIGURES[c]]
            self.assertEqual(got, expected)

    def test_idempotent_without_rewriting_originals(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            main = make_tex(Path(tmp), ALL_CHAPTERS)
            install_book02_visuals(main)
            saved = main.read_bytes()
            self.assertEqual(install_book02_visuals(main), [])
            self.assertEqual(main.read_bytes(), saved)

    def test_other_books_untouched(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            main = make_tex(Path(tmp), ALL_CHAPTERS, stage=1)
            saved = main.read_bytes()
            self.assertEqual(install_book02_visuals(main), [])
            self.assertEqual(main.read_bytes(), saved)

    def test_no_valid_chapter_anchor_is_an_error(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            main = make_tex(Path(tmp), ["other-chapter"])
            with self.assertRaises(RuntimeError):
                install_book02_visuals(main)
            self.assertNotIn("book02-figures", main.read_text())

    def test_ambiguous_duplicate_chapter_input_is_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            main = make_tex(Path(tmp), [ALL_CHAPTERS[2], ALL_CHAPTERS[2]])
            with self.assertRaises(RuntimeError):
                install_book02_visuals(main)


if __name__ == "__main__":
    unittest.main()
