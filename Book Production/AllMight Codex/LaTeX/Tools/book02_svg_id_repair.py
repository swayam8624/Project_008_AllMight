#!/usr/bin/env python3
"""Surgical repair for duplicate *unreferenced* raster-image IDs in EPUB SVGs.

Matplotlib/Pandoc can place multiple <image> nodes bearing the same generated
'id' inside one SVG. This is invalid SVG/XML ID semantics (EPUBCheck RSC-005),
even when the raster images themselves are perfectly valid.

This repair deliberately does NOT rasterize, recreate, parse-and-re-serialize,
or drop any scientific drawing. Only the duplicate id attribute values are
rewritten in the EPUB's SVG byte streams. In particular, image data, geometry,
captions, paths, styles, equations and all source ASTs remain unchanged.

If a duplicate id has any references, is on a non-image element, or cannot be
located unambiguously, FAIL instead of risking a visual change.
"""
from __future__ import annotations

from collections import Counter, defaultdict
from copy import copy
from pathlib import Path
import re
import tempfile
import os
import xml.etree.ElementTree as ET
import zipfile

_IMAGE_TAG = re.compile(rb"<(?:[A-Za-z_][\w.-]*:)?image(?=[\s/>])[^>]*>", re.DOTALL)
_ID_ATTRIBUTE = re.compile(rb"(?<![\w:-])id\s*=\s*([\"'])([^\"']+)\1")


def _svg_id_inventory(svg: bytes, label: str) -> tuple[Counter[str], dict[str, set[str]]]:
    """Parse XML without rewriting it, counting ID owners and element types."""
    try:
        root = ET.fromstring(svg)
    except ET.ParseError as exc:
        raise RuntimeError(f"{label}: invalid SVG XML before repair: {exc}") from exc
    counts: Counter[str] = Counter()
    owners: dict[str, set[str]] = defaultdict(set)
    for element in root.iter():
        ident = element.attrib.get("id")
        if ident is None:
            continue
        counts[ident] += 1
        owners[ident].add(element.tag.rsplit("}", 1)[-1].rsplit(":", 1)[-1])
    return counts, owners


def repair_svg_duplicate_image_ids(svg: bytes, *, label: str = "image.svg") -> tuple[bytes, dict[str, int]]:
    """Rename 2nd+ unreferenced duplicate <image> IDs, preserving SVG pixels.

    Deliberately refuses a referenced ID: a reference to an ambiguous source
    cannot be reassigned safely without knowing which image was intended.
    """
    counts, owners = _svg_id_inventory(svg, label)
    duplicates = {ident: count for ident, count in counts.items() if count > 1}
    if not duplicates:
        return svg, {}
    for ident, count in duplicates.items():
        if owners[ident] != {"image"}:
            raise RuntimeError(
                f"{label}: duplicated SVG id {ident!r} appears on "
                f"{sorted(owners[ident])}, not exclusively <image> nodes"
            )
        raw_id = ident.encode("utf-8")
        # The declaration count is the ONLY permitted use of this identifier.
        # This detects href="#id", url(#id), selectors, ARIA references, etc.,
        # even if they appear in stylesheet CDATA or a different namespace.
        if svg.count(raw_id) != count:
            raise RuntimeError(
                f"{label}: duplicated SVG id {ident!r} is referenced or appears "
                "outside its image-id declarations; refusing unsafe rewrite"
            )
    original_ids = set(counts)
    seen: Counter[str] = Counter()
    changes: Counter[str] = Counter()
    used = set(original_ids)

    def patch_image_tag(match: re.Match[bytes]) -> bytes:
        tag = match.group(0)
        attr = _ID_ATTRIBUTE.search(tag)
        if not attr:
            return tag
        ident = attr.group(2).decode("utf-8")
        if ident not in duplicates:
            return tag
        seen[ident] += 1
        if seen[ident] == 1:
            return tag
        # Namespace-legal, deterministic suffix. Never reuse an existing id.
        i = seen[ident]
        replacement = f"{ident}-book02-{i}"
        while replacement in used:
            i += 1
            replacement = f"{ident}-book02-{i}"
        used.add(replacement)
        changes[ident] += 1
        return tag[:attr.start(2)] + replacement.encode("utf-8") + tag[attr.end(2):]

    repaired = _IMAGE_TAG.sub(patch_image_tag, svg)
    for ident, count in duplicates.items():
        if seen[ident] != count or changes[ident] != count - 1:
            raise RuntimeError(
                f"{label}: matched {seen[ident]} <image> declarations of "
                f"{count} for {ident!r}; refusing incomplete SVG repair"
            )
    final_counts, _ = _svg_id_inventory(repaired, label)
    if any(n > 1 for n in final_counts.values()):
        raise RuntimeError(f"{label}: duplicate SVG IDs remain after repair")
    # This must not be a rasterization or reconstruction. All non-id bytes are
    # unchanged, and the resulting XML is syntactically well-formed.
    return repaired, dict(changes)


def repair_epub_duplicate_svg_image_ids(epub_path: str | Path) -> dict:
    """Atomically update only affected EPUB SVG zip members.

    Preserve the original member ordering, compression settings, zip metadata
    and EPUB-required mimetype-first/uncompressed invariants.
    """
    epub_path = Path(epub_path)
    updates: dict[str, bytes] = {}
    fixes: dict[str, dict[str, int]] = {}
    with zipfile.ZipFile(epub_path, "r") as src:
        names = src.namelist()
        if not names or names[0] != "mimetype" or src.getinfo("mimetype").compress_type != zipfile.ZIP_STORED:
            raise RuntimeError("Refusing SVG repair: EPUB mimetype is not first and uncompressed")
        for info in src.infolist():
            if not info.filename.lower().endswith(".svg"):
                continue
            original = src.read(info.filename)
            repaired, renamed = repair_svg_duplicate_image_ids(
                original, label=info.filename)
            if renamed:
                updates[info.filename] = repaired
                fixes[info.filename] = renamed
        if not updates:
            return {"svg_files_repaired": 0, "id_renames": 0, "details": {}}
        # Write under the same parent to make the replace filesystem-atomic.
        with tempfile.NamedTemporaryFile(
            prefix=epub_path.name + ".svgfix-", suffix=".tmp",
            dir=epub_path.parent, delete=False) as temp:
            temp_path = Path(temp.name)
        try:
            with zipfile.ZipFile(temp_path, "w") as dst:
                for info in src.infolist():
                    # zipfile.writestr updates ZipInfo.header_offset in-place.
                    # The same ZipInfo belongs to the still-open source ZIP;
                    # passing it directly corrupts subsequent src.read calls.
                    dst.writestr(copy(info),
                                 updates.get(info.filename, src.read(info.filename)))
            with zipfile.ZipFile(temp_path, "r") as verify:
                if verify.namelist() != names:
                    raise RuntimeError("EPUB member order changed while repairing SVG")
                for name in names:
                    if name not in updates and verify.read(name) != src.read(name):
                        raise RuntimeError("Unrelated EPUB member changed: " + name)
            os.replace(temp_path, epub_path)
        finally:
            temp_path.unlink(missing_ok=True)
    return {"svg_files_repaired":len(updates),
            "id_renames":sum(sum(c.values()) for c in fixes.values()),
            "details":fixes}
