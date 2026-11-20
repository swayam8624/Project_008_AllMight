"""Regression test: the Book Two cover must actually display in EPUB readers.

An EPUB can pass EPUBCheck with a JPEG in its OPF but have a blank opening
page: Pandoc's inline SVG <image> was accidentally escaped to '&lt;image'.
The contract verifies real XHTML display markup, image provenance, OPF
manifest, spine order, safe archive repacking, and repair idempotence.
"""
from __future__ import annotations

from pathlib import Path
import tempfile
import unittest
import xml.etree.ElementTree as ET
import zipfile

from book02_epub_cover_repair import repair_book02_epub_cover

OPF="http://www.idpf.org/2007/opf"
XHTML="http://www.w3.org/1999/xhtml"
COVER=b"\xff\xd8\xff\xe0\x00\x10JFIF\x00\x01\x02test-jpeg-source-preserved"
OPF_XML=b"""<?xml version="1.0" encoding="utf-8"?>
<package xmlns="http://www.idpf.org/2007/opf" version="3.0" unique-identifier="id">
<metadata><meta name="cover" content="file65_jpg"/></metadata>
<manifest>
 <item id="cover_xhtml" href="text/cover.xhtml" media-type="application/xhtml+xml" properties="svg"/>
 <item id="file65_jpg" href="media/file65.jpg" media-type="image/jpeg" properties="cover-image"/>
 <item id="ch001" href="text/ch001.xhtml" media-type="application/xhtml+xml"/>
</manifest>
<spine><itemref idref="cover_xhtml"/><itemref idref="ch001"/></spine>
</package>"""
BLANK=b"""<?xml version="1.0" encoding="utf-8"?>
<html xmlns="http://www.w3.org/1999/xhtml"><head><title>From Equations to Geometry</title></head>
<body id="cover"><div id="cover-image">
<svg xmlns="http://www.w3.org/2000/svg" width="100%" height="100%" viewBox="0 0 1600 2560">
&lt;image width="1600" height="2560" xlink:href="../media/file65.jpg" />
</svg>
</div></body></html>"""
CHAPTER=b"""<html xmlns="http://www.w3.org/1999/xhtml"><body>
<p>Do not change the original Book Two chapter or its MathML.</p></body></html>"""


def fixture(path: Path, *, cover:bytes=BLANK, opf:bytes=OPF_XML) -> None:
    with zipfile.ZipFile(path,"w") as epub:
        epub.writestr("mimetype","application/epub+zip",
                      compress_type=zipfile.ZIP_STORED)
        epub.writestr("EPUB/content.opf",opf,compress_type=zipfile.ZIP_DEFLATED)
        epub.writestr("EPUB/text/cover.xhtml",cover,compress_type=zipfile.ZIP_DEFLATED)
        epub.writestr("EPUB/media/file65.jpg",COVER,
                      compress_type=zipfile.ZIP_DEFLATED)
        epub.writestr("EPUB/text/ch001.xhtml",CHAPTER,
                      compress_type=zipfile.ZIP_DEFLATED)


class BookTwoVisibleCover(unittest.TestCase):
    def test_blank_pandoc_svg_cover_becomes_visible_native_xhtml_image(self):
        with tempfile.TemporaryDirectory() as t:
            path=Path(t)/"book02.epub"
            fixture(path)
            with zipfile.ZipFile(path) as original:
                originals={name:original.read(name) for name in original.namelist()}
            result=repair_book02_epub_cover(path)
            self.assertTrue(result["visible_cover_image"])
            self.assertEqual(set(result["modified_members"]),
                             {"EPUB/text/cover.xhtml","EPUB/content.opf"})
            with zipfile.ZipFile(path) as fixed:
                self.assertIsNone(fixed.testzip())
                self.assertEqual(fixed.getinfo("mimetype").compress_type,
                                 zipfile.ZIP_STORED)
                for name,blob in originals.items():
                    if name not in result["modified_members"]:
                        self.assertEqual(fixed.read(name),blob)
                page=fixed.read("EPUB/text/cover.xhtml")
                doc=ET.fromstring(page)
                self.assertNotIn(b"&lt;image",page)
                imgs=[n for n in doc.iter()
                      if n.tag==f"{{{XHTML}}}img"]
                self.assertEqual(len(imgs),1)
                self.assertEqual(imgs[0].get("src"),"../media/file65.jpg")
                self.assertIn("From Equations to Geometry",imgs[0].get("alt"))
                opf=ET.fromstring(fixed.read("EPUB/content.opf"))
                cover_item=next(n for n in opf.iter()
                                if n.get("id")=="cover_xhtml")
                self.assertNotIn("svg",cover_item.get("properties","").split())
                jpeg_item=next(n for n in opf.iter()
                               if n.get("id")=="file65_jpg")
                self.assertIn("cover-image",jpeg_item.get("properties","").split())
            again=repair_book02_epub_cover(path)
            self.assertEqual(again["modified_members"],[])

    def test_missing_cover_asset_does_not_modify_original_epub(self):
        with tempfile.TemporaryDirectory() as t:
            path=Path(t)/"bad.epub"
            # Deliberately point metadata to a non-existent media asset.
            fixture(path,opf=OPF_XML.replace(b'media/file65.jpg',
                                           b'media/missing.jpg'))
            before=path.read_bytes()
            with self.assertRaisesRegex(RuntimeError,"missing"):
                repair_book02_epub_cover(path)
            self.assertEqual(path.read_bytes(),before)

    def test_unrecognized_cover_wrapper_fails_closed(self):
        with tempfile.TemporaryDirectory() as t:
            path=Path(t)/"bad.epub"
            fixture(path,cover=BLANK.replace(b'&lt;image',b'&lt;random-art'))
            before=path.read_bytes()
            with self.assertRaisesRegex(RuntimeError,"Unexpected SVG cover"):
                repair_book02_epub_cover(path)
            self.assertEqual(path.read_bytes(),before)


if __name__=="__main__":
    unittest.main()
