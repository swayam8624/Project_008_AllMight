#!/usr/bin/env python3
"""Independent read-only preflight of split print PDFs and source assignment."""
import argparse
import collections
import hashlib
import json
import re
import sys
import unicodedata
from pathlib import Path
from urllib.parse import unquote

from pypdf import PdfReader

EXPECTED_CHAPTERS = 129
EXPECTED_COMPANIONS = 439
EXPECTED_STAGE_COUNT = 7
TARGET_BOX = (576.0, 720.0)  # KDP 8 x 10 in, in PDF points.
BOX_TOLERANCE = 0.75
INVISIBLE_FORMAT_ALLOWLIST = {0x200B, 0x200C, 0x200D, 0x2060, 0xFEFF}


def obj(value):
    try:
        return value.get_object()
    except AttributeError:
        return value


def key(value):
    return str(value) if value is not None else ""


def page_destinations(reader):
    names = set()
    for name, destination in reader.named_destinations.items():
        names.add(str(name).lstrip("/#"))
    return names


class PdfObjectCache:
    """Cache PDF readers and named destinations for one preflight run.

    The key includes resolved path, size, and nanosecond mtime so replacing a
    PDF during an interactive QA session cannot reuse stale destinations.
    """

    def __init__(self):
        self._readers = {}
        self._destinations = {}

    @staticmethod
    def key(path):
        resolved = Path(path).resolve()
        stat = resolved.stat()
        return str(resolved), stat.st_size, stat.st_mtime_ns

    def reader(self, path):
        cache_key = self.key(path)
        if cache_key not in self._readers:
            self._readers[cache_key] = PdfReader(cache_key[0], strict=False)
        return self._readers[cache_key]

    def destinations(self, path, reader=None):
        cache_key = self.key(path)
        if cache_key not in self._destinations:
            reader = reader or self.reader(path)
            self._destinations[cache_key] = page_destinations(reader)
        return self._destinations[cache_key]


def font_report(reader):
    """Walk page and nested Form XObject resources, checking embedded programs."""
    found = []
    seen_resources = set()

    def visit_resources(resources, page_number):
        resources = obj(resources)
        if not resources:
            return
        indirect = getattr(resources, "indirect_reference", None)
        ident = ((getattr(indirect, "idnum", None), getattr(indirect, "generation", None))
                 if indirect is not None else id(resources))
        if ident in seen_resources:
            return
        seen_resources.add(ident)

        fonts = obj(resources.get("/Font", {}))
        for font_name, font_ref in fonts.items():
            font = obj(font_ref)
            base = key(font.get("/BaseFont"))
            subtype = key(font.get("/Subtype"))
            descriptors = []
            descriptor = font.get("/FontDescriptor")
            if descriptor:
                descriptors.append(obj(descriptor))
            for descendant_ref in obj(font.get("/DescendantFonts", [])) or []:
                descendant = obj(descendant_ref)
                desc = descendant.get("/FontDescriptor")
                if desc:
                    descriptors.append(obj(desc))
            embedded = any(any(d.get(k) for k in ("/FontFile", "/FontFile2", "/FontFile3")) for d in descriptors)
            if subtype == "/Type3":
                embedded = bool(font.get("/CharProcs"))
            found.append({"page": page_number, "resource": key(font_name), "base_font": base,
                          "subtype": subtype, "embedded": bool(embedded),
                          "embedding_kind": ("Type3 CharProcs" if subtype == "/Type3" and embedded else
                                             next((k for d in descriptors for k in ("/FontFile", "/FontFile2", "/FontFile3") if d.get(k)), None))})

        xobjects = obj(resources.get("/XObject", {}))
        for xref in xobjects.values():
            xo = obj(xref)
            if key(xo.get("/Subtype")) == "/Form" and xo.get("/Resources"):
                visit_resources(xo.get("/Resources"), page_number)

    for index, page in enumerate(reader.pages, 1):
        visit_resources(page.get("/Resources"), index)
    return found


def destination_name(destination):
    destination = obj(destination)
    if isinstance(destination, str):
        name = destination.lstrip("/#")
        if name.startswith("nameddest="):
            name = unquote(name.removeprefix("nameddest="))
        return name
    if hasattr(destination, "get"):
        name = destination.get("/D")
        if name is not None:
            return destination_name(name)
    if isinstance(destination, (list, tuple)) and destination:
        return destination_name(destination[0])
    return key(destination).lstrip("/#")


def check_local_destination(reader, destination, named_destinations=None):
    destination = obj(destination)
    if isinstance(destination, (list, tuple)) and destination:
        page_ref = obj(destination[0])
        # Remote GoTo actions may encode the page as a zero-based integer
        # rather than an indirect page object.
        if isinstance(page_ref, int) and not isinstance(page_ref, bool):
            if 0 <= page_ref < len(reader.pages):
                return "valid_local_page_destination", {"page": page_ref + 1}
            return "unresolved_local_destination", {"page_index": page_ref}
        try:
            page = reader.get_page_number(page_ref) + 1
            return "valid_local_page_destination", {"page": page}
        except Exception:
            pass
    name = destination_name(destination)
    if name in (named_destinations if named_destinations is not None else page_destinations(reader)):
        return "valid_local_named_destination", {"destination": name}
    return "unresolved_local_destination", {"destination": name}


