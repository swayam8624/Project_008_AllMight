#!/usr/bin/env python3
"""Small synthetic-PDF checks for the actual KDP publication proof gate."""
from pathlib import Path
from tempfile import TemporaryDirectory
import unittest

import book01_pdf_proof as proof
import build_book01_expanded_reader as builder

try:
    import fitz
except ImportError:
    fitz = None


@unittest.skipUnless(fitz is not None, "PyMuPDF is not installed")
class ProofTests(unittest.TestCase):
    def make_pdf(self, folder: Path, *, width=504, title="Test Title",
                 header_at_trim=False):
        out = folder / "proof.pdf"
        doc = fitz.open()
        for n in range(3):
            page = doc.new_page(width=width, height=720)
            page.insert_text((78, 120), "A complete, readable paragraph.",
                             fontsize=10)
            if header_at_trim and n == 2:
                page.insert_text((2, 45), "pter 3: Cropped running heading",
                                 fontsize=9)
        if title:
            doc.set_metadata({"title": title, "author": "Book Test"})
        doc.save(out)
        doc.close()
        return out

    def test_print_titles_have_deliberate_unhyphenated_breaks(self):
        volume_one = builder.paperback_cover_title(
            "Volume I — From Bits to Programs"
        )
        volume_two = builder.paperback_cover_title(
            "Volume II — From Programs to GPUs"
        )
        self.assertEqual(
            volume_one,
            r"Volume I\\[0.25em]\mbox{From Bits to Programs}"
        )
        self.assertEqual(
            volume_two,
            r"Volume II\\[0.25em]\mbox{From Programs to GPUs}"
        )
        self.assertEqual(
            builder.paperback_cover_title("From Bits to Parallel Systems"),
            "From Bits to Parallel Systems"
        )
        self.assertIn(
            "pdftitle={Volume I — From Bits to Programs}",
            builder.book_pdf_metadata(
                "Volume I — From Bits to Programs",
                "Representation, Ownership, and Reliable C++23"
            )
        )

    def test_reader_resets_header_width_after_final_geometry(self):
        # The print profile initializes fancyhdr at a different trim size.
        # These directives must remain in this order, or long paperback
        # running heads can extend outside the 7-inch physical page.
        root = Path(__file__).resolve().parents[2]
        style = (root / "Notes/Book Production/Seven-Book Curriculum/Book01"
                 / "book01-reader.sty").read_text("utf-8")
        self.assertIn(r"\RequirePackage{emptypage}", style)
        geometry = style.index(r"\geometry{")
        headwidth = style.index(r"\setlength{\headwidth}{\textwidth}")
        running = style.index(r"\fancyhead[LE]")
        self.assertLess(geometry, headwidth)
        self.assertLess(headwidth, running)

    def test_book_one_series_identity_and_clean_reader_imprint(self):
        # Book 1 is part of a seven-book sequence; print halves are formats.
        from book01_editorial import SERIES_TITLE, SERIES_POSITION, BOOK_TITLE
        self.assertEqual(SERIES_TITLE, "From Bits to Research")
        self.assertEqual(SERIES_POSITION, "Book One of Seven")
        self.assertEqual(BOOK_TITLE, "From Bits to Parallel Systems")
        self.assertEqual(
            builder.book_imprint("Reader Title", "Reader Subtitle"),
            r"\BookImprintPage{Reader Title}{Reader Subtitle}"
        )

        root = Path(__file__).resolve().parents[2]
        style = (root / "Notes/Book Production/Seven-Book Curriculum/Book01"
                 / "book01-reader.sty").read_text("utf-8")
        self.assertIn(r"\newcommand{\BookSeriesTitle}{From Bits to Research}", style)
        self.assertIn(r"\newcommand{\BookImprintPage}[2]", style)
        self.assertIn("github.com/swayam8624/Project_008_AllMight", style)
        for unwanted in ("Academic working proof", "Academic production proof:",
                         "No ISBN or publishing imprint", "Source revision:"):
            self.assertNotIn(unwanted, style)

    def test_reader_bibliography_identifiers_are_real_source_links(self):
        root = Path(__file__).resolve().parents[2]
        path = (root / "Notes/Book Production/Seven-Book Curriculum/Book01"
                / "ACADEMIC_BACKMATTER.md")
        references = path.read_text("utf-8")
        for identifier in (
            "10.1145/103162.103163",
            "10.1145/1498765.1498785",
            "10.1145/1375581.1375591",
            "10.1002/9781394193202",
            "10.1201/B17511",
            "978-0-13-681648-5",
            "github.com/swayam8624/NESEmulator",
            "github.com/swayam8624/Maveb",
        ):
            self.assertIn(identifier, references)

    def test_unrendered_index_and_math_command_block_release(self):
        # A literal TeX command in actual reader text is a production defect.
        with TemporaryDirectory() as temp:
            path = Path(temp) / "raw-tex-leak.pdf"
            document = fitz.open()
            for number in range(14):
                page = document.new_page(width=504, height=720)
                page.insert_text(
                    (78, 120),
                    r"Bad index command: \index{SIMD}" if number == 13
                    else "Valid reader content",
                    fontsize=10,
                )
            document.set_metadata({"title": "Reader", "author": "Test"})
            document.save(path)
            document.close()
            result = proof.inspect(path, paperback=False)
            self.assertIn("raw_tex_leak", result["counts"])
            self.assertGreater(result["fatal_count"], 0)

    def test_valid_minimal_print_pdf(self):
        with TemporaryDirectory() as temp:
            path = self.make_pdf(Path(temp))
            result = proof.inspect(path, paperback=True)
            self.assertEqual(result["pages"], 3)
            self.assertEqual(result["fatal_count"], 0, result["counts"])

    def test_missing_metadata_blocks_release(self):
        with TemporaryDirectory() as temp:
            result = proof.inspect(
                self.make_pdf(Path(temp), title=""), paperback=True)
            self.assertIn("missing_metadata", result["counts"])
            self.assertGreater(result["fatal_count"], 0)

    def test_wrong_paperback_trim_blocks_release(self):
        with TemporaryDirectory() as temp:
            result = proof.inspect(
                self.make_pdf(Path(temp), width=510), paperback=True)
            self.assertIn("trim", result["counts"])
            self.assertGreater(result["fatal_count"], 0)

    def test_sub_eight_point_code_is_still_a_release_blocker(self):
        with TemporaryDirectory() as temp:
            path = self.make_pdf(Path(temp))
            document = fitz.open(path)
            # Courier represents the same physical size check as Menlo,
            # independently of whether the font is installed in CI.
            document[1].insert_text(
                (78, 160), "int main() { return 0; }",
                fontname="cour", fontsize=7.0
            )
            copy = Path(temp) / "tiny-code.pdf"
            document.save(copy)
            document.close()
            result = proof.inspect(copy, paperback=True)
            self.assertIn("tiny_code", result["counts"])
            self.assertGreater(result["fatal_count"], 0)

    def test_nine_point_code_is_not_a_false_positive(self):
        with TemporaryDirectory() as temp:
            path = self.make_pdf(Path(temp))
            document = fitz.open(path)
            document[1].insert_text(
                (78, 160), "int main() { return 0; }",
                fontname="cour", fontsize=9.0
            )
            copy = Path(temp) / "readable-code.pdf"
            document.save(copy)
            document.close()
            result = proof.inspect(copy, paperback=True)
            self.assertNotIn("tiny_code", result["counts"])

    def test_cropped_header_blocks_release(self):
        with TemporaryDirectory() as temp:
            result = proof.inspect(
                self.make_pdf(Path(temp), header_at_trim=True),
                paperback=True)
            self.assertIn("clipped_header", result["counts"])
            self.assertGreater(result["fatal_count"], 0)


if __name__ == "__main__":
    unittest.main()
