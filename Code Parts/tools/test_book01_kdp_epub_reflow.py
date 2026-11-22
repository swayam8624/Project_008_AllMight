#!/usr/bin/env python3
"""Check that Book One's long-MathML layout safeguard never edits content."""
from __future__ import annotations

from pathlib import Path
from tempfile import TemporaryDirectory
import unittest
import zipfile
from xml.etree import ElementTree as ET

import build_book01_kdp_epub as book


class EbookReflowTests(unittest.TestCase):
    def test_display_math_wrapped_without_editing_code_or_math(self):
        sample = (
            '<?xml version="1.0" encoding="utf-8"?>'
            '<html xmlns="http://www.w3.org/1999/xhtml">'
            '<body><p>Long mathematical pipeline '
            '<math xmlns="http://www.w3.org/1998/Math/MathML" display="block">'
            '<mrow><mi>source</mi><mo>→</mo><mi>GPU</mi></mrow>'
            '</math></p><pre>int x = 2 &lt; 3;</pre></body></html>'
        )
        with TemporaryDirectory() as folder:
            epub = Path(folder) / "proof.epub"
            with zipfile.ZipFile(epub, "w") as out:
                out.writestr("mimetype", "application/epub+zip",
                             compress_type=zipfile.ZIP_STORED)
                out.writestr("EPUB/text/ch001.xhtml", sample)
                out.writestr("EPUB/styles/stylesheet1.css", "body { line-height: 1.4; }")
            wrapped = book.contain_wide_epub_content(epub)
            self.assertEqual(wrapped, 1)
            with zipfile.ZipFile(epub) as z:
                self.assertIsNone(z.testzip())
                self.assertEqual(z.namelist()[0], "mimetype")
                before = ET.fromstring(sample)
                after_bytes = z.read("EPUB/text/ch001.xhtml")
                after = ET.fromstring(after_bytes)
                self.assertEqual("".join(before.itertext()),
                                 "".join(after.itertext()))
                self.assertIn(b'math-scroll-container', after_bytes)
                self.assertIn(b'int x = 2 &lt; 3;', after_bytes)
                self.assertIn(b"overflow-x: auto",
                              z.read("EPUB/styles/stylesheet1.css"))


if __name__ == "__main__":
    unittest.main()