def normalized_heading_text(value):
    """Normalize extracted PDF title text for a conservative page match."""
    # Source catalog titles may carry a leading ordinal (for example,
    # ``05 - Reflectance...``), while the emitted chapter heading uses the
    # book's logical chapter number and omits that source-file prefix.
    value = re.sub(r"^\s*\d+\s*[-.:]\s*", "", str(value), count=1)
    return "".join(ch.casefold() for ch in value if ch.isalnum())


def chapter_heading_destination_report(reader, chapters, part_id):
    """Check chapter anchors land on the PDF page that actually contains each title."""
    named = reader.named_destinations
    heading_text_by_page = []
    for page in reader.pages:
        heading_chunks = []
        def collect_large_text(text, cm, tm, font, font_size):
            # Chapter titles are set in display type. Running heads repeat the
            # same title in body-sized text, so they cannot prove heading-page
            # ownership even when the stable destination lands there.
            if text and font_size >= 14:
                heading_chunks.append(text)
        page.extract_text(visitor_text=collect_large_text)
        heading_text_by_page.append("".join(heading_chunks))
    result = []
    for chapter in chapters:
        chapter_id, title = chapter.get("id"), chapter.get("title")
        # Portable cross-part URLs use the stable chapter ID as their named destination.
        # TeX labels and input-file aliases may exist on other pages and are not substitutes.
        anchor_name = str(chapter_id)
        anchor = named.get(anchor_name)
        expected = normalized_heading_text(title or "")
        title_pages = [i + 1 for i, text in enumerate(heading_text_by_page)
                       if expected and expected in normalized_heading_text(text)]
        entry = {"chapter_id": chapter_id, "title": title, "part_id": part_id,
                 "anchor_name": anchor_name if anchor is not None else None,
                 "title_pages": title_pages}
        if anchor is None:
            entry.update({"status": "missing_stable_chapter_destination", "anchor_page": None})
        else:
            try:
                entry["anchor_page"] = reader.get_page_number(obj(anchor.page)) + 1
            except Exception:
                entry["anchor_page"] = None
            if not expected or not title_pages:
                entry["status"] = "chapter_heading_page_unverified"
            elif entry["anchor_page"] in title_pages:
                # Running heads repeat chapter titles on many later pages. The
                # destination is verified when the actual heading page is among
                # the matching pages, even if those headers add more matches.
                entry["status"] = "destination_matches_heading_page"
            elif len(title_pages) > 1:
                entry["status"] = "chapter_title_matches_multiple_pages"
            else:
                entry["status"] = "destination_not_on_heading_page"
        result.append(entry)
    return result


def parse_front_navigation(tex):
    """Read only the generated chapter-front metadata navigation paragraph."""
    nav_line = next((line for line in tex.splitlines()
                     if "Reading map" in line and ("Previous chapter" in line or "Next chapter" in line)), "")
    destinations = {}
    pattern = re.compile(
        r"\\hyperref\[([^\]]+)\]\{(Previous|Next) chapter(?:[^}]*)\}"
        r"|\\href\{([^}]+)\}\{(Previous|Next) chapter(?:[^}]*)\}"
    )
    for match in pattern.finditer(nav_line):
        if match.group(1) is not None:
            direction, target = match.group(2).lower(), match.group(1)
        else:
            direction, uri = match.group(4).lower(), match.group(3)
            # Cross-part links now use the canonical PDF URL fragment form
            # ``file.pdf#stable-id``. Accept the earlier Hyperref form too;
            # TeX source represents the literal hash as ``\#``.
            fragment = re.search(r"\\?#(?:nameddest=)?([^#}]+)$", uri)
            target = unquote(fragment.group(1)) if fragment else None
        destinations[direction] = target
    reading = re.search(r"\\hyperref\[([^\]]+)\]\{Reading map\}", nav_line)
    return {"line": nav_line, "directions": destinations,
            "reading_map_target": reading.group(1) if reading else None,
            "reading_map_is_local": bool(reading)}


