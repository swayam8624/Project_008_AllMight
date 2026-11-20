#!/usr/bin/env python3
"""Repair the visible Book Two EPUB cover without changing its cover artwork.

The input EPUB already has correct EPUB3 cover-image metadata and a valid
JPEG. Pandoc emits a cover.xhtml containing an inline SVG <image>, but the
legacy unknown-HTML-tag escaping pass converts that element to '&lt;image'.
Consequently, the first spine document is blank in readers despite the
JPEG being present in the archive.

A standard XHTML <img> with an explicit alt is simpler and better supported
than a nested SVG external-image reference. The corresponding OPF manifest
entry loses its obsolete 'svg' property. All underlying bytes of the cover
JPEG, all mathematical content and all other EPUB members are preserved.
"""
from __future__ import annotations

from copy import copy
from hashlib import sha256
from html import escape, unescape
from pathlib import Path
import os
import posixpath
import re
import tempfile
import urllib.parse
import xml.etree.ElementTree as ET
import zipfile

OPF_NS = "http://www.idpf.org/2007/opf"
XHTML_NS = "http://www.w3.org/1999/xhtml"
ET.register_namespace("", OPF_NS)
ET.register_namespace("dc", "http://purl.org/dc/elements/1.1/")

_COVER_DIV = re.compile(
    rb'(<div\s+id=["\x27]cover-image["\x27][^>]*>)(.*?)(</div>)',
    re.DOTALL,
)


def repair_book02_epub_cover(path: str | Path) -> dict:
    path = Path(path)
    updates: dict[str, bytes] = {}
    with zipfile.ZipFile(path, "r") as original:
        infos = original.infolist()
        if not infos or infos[0].filename != "mimetype" or infos[0].compress_type != zipfile.ZIP_STORED:
            raise RuntimeError("Not a properly ordered EPUB archive")
        opf_path = "EPUB/content.opf"
        opf = ET.fromstring(original.read(opf_path))
        manifest = opf.find(f"{{{OPF_NS}}}manifest")
        spine = opf.find(f"{{{OPF_NS}}}spine")
        if manifest is None or spine is None:
            raise RuntimeError("EPUB manifest or reading spine is missing")
        items = list(manifest)
        cover_assets = [item for item in items
                        if "cover-image" in item.get("properties", "").split()]
        if len(cover_assets) != 1:
            raise RuntimeError("Expected one EPUB3 cover-image item")
        image_item = cover_assets[0]
        image_path = posixpath.normpath(posixpath.join(
            posixpath.dirname(opf_path),
            urllib.parse.unquote(image_item.get("href", ""))))
        if image_path not in original.namelist():
            raise RuntimeError("EPUB cover JPEG referenced by the OPF is missing")
        mime = image_item.get("media-type")
        signatures = {"image/jpeg": b"\xff\xd8", "image/png": b"\x89PNG"}
        image_bytes=original.read(image_path)
        if mime not in signatures or not image_bytes.startswith(signatures[mime]):
            raise RuntimeError("EPUB cover-image is not a usable JPEG/PNG")
        cover_digest=sha256(image_bytes).hexdigest()
        cover_docs = [item for item in items if item.get("media-type") == "application/xhtml+xml"
                      and posixpath.basename(item.get("href", "")) == "cover.xhtml"]
        if len(cover_docs) != 1:
            raise RuntimeError("Expected one cover.xhtml in EPUB manifest")
        cover_item = cover_docs[0]
        spine_items = list(spine)
        if not spine_items or spine_items[0].get("idref") != cover_item.get("id"):
            raise RuntimeError("EPUB cover.xhtml is not the opening spine document")
        page_path = posixpath.normpath(posixpath.join(
            posixpath.dirname(opf_path), urllib.parse.unquote(cover_item.get("href"))))
        source = original.read(page_path)
        if len(_COVER_DIV.findall(source)) != 1:
            raise RuntimeError("Cover XHTML lacks a unique image wrapper")
        expected_src = posixpath.relpath(image_path, posixpath.dirname(page_path))
        found_img = re.findall(rb'<img(?=[\s/>])[^>]*>', source)
        if found_img:
            # Idempotence: a previously repaired edition must still link the
            # correct cover. Never add a duplicate visible image.
            if len(found_img) != 1 or expected_src.encode() not in found_img[0]:
                raise RuntimeError("Existing cover img does not target the EPUB cover asset")
            ET.fromstring(source)
        else:
            wrapper = _COVER_DIV.search(source)
            payload = wrapper.group(2)
            if b"<svg" not in payload or (
                b"&lt;image" not in payload and b"<image" not in payload
            ) or expected_src.encode() not in payload:
                raise RuntimeError("Unexpected SVG cover contents; refusing blind replacement")
            # The cover JPEG itself contains only the title, subtitle and
            # author. This alt text is nonvisual accessibility metadata.
            label = "Cover of From Equations to Geometry by Swayam Singal"
            tag = (
                '<img src="' + escape(expected_src, quote=True) +
                '" alt="' + escape(label, quote=True) +
                '" style="display:block;width:100%;max-width:100%;height:auto;margin:0 auto;" />'
            ).encode("utf-8")
            changed = source[:wrapper.start(2)] + b"\n" + tag + b"\n" + source[wrapper.end(2):]
            doc = ET.fromstring(changed)
            images = [e for e in doc.iter() if e.tag == f"{{{XHTML_NS}}}img"]
            if len(images) != 1 or images[0].get("src") != expected_src:
                raise RuntimeError("Repaired cover XHTML does not display its JPEG")
            updates[page_path] = changed
        # Remove inaccurate manifest feature metadata: the XHTML page no
        # longer embeds SVG, but the independent scientific SVGs are intact.
        prior = cover_item.get("properties", "").split()
        if "svg" in prior:
            remaining = [x for x in prior if x != "svg"]
            if remaining:
                cover_item.set("properties", " ".join(remaining))
            else:
                cover_item.attrib.pop("properties", None)
            updates[opf_path] = ET.tostring(opf, encoding="utf-8", xml_declaration=True)
        if updates:
            with tempfile.NamedTemporaryFile(
                prefix=path.name + ".cover-", suffix=".tmp",
                dir=path.parent, delete=False
            ) as tmp_file:
                tmp = Path(tmp_file.name)
            try:
                with zipfile.ZipFile(tmp, "w") as output:
                    for info in infos:
                        # Copy ZipInfo: zipfile.writestr mutates header_offset.
                        output.writestr(copy(info),
                            updates.get(info.filename, original.read(info.filename)))
                with zipfile.ZipFile(tmp) as check:
                    if check.testzip() is not None or check.namelist() != original.namelist():
                        raise RuntimeError("Cover-repaired EPUB ZIP integrity check failed")
                    for info in infos:
                        if info.filename not in updates and check.read(info.filename) != original.read(info.filename):
                            raise RuntimeError("Unrelated EPUB member was modified: " + info.filename)
                    fixed = ET.fromstring(check.read(page_path))
                    actual_images = [e for e in fixed.iter() if e.tag == f"{{{XHTML_NS}}}img"]
                    if len(actual_images) != 1 or actual_images[0].get("src") != expected_src:
                        raise RuntimeError("Repaired EPUB cover has no visible JPEG element")
                os.replace(tmp, path)
            finally:
                tmp.unlink(missing_ok=True)
        return {
            "visible_cover_image": True,
            "cover_asset": image_path,
            "cover_sha256": cover_digest,
            "cover_first_in_spine": True,
            "modified_members": sorted(updates),
        }



