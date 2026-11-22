"""Regression tests for EPUBCheck RSC-005 duplicate SVG raster-image IDs.

Uses only Python's standard library and tiny synthetic EPUBs. No original
Book Two chapter, PDF, EPUB or image source is rewritten by these tests.
"""
from __future__ import annotations

from pathlib import Path
import tempfile
import unittest
import xml.etree.ElementTree as ET
import zipfile

from book02_svg_id_repair import (
    repair_svg_duplicate_image_ids,
    repair_epub_duplicate_svg_image_ids,
)

SVG_HEAD = b'<svg xmlns="http://www.w3.org/2000/svg" width="12" height="12">'
# This reproduces the relevant shape of the real file40.svg RSC-005 error:
# the same Matplotlib-style image ID appears on three raster nodes.
DUPLICATE_SVG = (
    SVG_HEAD +
    b'<image id="image20cdbf0466" x="0" width="4" href="data:image/png;base64,AAAA"/>'
    b'<image id="image20cdbf0466" x="5" width="4" href="data:image/png;base64,BBBB"/>'
    b'<image id="image20cdbf0466" x="9" width="3" href="data:image/png;base64,CCCC"/>'
    b'</svg>'
)


def write_epub(path: Path, svg: bytes) -> None:
    with zipfile.ZipFile(path, "w") as output:
        output.writestr("mimetype", b"application/epub+zip",
                        compress_type=zipfile.ZIP_STORED)
        output.writestr("META-INF/container.xml", b"<container/>",
                        compress_type=zipfile.ZIP_DEFLATED)
        output.writestr("EPUB/media/file40.svg", svg,
                        compress_type=zipfile.ZIP_DEFLATED)
        output.writestr("EPUB/chapters/chapter1.xhtml",
                        b"<html>Keep all 16 original chapter assets intact</html>",
                        compress_type=zipfile.ZIP_DEFLATED)


class BookTwoSVGIdRepair(unittest.TestCase):
    def test_repeated_unreferenced_image_ids_are_renamed_without_changing_pixels(self):
        out, report = repair_svg_duplicate_image_ids(DUPLICATE_SVG, label="file40.svg")
        self.assertEqual(report, {"image20cdbf0466": 2})
        self.assertEqual(out.count(b'id="image20cdbf0466"'), 1)
        self.assertIn(b'id="image20cdbf0466-book02-2"', out)
        self.assertIn(b'id="image20cdbf0466-book02-3"', out)
        for payload in (b"AAAA", b"BBBB", b"CCCC"):
            self.assertIn(b"data:image/png;base64," + payload, out)
        elements = list(ET.fromstring(out))
        self.assertEqual([e.attrib["x"] for e in elements], ["0", "5", "9"])
        self.assertEqual([e.attrib["width"] for e in elements], ["4", "4", "3"])
        self.assertEqual(len({e.attrib["id"] for e in elements}), 3)
        # The exact original syntax is untouched except for the two strings.
        self.assertEqual(
            out.replace(b"image20cdbf0466-book02-2",b"image20cdbf0466")
               .replace(b"image20cdbf0466-book02-3",b"image20cdbf0466"),
            DUPLICATE_SVG
        )
        again,second=repair_svg_duplicate_image_ids(out)
        self.assertEqual(out,again)
        self.assertEqual(second,{})

    def test_referenced_duplicate_is_not_guessed(self):
        svg=(SVG_HEAD+
             b'<image id="image20cdbf0466" href="data:image/png;base64,AAAA"/>'
             b'<image id="image20cdbf0466" href="data:image/png;base64,BBBB"/>'
             b'<use href="#image20cdbf0466"/>' + b'</svg>')
        with self.assertRaisesRegex(RuntimeError, "referenced"):
            repair_svg_duplicate_image_ids(svg)

    def test_nonimage_duplicate_is_not_modified(self):
        svg=(SVG_HEAD+b'<path id="same" d="M0 0"/>'
             b'<path id="same" d="M1 1"/></svg>')
        with self.assertRaisesRegex(RuntimeError, "not exclusively"):
            repair_svg_duplicate_image_ids(svg)

    def test_epub_repack_preserves_mimetype_order_and_unchanged_members(self):
        with tempfile.TemporaryDirectory() as root:
            path=Path(root)/"book02.epub"
            write_epub(path,DUPLICATE_SVG)
            with zipfile.ZipFile(path) as original:
                names=original.namelist()
                all_before={name:original.read(name) for name in names}
            report=repair_epub_duplicate_svg_image_ids(path)
            self.assertEqual(report["svg_files_repaired"],1)
            self.assertEqual(report["id_renames"],2)
            self.assertEqual(report["details"]["EPUB/media/file40.svg"],
                             {"image20cdbf0466":2})
            with zipfile.ZipFile(path) as completed:
                self.assertEqual(completed.namelist(),names)
                self.assertEqual(completed.getinfo("mimetype").compress_type,
                                 zipfile.ZIP_STORED)
                for name in names:
                    if name.endswith(".svg"):
                        self.assertNotEqual(completed.read(name),all_before[name])
                    else:
                        self.assertEqual(completed.read(name),all_before[name])
            self.assertEqual(repair_epub_duplicate_svg_image_ids(path)["id_renames"],0)

    def test_unreferenced_only_abort_is_atomic(self):
        with tempfile.TemporaryDirectory() as root:
            path=Path(root)/"book02.epub"
            bad=(SVG_HEAD+
                 b'<image id="bad" href="data:image/png;base64,AAAA"/>'
                 b'<image id="bad" href="data:image/png;base64,BBBB"/>'
                 b'<use href="#bad"/></svg>')
            write_epub(path,bad)
            old=path.read_bytes()
            with self.assertRaisesRegex(RuntimeError,"referenced"):
                repair_epub_duplicate_svg_image_ids(path)
            self.assertEqual(path.read_bytes(),old)


if __name__ == "__main__":
    unittest.main()
