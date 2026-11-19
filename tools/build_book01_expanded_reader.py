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

ROOT = Path(__file__).resolve().parent.parent
MASTER = ROOT / "Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md"
EDITION = ROOT / "Chapter Edition/01 - Computing Foundations"
LATEX = ROOT / "Book Production/AllMight Codex/LaTeX"
OUTPUT = ROOT / "build/book01-expanded-reader"
SOURCE_BEGIN = b"<!-- CHAPTER-SOURCE-BEGIN -->\n"
SOURCE_END = b"<!-- CHAPTER-SOURCE-END -->"
PARTS = ((1, 4), (5, 8), (9, 12), (13, 16))
NUMBER_OF_CHAPTERS = 16
EXPECTED_ADDITIONS = 18


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
        if result.count(spec.anchor) != 1:
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
    mapped = specs_by_chapter()
    all_specs = list(inserts.INSERTS)
    for spec in all_specs:
        if master.count(spec.anchor) != 1:
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
    metadata = {
        "schema": "allmight-book01-expanded-reader-v1",
        "scope": "16 original Foundations chapters, 18 additive lessons; distinct from the archived seven-stage typeset snapshots",
        "canonical_source": MASTER.relative_to(ROOT).as_posix(),
        "canonical_source_sha256": sha256(master),
        "canonical_source_git_blob": git_blob(master),
        "chapter_count": NUMBER_OF_CHAPTERS,
        "addition_count": total,
        "part_ranges": [list(x) for x in PARTS],
    }
    return rows, metadata


def tex_escape(text: str) -> str:
    mapping = {"\\": r"\textbackslash{}", "{": r"\{", "}": r"\}",
               "&": r"\&", "%": r"\%", "$": r"\$", "#": r"\#",
               "_": r"\_", "~": r"\textasciitilde{}", "^": r"\textasciicircum{}"}
    return "".join(mapping.get(char, char) for char in text)


def pandoc_binary() -> str:
    binary = shutil.which("pandoc")
    if not binary:
        raise RuntimeError("Pandoc is required for --prepare: brew install pandoc")
    return binary


def prepare(rows: list[dict], metadata: dict) -> None:
    pandoc = pandoc_binary()
    OUTPUT.mkdir(parents=True, exist_ok=True)
    style = LATEX / "allmight-book.sty"
    fonts = LATEX / "Assets/Fonts"
    if not style.is_file() or not (fonts / "NotoEmoji-Static.ttf").is_file():
        raise FileNotFoundError("The existing LaTeX profile and its bundled local font must be available")
    report = {**metadata, "parts": [], "chapters": []}
    for part_number, (low, high) in enumerate(PARTS, 1):
        name = f"book01-expanded-p{part_number:02d}"
        directory = OUTPUT / name
        content_dir = directory / "content"
        content_dir.mkdir(parents=True, exist_ok=True)
        shutil.copy2(style, directory / "allmight-book.sty")
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
               r"\usepackage[print]{allmight-book}",
               r"\SetBookStage{1}{Bits, Programs, and Reliable Systems}",
               r"\title{Bits, Programs, and Reliable Systems — Expanded Reader, Part " + str(part_number) + " of 4}",
               r"\author{Swayam Singal}",
               r"\begin{document}", r"\maketitle",
               r"\tableofcontents", r"\mainmatter",
               r"\setcounter{chapter}{" + str(low - 1) + "}"]
        part_rows = []
        for row in rows[low - 1:high]:
            num = row["number"]
            fragment = f"chapter-{num:02d}"
            raw_path = content_dir / (fragment + ".md")
            tex_path = content_dir / (fragment + ".tex")
            payload = row["_preface"] + b"\n" + row["_expanded"]
            raw_path.write_bytes(payload)
            command = [pandoc, "-f",
                       "markdown+wikilinks_title_after_pipe+tex_math_single_backslash+tex_math_double_backslash",
                       "-t", "latex", "--top-level-division=section",
                       "--wrap=none", "--no-highlight",
                       "-o", str(tex_path), str(raw_path)]
            run = subprocess.run(command, capture_output=True, text=True)
            if run.returncode:
                raise RuntimeError(f"Pandoc failed for chapter {num}: {run.stderr[:3000]}")
            if not tex_path.is_file() or not tex_path.stat().st_size:
                raise RuntimeError(f"Missing/nontext TeX output for chapter {num}")
            tex.append(r"\chapter{" + tex_escape(row["title"]) + "}")
            tex.append(r"\input{content/" + fragment + "}")
            clean = {k: v for k, v in row.items() if not k.startswith("_")}
            clean.update({"part": name, "markdown_sha256": sha256(payload),
                          "latex_sha256": sha256(tex_path.read_bytes()),
                          "markdown_path": (raw_path.relative_to(OUTPUT)).as_posix(),
                          "latex_path": (tex_path.relative_to(OUTPUT)).as_posix()})
            report["chapters"].append(clean)
            part_rows.append(num)
        tex.append(r"\end{document}")
        master_path = directory / "main.tex"
        master_path.write_text("\n".join(tex) + "\n", encoding="utf-8")
        report["parts"].append({"id": name, "chapters": part_rows,
                                "master_path": (master_path.relative_to(OUTPUT)).as_posix(),
                                "master_sha256": sha256(master_path.read_bytes())})
    manifest = OUTPUT / "manifest.json"
    manifest.write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    verify_prepared(rows, metadata)
    print(f"PREPARED {NUMBER_OF_CHAPTERS} preserved chapters and {EXPECTED_ADDITIONS} additive lessons")
    print("Four independent LuaLaTeX book parts ready under", OUTPUT)
    print("No LaTeX engine or PDF compiler has been called.")


