"""Safety and integrity regressions for the seven-book publication library."""
from __future__ import annotations

import contextlib
from io import StringIO
from pathlib import Path
import tempfile
import unittest
import xml.etree.ElementTree as ET
import zipfile
from types import SimpleNamespace

from tools import publication_library as pl


def fixture_epub(file: Path, title="From Equations to Geometry",
                 cover=True) -> None:
    xml=(
        '<?xml version="1.0" encoding="utf-8"?>'
        '<container xmlns="urn:oasis:names:tc:opendocument:xmlns:container" version="1.0">'
        '<rootfiles><rootfile full-path="EPUB/content.opf" '
        'media-type="application/oebps-package+xml"/></rootfiles></container>'
    )
    items=(
        '<item id="page" href="text/cover.xhtml" media-type="application/xhtml+xml"/>'
        + (
            '<item id="cover" href="media/cover.jpg" media-type="image/jpeg" properties="cover-image"/>'
            if cover else ""
        )
    )
    opf=(
        '<?xml version="1.0" encoding="utf-8"?>'
        '<package xmlns="http://www.idpf.org/2007/opf" version="3.0">'
        '<metadata><dc:title xmlns:dc="http://purl.org/dc/elements/1.1/">'
        + title + '</dc:title></metadata><manifest>'+items
        +'</manifest><spine><itemref idref="page"/></spine></package>'
    )
    with zipfile.ZipFile(file,'w') as z:
        z.writestr("mimetype","application/epub+zip",compress_type=zipfile.ZIP_STORED)
        z.writestr("META-INF/container.xml",xml,compress_type=zipfile.ZIP_DEFLATED)
        z.writestr("EPUB/content.opf",opf,compress_type=zipfile.ZIP_DEFLATED)
        z.writestr("EPUB/text/cover.xhtml",
                   '<html xmlns="http://www.w3.org/1999/xhtml"><body>'
                   '<img src="../media/cover.jpg" alt="Book cover"/></body></html>',
                   compress_type=zipfile.ZIP_DEFLATED)
        z.writestr("EPUB/media/cover.jpg",b"\xff\xd8author-art\xff\xd9",
                   compress_type=zipfile.ZIP_DEFLATED)


def fixture_pdf(file: Path) -> None:
    file.write_bytes(b"%PDF-1.7\n"+b"x"*95000+b"\n%%EOF\n")


def call_silent(fn,*args,**kwargs):
    with contextlib.redirect_stdout(StringIO()):
        return fn(*args,**kwargs)


