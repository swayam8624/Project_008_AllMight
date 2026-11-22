#!/usr/bin/env python3
"""Small pypdf fixtures for the independent physical-parts preflight."""
import json
import hashlib
import os
import subprocess
import sys
import tempfile
from pathlib import Path

from pypdf import PdfReader, PdfWriter
from pypdf.generic import (
    ArrayObject, DecodedStreamObject, DictionaryObject, FloatObject,
    NameObject, NumberObject, TextStringObject,
)

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from QA.preflight_physical_parts import (PdfObjectCache, chapter_heading_destination_report, font_report,
                                         chapter_navigation_report, page_destinations, pdf_link_report,
                                         source_assignment_report)


def add_link(writer, page, action):
    writer.add_annotation(page, {
        NameObject("/Type"): NameObject("/Annot"),
        NameObject("/Subtype"): NameObject("/Link"),
        NameObject("/Rect"): ArrayObject([FloatObject(0), FloatObject(0), FloatObject(12), FloatObject(12)]),
        NameObject("/Border"): ArrayObject([NumberObject(0), NumberObject(0), NumberObject(0)]),
        NameObject("/A"): action,
    })


def attach_font(page, subtype, embedded):
    font = DictionaryObject({
        NameObject("/Type"): NameObject("/Font"),
        NameObject("/Subtype"): NameObject(subtype),
        NameObject("/BaseFont"): NameObject("/FixtureFont"),
    })
    if subtype == "/Type3":
        stream = DecodedStreamObject()
        stream.set_data(b"500 0 d0")
        font[NameObject("/CharProcs")] = DictionaryObject({NameObject("/g"): stream})
    resources = page[NameObject("/Resources")]
    resources[NameObject("/Font")] = DictionaryObject({NameObject("/Ffixture"): font})


def make_pdf(path, pages, named_dest=None, font=None, actions=()):
    writer = PdfWriter()
    for _ in range(pages):
        page = writer.add_blank_page(width=576, height=720)
        if font:
            attach_font(page, font[0], font[1])
    if named_dest:
        writer.add_named_destination(named_dest, 0)
    for page_number, action in actions:
        add_link(writer, page_number, action)
    with path.open("wb") as handle:
        writer.write(handle)


def make_two_chapter_pdf(path, second_anchor_page, include_stable_ids=True, first_anchor_page=0):
    """Model heading destinations plus unrelated Pandoc input-file aliases."""
    writer = PdfWriter()
    font = DictionaryObject({NameObject("/Type"): NameObject("/Font"),
                             NameObject("/Subtype"): NameObject("/Type1"),
                             NameObject("/BaseFont"): NameObject("/Helvetica")})
    font_ref = writer._add_object(font)
    for index, title in enumerate(("Chapter One", "Chapter Two", "Chapter One")):
        page = writer.add_blank_page(width=576, height=720)
        page[NameObject("/Resources")] = DictionaryObject({NameObject("/Font"): DictionaryObject({NameObject("/F1"): font_ref})})
        stream = DecodedStreamObject()
        font_size = 9 if index == 2 else 18
        running_head = "Chapter 6. " if index == 2 else ""
        stream.set_data(f"BT /F1 {font_size} Tf 72 650 Td ({running_head}{title}) Tj ET".encode("ascii"))
        page[NameObject("/Contents")] = writer._add_object(stream)
    if include_stable_ids:
        writer.add_named_destination("chapter-one", first_anchor_page)
        writer.add_named_destination("chapter-two", second_anchor_page)
    writer.add_named_destination("chapter-one-source-title", first_anchor_page)
    writer.add_named_destination("chapter-two-source-title", second_anchor_page)
    writer.add_named_destination("filealias01-source-chapter-one", 0)
    writer.add_named_destination("filealias02-source-chapter-two", 1)
    with path.open("wb") as handle:
        writer.write(handle)


def make_indirect_annotations_pdf(path):
    """Exercise a legal PDF page whose /Annots array is an indirect object."""
    writer = PdfWriter()
    page = writer.add_blank_page(width=576, height=720)
    add_link(writer, page, action("/GoTo", "/fixture-target"))
    annotations = page[NameObject("/Annots")]
    page[NameObject("/Annots")] = writer._add_object(annotations)
    writer.add_named_destination("fixture-target", 0)
    with path.open("wb") as handle:
        writer.write(handle)