def verify_prepared(rows: list[dict], metadata: dict) -> None:
    manifest = OUTPUT / "manifest.json"
    if not manifest.is_file():
        raise FileNotFoundError("Run --prepare first; no release manifest")
    data = json.loads(manifest.read_text("utf-8"))
    if any(data.get(key) != value for key, value in metadata.items()):
        raise ValueError("Prepared release no longer matches the current canonical source")
    if len(data.get("chapters", [])) != NUMBER_OF_CHAPTERS or len(data.get("parts", [])) != 4:
        raise ValueError("Incomplete Book01 release manifest")
    if sum(len(x.get("lesson_receipts", [])) for x in data["chapters"]) != EXPECTED_ADDITIONS:
        raise ValueError("Missing an additive lesson")
    for authored, recorded in zip(rows, data["chapters"]):
        if authored["original_body_sha256"] != recorded["original_body_sha256"]:
            raise ValueError("Original chapter changed since --prepare")
        if authored["expanded_body_sha256"] != recorded["expanded_body_sha256"]:
            raise ValueError("Original/additive composition changed since --prepare")
        for kind in ("markdown", "latex"):
            path = OUTPUT / recorded[kind + "_path"]
            if not path.is_file() or sha256(path.read_bytes()) != recorded[kind + "_sha256"]:
                raise ValueError("Prepared chapter " + kind + " altered or missing: " + str(path))
    for record in data["parts"]:
        path = OUTPUT / record["master_path"]
        if not path.is_file() or sha256(path.read_bytes()) != record["master_sha256"]:
            raise ValueError("Prepared LaTeX master altered: " + str(path))
    print("VERIFIED prepared output and exact source provenance:", metadata["chapter_count"],
          "chapters,", metadata["addition_count"], "lessons, four LaTeX parts")


def compile_parts(rows: list[dict], metadata: dict, part: int | None) -> None:
    verify_prepared(rows, metadata)
    engine = shutil.which("latexmk")
    if not engine:
        raise RuntimeError("LuaLaTeX build requires latexmk: install MacTeX (or BasicTeX plus required packages)")
    if not shutil.which("lualatex"):
        raise RuntimeError("lualatex unavailable: install a compatible TeX Live/MacTeX distribution")
    selected = [part] if part else range(1, 5)
    for number in selected:
        directory = OUTPUT / f"book01-expanded-p{number:02d}"
        (directory / "build").mkdir(exist_ok=True)
        cmd = [engine, "-g", "-lualatex", "-interaction=nonstopmode",
               "-halt-on-error", "-file-line-error", "-outdir=build", "main.tex"]
        print("Compiling", directory.name, flush=True)
        result = subprocess.run(cmd, cwd=directory)
        pdf = directory / "build/main.pdf"
        if result.returncode or not pdf.is_file():
            raise RuntimeError("LaTeX compilation failed; see " + str(directory / "build/main.log"))
        destination = OUTPUT / f"Book01-Bits-Programs-Reliable-Systems-P{number:02d}.pdf"
        shutil.copy2(pdf, destination)
        print("PDF:", destination)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    choice = parser.add_mutually_exclusive_group(required=True)
    choice.add_argument("--check", action="store_true", help="Read-only source-preservation audit")
    choice.add_argument("--prepare", action="store_true", help="Generate Markdown + TeX; never compile")
    choice.add_argument("--verify-prepared", action="store_true", help="Verify generated sources, never compile")
    choice.add_argument("--compile", action="store_true", help="Explicitly compile PDFs on this machine")
    parser.add_argument("--part", type=int, choices=(1, 2, 3, 4),
                        help="For --compile, build only this part")
    args = parser.parse_args()
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
        elif args.compile:
            compile_parts(rows, metadata, args.part)
    except (OSError, ValueError, RuntimeError, AssertionError, UnicodeError) as exc:
        print("Book01 release refused:", exc, file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
