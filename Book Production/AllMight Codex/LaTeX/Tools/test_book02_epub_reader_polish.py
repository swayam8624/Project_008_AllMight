"""Book Two EPUB reader-polish contracts, independent of the private cache.

The full source EPUB still needs the author's Mac to generate. These tests
audit SVG/EPUB safety and the image/TOC transformations before publication.
"""
from __future__ import annotations
import hashlib
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch
import xml.etree.ElementTree as ET
import zipfile

from book02_epub_reader_polish import (
    ALT_BY_FILENAME,_alt_fix_bytes,_prune_ul,_prune_ncx,_collect_nav,
)

class BookTwoReaderPolish(unittest.TestCase):
    def test_complete_reviewed_figure_inventory_has_unique_sha_guards(self):
        self.assertEqual(len(ALT_BY_FILENAME),51)
        digests=[value[0] for value in ALT_BY_FILENAME.values()]
        self.assertEqual(len(set(digests)),51)
        self.assertTrue(all(len(d)==16 for d in digests))
        self.assertTrue(all(len(alt)>=35 and alt.endswith(".")
                            for _,alt in ALT_BY_FILENAME.values()))
        self.assertEqual(set(ALT_BY_FILENAME)&{"file0.png","file64.png"},
                         {"file0.png","file64.png"})

    def test_only_weak_alt_is_replaced_and_image_data_is_unchanged(self):
        image=b"\x89PNG\r\n\x1a\nstandalone synthetic test bytes"
        prefix=hashlib.sha256(image).hexdigest()[:16]
        xhtml=b'<html><body><p><img src="../media/file777.png" alt="Diagram"/></p></body></html>'
        with tempfile.TemporaryDirectory() as temp:
            path=Path(temp)/"sample.epub"
            with zipfile.ZipFile(path,"w") as z:
                z.writestr("EPUB/media/file777.png",image)
            with zipfile.ZipFile(path) as z,patch.dict(ALT_BY_FILENAME,{
                "file777.png":[prefix,"Diagram of a test image with a mathematical relationship."]
            }):
                seen=set()
                result=_alt_fix_bytes(xhtml,"EPUB/text/ch006.xhtml",z,seen)
                self.assertIn(b'alt="Diagram of a test image with a mathematical relationship."',result)
                self.assertEqual(seen,{"EPUB/media/file777.png"})
                self.assertEqual(_alt_fix_bytes(result,"EPUB/text/ch006.xhtml",z,set()),result)
                with self.assertRaisesRegex(RuntimeError,"image .* changed"):
                    with patch.dict(ALT_BY_FILENAME,{
                        "file777.png":["0"*16,"Incorrect description."]
                    }):
                        _alt_fix_bytes(xhtml,"EPUB/text/ch006.xhtml",z,set())
                with self.assertRaisesRegex(RuntimeError,"Unreviewed weak image"):
                    _alt_fix_bytes(b'<img src="../media/file999.png" alt="Diagram" />',
                                   "EPUB/text/ch006.xhtml",z,set())

    def test_reduces_toc_only_not_content(self):
        root=ET.fromstring('''<html><body><nav id="toc">
          <ol><li><a href="chapter.xhtml">Chapter</a>
           <ol><li><a href="chapter.xhtml#section">Section</a>
            <ol><li><a href="chapter.xhtml#subsection">Subsection</a></li></ol>
           </li></ol>
          </li></ol></nav></body></html>''')
        nav=_collect_nav(root)
        first=[n for n in nav.iter() if n.tag=="li"]
        self.assertEqual(len(first),3)
        removed=_prune_ul(next(n for n in nav if n.tag=="ol"))
        self.assertEqual(removed,1)
        self.assertEqual(len([n for n in nav.iter() if n.tag=="li"]),2)
        self.assertIn('chapter.xhtml#section',ET.tostring(nav,encoding="unicode"))
        self.assertNotIn('chapter.xhtml#subsection',ET.tostring(nav,encoding="unicode"))

    def test_fallback_ncx_preserves_book_and_sections(self):
        root=ET.fromstring('''<ncx><navMap>
         <navPoint id="first"><navLabel/><content src="a"/>
          <navPoint id="second"><content src="b"/>
           <navPoint id="third"><content src="c"/></navPoint>
          </navPoint>
         </navPoint></navMap></ncx>''')
        nav=next(n for n in root.iter() if n.tag=="navMap")
        self.assertEqual(_prune_ncx(nav),1)
        self.assertEqual(len([n for n in root.iter() if n.tag=="navPoint"]),2)
        self.assertIn('src="b"',ET.tostring(root,encoding="unicode"))
        self.assertNotIn('src="c"',ET.tostring(root,encoding="unicode"))

if __name__=="__main__":
    unittest.main()