def make_two_chapter_navigation(root, wrong_previous=False, direct_fragments=False):
    chapters = [{"id": "stage-chapter-one", "title": "Stage Chapter One", "part_id": "part-one"},
                {"id": "stage-chapter-two", "title": "Stage Chapter Two", "part_id": "part-two"}]
    stages = [{"id": "stage-a", "chapters": [chapters[0]], "parts": [
        {"id": "part-one", "chapter_ids": [chapters[0]["id"]], "companion_ids": [],
         "reading_map_destination": "volume-reading-map"}]},
        {"id": "stage-b", "chapters": [chapters[1]], "parts": [
        {"id": "part-two", "chapter_ids": [chapters[1]["id"]], "companion_ids": [],
         "reading_map_destination": "volume-reading-map"}]}]
    for part, chapter, previous, next_chapter in [
        ("part-one", chapters[0], None, chapters[1]["id"]),
        ("part-two", chapters[1], chapters[0]["id"], None),
    ]:
        (root / part / "content").mkdir(parents=True, exist_ok=True)
        (root / part / "main.tex").write_text("\\chapter*{Reading Map}\n\\hypertarget{volume-reading-map}{}\n", encoding="utf-8")
        links = [r"\hyperref[volume-reading-map]{Reading map}"]
        if previous:
            target = "old-volume-chapter" if wrong_previous and chapter is chapters[1] else previous
            links.append(fr"\hyperref[{target}]{{Previous chapter}}")
        if next_chapter:
            destination = f"{next_chapter}-source-title"
            fragment = destination if direct_fragments else f"nameddest={next_chapter}"
            links.append(fr"\href{{../part-two/main.pdf\#{fragment}}}{{Next chapter (Stage B)}}")
        slug = chapter["id"] + "-source-title"
        tex = fr"\chapter{{{chapter['title']}}}\label{{{slug}}}\hypertarget{{{chapter['id']}}}{{}}" + "\n" + " · ".join(links) + "\n"
        if previous:
            tex = tex.replace(fr"\hyperref[{previous}]{{Previous chapter}}",
                              fr"\hyperref[{previous}-source-title]{{Previous chapter}}")
        if next_chapter:
            if not direct_fragments:
                tex = tex.replace(fr"nameddest={next_chapter}", fr"nameddest={next_chapter}-source-title")
        (root / part / "content" / f"{chapter['id']}.tex").write_text(tex, encoding="utf-8")
    manifest = {"rows": [{"id": chapter["id"], "kind": "chapter", "volume": chapter["part_id"],
                           "source": f"{chapter['id']}.md", "tex": f"{chapter['id']}.tex"}
                          for chapter in chapters]}
    return {"stages": stages}, manifest


def action(kind, destination, file=None):
    result = DictionaryObject({NameObject("/S"): NameObject(kind)})
    if kind == "/GoToR":
        result[NameObject("/F")] = TextStringObject(file)
    if kind == "/URI":
        result[NameObject("/URI")] = TextStringObject(destination)
    else:
        result[NameObject("/D")] = NameObject(destination)
    return result


def page_gotor(file, page_index):
    result = DictionaryObject({NameObject("/S"): NameObject("/GoToR"),
                               NameObject("/F"): TextStringObject(file),
                               NameObject("/D"): ArrayObject([
                                   NumberObject(page_index), NameObject("/Fit")])})
    return result