def chapter_navigation_report(data, manifest, generation_root):
    """Compare chapter-front nav with actual previous/next order in the 7-stage catalog."""
    problems, checks = [], []
    rows = {row.get("id"): row for row in manifest.get("rows", []) if row.get("kind") == "chapter"}
    ordered = [(stage, chapter) for stage in data.get("stages", []) for chapter in stage.get("chapters", [])]
    part_by_id = {part.get("id"): part for stage in data.get("stages", []) for part in stage.get("parts", [])}
    # The conversion manifest is written from the current stage catalog. The
    # physical-parts map can lag at cross-stage boundaries and its records do
    # not always carry chapter_id, so join both ledgers by stable source path.
    def navigation_records(container):
        if isinstance(container, dict):
            records = container.get("records", [])
            if not records and all(isinstance(value, dict) for value in container.values()):
                records = list(container.values())
        else:
            records = container
        by_id = {}
        by_source = {}
        for item in records or []:
            if not isinstance(item, dict):
                continue
            if item.get("chapter_id") or item.get("id"):
                by_id[item.get("chapter_id", item.get("id"))] = item
            if item.get("source"):
                by_source[str(Path(item["source"]).resolve())] = item
        for chapter_id, row in rows.items():
            source = row.get("source")
            if source and str(Path(source).resolve()) in by_source:
                by_id.setdefault(chapter_id, by_source[str(Path(source).resolve())])
        return by_id

    physical_navigation = navigation_records(data.get("chapter_navigation", []))
    manifest_navigation = navigation_records(manifest.get("chapter_navigation", []))
    navigation_by_id = {**physical_navigation, **manifest_navigation}
    # A source-level alias (the Pandoc heading label) is a valid link target
    # for its owning chapter. Keep ownership exact so an unrelated heading
    # slug cannot satisfy a previous/next check.
    aliases_by_id = {chapter_id: {chapter_id} for chapter_id in rows}
    alias_owner = {chapter_id: chapter_id for chapter_id in rows}
    for chapter_id, row in rows.items():
        part_id = row.get("volume")
        tex_name = Path(row.get("tex") or "").name
        fragment = generation_root / part_id / "content" / tex_name
        if not fragment.is_file():
            continue
        source_tex = fragment.read_text(encoding="utf-8", errors="replace")
        heading = re.search(r"\\chapter(?:\[[^\]]*\])?\{(?:[^{}]|\{[^{}]*\})*\}\\label\{([^}]+)\}", source_tex)
        if heading:
            alias = heading.group(1)
            aliases_by_id[chapter_id].add(alias)
            alias_owner[alias] = chapter_id
    all_ids = [chapter.get("id") for _, chapter in ordered]
    stage_positions = {}
    for stage in data.get("stages", []):
        for logical_number, chapter in enumerate(stage.get("chapters", []), 1):
            stage_positions[chapter.get("id")] = logical_number
    for index, (stage, chapter) in enumerate(ordered):
        chapter_id = chapter.get("id")
        expected = {"previous": all_ids[index - 1] if index else None,
                    "next": all_ids[index + 1] if index + 1 < len(all_ids) else None}
        row = rows.get(chapter_id, {})
        part_id = chapter.get("part_id")
        part = part_by_id.get(part_id, {})
        raw_tex = row.get("tex") or chapter.get("tex_path")
        tex_name = Path(raw_tex or "").name
        fragment = Path(raw_tex) if raw_tex else generation_root / part_id / "content" / tex_name
        if not fragment.is_absolute() and not fragment.is_file():
            fragment = generation_root / part_id / "content" / tex_name
        if not fragment.is_absolute():
            fragment = (generation_root / fragment).resolve()
        entry = {"chapter_id": chapter_id, "stage_id": stage.get("id"), "part_id": part_id,
                 "tex_path": str(fragment), "expected": expected,
                 "expected_logical_chapter_number": stage_positions.get(chapter_id)}
        if not fragment.is_file():
            entry["status"] = "chapter_tex_missing"
            problems.append({"kind": "chapter_navigation_tex_missing", **entry})
            checks.append(entry)
            continue
        nav = parse_front_navigation(fragment.read_text(encoding="utf-8", errors="replace"))
        entry.update({"actual": nav["directions"], "reading_map_target": nav["reading_map_target"],
                      "front_navigation": nav["line"]})
        mismatch = {}
        for direction, target in expected.items():
            actual = nav["directions"].get(direction)
            actual_owner = alias_owner.get(actual)
            if actual != target and actual_owner != target:
                mismatch[direction] = {"expected": target, "actual": actual,
                                       "actual_owner": actual_owner}
        if mismatch:
            entry["status"] = "chapter_navigation_order_mismatch"
            entry["mismatches"] = mismatch
            problems.append({"kind": "chapter_navigation_order_mismatch", **entry})
        else:
            entry["status"] = "pass"

        expected_map = (part.get("reading_map_destination") or part.get("reading_map_target")
                        or part.get("reading_map_id") or "volume-reading-map")
        main_tex = Path(part.get("tex_path") or generation_root / part_id / "main.tex")
        if not main_tex.is_absolute():
            main_tex = (generation_root / main_tex).resolve()
        if not nav["reading_map_is_local"] or nav["reading_map_target"] != expected_map:
            entry["reading_map_status"] = "reading_map_not_local_part_target"
            problems.append({"kind": "reading_map_not_local_part_target", "chapter_id": chapter_id,
                             "part_id": part_id, "expected": expected_map,
                             "actual": nav["reading_map_target"]})
        elif not main_tex.is_file() or not re.search(
                r"\\(?:hypertarget|label)\{" + re.escape(expected_map) + r"\}",
                main_tex.read_text(encoding="utf-8", errors="replace")):
            entry["reading_map_status"] = "reading_map_destination_missing_from_part_tex"
            problems.append({"kind": "reading_map_destination_missing_from_part_tex", "chapter_id": chapter_id,
                             "part_id": part_id, "expected": expected_map, "part_tex": str(main_tex)})
        else:
            entry["reading_map_status"] = "pass_local_part_target"
        # The conversion manifest is authoritative for this generated PDF;
        # stage chapter metadata is still compared below against that record.
        manifest_metadata = navigation_by_id.get(chapter_id, {})
        for direction, field in (("previous", "previous_chapter_id"), ("next", "next_chapter_id")):
            if field in manifest_metadata and manifest_metadata.get(field) != expected[direction]:
                problems.append({"kind": "chapter_navigation_catalog_metadata_mismatch",
                                 "chapter_id": chapter_id, "direction": direction,
                                 "expected": expected[direction],
                                 "metadata": manifest_metadata.get(field),
                                 "source": "conversion_manifest"})
            for source_name, candidate in (("physical_parts_chapter", chapter),
                                           ("physical_parts_navigation", physical_navigation.get(chapter_id, {}))):
                if field in candidate and candidate.get(field) != expected[direction]:
                    problems.append({"kind": "chapter_navigation_catalog_metadata_mismatch",
                                     "chapter_id": chapter_id, "direction": direction,
                                     "expected": expected[direction], "metadata": candidate.get(field),
                                     "source": source_name})
        if "logical_chapter_number" in manifest_metadata and manifest_metadata.get("logical_chapter_number") != stage_positions.get(chapter_id):
            problems.append({"kind": "chapter_navigation_catalog_metadata_mismatch",
                             "chapter_id": chapter_id, "field": "logical_chapter_number",
                             "expected": stage_positions.get(chapter_id),
                             "metadata": manifest_metadata.get("logical_chapter_number"),
                             "source": "conversion_manifest"})
        for source_name, candidate in (("physical_parts_chapter", chapter),
                                       ("physical_parts_navigation", physical_navigation.get(chapter_id, {}))):
            if ("logical_chapter_number" in candidate and
                    candidate.get("logical_chapter_number") != stage_positions.get(chapter_id)):
                problems.append({"kind": "chapter_navigation_catalog_metadata_mismatch",
                                 "chapter_id": chapter_id, "field": "logical_chapter_number",
                                 "expected": stage_positions.get(chapter_id),
                                 "metadata": candidate.get("logical_chapter_number"),
                                 "source": source_name})
        checks.append(entry)
    return {"chapter_count": len(ordered), "checked": len(checks), "problems": problems, "chapters": checks}


