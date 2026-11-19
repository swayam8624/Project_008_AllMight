#!/usr/bin/env python3
"""Produce the complete 16-chapter expanded Foundations reader, additively.

This does not mutate the three continuous master manuscripts, the Chapter
Edition, the seven-stage LaTeX generation, or any existing print/EPUB books.

--check            verify all exact anchors, lesson identity, and reversibility
--prepare          emit source-pinned Markdown and LaTeX fragments (NO PDF)
--verify-prepared  inspect source hashes and all emitted files (NO PDF)
--compile          build the four prepared PDFs on the caller's machine

Every chapter's original CHAPTER-SOURCE body is included byte-for-byte, in
order; 18 marker-delimited learning additions are spliced before the exact
headings established by the preservation tool. The reader keeps the entire
original teaching body, not a summary or isolated two-page replacement.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys

import apply_cpp23_project_inserts as inserts
import book01_render_support as render
import book01_publication_edit as publication
import book01_index as idx
from book01_editorial import BOOK_TITLE, BOOK_SUBTITLE, CHAPTER_TITLES, PRINT_VOLUMES

ROOT = Path(__file__).resolve().parent.parent
MASTER = ROOT / "Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md"
EDITION = ROOT / "Chapter Edition/01 - Computing Foundations"
LATEX = ROOT / "Book Production/AllMight Codex/LaTeX"
CAPSTONE = ROOT / "Book Production/Seven-Book Curriculum/Book01/FINAL_CAPSTONE_AND_MASTERY_GATE.md"
READER_STYLE = ROOT / "Book Production/Seven-Book Curriculum/Book01/book01-reader.sty"
ACADEMIC_PREFACE = ROOT / "Book Production/Seven-Book Curriculum/Book01/ACADEMIC_PREFACE.md"
SECOND_VOLUME_PREFACE = ROOT / "Book Production/Seven-Book Curriculum/Book01/VOLUME_II_PREFACE.md"
ACADEMIC_BACKMATTER = ROOT / "Book Production/Seven-Book Curriculum/Book01/ACADEMIC_BACKMATTER.md"
OUTPUT = ROOT / "build/book01-expanded-reader"
SOURCE_BEGIN = b"<!-- CHAPTER-SOURCE-BEGIN -->\n"
SOURCE_END = b"<!-- CHAPTER-SOURCE-END -->"
PARTS = ((1, 4), (5, 8), (9, 12), (13, 16))
NUMBER_OF_CHAPTERS = 16
EXPECTED_ADDITIONS = 18
ENRICHMENT_FILES = {
    n: ROOT / "Book Production/Seven-Book Curriculum/Book01/Enrichments" / filename
    for n, filename in {
        1: "CH01_CPU_FROM_LOGIC.md",
        5: "CH05_CPP23_FEATURE_CONTRACTS.md",
        9: "CH09_GPU_MEMORY_VISUAL_LAB.md",
        12: "CH12_GPU_LIFETIME_TIMELINE.md",
        16: "CH16_MEASUREMENT_AND_EVIDENCE.md",
    }.items()
}
# Original foundation master before this branch's additive work. The exact
# same identity is required after reversing any already-applied lesson
# blocks; otherwise the derived Chapter Edition may be stale.
EXPECTED_BASE_BLOB = "d3202af31e1eb644a74565eeb63e7a3352438823"


def sha256(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def git_blob(value: bytes) -> str:
    return hashlib.sha1(("blob " + str(len(value)) + "\0").encode() + value).hexdigest()


def specs_by_chapter() -> dict[int, list[inserts.Insert]]:
    specifications = list(inserts.INSERTS)
    if len(specifications) != EXPECTED_ADDITIONS:
        raise ValueError("The expanded reader requires all 18 known additions")
    if len({x.name for x in specifications}) != len(specifications):
        raise ValueError("Duplicate staged lesson identity")
    # The first 16 insertions were authored for chapters 01..16. These
    # two later gap closures belong in chapter 06 and chapter 10.
    result = {n: [specifications[n - 1]] for n in range(1, 17)}
    known_extra = {"CPP-18A": 6, "CPP-19E": 10}
    for spec in specifications[16:]:
        if spec.name not in known_extra:
            raise ValueError("A new supplement needs an explicit chapter route: " + spec.name)
        result[known_extra[spec.name]].append(spec)
    return result


def chapters() -> list[Path]:
    files = sorted(p for p in EDITION.glob("*.md")
                   if re.match(r"^\d\d - ", p.name) and p.name[:2] != "00")
    selected = files[:NUMBER_OF_CHAPTERS]
    if len(selected) != NUMBER_OF_CHAPTERS:
        raise ValueError("Missing one of the first 16 original reading chapters")
    for i, path in enumerate(selected, 1):
        if not path.name.startswith(f"{i:02d} - "):
            raise ValueError(f"Chapter number {i:02d} missing or out of order")
    return selected


def chapter_regions(raw: bytes) -> tuple[bytes, bytes, bytes]:
    if raw.count(SOURCE_BEGIN) != 1 or raw.count(SOURCE_END) != 1:
        raise ValueError("Original chapter-body sentinels must each occur exactly once")
    before, remain = raw.split(SOURCE_BEGIN, 1)
    body, after = remain.split(SOURCE_END, 1)
    return before, body, after


def original_prelude(before: bytes) -> bytes:
    """Keep the chapter-specific narrative/learning objectives, not nav UI.

    Chapter Edition wrapper headings, wiki navigation and provenance comments
    are generated scaffolding. The original master body is NOT transformed.
    """
    lines = before.decode("utf-8").splitlines(keepends=True)
    kept = []
    for line in lines:
        if line.startswith("# "):
            continue
        if line.startswith("[[Chapter Edition/"):
            continue
        if line.startswith("<!-- Derived from "):
            continue
        kept.append(line)
    return "".join(kept).encode("utf-8")


def insert_and_prove(base: bytes, specs: list[inserts.Insert]) -> tuple[bytes, list[dict]]:
    result = base
    receipt = []
    for spec in specs:
        begin, end, block = inserts.get_block(spec)
        if begin in result or end in result:
            raise ValueError(spec.name + ": unexpected existing marker in derived chapter")
        if inserts.overlapping_count(result, spec.anchor) != 1:
            raise ValueError(spec.name + ": original derived heading is not unique")
        prior = result
        result = result.replace(spec.anchor, block + spec.anchor, 1)
        if result.replace(block, b"", 1) != prior:
            raise AssertionError(spec.name + ": original chapter bytes changed")
        receipt.append({"id": spec.name, "source": spec.source,
                        "source_sha256": sha256((ROOT / spec.source).read_bytes()),
                        "added_sha256": sha256(block),
                        "added_bytes": len(block),
                        "anchor": spec.anchor.decode("utf-8").strip()})
    # Strip all inserted blocks in reverse order to prove the combined
    # chapter still contains the ORIGINAL body verbatim and in exact order.
    stripped = result
    for spec in reversed(specs):
        _, _, block = inserts.get_block(spec)
        if stripped.count(block) != 1:
            raise AssertionError("Missing/duplicated exact lesson: " + spec.name)
        stripped = stripped.replace(block, b"", 1)
    if stripped != base:
        raise AssertionError("All-lesson removal did not recover original chapter bytes")
    return result, receipt


def audit_sources() -> tuple[list[dict], dict]:
    master = MASTER.read_bytes()
    master.decode("utf-8")
    preimage = master
    for spec in inserts.INSERTS:
        begin, end, block = inserts.get_block(spec)
        if begin in preimage or end in preimage:
            if preimage.count(begin) != 1 or preimage.count(end) != 1 or preimage.count(block) != 1:
                raise ValueError(spec.name + ": modified or partial canonical addition")
            preimage = preimage.replace(block, b"", 1)
    if git_blob(preimage) != EXPECTED_BASE_BLOB:
        raise ValueError(
            "Canonical source changed beyond exact staged additions; "
            "the 16 derived chapter bodies must be regenerated and audited "
            "before an updated Book01 release can be trusted"
        )
    mapped = specs_by_chapter()
    all_specs = list(inserts.INSERTS)
    for spec in all_specs:
        if inserts.overlapping_count(master, spec.anchor) != 1:
            raise ValueError(spec.name + ": canonical master anchor not unique")
        # The established patcher refuses altered/partial prior applications.
    inserts.process(master)
    rows = []
    identities = []
    for index, path in enumerate(chapters(), 1):
        raw = path.read_bytes()
        raw.decode("utf-8")
        prelude, body, suffix = chapter_regions(raw)
        # Derived chapter editions append a generated horizontal rule plus
        # previous/next-reading-map wiki links AFTER the original-source
        # sentinel. These are navigation chrome, not original teaching.
        tail = [line.strip() for line in suffix.decode("utf-8").splitlines()
                if line.strip()]
        if tail and not (len(tail) == 2 and tail[0] == "---"
                         and tail[1].startswith("[[Chapter Edition/")
                         and tail[1].endswith("]]")):
            raise ValueError(path.name + ": unexpected material after source sentinel")
        expanded, receipts = insert_and_prove(body, mapped[index])
        identities.extend(x["id"] for x in receipts)
        rows.append({
            "number": index,
            "title": path.stem.split(" - ", 1)[1],
            "source": path.relative_to(ROOT).as_posix(),
            "derived_sha256": sha256(raw),
            "original_body_sha256": sha256(body),
            "original_body_bytes": len(body),
            "expanded_body_sha256": sha256(expanded),
            "expanded_body_bytes": len(expanded),
            "source_prelude_sha256": sha256(prelude),
            "lesson_receipts": receipts,
            "_preface": original_prelude(prelude),
            "_expanded": expanded,
            "_original": body,
        })
    if identities != [x.name for x in all_specs]:
        # Compare sets rather than stage order: CPP18/19 are routed to
        # earlier chapters, not appended after Chapter16 in the reading flow.
        if len(identities) != EXPECTED_ADDITIONS or set(identities) != set(x.name for x in all_specs):
            raise ValueError("Not every staged addition is placed once")
    total = sum(len(x["lesson_receipts"]) for x in rows)
    if total != EXPECTED_ADDITIONS:
        raise AssertionError("Incomplete source lesson coverage")
    capstone_data = CAPSTONE.read_bytes()
    if not capstone_data.startswith(b"# Book 1 ") or len(capstone_data) < 5000:
        raise ValueError("Final capstone is missing, abbreviated or incorrectly headed")
    capstone_data.decode("utf-8")
    metadata = {
        "schema": "allmight-book01-expanded-reader-v1",
        "final_capstone_source": CAPSTONE.relative_to(ROOT).as_posix(),
        "final_capstone_sha256": sha256(capstone_data),
        "scope": "16 original Foundations chapters, 18 additive lessons; distinct from the archived seven-stage typeset snapshots",
        "canonical_source": MASTER.relative_to(ROOT).as_posix(),
        "canonical_source_sha256": sha256(master),
        "canonical_source_git_blob": git_blob(master),
        "original_canonical_git_blob_after_stripping_additions": git_blob(preimage),
        "chapter_count": NUMBER_OF_CHAPTERS,
        "addition_count": total,
        "reader_style_sha256": sha256(READER_STYLE.read_bytes()),
        "academic_preface_sha256": sha256(ACADEMIC_PREFACE.read_bytes()),
        "second_volume_preface_sha256": sha256(SECOND_VOLUME_PREFACE.read_bytes()),
        "academic_backmatter_sha256": sha256(ACADEMIC_BACKMATTER.read_bytes()),
        "academic_enrichment_sha256": {str(n): sha256(source.read_bytes())
                                        for n, source in ENRICHMENT_FILES.items()},
        "academic_title": BOOK_TITLE,
        "academic_subtitle": BOOK_SUBTITLE,
        "part_ranges": [list(x) for x in PARTS],
    }
    git_revision = subprocess.run(["git", "rev-parse", "--short=12", "HEAD"],
                                  cwd=ROOT, capture_output=True, text=True)
    metadata["source_revision"] = (git_revision.stdout.strip() if git_revision.returncode == 0
                                   else "unresolved")
    return rows, metadata


def tex_escape(text: str) -> str:
    mapping = {"\\": r"\textbackslash{}", "{": r"\{", "}": r"\}",
               "&": r"\&", "%": r"\%", "$": r"\$", "#": r"\#",
               "_": r"\_", "~": r"\textasciitilde{}", "^": r"\textasciicircum{}"}
    return "".join(mapping.get(char, char) for char in text)


def book_pdf_metadata(title: str, subtitle: str) -> str:
    """Populate actual PDF document information independently of cover text."""
    return (r"\hypersetup{pdftitle={" + tex_escape(title) +
            r"},pdfauthor={Swayam Singal},pdfsubject={" + tex_escape(subtitle) +
            r"},pdfkeywords={computing, systems programming, C++23, CPU, GPU, Vulkan, Metal}}")

def book_imprint(title: str, subtitle: str, revision: str) -> str:
    return (r"\BookImprintPage{" + tex_escape(title) + "}{" +
            tex_escape(subtitle) + "}{" + tex_escape(revision) + "}")


def pandoc_binary() -> str:
    binary = shutil.which("pandoc")
    if not binary:
        raise RuntimeError("Pandoc is required for --prepare: brew install pandoc")
    return binary


def amend_generated_tex(path: Path, figures: list[dict] | None = None, math_stats: dict | None = None, chapter: int = 0, math_entries: list[dict] | None = None, index_entries: list[dict] | None = None) -> None:
    """Correct only generated TeX, never the original teaching Markdown."""
    content = path.read_text("utf-8")
    content = idx.replace_index_tokens(content, index_entries or [])
    content = render.replace_math_tokens(content, math_entries or [])
    def correct(match: re.Match[str]) -> str:
        return r"\texttt{" + re.sub(r"(?<!\\)_", r"\\_", match.group(1)) + "}"
    content = re.sub(r"\\texttt\{([^{}\n]*)\}", correct, content)
    content = content.replace(r"\begin{verbatim}", r"\begin{BookCode}")
    content = content.replace(r"\end{verbatim}", r"\end{BookCode}")
    content = render.replace_figure_tokens(content, figures or [])
    if math_stats is not None:
        render.check_math_rendered(math_stats, content, chapter)
    path.write_text(content, encoding="utf-8")


def prepare(rows: list[dict], metadata: dict) -> None:
    pandoc = pandoc_binary()
    OUTPUT.mkdir(parents=True, exist_ok=True)
    style = LATEX / "allmight-book.sty"
    fonts = LATEX / "Assets/Fonts"
    if not style.is_file() or not READER_STYLE.is_file() or not (fonts / "NotoEmoji-Static.ttf").is_file():
        raise FileNotFoundError("The existing LaTeX profile and its bundled local font must be available")
    report = {**metadata, "parts": [], "chapters": [], "diagrams": [],
              "publication_edits": [], "enrichments": []}
    for part_number, (low, high) in enumerate(PARTS, 1):
        name = f"book01-expanded-p{part_number:02d}"
        directory = OUTPUT / name
        content_dir = directory / "content"
        content_dir.mkdir(parents=True, exist_ok=True)
        shutil.copy2(style, directory / "allmight-book.sty")
        shutil.copy2(READER_STYLE, directory / "book01-reader.sty")
        asset_root = directory / "Assets"
        asset_root.mkdir(exist_ok=True)
        font_link = asset_root / "Fonts"
        if font_link.is_symlink():
            if font_link.resolve() != fonts.resolve():
                raise ValueError("Refusing to replace an unexpected asset link")
        elif font_link.exists():
            raise ValueError("Refusing to overwrite a real asset directory")
        else:
            font_link.symlink_to(os.path.relpath(fonts, asset_root), target_is_directory=True)

        tex = [r"\documentclass[11pt,twoside,openany]{book}",
               r"\usepackage[ebook]{allmight-book}",
               r"\usepackage{book01-reader}",
               book_pdf_metadata(BOOK_TITLE, BOOK_SUBTITLE),
               r"\SetBookStage{1}{Bits, Programs, and Reliable Systems}",
               r"\title{" + tex_escape(BOOK_TITLE) + " — Diagnostic Part " + str(part_number) + "}",
               r"\author{Swayam Singal}",
               r"\begin{document}", r"\frontmatter", r"\maketitle",
               book_imprint(BOOK_TITLE, BOOK_SUBTITLE, metadata["source_revision"]),
               r"\setcounter{tocdepth}{1}", r"\tableofcontents", r"\listoffigures", r"\mainmatter",
               r"\setcounter{chapter}{" + str(low - 1) + "}"]
        part_rows = []
        for row in rows[low - 1:high]:
            num = row["number"]
            fragment = f"chapter-{num:02d}"
            raw_path = content_dir / (fragment + ".md")
            tex_path = content_dir / (fragment + ".tex")
            payload = row["_preface"] + b"\n" + row["_expanded"]
            raw_path.write_bytes(payload)
            source_text = payload.decode("utf-8")
            math_stats = render.math_preflight(source_text, f"Chapter {num}")
            reading_text, structure = render.prepare_reading_structure(source_text)
            reading_text, editorial = publication.apply_reader_edits(reading_text, num)
            converted, figures = render.plan_figures(reading_text, num, part_number, directory, OUTPUT)
            converted, duplicate_figure_labels = publication.remove_duplicate_figure_labels(converted)
            editorial["removed_duplicate_figure_labels"] = duplicate_figure_labels
            editorial["remaining_merge_markers"] = publication.editorial_quality_report(converted)
            report["publication_edits"].append(editorial)
            indexed_source, index_entries = idx.insert_index_tokens(converted, num)
            prepared_math, math_entries = render.plan_math(indexed_source, num)
            prepared_math, fence_metadata_removed = render.normalize_code_fence_metadata(prepared_math)
            if len(math_entries) != math_stats["display_math"]:
                raise ValueError(f"Chapter {num}: display math count changed during reader reflow")
            render_path = content_dir / (fragment + ".reader.md")
            render_path.write_text(prepared_math, encoding="utf-8")
            report["diagrams"].extend(figures)
            command = [pandoc, "-f",
                       "markdown+wikilinks_title_after_pipe+tex_math_single_backslash+tex_math_double_backslash",
                       "-t", "latex", "--top-level-division=section",
                       "--wrap=none", "--no-highlight",
                       "-o", str(tex_path), str(render_path)]
            run = subprocess.run(command, capture_output=True, text=True)
            if run.returncode:
                raise RuntimeError(f"Pandoc failed for chapter {num}: {run.stderr[:3000]}")
            if not tex_path.is_file() or not tex_path.stat().st_size:
                raise RuntimeError(f"Missing/nontext TeX output for chapter {num}")
            tex.append(r"\chapter{" + tex_escape(CHAPTER_TITLES[num - 1]) + "}")
            amend_generated_tex(tex_path, figures, math_stats, num, math_entries, index_entries)
            tex.append(r"\input{content/" + fragment + "}")
            clean = {k: v for k, v in row.items() if not k.startswith("_")}
            clean.update({"part": name, "markdown_sha256": sha256(payload),
                          "render_markdown_path": render_path.relative_to(OUTPUT).as_posix(),
                          "render_markdown_sha256": sha256(render_path.read_bytes()),
                          "math_preflight": {key: value for key, value in math_stats.items() if not key.startswith("_")},
                          "layout_reflow": structure,
                          "academic_editorial_edits": {k: v for k, v in editorial.items() if k != "removed_editorial_notes"},
                          "protected_math_displays": len(math_entries),
                          "index_entry_count": len(index_entries),
                          "code_fence_metadata_normalized": fence_metadata_removed,
                          "figure_count": len(figures),
                          "reader_title": CHAPTER_TITLES[num - 1],
                          "latex_sha256": sha256(tex_path.read_bytes()),
                          "markdown_path": (raw_path.relative_to(OUTPUT)).as_posix(),
                          "latex_path": (tex_path.relative_to(OUTPUT)).as_posix()})
            report["chapters"].append(clean)
            part_rows.append(num)
            if num in ENRICHMENT_FILES:
                extra_source = ENRICHMENT_FILES[num]
                extra_bytes = extra_source.read_bytes()
                extra_md = content_dir / f"academic-ch{num:02d}.md"
                extra_rendered = content_dir / f"academic-ch{num:02d}.reader.md"
                extra_tex = content_dir / f"academic-ch{num:02d}.tex"
                extra_md.write_bytes(extra_bytes)
                extra_text = extra_bytes.decode("utf-8")
                extra_stats = render.math_preflight(extra_text, f"Enrichment {num}")
                extra_figured, extra_diagrams = render.plan_figures(
                    extra_text, num, part_number, directory, OUTPUT,
                    first_index=len(figures) + 1,
                )
                extra_indexed, extra_index = idx.insert_index_tokens(extra_figured, num)
                extra_ready, extra_math = render.plan_math(extra_indexed, num)
                extra_ready, extra_code_ids = render.normalize_code_fence_metadata(extra_ready)
                if len(extra_math) != extra_stats["display_math"]:
                    raise ValueError(f"Enrichment {num} lost display math before Pandoc")
                extra_rendered.write_text(extra_ready, encoding="utf-8")
                report["diagrams"].extend(extra_diagrams)
                cmd = [pandoc, "-f",
                       "markdown+wikilinks_title_after_pipe+tex_math_single_backslash+tex_math_double_backslash",
                       "-t", "latex", "--top-level-division=section",
                       "--wrap=none", "--no-highlight",
                       "-o", str(extra_tex), str(extra_rendered)]
                run = subprocess.run(cmd, capture_output=True, text=True)
                if run.returncode or not extra_tex.is_file():
                    raise RuntimeError(f"Pandoc could not render academic enrichment {num}: " + run.stderr[:2500])
                amend_generated_tex(extra_tex, extra_diagrams, extra_stats, num, extra_math, extra_index)
                tex.append(r"\input{content/academic-ch" + f"{num:02d}" + "}")
                report["enrichments"].append({
                    "chapter": num,
                    "source_path": extra_source.relative_to(ROOT).as_posix(),
                    "source_sha256": sha256(extra_bytes),
                    "markdown_path": extra_md.relative_to(OUTPUT).as_posix(),
                    "markdown_sha256": sha256(extra_md.read_bytes()),
                    "render_markdown_path": extra_rendered.relative_to(OUTPUT).as_posix(),
                    "render_markdown_sha256": sha256(extra_rendered.read_bytes()),
                    "latex_path": extra_tex.relative_to(OUTPUT).as_posix(),
                    "latex_sha256": sha256(extra_tex.read_bytes()),
                    "figure_count": len(extra_diagrams),
                    "index_entry_count": len(extra_index),
                    "protected_math_displays": len(extra_math),
                    "code_fence_metadata_normalized": extra_code_ids,
                })
        # The multi-project competency chapter belongs AFTER all sixteen
        # original chapters. It has its own documented source identity and
        # does not masquerade as a nineteenth exact-anchor insertion.
        if part_number == 4:
            challenge_md = content_dir / "final-mastery.md"
            challenge_tex = content_dir / "final-mastery.tex"
            challenge_bytes = CAPSTONE.read_bytes()
            challenge_md.write_bytes(challenge_bytes)
            challenge_stats = render.math_preflight(challenge_bytes.decode("utf-8"), "Final capstone")
            cap_figured, cap_figures = render.plan_figures(challenge_bytes.decode("utf-8"), 17, part_number, directory, OUTPUT)
            cap_indexed, cap_index = idx.insert_index_tokens(cap_figured, 17)
            cap_prepared, cap_math = render.plan_math(cap_indexed, 17)
            cap_prepared, cap_fences = render.normalize_code_fence_metadata(cap_prepared)
            if len(cap_math) != challenge_stats["display_math"]:
                raise ValueError("Final capstone lost display math before Pandoc")
            cap_reader = content_dir / "final-mastery.reader.md"
            cap_reader.write_text(cap_prepared, encoding="utf-8")
            report["diagrams"].extend(cap_figures)
            command = [pandoc, "-f",
                       "markdown+wikilinks_title_after_pipe+tex_math_single_backslash+tex_math_double_backslash",
                       "-t", "latex", "--top-level-division=chapter",
                       "--wrap=none", "--no-highlight",
                       "-o", str(challenge_tex), str(cap_reader)]
            run = subprocess.run(command, capture_output=True, text=True)
            if run.returncode or not challenge_tex.is_file() or not challenge_tex.stat().st_size:
                raise RuntimeError("Pandoc could not render the final capstone: " + run.stderr[:3000])
            amend_generated_tex(challenge_tex, cap_figures, challenge_stats, 17, cap_math, cap_index)
            tex.append(r"\input{content/final-mastery}")
            tex.append(r"\appendix")
            tex.append(r"\chapter{Learning Journal and Experiment Checklist}")
            tex.append(r"\begin{BookLearningNote}[How to use this appendix]")
            tex.append(r"Treat each completed lab as evidence: retain its inputs, expected behavior, observed behavior, environment, and reproducible test.")
            tex.append(r"\end{BookLearningNote}")
            tex.append(r"\section{Experiment record}")
            tex.append(r"\begin{BookPractice}[Repeatable experiment template]")
            tex.append(r"Question; hypothesis; baseline; test setup; inputs and seeds; measurement units; observations; failures; alternative explanations; next test.")
            tex.append(r"\end{BookPractice}")
            tex.append(r"\section{Code review checklist}")
            tex.append(r"Review ownership, lifetimes, failure handling, concurrency, input validation, resource limits, reproducibility, and whether measured results support the claim.")
            tex.append(r"\section{Personal research notes}")
            tex.append(r"Record questions, unexpected outcomes, experiments that failed, and evidence that would revise your conclusions.")
            report["final_capstone"] = {
                "markdown_path": challenge_md.relative_to(OUTPUT).as_posix(),
                "markdown_sha256": sha256(challenge_bytes),
                "render_markdown_path": cap_reader.relative_to(OUTPUT).as_posix(),
                "render_markdown_sha256": sha256(cap_reader.read_bytes()),
                "protected_math_displays": len(cap_math),
                "code_fence_metadata_normalized": cap_fences,
                "figure_count": len(cap_figures),
                "index_entry_count": len(cap_index),
                "latex_path": challenge_tex.relative_to(OUTPUT).as_posix(),
                "latex_sha256": sha256(challenge_tex.read_bytes()),
            }
        tex.append(r"\end{document}")
        master_path = directory / "main.tex"
        master_path.write_text("\n".join(tex) + "\n", encoding="utf-8")
        report["parts"].append({"id": name, "chapters": part_rows,
                                "master_path": (master_path.relative_to(OUTPUT)).as_posix(),
                                "master_sha256": sha256(master_path.read_bytes())})
    # An actual single-book LaTeX master: one cover, one continuous
    # page-number sequence, one complete table of contents, all 16 chapters.
    # Four part masters remain available for isolation of difficult TeX errors.
    complete_dir = OUTPUT / "book01-complete"
    complete_dir.mkdir(parents=True, exist_ok=True)
    shutil.copy2(style, complete_dir / "allmight-book.sty")
    shutil.copy2(READER_STYLE, complete_dir / "book01-reader.sty")
    complete_assets = complete_dir / "Assets"
    complete_assets.mkdir(exist_ok=True)
    complete_fonts = complete_assets / "Fonts"
    if complete_fonts.is_symlink():
        if complete_fonts.resolve() != fonts.resolve():
            raise ValueError("Unexpected complete-book font asset symlink")
    elif complete_fonts.exists():
        raise ValueError("Refusing to overwrite existing complete-book assets")
    else:
        complete_fonts.symlink_to(
            os.path.relpath(fonts, complete_assets), target_is_directory=True)
    book = [
        r"\documentclass[10pt,twoside,openany]{book}",
        r"\usepackage[ebook]{allmight-book}",
        r"\usepackage{book01-reader}",
        book_pdf_metadata(BOOK_TITLE, BOOK_SUBTITLE),
        r"\SetBookStage{1}{Bits, Programs, and Reliable Systems}",
        r"\title{" + tex_escape(BOOK_TITLE) + r"\\[0.6em]\large " + tex_escape(BOOK_SUBTITLE) + "}",
        r"\author{Swayam Singal}",
        r"\begin{document}",
        r"\frontmatter",
        r"\maketitle",
        book_imprint(BOOK_TITLE, BOOK_SUBTITLE, metadata["source_revision"]),
        r"\setcounter{tocdepth}{1}",
        r"\tableofcontents",
        r"\listoffigures",
        r"\input{frontmatter}",
        r"\mainmatter",
    ]
    for row in rows:
        number = row["number"]
        part_number = ((number - 1) // 4) + 1
        book.append(r"\chapter{" + tex_escape(CHAPTER_TITLES[number - 1]) + "}")
        book.append(
            r"\input{../book01-expanded-p" + f"{part_number:02d}" +
            r"/content/chapter-" + f"{number:02d}" + "}"
        )
        if number in ENRICHMENT_FILES:
            book.append(r"\input{../book01-expanded-p" + f"{part_number:02d}" +
                        r"/content/academic-ch" + f"{number:02d}" + "}")
    book.extend([
        r"\input{../book01-expanded-p04/content/final-mastery}",
        r"\appendix",
        r"\input{backmatter}",
        r"\clearpage\phantomsection\addcontentsline{toc}{chapter}{Index}\printindex",
        r"\end{document}",
    ])
    def prepare_preface(directory: Path, source: Path = ACADEMIC_PREFACE) -> dict:
        path = directory / "frontmatter.tex"
        cmd = [pandoc, "-f",
               "markdown+tex_math_single_backslash+tex_math_double_backslash",
               "-t", "latex", "--top-level-division=chapter",
               "--wrap=none", "--no-highlight",
               "-o", str(path), str(source)]
        conversion = subprocess.run(cmd, capture_output=True, text=True)
        if conversion.returncode or not path.is_file():
            raise RuntimeError("Preface LaTeX conversion failed: " + conversion.stderr[:2000])
        return {"path": path.relative_to(OUTPUT).as_posix(),
                "source": source.relative_to(ROOT).as_posix(),
                "sha256": sha256(path.read_bytes())}

    def prepare_backmatter(directory: Path) -> dict:
        path = directory / "backmatter.tex"
        cmd = [pandoc, "-f", "markdown+tex_math_single_backslash+tex_math_double_backslash",
               "-t", "latex", "--top-level-division=chapter", "--wrap=none",
               "--no-highlight", "-o", str(path), str(ACADEMIC_BACKMATTER)]
        result = subprocess.run(cmd, capture_output=True, text=True)
        if result.returncode or not path.is_file():
            raise RuntimeError("Academic backmatter conversion failed: " + result.stderr[:2000])
        return {"path": path.relative_to(OUTPUT).as_posix(),
                "sha256": sha256(path.read_bytes())}

    report["prefaces"] = [prepare_preface(complete_dir)]
    report["backmatters"] = [prepare_backmatter(complete_dir)]
    report["print_volumes"] = []
    for volume_number, (low, high, title, subtitle) in enumerate(PRINT_VOLUMES, 1):
        directory = OUTPUT / f"book01-print-v{volume_number:02d}"
        directory.mkdir(exist_ok=True)
        shutil.copy2(style, directory / "allmight-book.sty")
        shutil.copy2(READER_STYLE, directory / "book01-reader.sty")
        asset_root = directory / "Assets"
        asset_root.mkdir(exist_ok=True)
        font_link = asset_root / "Fonts"
        if font_link.is_symlink():
            if font_link.resolve() != fonts.resolve():
                raise ValueError("Unexpected paperback font symlink")
        elif font_link.exists():
            raise ValueError("Refusing to overwrite paperback font assets")
        else:
            font_link.symlink_to(
                os.path.relpath(fonts, asset_root), target_is_directory=True)
        preface = prepare_preface(directory, SECOND_VOLUME_PREFACE if volume_number == 2 else ACADEMIC_PREFACE)
        report["prefaces"].append(preface)
        report["backmatters"].append(prepare_backmatter(directory))
        volume = [
            r"\documentclass[10pt,twoside,openany]{book}",
            r"\usepackage[print]{allmight-book}",
            r"\usepackage{book01-reader}",
            book_pdf_metadata(title, subtitle),
            r"\SetBookStage{1}{Bits, Programs, and Reliable Systems}",
            r"\title{" + tex_escape(title) + r"\\[0.6em]\large " + tex_escape(subtitle) + "}",
            r"\author{Swayam Singal}",
            r"\begin{document}",
            r"\frontmatter", r"\maketitle",
            book_imprint(title, subtitle, metadata["source_revision"]),
            r"\setcounter{tocdepth}{1}", r"\tableofcontents", r"\listoffigures",
            r"\input{frontmatter}", r"\mainmatter",
            r"\setcounter{chapter}{" + str(low - 1) + "}",
        ]
        for number in range(low, high + 1):
            source_part = ((number - 1) // 4) + 1
            volume.append(r"\chapter{" + tex_escape(CHAPTER_TITLES[number - 1]) + "}")
            volume.append(
                r"\input{../book01-expanded-p" + f"{source_part:02d}" +
                r"/content/chapter-" + f"{number:02d}" + "}")
            if number in ENRICHMENT_FILES:
                volume.append(r"\input{../book01-expanded-p" + f"{source_part:02d}" +
                              r"/content/academic-ch" + f"{number:02d}" + "}")
        if volume_number == len(PRINT_VOLUMES):
            volume.append(r"\input{../book01-expanded-p04/content/final-mastery}")
        volume.extend([
            r"\appendix",
            r"\input{backmatter}",
            r"\clearpage\phantomsection\addcontentsline{toc}{chapter}{Index}\printindex",
            r"\end{document}",
        ])
        master = directory / "main.tex"
        master.write_text("\n".join(volume) + "\n", encoding="utf-8")
        report["print_volumes"].append({
            "volume": volume_number, "chapters": [low, high],
            "title": title, "subtitle": subtitle,
            "master_path": master.relative_to(OUTPUT).as_posix(),
            "master_sha256": sha256(master.read_bytes()),
        })
    complete_master = complete_dir / "main.tex"
    complete_master.write_text("\n".join(book) + "\n", encoding="utf-8")
    report["complete"] = {
        "master_path": complete_master.relative_to(OUTPUT).as_posix(),
        "master_sha256": sha256(complete_master.read_bytes()),
        "style_sha256": sha256((complete_dir / "book01-reader.sty").read_bytes()),
    }
    manifest = OUTPUT / "manifest.json"
    manifest.write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    verify_prepared(rows, metadata)
    print(f"PREPARED {NUMBER_OF_CHAPTERS} preserved chapters and {EXPECTED_ADDITIONS} additive lessons")
    print("One digital master, two 828-page-limited paperback masters, and four diagnostic parts prepared under", OUTPUT)
    print("Mermaid diagrams planned:", len(report["diagrams"]), "; original chapter bodies untouched")
    print("No LaTeX engine or PDF compiler has been called.")


def verify_prepared(rows: list[dict], metadata: dict) -> None:
    manifest = OUTPUT / "manifest.json"
    if not manifest.is_file():
        raise FileNotFoundError("Run --prepare first; no release manifest")
    data = json.loads(manifest.read_text("utf-8"))
    if any(data.get(key) != value for key, value in metadata.items()):
        raise ValueError("Prepared release is stale or --prepare previously failed; rerun --prepare successfully before verifying or compiling. The original manuscript has not been changed.")
    if len(data.get("chapters", [])) != NUMBER_OF_CHAPTERS or len(data.get("parts", [])) != 4:
        raise ValueError("Incomplete Book01 release manifest")
    if sum(len(x.get("lesson_receipts", [])) for x in data["chapters"]) != EXPECTED_ADDITIONS:
        raise ValueError("Missing an additive lesson")
    for authored, recorded in zip(rows, data["chapters"]):
        if authored["original_body_sha256"] != recorded["original_body_sha256"]:
            raise ValueError("Original chapter changed since --prepare")
        if authored["expanded_body_sha256"] != recorded["expanded_body_sha256"]:
            raise ValueError("Original/additive composition changed since --prepare")
        for kind in ("markdown", "latex", "render_markdown"):
            path = OUTPUT / recorded[kind + "_path"]
            if not path.is_file() or sha256(path.read_bytes()) != recorded[kind + "_sha256"]:
                raise ValueError("Prepared chapter " + kind + " altered or missing: " + str(path))
    if len(data.get("enrichments", [])) != len(ENRICHMENT_FILES):
        raise ValueError("Academic enrichment sources missing in generated reader")
    for extra in data["enrichments"]:
        chapter = extra["chapter"]
        source = ENRICHMENT_FILES[chapter]
        if sha256(source.read_bytes()) != extra["source_sha256"]:
            raise ValueError("Academic enrichment source changed: " + str(source))
        for kind in ("markdown", "render_markdown", "latex"):
            path = OUTPUT / extra[kind + "_path"]
            if not path.is_file() or sha256(path.read_bytes()) != extra[kind + "_sha256"]:
                raise ValueError("Generated academic enrichment changed: " + str(path))
    capstone = data.get("final_capstone")
    if not isinstance(capstone, dict):
        raise ValueError("Missing multi-project final mastery chapter")
    for kind in ("markdown", "render_markdown", "latex"):
        path = OUTPUT / capstone[kind + "_path"]
        if not path.is_file() or sha256(path.read_bytes()) != capstone[kind + "_sha256"]:
            raise ValueError("Final mastery " + kind + " missing or changed: " + str(path))
    if capstone["markdown_sha256"] != metadata["final_capstone_sha256"]:
        raise ValueError("Final capstone does not match committed source")
    complete = data.get("complete")
    if not isinstance(complete, dict):
        raise ValueError("Missing single-volume source and its provenance")
    complete_master = OUTPUT / complete["master_path"]
    if not complete_master.is_file() or sha256(complete_master.read_bytes()) != complete["master_sha256"]:
        raise ValueError("Complete-book LaTeX master changed or missing")
    complete_style = complete_master.parent / "book01-reader.sty"
    if not complete_style.is_file() or sha256(complete_style.read_bytes()) != metadata["reader_style_sha256"]:
        raise ValueError("Complete-book typography changed since preparation")
    if len(data.get("backmatters", [])) != 3:
        raise ValueError("Academic apparatus not generated for all outputs")
    for appendix in data["backmatters"]:
        path = OUTPUT / appendix["path"]
        if not path.is_file() or sha256(path.read_bytes()) != appendix["sha256"]:
            raise ValueError("Academic backmatter missing or changed")
    for preface in data.get("prefaces", []):
        path = OUTPUT / preface["path"]
        if not path.is_file() or sha256(path.read_bytes()) != preface["sha256"]:
            raise ValueError("Academic frontmatter missing or changed")
    if len(data.get("print_volumes", [])) != len(PRINT_VOLUMES):
        raise ValueError("Paperback volume masters incomplete")
    for record in data["print_volumes"]:
        path = OUTPUT / record["master_path"]
        if not path.is_file() or sha256(path.read_bytes()) != record["master_sha256"]:
            raise ValueError("Paperback volume master stale or altered")
    for fig in data.get("diagrams", []):
        path = OUTPUT / fig["source"]
        if not path.is_file() or sha256(path.read_bytes()) != fig["diagram_sha256"]:
            raise ValueError("Mermaid source missing or modified: " + str(path))
    if sum(record.get("figure_count", 0) for record in data["chapters"]) + sum(extra.get("figure_count", 0) for extra in data["enrichments"]) + capstone.get("figure_count", 0) != len(data.get("diagrams", [])):
        raise ValueError("Diagram manifest does not account for all figures")
    for record in data["parts"]:
        reader_style = OUTPUT / record["id"] / "book01-reader.sty"
        if not reader_style.is_file() or sha256(reader_style.read_bytes()) != metadata["reader_style_sha256"]:
            raise ValueError("Reader style missing or stale; rerun --prepare")
        path = OUTPUT / record["master_path"]
        if not path.is_file() or sha256(path.read_bytes()) != record["master_sha256"]:
            raise ValueError("Prepared LaTeX master altered: " + str(path))
    print("VERIFIED prepared output and exact source provenance:", metadata["chapter_count"],
          "chapters,", metadata["addition_count"], "lessons, four LaTeX parts")


def compile_parts(rows: list[dict], metadata: dict, part: int | None, open_pdf: bool = False, paperback: bool = False) -> None:
    verify_prepared(rows, metadata)
    if paperback and part is not None:
        raise ValueError("Use paperback or --part; not both")
    # Figure inclusion is mandatory: do not compile an incomplete book.
    manifest_path = OUTPUT / "manifest.json"
    manifest = json.loads(manifest_path.read_text("utf-8"))
    render.render_figures(manifest, OUTPUT)
    # Cache source-matched PNG digests so diagnostic/full builds do not rerender
    # identical diagrams every time. This is generated output, not manuscript.
    manifest_path.write_text(json.dumps(manifest, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    engine = shutil.which("latexmk")
    lualatex = shutil.which("lualatex")
    if not lualatex:
        raise RuntimeError("lualatex unavailable: install a compatible TeX Live/MacTeX distribution")
    # BasicTeX often has LuaLaTeX but not latexmk. Re-run LuaLaTeX itself
    # for cross-references/TOC rather than forcing installation of full MacTeX.
    # Default is one continuous TeX book, not four compiled volumes
    # merged into a PDF with repeated covers and incomplete contents.
    if paperback:
        selected = [
            (OUTPUT / f"book01-print-v{n:02d}",
             OUTPUT / ("Book01-From-Bits-to-Programs-KDP-Volume-01.pdf" if n == 1 else "Book01-From-Programs-to-GPUs-KDP-Volume-02.pdf"))
            for n in range(1, len(PRINT_VOLUMES) + 1)
        ]
    elif part is None:
        selected = [(OUTPUT / "book01-complete",
                     OUTPUT / "Book01-From-Bits-to-Parallel-Systems-COMPLETE.pdf")]
    else:
        selected = [(OUTPUT / f"book01-expanded-p{part:02d}",
                     OUTPUT / f"Book01-Bits-Programs-Reliable-Systems-P{part:02d}.pdf")]
    for directory, destination in selected:
        (directory / "build").mkdir(exist_ok=True)
        if engine:
            commands = [[engine, "-g", "-lualatex", "-interaction=nonstopmode",
                         "-halt-on-error", "-file-line-error", "-outdir=build", "main.tex"]]
        else:
            commands = [[lualatex, "-interaction=nonstopmode", "-halt-on-error",
                         "-file-line-error", "-output-directory=build", "main.tex"]] * 3
        print("Compiling", directory.name, flush=True)
        pdf = directory / "build/main.pdf"
        for pass_number, command in enumerate(commands, 1):
            result = subprocess.run(command, cwd=directory, stdout=subprocess.PIPE,
                                    stderr=subprocess.STDOUT, text=True, errors="replace")
            (directory / "build" / f"compile-pass-{pass_number}.txt").write_text(
                result.stdout, encoding="utf-8")
            if result.returncode:
                excerpt = "\n".join(result.stdout.splitlines()[-35:])
                raise RuntimeError(
                    "LaTeX compilation failed in " + directory.name +
                    " (pass " + str(pass_number) + "); see " +
                    str(directory / "build/main.log") + "\n" + excerpt)
            print("  TeX pass", pass_number, "OK;",
                  result.stdout.count("Overfull \\hbox"), "overfull line warnings")
        # A genuine word index needs a makeindex pass after the first LuaLaTeX
        # pass. latexmk typically discovers this itself; BasicTeX's direct
        # lualatex fallback needs explicit handling and two subsequent passes.
        idx_file = directory / "build/main.idx"
        ind_file = directory / "build/main.ind"
        if idx_file.is_file() and idx_file.stat().st_size > 0:
            if not ind_file.is_file() or ind_file.stat().st_mtime < idx_file.stat().st_mtime:
                makeindex = shutil.which("makeindex")
                if not makeindex:
                    raise RuntimeError("Index source exists, but makeindex is unavailable. Install the TeX Live index utility.")
                result = subprocess.run([makeindex, "-o", "build/main.ind", "build/main.idx"],
                                        cwd=directory, capture_output=True, text=True)
                if result.returncode or not ind_file.is_file():
                    raise RuntimeError("makeindex failed: " + (result.stderr or result.stdout)[-2000:])
                for i in range(2):
                    rerun = subprocess.run(
                        [lualatex, "-interaction=nonstopmode", "-halt-on-error",
                         "-file-line-error", "-output-directory=build", "main.tex"],
                        cwd=directory, stdout=subprocess.PIPE,
                        stderr=subprocess.STDOUT, text=True, errors="replace"
                    )
                    (directory / "build" / f"index-final-pass-{i+1}.txt").write_text(
                        rerun.stdout, encoding="utf-8")
                    if rerun.returncode:
                        raise RuntimeError("LuaLaTeX failed after building the index: " +
                                           "\n".join(rerun.stdout.splitlines()[-35:]))
            if not ind_file.is_file() or not ind_file.stat().st_size:
                raise RuntimeError("Index has entries but generated no index output")
        if not pdf.is_file() or not pdf.stat().st_size:
            raise RuntimeError("No PDF from " + directory.name + "; see " + str(directory / "build/main.log"))
        # Do not publish a file under the KDP release name until preflight passes.
        pages = render.pdf_preflight(pdf, paperback=paperback)
        shutil.copy2(pdf, destination)
        print("PDF:", destination, "—", pages, "pages")
        if not paperback and pages > render.KDP_MAX_WHITE_BW:
            print("NOTE: continuous digital PDF exceeds 828-page paperback limit; use --compile-paperback")
        if open_pdf:
            opener = shutil.which("open") if sys.platform == "darwin" else shutil.which("xdg-open")
            if opener:
                subprocess.Popen([opener, str(destination)], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            else:
                print("Open manually:", destination)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    choice = parser.add_mutually_exclusive_group(required=True)
    choice.add_argument("--check", action="store_true", help="Read-only source-preservation audit")
    choice.add_argument("--prepare", action="store_true", help="Generate Markdown + TeX; never compile")
    choice.add_argument("--verify-prepared", action="store_true", help="Verify generated sources, never compile")
    choice.add_argument("--compile", action="store_true", help="Compile the continuous digital book")
    choice.add_argument("--compile-paperback", action="store_true", help="Compile two KDP 7x10 paperback interiors with 828-page ceiling")
    parser.add_argument("--part", type=int, choices=(1, 2, 3, 4),
                        help="For --compile, build only this part")
    parser.add_argument("--open", action="store_true", help="Open each successfully compiled PDF in the default viewer")
    args = parser.parse_args()
    if args.open and not (args.compile or args.compile_paperback):
        parser.error("--open requires --compile or --compile-paperback")
    if args.part and not args.compile:
        parser.error("--part is only meaningful with --compile")
    try:
        rows, metadata = audit_sources()
        print("SOURCE CHECK PASSED:", len(rows), "original chapters,",
              sum(len(x["lesson_receipts"]) for x in rows), "reversible additions")
        if args.prepare:
            prepare(rows, metadata)
        elif args.verify_prepared:
            verify_prepared(rows, metadata)
        elif args.compile or args.compile_paperback:
            compile_parts(rows, metadata, args.part, args.open, args.compile_paperback)
    except (OSError, ValueError, RuntimeError, AssertionError, UnicodeError) as exc:
        print("Book01 release refused:", exc, file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