def main():
    with tempfile.TemporaryDirectory(prefix="allmight-pdf-qa-") as tmp:
        root = Path(tmp)
        local = root / "local.pdf"
        remote = root / "remote.pdf"
        headings = root / "two-chapters.pdf"
        indirect_annotations = root / "indirect-annots.pdf"
        misplaced_heading = root / "two-chapters-wrong-anchor.pdf"
        missing_heading = root / "two-chapters-alias-only.pdf"
        running_head_anchor = root / "two-chapters-running-head-anchor.pdf"
        make_pdf(remote, 1, named_dest="remote-dest", font=("/Type1", False))
        cache = PdfObjectCache()
        cached_reader = cache.reader(remote)
        assert "remote-dest" in cache.destinations(remote, cached_reader)
        cached_stat = remote.stat()
        make_pdf(remote, 1, named_dest="remote-dest-updated", font=("/Type1", False))
        os.utime(remote, ns=(cached_stat.st_atime_ns, cached_stat.st_mtime_ns + 1_000_000_000))
        refreshed_reader = cache.reader(remote)
        refreshed_destinations = cache.destinations(remote, refreshed_reader)
        assert refreshed_reader is not cached_reader
        assert "remote-dest-updated" in refreshed_destinations and "remote-dest" not in refreshed_destinations
        updated_stat = remote.stat()
        make_pdf(remote, 1, named_dest="remote-dest", font=("/Type1", False))
        os.utime(remote, ns=(updated_stat.st_atime_ns, updated_stat.st_mtime_ns + 1_000_000_000))
        make_indirect_annotations_pdf(indirect_annotations)
        make_two_chapter_pdf(headings, second_anchor_page=1)
        make_two_chapter_pdf(misplaced_heading, second_anchor_page=0)
        make_two_chapter_pdf(missing_heading, second_anchor_page=1, include_stable_ids=False)
        make_two_chapter_pdf(running_head_anchor, second_anchor_page=1, first_anchor_page=2)
        heading_reader = PdfReader(str(headings), strict=False)
        heading_rows = [{"id": "chapter-one", "title": "01 - Chapter One"},
                        {"id": "chapter-two", "title": "02 - Chapter Two"}]
        heading_checks = chapter_heading_destination_report(heading_reader, heading_rows, "fixture-part")
        assert [row["status"] for row in heading_checks] == [
            "destination_matches_heading_page", "destination_matches_heading_page"]
        assert len(heading_checks[0]["title_pages"]) == 1, heading_checks
        wrong_heading_checks = chapter_heading_destination_report(
            PdfReader(str(misplaced_heading), strict=False), heading_rows, "fixture-part")
        assert wrong_heading_checks[1]["status"] == "destination_not_on_heading_page", wrong_heading_checks
        alias_only_checks = chapter_heading_destination_report(
            PdfReader(str(missing_heading), strict=False), heading_rows, "fixture-part")
        assert all(row["status"] == "missing_stable_chapter_destination" for row in alias_only_checks), alias_only_checks
        running_head_checks = chapter_heading_destination_report(
            PdfReader(str(running_head_anchor), strict=False), heading_rows, "fixture-part")
        assert running_head_checks[0]["status"] == "destination_not_on_heading_page", running_head_checks

        nav_root = root / "navigation"
        nav_map, nav_manifest = make_two_chapter_navigation(nav_root)
        nav = chapter_navigation_report(nav_map, nav_manifest, nav_root)
        assert nav["checked"] == 2 and nav["problems"] == [], nav
        direct_nav_root = root / "navigation-direct-fragments"
        direct_nav_map, direct_nav_manifest = make_two_chapter_navigation(
            direct_nav_root, direct_fragments=True)
        direct_nav = chapter_navigation_report(direct_nav_map, direct_nav_manifest, direct_nav_root)
        assert direct_nav["checked"] == 2 and direct_nav["problems"] == [], direct_nav
        wrong_nav_map, wrong_nav_manifest = make_two_chapter_navigation(root / "wrong-navigation", wrong_previous=True)
        wrong_nav = chapter_navigation_report(wrong_nav_map, wrong_nav_manifest, root / "wrong-navigation")
        assert any(problem["kind"] == "chapter_navigation_order_mismatch" for problem in wrong_nav["problems"]), wrong_nav
        make_pdf(local, 2, named_dest="local-dest", font=("/Type3", True), actions=[
            (0, action("/GoTo", "/local-dest")),
            (0, action("/GoTo", "/missing-local")),
            (0, action("/GoToR", "/remote-dest", "remote.pdf")),
            (0, page_gotor("remote.pdf", 0)),
            (0, page_gotor("remote.pdf", 3)),
            (0, action("/GoToR", "/missing-remote", "remote.pdf")),
            (0, action("/GoToR", "/remote-dest", str(remote.resolve()))),
        ])

        local_reader = PdfReader(str(local), strict=False)
        remote_reader = PdfReader(str(remote), strict=False)
        assert "local-dest" in page_destinations(local_reader)
        assert font_report(local_reader)[0]["embedded"] is True
        assert font_report(remote_reader)[0]["embedded"] is False
        links = pdf_link_report(local_reader, local, {local.resolve(), remote.resolve()})
        statuses = [link["status"] for link in links]
        assert "valid_local_named_destination" in statuses, statuses
        assert "unresolved_local_destination" in statuses, statuses
        assert "valid_cross_pdf_destination" in statuses, statuses
        assert sum(link["status"] == "valid_cross_pdf_destination" for link in links) == 2, statuses
        assert "missing_remote_named_destination" in statuses, statuses
        assert any(link.get("target_page") == 1 for link in links), links
        assert any(link.get("target_page_index") == 3 for link in links), links
        assert "nonportable_absolute_pdf_reference" in statuses, statuses

        # A bundled print image is a valid relative URI only when it resolves
        # under the allowed asset root and its filename is its content hash.
        book_root = root / "book"
        source_dir = book_root / "part-one"
        asset_root = book_root / "current" / "Assets"
        source_dir.mkdir(parents=True)
        print_assets = asset_root / "Print"
        print_assets.mkdir(parents=True)
        asset_bytes = b"fixture png bytes"
        asset_hash = hashlib.sha256(asset_bytes).hexdigest()
        asset_path = print_assets / f"{asset_hash}.png"
        asset_path.write_bytes(asset_bytes)
        asset_source = source_dir / "main.pdf"
        make_pdf(asset_source, 1, actions=[
            (0, action("/URI", f"../current/Assets/Print/{asset_hash}.png")),
            (0, action("/URI", "../current/Assets/Print/not-the-content-hash.png")),
        ])
        asset_reader = PdfReader(str(asset_source), strict=False)
        asset_links = pdf_link_report(asset_reader, asset_source, {asset_source.resolve()},
                                      asset_root=asset_root)
        assert [link["status"] for link in asset_links] == [
            "valid_local_asset_uri", "nonportable_or_missing_uri_target"], asset_links
        assert asset_links[0]["sha256"] == asset_hash, asset_links

        indirect_reader = PdfReader(str(indirect_annotations), strict=False)
        assert indirect_reader.pages[0].raw_get(NameObject("/Annots")).__class__.__name__ == "IndirectObject"
        indirect_links = pdf_link_report(indirect_reader, indirect_annotations,
                                         {indirect_annotations.resolve()})
        assert [link["status"] for link in indirect_links] == ["valid_local_named_destination"], indirect_links

        base = {
            "expected_counts": {"stages": 1, "chapters": 1, "companions": 1},
            "stages": [{"id": "stage-1", "chapters": [{"id": "ch-1", "source": str(root / "chapter.md"), "part_id": "part-1"}],
                        "companions": [{"id": "ref-1", "source": "reference.md", "part_id": "part-1"}],
                        "parts": [{"id": "part-1", "chapter_ids": ["ch-1"], "companion_ids": ["ref-1"]}]}],
        }
        assert source_assignment_report(base)["problems"] == []
        duplicated = json.loads(json.dumps(base))
        duplicated["stages"][0]["chapters"].append(dict(duplicated["stages"][0]["chapters"][0]))
        duplicated["stages"][0]["parts"][0]["chapter_ids"].append("ch-1")
        assert any(p["kind"] == "chapter_source_not_exactly_once" for p in source_assignment_report(duplicated)["problems"])
        missing = json.loads(json.dumps(base))
        missing["stages"][0]["chapters"].clear()
        assert any(p["kind"] == "chapter_assignment_count" for p in source_assignment_report(missing)["problems"])

        # The full CLI reads actual PDFs and enforces a deliberately low fixture limit.
        cli_map = {
            **base, "generation_id": "fixture", "target_pages": 1, "max_pages": 1,
            "stages": [{**base["stages"][0], "parts": [
                {"id": "part-1", "pdf_path": "local.pdf", "chapter_ids": ["ch-1"], "companion_ids": ["ref-1"]},
                {"id": "part-2", "pdf_path": "remote.pdf", "chapter_ids": [], "companion_ids": []},
            ]}],
        }
        source_context = "zero\u200bwidth token"
        source_path = root / "chapter.md"
        source_path.write_text(source_context + "\n", encoding="utf-8")
        compile_log = root / "part.log"
        compile_log.write_text(
            "Missing character: There is no A (U+0041) in font Fixture!\n"
            "Missing character: There is no \u200b (U+200B) in font Fixture!\n"
            "Overfull \\hbox (3.0pt too wide) in paragraph at lines 5--6\n"
            "Overfull \\vbox (2.0pt too high) has occurred while \\output is active\n",
            encoding="utf-8")
        evidence_path = root / "rendered-page.png"
        evidence_path.write_bytes(b"fixture-render-evidence")
        cli_map["invisible_control_allowlist"] = [{
            "part_id": "part-1", "source": str(source_path), "source_sha256": hashlib.sha256(source_path.read_bytes()).hexdigest(),
            "codepoint": "U+200B", "source_context": source_context,
            "reason": "Zero-width join control is required by this exact source token.",
        }]
        cli_map["stages"][0]["parts"][0]["compile_log_path"] = "part.log"
        cli_map["overfull_render_reviews"] = [{
            "part_id": "part-1", "log_path": str(compile_log), "log_line": 3,
            "log_text": "Overfull \\hbox (3.0pt too wide) in paragraph at lines 5--6",
            "page": 1, "evidence_path": str(evidence_path), "reviewer": "fixture reviewer",
            "result": "reviewed_no_clipping",
        }]
        map_path = root / "physical-parts.json"
        map_path.write_text(json.dumps(cli_map), encoding="utf-8")
        report_path = root / "report.json"
        checker = Path(__file__).resolve().parents[1] / "preflight_physical_parts.py"
        run = subprocess.run([sys.executable, str(checker), str(map_path), "--output", str(report_path)],
                             capture_output=True, text=True)
        assert run.returncode == 1, (run.returncode, run.stdout, run.stderr)
        report = json.loads(report_path.read_text(encoding="utf-8"))
        assert any(issue["kind"] == "page_count_exceeds_max" for issue in report["issues"]), report["issues"]
        assert any(issue["kind"] == "unembedded_fonts" for part in report["parts"] for issue in part["issues"]), report["parts"]
        assert any(issue["kind"] == "missing_glyph_unresolved" and issue.get("codepoint") == "U+0041" for issue in report["issues"]), report["issues"]
        assert any(issue["kind"] == "overfull_requires_render_review" and issue.get("warning_kind") == "overfull_vbox" for issue in report["issues"]), report["issues"]
        glyph_warnings = report["parts"][0]["compile_logs"][0]["glyph_warnings"]
        assert any(glyph["codepoint"] == "U+200B" and glyph["status"] == "allowlisted_invisible_control_with_source_evidence" for glyph in glyph_warnings), glyph_warnings
        overfull_warnings = report["parts"][0]["compile_logs"][0]["overfull_warnings"]
        assert any(warning["status"] == "resolved_with_rendered_page_evidence" for warning in overfull_warnings), overfull_warnings
        assert report["final_delivery_gates"]["visible_missing_glyphs"] == "fail"
        assert report["final_delivery_gates"]["overfull_content_render_review"] == "incomplete"
        print(json.dumps({
            "status": "pass", "fixtures": ["valid-and-missing-local-named-destinations",
                                               "valid-and-missing-cross-pdf-destinations",
                                               "absolute-cross-pdf-path-rejected-as-nonportable",
                                               "indirect-page-annotations-array-is-dereferenced",
                                               "embedded-Type3-and-unembedded-Type1-fonts",
                                               "source-ordinal-prefixed-titles-match-stable-heading-pages-despite-file-aliases",
                                               "wrong-stable-heading-destination-and-alias-only-cases-rejected",
                                               "two-stage-chapter-front-navigation-follows-catalog-order-and-local-reading-maps",
                                               "stale-cross-stage-previous-link-rejected",
                                               "visible-glyph-failure-and-source-hash-bound-zero-width-allowlist",
                                               "overfull-warning-requires-page-evidence-review",
                                               "duplicate-and-missing-source-assignments",
                                               "configurable-page-limit-on-two-page-PDF"],
            "pdfs": [str(local), str(remote)], "parts_map": str(map_path),
            "cli_page_limit_and_unembedded_font_negative_cases": "pass",
        }, indent=2))
        print("fixture map remains inside temporary directory and is removed at exit")


if __name__ == "__main__":
    main()