def resolve_pdf_path(raw_path, map_dir, pdf_dir):
    candidate = Path(str(raw_path))
    if candidate.is_absolute():
        return candidate
    for base in (map_dir, pdf_dir):
        p = (base / candidate).resolve()
        if p.is_file():
            return p
    return (map_dir / candidate).resolve()


def asset_uri_report(target, asset_root):
    """Validate a bundled image URI by containment and content-addressed name."""
    target = Path(target).resolve()
    if asset_root is None or target.suffix.lower() not in {".png", ".svg", ".jpg", ".jpeg"}:
        return None
    asset_root = Path(asset_root).resolve()
    try:
        target.relative_to(asset_root)
    except ValueError:
        return None
    if not target.is_file():
        return None
    digest = hashlib.sha256(target.read_bytes()).hexdigest()
    if target.stem != digest:
        return {"status": "invalid_local_asset_hash", "target_asset": str(target),
                "actual_sha256": digest, "expected_sha256": target.stem}
    return {"status": "valid_local_asset_uri", "target_asset": str(target),
            "sha256": digest, "size_bytes": target.stat().st_size}


def pdf_link_report(reader, source_pdf, known_pdfs, pdf_cache=None, asset_root=None):
    links = []
    pdf_cache = pdf_cache or PdfObjectCache()
    named = pdf_cache.destinations(source_pdf, reader)
    local_destinations = set()
    for page_index, page in enumerate(reader.pages, 1):
        for annot_ref in obj(page.get("/Annots", [])) or []:
            annot = obj(annot_ref)
            if key(annot.get("/Subtype")) != "/Link":
                continue
            action = obj(annot.get("/A", {}))
            kind = key(action.get("/S"))
            item = {"source_pdf": source_pdf.name, "page": page_index, "action": kind, "status": "unverified"}
            if kind == "/GoTo":
                status, details = check_local_destination(reader, action.get("/D"), named)
                item["status"] = status
                item.update(details)
            elif kind == "/GoToR":
                raw = action.get("/F")
                if hasattr(raw, "get"):
                    raw = raw.get("/UF") or raw.get("/F")
                raw = str(raw or "")
                dest = destination_name(action.get("/D"))
                target = resolve_pdf_path(raw, source_pdf.parent, source_pdf.parent)
                item.update({"target_file": raw, "destination": dest})
                if Path(raw.removeprefix("file://")).is_absolute():
                    item["status"] = "nonportable_absolute_pdf_reference"
                elif target not in known_pdfs:
                    item["status"] = "nonportable_or_missing_pdf_target"
                else:
                    try:
                        target_reader = pdf_cache.reader(target)
                        target_named = pdf_cache.destinations(target, target_reader)
                        target_status, target_details = check_local_destination(
                            target_reader, action.get("/D"), target_named)
                        item.update({"target_" + k: v for k, v in target_details.items()})
                        if target_status == "unresolved_local_destination":
                            item["status"] = "missing_remote_named_destination"
                        else:
                            item["status"] = "valid_cross_pdf_destination"
                    except Exception as exc:
                        item["status"] = "unreadable_remote_pdf"
                        item["detail"] = str(exc)[:200]
            elif kind == "/URI":
                uri = str(action.get("/URI") or "")
                item["uri"] = uri
                if uri.startswith(("http://", "https://", "mailto:")):
                    item["status"] = "external_uri_unfetched"
                else:
                    # file: URIs and bare relative PDF paths with #named-destination.
                    pathpart, sep, fragment = uri.partition("#")
                    if fragment.startswith("nameddest="):
                        fragment = fragment.removeprefix("nameddest=")
                    pathpart = unquote(pathpart.removeprefix("file://"))
                    fragment = unquote(fragment)
                    item["destination"] = fragment
                    target = source_pdf if not pathpart else resolve_pdf_path(pathpart, source_pdf.parent, source_pdf.parent)
                    if pathpart and Path(pathpart).is_absolute():
                        item["status"] = "nonportable_absolute_pdf_reference"
                    elif target not in known_pdfs:
                        asset_report = asset_uri_report(target, asset_root)
                        if asset_report:
                            item.update(asset_report)
                        else:
                            item["status"] = "nonportable_or_missing_uri_target"
                    elif not fragment:
                        item["status"] = "local_pdf_uri_without_named_destination"
                    else:
                        try:
                            target_reader = pdf_cache.reader(target)
                            target_named = pdf_cache.destinations(target, target_reader)
                            item["status"] = ("valid_uri_named_destination" if fragment.lstrip("/") in target_named
                                              else "missing_uri_named_destination")
                        except Exception as exc:
                            item["status"] = "unreadable_uri_pdf"
                            item["detail"] = str(exc)[:200]
            else:
                item["status"] = "unsupported_or_actionless_link"
            links.append(item)
    return links