class PublicationLibrarySafety(unittest.TestCase):
    def setUp(self):
        self.tmp=tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.master=Path(self.tmp.name)/"Master"
        self.master.mkdir()
        self.library=self.master/pl.LIBRARY_NAME
        self.review=self.master/"Book02-Review"
        self.review.mkdir()

    def args(self,**kwargs):
        return SimpleNamespace(master=self.master,library=self.library,**kwargs)

    def test_initialization_creates_only_neat_output_structure_and_is_idempotent(self):
        call_silent(pl.init_library,self.library)
        self.assertTrue((self.library/"EPUB").is_dir())
        self.assertTrue((self.library/"PDF").is_dir())
        self.assertTrue((self.library/"Archive").is_dir())
        self.assertTrue((self.library/"Manifests"/"publication-manifest.json").is_file())
        self.assertIn("FROM BITS TO RESEARCH",
                      (self.library/"README.md").read_text())
        original=pl.sha256(self.library/"README.md")
        call_silent(pl.init_library,self.library)
        self.assertEqual(original,pl.sha256(self.library/"README.md"))

    def test_scan_is_read_only_and_identifies_legacy_stage_exports(self):
        source=self.master/"Notes/Book Production/AllMight Codex/LaTeX/Books/Ebooks"
        source.mkdir(parents=True)
        fixture_epub(source/"02-Mathematics-for-Visual-Computing.epub")
        fixture_epub(self.review/"From Equations to Geometry - Final Reader Edition.epub")
        phys=self.master/"Notes/Book Production/Parts"
        phys.mkdir(parents=True)
        fixture_pdf(phys/"02-Mathematics-for-Visual-Computing-P01.pdf")
        inventory=pl.inventory(self.master,self.library)
        self.assertEqual(len(inventory["02"]["epub"]),2)
        self.assertTrue(inventory["02"]["pdf"][0]["physical_part_or_preview"])
        self.assertFalse(self.library.exists())

    def test_dry_run_does_not_create_or_move_artifacts(self):
        src=self.review/"From Equations to Geometry - Final Reader Edition.epub"
        fixture_epub(src)
        args=self.args(book=2,epub=src,pdf=None,replace=False,
                       move_source=False,apply=False)
        call_silent(pl.promote,args)
        self.assertTrue(src.exists())
        self.assertFalse(self.library.exists())

    def test_explicit_promotion_maintains_exactly_one_epub_and_pdf_and_manifest(self):
        epub=self.review/"From Equations to Geometry - Final Reader Edition.epub"
        pdf=self.review/"From Equations to Geometry.pdf"
        fixture_epub(epub)
        fixture_pdf(pdf)
        args=self.args(book=2,epub=epub,pdf=pdf,replace=False,
                       move_source=False,apply=True)
        call_silent(pl.promote,args)
        target_epub=self.library/"EPUB"/pl.canonical(2,"epub")
        target_pdf=self.library/"PDF"/pl.canonical(2,"pdf")
        self.assertTrue(target_epub.exists())
        self.assertTrue(target_pdf.exists())
        self.assertEqual(pl.sha256(target_epub),pl.sha256(epub))
        self.assertEqual(pl.sha256(target_pdf),pl.sha256(pdf))
        manifest=pl.manifest_read(self.library)
        self.assertEqual(manifest["books"]["02"]["epub"]["sha256"],pl.sha256(epub))
        self.assertEqual(manifest["books"]["02"]["pdf"]["sha256"],pl.sha256(pdf))
        call_silent(pl.verify,self.args())

    def test_replacement_requires_explicit_flag_and_archives_previous(self):
        first=self.review/"From Equations to Geometry - First.epub"
        later=self.review/"From Equations to Geometry - Revised.epub"
        fixture_epub(first)
        fixture_epub(later,title="From Equations to Geometry")
        # Identical EPUB by default; introduce different (valid) stored cover bytes.
        with zipfile.ZipFile(later,mode="a") as z:
            z.writestr("EPUB/media/extra-reader-asset.png",b"irrelevant")
        args=self.args(book=2,epub=first,pdf=None,replace=False,
                       move_source=False,apply=True)
        call_silent(pl.promote,args)
        second=self.args(book=2,epub=later,pdf=None,replace=False,
                         move_source=False,apply=True)
        with self.assertRaisesRegex(ValueError,"DIFFERENT bytes"):
            call_silent(pl.promote,second)
        second.replace=True
        call_silent(pl.promote,second)
        target=self.library/"EPUB"/pl.canonical(2,"epub")
        self.assertEqual(pl.sha256(target),pl.sha256(later))
        previous=list((self.library/"Archive"/"EPUB"/"Superseded").glob("*.epub"))
        self.assertEqual(len(previous),1)
        self.assertEqual(pl.sha256(previous[0]),pl.sha256(first))
        call_silent(pl.verify,self.args())

    def test_exact_duplicate_tidy_quarantines_only_untracked_review_export(self):
        src=self.review/"From Equations to Geometry - Reader Edition.epub"
        fixture_epub(src)
        call_silent(pl.promote,self.args(
            book=2,epub=src,pdf=None,replace=False,move_source=False,apply=True))
        call_silent(pl.tidy,self.args(apply=False))
        self.assertTrue(src.exists())
        call_silent(pl.tidy,self.args(apply=True))
        self.assertFalse(src.exists())
        targets=list((self.library/"Archive"/"EPUB"/"Duplicate Review Copies").glob("*.epub"))
        self.assertEqual(len(targets),1)
        self.assertEqual(pl.sha256(targets[0]),
                         pl.sha256(self.library/"EPUB"/pl.canonical(2,"epub")))
        call_silent(pl.verify,self.args())

    def test_physical_print_part_is_rejected_not_mistaken_for_unified_pdf(self):
        part=self.review/"02-Mathematics-for-Visual-Computing-P01.pdf"
        fixture_pdf(part)
        with self.assertRaisesRegex(ValueError,"physical part"):
            pl.validate_candidate(part,2,"pdf")
        self.assertFalse(self.library.exists())

    def test_cross_book_or_invalid_cover_not_promoted(self):
        wrong=self.review/"From Equations to Geometry - Wrong.epub"
        fixture_epub(wrong,title="From Bits to Parallel Systems")
        with self.assertRaisesRegex(ValueError,"does not match Book"):
            pl.validate_candidate(wrong,2,"epub")
        broken=self.review/"From Equations to Geometry - Missing Cover.epub"
        fixture_epub(broken,cover=False)
        with self.assertRaisesRegex(ValueError,"cover image"):
            pl.validate_candidate(broken,2,"epub")

    def test_quarantine_never_touches_generated_or_nonreview_sources(self):
        generated=self.master/"Notes/Book Production/AllMight Codex/LaTeX/Build"/"run"
        generated.mkdir(parents=True)
        internal=generated/"From Equations to Geometry.epub"
        fixture_epub(internal)
        with self.assertRaisesRegex(ValueError,"Only original non-source"):
            pl.safe_archive_review(self.master,self.library,internal,
                                   "EPUB/Review",apply=False)
        self.assertTrue(internal.exists())


if __name__=="__main__":
    unittest.main()