def main() -> int:
    """Fix the EPUB *already in Master*, without rebuilding its chapters.

    An explicit --output path creates a new reviewed edition; the source EPUB
    is never modified. An existing output is protected unless --force is used.
    """
    import argparse
    import json
    import shutil
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("epub", type=Path, help="Existing full Book Two EPUB")
    parser.add_argument("--output", type=Path, required=True,
                        help="New corrected EPUB path, e.g. Book02-Review/")
    parser.add_argument("--force", action="store_true",
                        help="Explicitly replace an existing output file")
    args = parser.parse_args()
    original = args.epub.expanduser().resolve()
    target = args.output.expanduser().resolve()
    if not original.is_file() or not zipfile.is_zipfile(original):
        parser.error("Existing Book Two EPUB file not found: " + str(original))
    if original == target:
        parser.error("The output must be a separate file; original EPUB is read-only")
    if target.exists() and not args.force:
        parser.error("Output exists; use --force to intentionally replace it: " + str(target))
    if not target.parent.is_dir():
        parser.error("Output directory does not exist: " + str(target.parent))
    with tempfile.NamedTemporaryFile(
        prefix=target.name + ".review-", suffix=".epub",
        dir=target.parent, delete=False
    ) as output_file:
        scratch = Path(output_file.name)
    try:
        shutil.copy2(original, scratch)
        report = repair_book02_epub_cover(scratch)
        os.replace(scratch, target)
    finally:
        scratch.unlink(missing_ok=True)
    print("BOOK02_COVER_VISIBLE_OK", json.dumps(report, sort_keys=True))
    print("BOOK02_REVIEWED_EPUB", target)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