def source_assignment_report(data):
    expected_counts = data.get("expected_counts", {})
    expected_stages = expected_counts.get("stages", EXPECTED_STAGE_COUNT)
    expected_chapters = expected_counts.get("chapters", EXPECTED_CHAPTERS)
    expected_companions = expected_counts.get("companions", EXPECTED_COMPANIONS)
    chapter_rows, companion_rows = [], []
    parts = []
    membership_problems = []
    for stage in data.get("stages", []):
        stage_parts = {part.get("id"): part for part in stage.get("parts", [])}
        declared_chapters = collections.Counter(chapter_id for part in stage_parts.values() for chapter_id in part.get("chapter_ids", []))
        declared_companions = collections.Counter(companion_id for part in stage_parts.values() for companion_id in part.get("companion_ids", []))
        chapter_part = {chapter_id: part.get("id") for part in stage_parts.values() for chapter_id in part.get("chapter_ids", [])}
        companion_part = {companion_id: part.get("id") for part in stage_parts.values() for companion_id in part.get("companion_ids", [])}
        for chapter in stage.get("chapters", []):
            chapter_rows.append((chapter.get("source"), chapter.get("id"), stage.get("id"), chapter.get("part_id")))
            if chapter.get("part_id") not in stage_parts:
                membership_problems.append({"kind": "chapter_references_unknown_part", "id": chapter.get("id"), "part_id": chapter.get("part_id")})
            if chapter.get("id") and declared_chapters[chapter["id"]] != 1:
                membership_problems.append({"kind": "chapter_not_listed_exactly_once_in_parts", "id": chapter["id"], "count": declared_chapters[chapter["id"]]})
            if chapter.get("id") and chapter.get("part_id") != chapter_part.get(chapter["id"]):
                membership_problems.append({"kind": "chapter_part_assignment_disagrees", "id": chapter["id"], "row_part_id": chapter.get("part_id"), "parts_list_part_id": chapter_part.get(chapter["id"])})
        for companion in stage.get("companions", []):
            companion_rows.append((companion.get("source"), companion.get("id"), stage.get("id"), companion.get("part_id")))
            if companion.get("part_id") not in stage_parts:
                membership_problems.append({"kind": "companion_references_unknown_part", "id": companion.get("id"), "part_id": companion.get("part_id")})
            if companion.get("id") and declared_companions[companion["id"]] != 1:
                membership_problems.append({"kind": "companion_not_listed_exactly_once_in_parts", "id": companion["id"], "count": declared_companions[companion["id"]]})
            if companion.get("id") and companion.get("part_id") != companion_part.get(companion["id"]):
                membership_problems.append({"kind": "companion_part_assignment_disagrees", "id": companion["id"], "row_part_id": companion.get("part_id"), "parts_list_part_id": companion_part.get(companion["id"])})
        valid_chapter_ids = {chapter.get("id") for chapter in stage.get("chapters", [])}
        valid_companion_ids = {companion.get("id") for companion in stage.get("companions", [])}
        for part in stage_parts.values():
            for chapter_id in part.get("chapter_ids", []):
                if chapter_id not in valid_chapter_ids:
                    membership_problems.append({"kind": "part_lists_unknown_chapter", "part_id": part.get("id"), "chapter_id": chapter_id})
            for companion_id in part.get("companion_ids", []):
                if companion_id not in valid_companion_ids:
                    membership_problems.append({"kind": "part_lists_unknown_companion", "part_id": part.get("id"), "companion_id": companion_id})
        parts.extend((stage, part) for part in stage.get("parts", []))
    count_ch = collections.Counter(row[0] for row in chapter_rows)
    count_ref = collections.Counter(row[0] for row in companion_rows)
    problems = []
    if len(data.get("stages", [])) != expected_stages:
        problems.append({"kind": "stage_count", "expected": expected_stages, "actual": len(data.get("stages", []))})
    if len(chapter_rows) != expected_chapters:
        problems.append({"kind": "chapter_assignment_count", "expected": expected_chapters, "actual": len(chapter_rows)})
    if len(companion_rows) != expected_companions:
        problems.append({"kind": "companion_assignment_count", "expected": expected_companions, "actual": len(companion_rows)})
    for label, counter in (("chapter", count_ch), ("companion", count_ref)):
        for source, count in counter.items():
            if not source:
                problems.append({"kind": f"{label}_missing_source_path"})
            elif count != 1:
                problems.append({"kind": f"{label}_source_not_exactly_once", "source": source, "count": count})
    problems.extend(membership_problems)
    return {"chapters": len(chapter_rows), "companions": len(companion_rows),
            "unique_chapter_sources": len(count_ch), "unique_companion_sources": len(count_ref),
            "parts": parts, "problems": problems,
            "source_by_part": {
                part.get("id"): [row[0] for row in chapter_rows + companion_rows
                                  if row[2] == stage.get("id") and row[3] == part.get("id")]
                for stage, part in parts
            }}


