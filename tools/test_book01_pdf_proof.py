#!/usr/bin/env python3
"""Small synthetic-PDF checks for the actual KDP publication proof gate."""
from pathlib import Path
from tempfile import TemporaryDirectory
import unittest

import book01_pdf_proof as proof

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

    def test_cropped_header_blocks_release(self):
        with TemporaryDirectory() as temp:
            result = proof.inspect(
                self.make_pdf(Path(temp), header_at_trim=True),
                paperback=True)
            self.assertIn("clipped_header", result["counts"])
            self.assertGreater(result["fatal_count"], 0)


if __name__ == "__main__":
    unittest.main()
