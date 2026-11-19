"""Book Two Pass 4: inspect every supplement and prove semantic EPUB packaging.

No private ~/.codex source cache, MathML cache, full 568-source conversion, or
macOS path is needed. This tests the *real* 36 added LaTeX sources and all 13
scientific SVG figures using pinned NumPy/SciPy/Matplotlib and Pandoc 3.x.

A full original 16-chapter KDP book still requires the author's frozen AST
source cache plus native book production validation; this is an additive
reading-order and image/content parity proof, not that full publication.
"""
from __future__ import annotations

import copy
import hashlib
import json
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import unittest
import zipfile
import xml.etree.ElementTree as ET

from book02_epub_supplements import (
    STAGE, ANALYTIC, RICH, ALTERNATIVES, Book02EpubSupplement,
    FIGURES, RICH_FIGURES, COVERAGE, PRELUDE_CHAPTER, all_source_filenames, walk
)

REPO=Path(__file__).resolve().parents[4]
TOOL=Path(__file__).resolve().parent


class Book02EpubParity(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp=tempfile.TemporaryDirectory()
        cls.root=Path(cls.tmp.name)
        cls.assets=cls.root/"assets"
        cls.assets.mkdir()
        def stage_asset(source,extension):
            path=Path(source)
            blob=path.read_bytes()
            name=hashlib.sha256(blob).hexdigest()+extension
            dest=cls.assets/name
            if not dest.exists():
                dest.write_bytes(blob)
            return "assets/"+name

        cls.supp=Book02EpubSupplement(cls.root,stage_asset)
        cls.chapter_ids=list(FIGURES)
        cls.blocks=[]
        cls.blocks.extend(cls.supp.chapter_blocks(PRELUDE_CHAPTER,"stage",prelude=True))
        cls.original_marker_indices={}
        for chapter in cls.chapter_ids:
            cls.original_marker_indices[chapter]=len(cls.blocks)
            cls.blocks.append({
                "t":"Header",
                "c":[1,[chapter,[],[]],[{"t":"Str","c":"ORIGINAL-"+chapter}]]
            })
            cls.blocks.extend(cls.supp.chapter_blocks(chapter,"stage"))
        cls.used=cls.supp.used

        # Synthetic original-chapter placeholders make the reading-order
        # contract testable without the private 568-row manuscript cache.
        manifest={"pandoc-api-version":[1,23,1],"meta":{},"blocks":cls.blocks}
        cls.json=cls.root/"book02-additive-proof.json"
        cls.json.write_text(json.dumps(manifest,ensure_ascii=False),encoding="utf-8")

    @classmethod
    def tearDownClass(cls):
        cls.tmp.cleanup()

    def test_36_original_source_files_preserved_as_separate_lessons(self):
        names=all_source_filenames()
        self.assertEqual(len(names),36)
        self.assertEqual(len(set(names)),36)
        self.assertEqual([x["source"] for x in self.used],names)
        self.assertTrue(all(x["sha256"] for x in self.used))
        self.assertEqual(len(self.chapter_ids),16)
        self.assertEqual(set(RICH),set(f"{n:02d}-atlas-{v}.tex" for n,v in [
            (28,"orientation"),(29,"geodesic"),(30,"geodesic-3d"),
            (31,"resolution"),(32,"fitting"),(33,"sampling"),(34,"ransac")]))
        self.assertEqual(len(ALTERNATIVES),13)

    def test_original_chapter_positions_not_replaced(self):
        # Each original chapter header appears exactly once, and all
        # supplemental blocks at that boundary follow it before the next.
        for i,chapter in enumerate(self.chapter_ids):
            original=[k for k,b in enumerate(self.blocks) if
                      b.get("t")=="Header" and b["c"][1][0]==chapter]
            self.assertEqual(len(original),1)
            self.assertEqual(original[0],self.original_marker_indices[chapter])
            related=[name for name in (*FIGURES[chapter],
                     *RICH_FIGURES.get(chapter,()))]
            after=self.blocks[original[0]+1:]
            if i+1<len(self.chapter_ids):
                next_original=self.original_marker_indices[self.chapter_ids[i+1]]
                after=self.blocks[original[0]+1:next_original]
            text=json.dumps(after)
            for name in related:
                self.assertIn("book02-"+Path(name).stem,text)
        # Reader contract must appear before original first chapter.
        first=json.dumps(self.blocks[:self.original_marker_indices[self.chapter_ids[0]]])
        self.assertIn("book02-06-reader-contract",first)

    def test_all_scientific_figures_have_captions_and_image_alt(self):
        images=[node for node in walk(self.blocks) if node.get("t")=="Image"]
        figures=[node for node in walk(self.blocks) if node.get("t")=="Figure"]
        self.assertEqual(len(images),13)
        self.assertEqual(len(figures),13)
        self.assertEqual(len(self.supp.figure_paths),13)
        for im in images:
            self.assertTrue(im["c"][1],"Image has no alternative text")
            self.assertTrue(im["c"][2][0].startswith("assets/"))
            self.assertTrue((self.root/im["c"][2][0]).is_file())
        for fig in figures:
            self.assertTrue(fig["c"][1][1], "Missing source-derived caption")
        self.assertFalse(any(n.get("t") in {"RawInline","RawBlock"} and
                             "tikz" in str(n.get("c","")).lower()
                             for n in walk(self.blocks)))

    def test_all_sources_converted_to_meaningful_math_and_prose(self):
        math=[node for node in walk(self.blocks) if node.get("t")=="Math"]
        self.assertGreaterEqual(len(math),30)
        self.assertGreater(len(self.blocks),100)
        for obj in self.used:
            self.assertGreaterEqual(obj["math_nodes"],0)
            self.assertEqual(obj["images"],int(obj["source"] in (
                set(ANALYTIC)|set(RICH)|set(COVERAGE))))
        # Renders semantic text and scientific math; raw tikz is forbidden.
        text=json.dumps(self.blocks,ensure_ascii=False)
        self.assertIn("Geodesic",text)
        self.assertIn("Reconstruction",text)
        self.assertIn("sampling",text.lower())

    def test_other_volume_and_collection_content_not_modified(self):
        # The adapter cannot be given a chapter id outside the Book02 map.
        self.assertEqual(self.supp.chapter_blocks("07-vision-ch999","stage"),[])
        self.assertEqual(self.supp.chapter_blocks("07-vision-ch999","collection"),[])
        # Stage vs full collection only changes heading hierarchy.
        stage=self.supp.chapter_blocks(self.chapter_ids[2],"stage")
        collection=self.supp.chapter_blocks(self.chapter_ids[2],"collection")
        a=next(n for n in walk(stage) if n.get("t")=="Header")
        b=next(n for n in walk(collection) if n.get("t")=="Header")
        self.assertEqual(b["c"][0],a["c"][0]+1)

    def test_proof_epub_reading_order_and_svg_package(self):
        cmd=["pandoc",self.json.name,"--from=json","--to=epub3",
             "--math-method=mathml","--output","Book02-Additive-EPUB-Proof.epub",
             "--metadata","title=Book Two Additive Reading Order Proof",
             "--metadata","author=Swayam Singal","--metadata","lang=en",
             "--split-level=1"]
        p=subprocess.run(cmd,cwd=self.root,text=True,
                         stdout=subprocess.PIPE,stderr=subprocess.PIPE)
        self.assertEqual(p.returncode,0,p.stderr[-3500:])
        epub=self.root/"Book02-Additive-EPUB-Proof.epub"
        self.assertGreater(epub.stat().st_size,80000)
        with zipfile.ZipFile(epub) as z:
            files=z.namelist()
            self.assertEqual(files[0],"mimetype")
            self.assertEqual(z.read("mimetype"),b"application/epub+zip")
            self.assertEqual(z.getinfo("mimetype").compress_type,zipfile.ZIP_STORED)
            svgs=[x for x in files if x.lower().endswith(".svg")]
            self.assertEqual(len(svgs),13)
            xhtml=[x for x in files if x.endswith(".xhtml")]
            self.assertGreater(len(xhtml),2)
            heads=[]
            im_count=0
            alt_count=0
            for filename in xhtml:
                tree=ET.fromstring(z.read(filename))
                for e in tree.iter():
                    if e.tag.endswith(("h1","h2","h3","h4","h5","h6")):
                        heads.append("".join(e.itertext()))
                    if e.tag.endswith("img"):
                        im_count+=1
                        alt_count+=bool(e.attrib.get("alt"))
            self.assertEqual(im_count,13)
            self.assertEqual(alt_count,13)
            self.assertEqual(sum("ORIGINAL-" in x for x in heads),16)
            self.assertTrue(any("Cumulative project" in x for x in heads))
            self.assertTrue(any("Visual laboratory" in x for x in heads))
            self.assertTrue(any("Research reconstruction" in x for x in heads))

if __name__=="__main__":
    unittest.main()