def invisible_control_approval(data, part_id, codepoint, sources, map_dir):
    """Require a source-hash-bound exact occurrence for invisible controls only."""
    if codepoint is None:
        return None
    char = chr(codepoint)
    category = unicodedata.category(char)
    if category not in {"Cc", "Cf"} or codepoint in range(0x202A, 0x202F) or codepoint in range(0x2066, 0x206A):
        return None
    if category == "Cf" and codepoint not in INVISIBLE_FORMAT_ALLOWLIST:
        return None
    for approval in data.get("invisible_control_allowlist", []):
        if approval.get("part_id") != part_id or str(approval.get("codepoint", "")).upper() != f"U+{codepoint:04X}":
            continue
        raw_source = approval.get("source")
        if raw_source not in sources or not approval.get("reason") or not approval.get("source_context"):
            continue
        source = Path(raw_source)
        if not source.is_absolute():
            source = (map_dir / source).resolve()
        if not source.is_file():
            continue
        source_bytes = source.read_bytes()
        if hashlib.sha256(source_bytes).hexdigest() != approval.get("source_sha256"):
            continue
        context = approval["source_context"]
        encoded_context = context.encode("utf-8")
        if char not in context or source_bytes.count(encoded_context) != 1:
            continue
        return {"part_id": part_id, "codepoint": f"U+{codepoint:04X}", "source": str(source),
                "source_sha256": approval["source_sha256"], "source_context": context,
                "reason": approval["reason"], "status": "allowlisted_invisible_control_with_source_evidence"}
    return None


def overfull_review_approval(data, part_id, log_path, line_number, line, pdf_page, map_dir, pdf_page_count):
    for approval in data.get("overfull_render_reviews", []):
        if (approval.get("part_id") != part_id or Path(approval.get("log_path", "")).resolve() != Path(log_path).resolve()
                or approval.get("log_line") != line_number or approval.get("log_text") != line
                or approval.get("page") != pdf_page or approval.get("result") != "reviewed_no_clipping"
                or not approval.get("reviewer")):
            continue
        if not isinstance(pdf_page, int) or pdf_page < 1 or pdf_page > pdf_page_count:
            continue
        evidence = Path(approval.get("evidence_path", ""))
        if not evidence.is_absolute():
            evidence = (map_dir / evidence).resolve()
        if not evidence.is_file():
            continue
        return {"part_id": part_id, "log_path": str(log_path), "log_line": line_number,
                "log_text": line, "page": pdf_page, "evidence_path": str(evidence),
                "reviewer": approval["reviewer"], "result": approval["result"],
                "status": "resolved_with_rendered_page_evidence"}
    return None


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("parts_map", type=Path, help="physical-parts.json")
    parser.add_argument("--pdf-dir", type=Path, help="override base directory for relative PDF paths")
    parser.add_argument("--output", type=Path, help="write JSON report here; defaults beside parts map")
    args = parser.parse_args()
    map_path = args.parts_map.resolve()
    data = json.loads(map_path.read_text(encoding="utf-8"))
    pdf_dir = (args.pdf_dir or map_path.parent).resolve()
    assignment = source_assignment_report(data)
    map_parts = [(stage, part) for stage, part in assignment.pop("parts")]
    source_by_part = assignment.pop("source_by_part", {})
    map_dir = map_path.parent
    generation_root = Path(data.get("generation_root") or data.get("proof_root") or map_dir).resolve()
    manifest_candidates = []
    if data.get("conversion_manifest_path"):
        manifest_candidates.append(Path(data["conversion_manifest_path"]))
    manifest_candidates.extend([map_dir / "conversion-manifest.json",
                                generation_root / "conversion-manifest.json"])
    manifest_path = next((candidate.resolve() for candidate in manifest_candidates if candidate.is_file()), None)
    if manifest_path:
        conversion_manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
        # The conversion manifest points to the active generation. Resolve QA
        # fragments and per-part TeX from that same generation, not a stale
        # proof_root that may still name a previous build directory.
        generation_root = manifest_path.parent
        navigation = chapter_navigation_report(data, conversion_manifest, generation_root)
    else:
        navigation = {"chapter_count": 0, "checked": 0,
                      "problems": [{"kind": "navigation_conversion_manifest_missing",
                                    "checked_paths": [str(path) for path in manifest_candidates]}],
                      "chapters": []}
    pdf_specs = []
    for stage, part in map_parts:
        raw = part.get("pdf_path")
        pdf = resolve_pdf_path(raw or "", map_dir, pdf_dir)
        pdf_specs.append((stage, part, pdf))
    known_pdfs = {pdf.resolve() for _, _, pdf in pdf_specs}
    part_reports = []
    all_links = []
    pdf_cache = PdfObjectCache()
    for stage, part, pdf in pdf_specs:
        report = {"stage_id": stage.get("id"), "stage_title": stage.get("title"),
                  "part_id": part.get("id"), "pdf": str(pdf), "exists": pdf.is_file(),
                  "page_count": None, "page_count_target": data.get("target_pages", 650),
                  "page_count_max": data.get("max_pages", 800), "page_boxes": [],
                  "fonts": [], "links": [], "compile_logs": [], "issues": []}
        if not pdf.is_file():
            report["issues"].append({"kind": "pdf_missing"})
            part_reports.append(report)
            continue
        try:
            reader = pdf_cache.reader(pdf)
            report["page_count"] = len(reader.pages)
            if report["page_count"] > report["page_count_max"]:
                report["issues"].append({"kind": "page_count_exceeds_max"})
            if report["page_count"] > report["page_count_target"]:
                report["issues"].append({"kind": "page_count_above_target"})
            boxes = []
            for index, page in enumerate(reader.pages, 1):
                box = page.mediabox
                width, height = float(box.width), float(box.height)
                boxes.append({"page": index, "width_pt": width, "height_pt": height})
                if abs(width - TARGET_BOX[0]) > BOX_TOLERANCE or abs(height - TARGET_BOX[1]) > BOX_TOLERANCE:
                    report["issues"].append({"kind": "unexpected_page_box", "page": index,
                                              "width_pt": width, "height_pt": height})
            report["page_boxes"] = boxes
            report["fonts"] = font_report(reader)
            report["unembedded_font_count"] = sum(not f["embedded"] for f in report["fonts"])
            if report["unembedded_font_count"]:
                report["issues"].append({"kind": "unembedded_fonts", "count": report["unembedded_font_count"]})
            report["links"] = pdf_link_report(reader, pdf, known_pdfs, pdf_cache,
                                               generation_root / "Assets")
            all_links.extend(report["links"])
            bad_link_status = {"unresolved_local_destination", "nonportable_or_missing_pdf_target",
                               "missing_remote_named_destination", "nonportable_or_missing_uri_target",
                               "nonportable_absolute_pdf_reference",
                               "missing_uri_named_destination", "local_pdf_uri_without_named_destination",
                               "unreadable_remote_pdf", "unreadable_uri_pdf", "unsupported_or_actionless_link"}
            invalid = [link for link in report["links"] if link["status"] in bad_link_status]
            if invalid:
                report["issues"].append({"kind": "invalid_or_nonportable_links", "count": len(invalid)})
            chapter_by_id = {chapter.get("id"): chapter for chapter in stage.get("chapters", [])}
            selected_chapters = [chapter_by_id[chapter_id] for chapter_id in part.get("chapter_ids", [])
                                 if chapter_id in chapter_by_id]
            report["chapter_heading_destinations"] = chapter_heading_destination_report(
                reader, selected_chapters, part.get("id"))
            bad_chapter_destinations = [entry for entry in report["chapter_heading_destinations"]
                                        if entry["status"] in {"missing_stable_chapter_destination",
                                                               "destination_not_on_heading_page"}]
            unverified_chapter_destinations = [entry for entry in report["chapter_heading_destinations"]
                                               if entry["status"] not in {"destination_matches_heading_page",
                                                                          "destination_not_on_heading_page",
                                                                          "missing_chapter_destination"}]
            if bad_chapter_destinations:
                report["issues"].append({"kind": "chapter_destination_not_on_heading_page",
                                          "chapters": bad_chapter_destinations})
            if unverified_chapter_destinations:
                report["issues"].append({"kind": "chapter_destination_heading_page_unverified",
                                          "chapters": unverified_chapter_destinations})
        except Exception as exc:
            report["issues"].append({"kind": "pdf_read_error", "detail": str(exc)[:300]})
        log_paths = list(part.get("compile_log_paths", []))
        if part.get("compile_log_path"):
            log_paths.append(part["compile_log_path"])
        for raw_log in log_paths:
            log = resolve_pdf_path(raw_log, map_dir, pdf_dir)
            if not log.is_file():
                report["compile_logs"].append({"path": str(log), "exists": False})
                report["issues"].append({"kind": "compile_log_missing", "path": str(log)})
                continue
            text = log.read_text(encoding="utf-8", errors="replace")
            lines = text.splitlines()
            glyphs, overfull = [], []
            for line_number, line in enumerate(lines, 1):
                if "Missing character: There is no " in line:
                    match = re.search(r"\(U\+([0-9A-Fa-f]{4,6})\)", line)
                    cp = int(match.group(1), 16) if match else None
                    approval = invisible_control_approval(data, part.get("id"), cp,
                                                          source_by_part.get(part.get("id"), []), map_dir)
                    entry = {"log_line": line_number, "log_text": line, "codepoint": f"U+{cp:04X}" if cp is not None else None,
                             "status": approval["status"] if approval else "unresolved_visible_or_unproven_glyph"}
                    if approval:
                        entry["source_evidence"] = approval
                    else:
                        report["issues"].append({"kind": "missing_glyph_unresolved", "log": str(log), **entry})
                    glyphs.append(entry)
                if re.search(r"Overfull \\hbox|Overfull \\vbox", line):
                    kind = "overfull_hbox" if r"\hbox" in line else "overfull_vbox"
                    matching = [a for a in data.get("overfull_render_reviews", [])
                                if a.get("part_id") == part.get("id") and Path(a.get("log_path", "")).resolve() == log.resolve()
                                and a.get("log_line") == line_number]
                    approved = None
                    for candidate in matching:
                        approved = overfull_review_approval(data, part.get("id"), str(log), line_number, line,
                                                             candidate.get("page"), map_dir, report["page_count"] or 0)
                        if approved:
                            break
                    entry = {"kind": kind, "log_line": line_number, "log_text": line,
                             "page": approved["page"] if approved else None,
                             "status": approved["status"] if approved else "unresolved_requires_render_review"}
                    if approved:
                        entry["review_evidence"] = approved
                    else:
                        report["issues"].append({"kind": "overfull_requires_render_review", "warning_kind": kind,
                                                  "log": str(log), "log_line": line_number,
                                                  "log_text": line, "status": "unresolved_requires_render_review"})
                    overfull.append(entry)
            report["compile_logs"].append({
                "path": str(log), "exists": True,
                "missing_character_count": len(glyphs),
                "unresolved_missing_character_count": sum(g["status"] != "allowlisted_invisible_control_with_source_evidence" for g in glyphs),
                "overfull_hbox_count": sum(o["kind"] == "overfull_hbox" for o in overfull),
                "overfull_vbox_count": sum(o["kind"] == "overfull_vbox" for o in overfull),
                "unreviewed_overfull_count": sum(o["status"] != "resolved_with_rendered_page_evidence" for o in overfull),
                "glyph_warnings": glyphs, "overfull_warnings": overfull,
            })
        part_reports.append(report)

    report = {
        "schema": "allmight-physical-pdf-preflight-v1",
        "parts_map": str(map_path), "generation_id": data.get("generation_id"),
        "target_pages": data.get("target_pages", 650), "max_pages": data.get("max_pages", 800),
        "page_size_target_points": list(TARGET_BOX),
        "assignment": assignment, "chapter_navigation": navigation,
        "parts": part_reports,
        "link_status_counts": dict(collections.Counter(link["status"] for link in all_links)),
        "limitations": [
            "PDF checks use emitted PDFs and their actual page trees, fonts, annotations, and named destinations.",
            "Compile-log glyph and overfull counts are review flags; they require contextual inspection and do not alone establish bad output.",
            "External HTTP/HTTPS/MAILTO URIs are reported but not fetched.",
            "Portable cross-part links are expected to use relative sibling PDF paths and a destination present in the actual target PDF.",
        ],
    }
    issues = (assignment["problems"] + navigation["problems"]
              + [issue for part in part_reports for issue in part["issues"]])
    hard_kinds = {"pdf_missing", "page_count_exceeds_max", "unexpected_page_box", "unembedded_fonts",
                  "invalid_or_nonportable_links", "chapter_destination_not_on_heading_page",
                  "chapter_destination_heading_page_unverified",
                  "chapter_navigation_order_mismatch", "chapter_navigation_tex_missing",
                  "reading_map_not_local_part_target", "reading_map_destination_missing_from_part_tex",
                  "chapter_navigation_catalog_metadata_mismatch",
                  "pdf_read_error", "compile_log_missing", "missing_glyph_unresolved"}
    hard_failure = bool(assignment["problems"]) or any(issue["kind"] in hard_kinds for issue in issues)
    report["status"] = "fail" if hard_failure else ("review_required" if issues else "pass")
    report["final_delivery_gates"] = {
        "visible_missing_glyphs": "fail" if any(i["kind"] == "missing_glyph_unresolved" for i in issues) else "pass",
        "overfull_content_render_review": "incomplete" if any(i["kind"] == "overfull_requires_render_review" for i in issues) else "pass",
        "page_limit": "fail" if any(i["kind"] == "page_count_exceeds_max" for i in issues) else "pass",
        "stable_chapter_destinations": "fail" if any(i["kind"] == "chapter_destination_not_on_heading_page" for i in issues)
        else "incomplete" if any(i["kind"] == "chapter_destination_heading_page_unverified" for i in issues)
        else "pass",
        "stage_order_navigation": "fail" if any(i["kind"] in {"chapter_navigation_order_mismatch",
                                                                  "chapter_navigation_tex_missing",
                                                                  "chapter_navigation_catalog_metadata_mismatch",
                                                                  "reading_map_not_local_part_target",
                                                                  "reading_map_destination_missing_from_part_tex"}
                                                  for i in issues)
        else "incomplete" if any(i["kind"] == "navigation_conversion_manifest_missing" for i in issues)
        else "pass",
    }
    report["issue_count"] = len(issues)
    report["issues"] = issues
    out = (args.output or map_path.with_name("physical-pdf-preflight.json")).resolve()
    out.write_text(json.dumps(report, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    print(json.dumps({"status": report["status"], "stages": len(data.get("stages", [])),
                      "parts": len(part_reports), "issue_count": len(issues), "report": str(out)}, indent=2))
    return 0 if report["status"] == "pass" else 1


if __name__ == "__main__":
    sys.exit(main())
