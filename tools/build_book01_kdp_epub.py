#!/usr/bin/env python3
"""Build a source-based, reflowable EPUB 3 from Book 1's prepared reader.

This deliberately does NOT convert PDF pages into screenshots. It restores
exact source mathematics, embeds all rendered diagram PNGs, retains the
expanded chapter/project text and produces semantic, reflowable XHTML.

Prerequisites, from repository root:
    python tools/build_book01_expanded_reader.py --check
    python tools/build_book01_expanded_reader.py --prepare
    python tools/build_book01_expanded_reader.py --verify-prepared
    python tools/build_book01_kdp_epub.py --cover "path/to/front-cover.jpg"

A cover asset is required; do not silently substitute a cover or publish an
incomplete ebook. Pandoc 3+ plus the Mermaid CLI (mmdc) are needed. Run the
result through EPUBCheck and Amazon Kindle Previewer before uploading to KDP.
"""
from __future__ import annotations

import argparse
import html
import json
from pathlib import Path
import re
import subprocess
import sys
import zipfile
from xml.etree import ElementTree as ET

import book01_render_support as render
import book01_learning_gates as gates
from book01_editorial import (
    SERIES_TITLE, SERIES_SUBTITLE, SERIES_POSITION,
    BOOK_TITLE, BOOK_SUBTITLE, CHAPTER_TITLES,
)
import build_book01_expanded_reader as builder

ROOT = Path(__file__).resolve().parent.parent
OUTPUT = builder.OUTPUT
BOOK1 = ROOT / "Book Production/Seven-Book Curriculum/Book01"
LATEX_INLINE_INDEX = re.compile(r"(?m)^[ \t]*\\index\{[^\n]*\}[ \t]*\n?")
MARKER = re.compile(r"ALLMIGHT(?:MATH|FIGURE|INDEX)CH\d+[A-Z]*INDEX\d+")


def run(command: list[str], **kwargs) -> None:
    result = subprocess.run(command, capture_output=True, text=True, **kwargs)
    if result.returncode:
        raise RuntimeError("Command failed:\n  " + " ".join(command)
                           + "\n" + result.stderr[-5000:])


def protect_markdown_fences(source: str, callback) -> str:
    """Apply prose-only transforms without touching any fenced code."""
    out: list[str] = []
    cursor = 0
    for match in render.FENCE.finditer(source):
        out.append(callback(source[cursor:match.start()]))
        out.append(match.group(0))
        cursor = match.end()
    out.append(callback(source[cursor:]))
    return "".join(out)


def restore_math(reader: str, raw: str, chapter: int) -> str:
    _protected, entries = render.plan_math(raw, chapter)
    for item in entries:
        token = item["token"]
        if reader.count(token) != 1:
            raise ValueError(f"Math source missing or duplicated in chapter {chapter}: {token}")
        # The source may contain literal percent signs in prose-like math
        # and underscores inside text macros. Normalize only those TeX
        # text-mode hazards before the EPUB MathML conversion, exactly as
        # the independent PDF rendering pipeline does.
        expression = render.sanitize_source_math_for_tex(item["body"].strip())
        # EPUB3 uses MathML from Pandoc's TeX-math parser; do not flatten
        # formulas into text or rasterized screenshots.
        reader = reader.replace(token, "\n\n$$\n" + expression + "\n$$\n\n")
    if re.search(r"ALLMIGHTMATHCH\d+INDEX\d+", reader):
        raise ValueError(f"Unresolved source-math marker in chapter {chapter}")
    return reader


def restore_diagrams(reader: str, diagrams: list[dict], chapter: int) -> str:
    for item in diagrams:
        token = item["token"]
        if reader.count(token) != 1:
            raise ValueError(f"Figure missing or duplicated in chapter {chapter}: {token}")
        source = OUTPUT / item["image"]
        if not source.exists() or source.stat().st_size < 500:
            raise FileNotFoundError("Missing rendered diagram: " + str(source))
        title = re.sub(r"^Figure\s+\d+\.\d+\s*:\s*", "", item["caption"])
        title = re.sub(r"^Figure\s+(?:B\d+|CPP[- ]\d+)\s*[-:]\s*", "", title)
        title = title.replace("[", "").replace("]", "").replace('"', "")
        # Pandoc accepts <...> around paths that contain spaces.
        markdown = '\n\n![' + title + '](<' + source.as_posix() + '>)\n\n'
        reader = reader.replace(token, markdown)
    if re.search(r"ALLMIGHTFIGURECH\d+INDEX\d+", reader):
        raise ValueError(f"Unresolved figure marker in chapter {chapter}")
    return reader


def prepare_fragment(
    raw_file: Path, reader_file: Path, chapter: int, diagrams: list[dict],
    level_shift: int = 0,
) -> str:
    raw = raw_file.read_text(encoding="utf-8")
    reader = reader_file.read_text(encoding="utf-8")
    reader = restore_math(reader, raw, chapter)
    reader = restore_diagrams(reader, diagrams, chapter)
    reader = LATEX_INLINE_INDEX.sub("", reader)
    if "\\index{" in reader:
        raise ValueError("Unremoved page-index command remains: " + str(reader_file))
    if level_shift:
        def promote(fragment: str) -> str:
            return re.sub(r"(?m)^(#{1,5})([ \t]+)",
                          lambda m: "#" * min(6, len(m.group(1)) + level_shift)
                          + m.group(2), fragment)
        reader = protect_markdown_fences(reader, promote)
    if MARKER.search(reader):
        raise ValueError("Unresolved production marker in: " + str(reader_file))
    return reader


def combine_book(manifest: dict) -> str:
    parts = [
        "---\n"
        'title: "' + BOOK_TITLE + '"\n'
        'subtitle: "' + BOOK_SUBTITLE + '"\n'
        'author: "Swayam Singal"\n'
        'lang: en-US\n'
        'rights: "Copyright © 2026 Swayam Singal. All rights reserved."\n'
        "---\n\n",
        "# Title and Copyright\n\n"
        "**From Bits to Research — Book One of Seven**\n\n"
        "**From Bits to Parallel Systems**\n\n"
        "*C++23, CPU Execution, GPU Memory, and Reliable Software*\n\n"
        "Swayam Singal\n\n"
        "Copyright © 2026 Swayam Singal. All rights reserved. "
        "Third-party works retain their own copyrights and licences.\n\n"
        "References, companion code and corrections: "
        "[Project AllMight](https://github.com/swayam8624/Project_008_AllMight).\n\n",
        builder.ACADEMIC_PREFACE.read_text(encoding="utf-8") + "\n\n",
    ]
    chapters = {int(item["number"]): item for item in manifest["chapters"]}
    diagrams = manifest.get("diagrams", [])
    for num in range(1, 17):
        record = chapters[num]
        directory = OUTPUT / record["part"] / "content"
        subset = [item for item in diagrams
                  if int(item["chapter"]) == num
                  and int(item["part"]) == int(record["part"][-2:])
                  and int(item["index"]) > 0]
        # The source and enrichment each start a new figure counter. Their
        # manifest diagrams therefore use the *same* chapter but different
        # indices. The builder sets first_index to avoid collisions.
        base_figs = [item for item in subset if item["token"] in
                     (directory / f"chapter-{num:02d}.reader.md").read_text("utf-8")]
        parts.append("# " + CHAPTER_TITLES[num - 1] + "\n\n")
        parts.append(prepare_fragment(
            directory / f"chapter-{num:02d}.md",
            directory / f"chapter-{num:02d}.reader.md",
            num, base_figs, level_shift=1,
        ))
        enrichment_raw = directory / f"academic-ch{num:02d}.md"
        enrichment_reader = directory / f"academic-ch{num:02d}.reader.md"
        if enrichment_raw.is_file():
            enriched = enrichment_reader.read_text(encoding="utf-8")
            enrichment_figs = [item for item in subset if item["token"] in enriched]
            parts.append("\n\n" + prepare_fragment(
                enrichment_raw, enrichment_reader, num, enrichment_figs,
                level_shift=1,
            ))
        names = ("Reconstruct", "Falsify", "Transfer")
        parts.append("\n\n## Reconstruct, Test, and Transfer\n\n")
        for title, sentence in zip(names, gates.GATES[num]):
            parts.append("- **" + title + ".** " + sentence + "\n")
        parts.append("\n\n")
    final_dir = OUTPUT / "book01-expanded-p04" / "content"
    final_reader = final_dir / "final-mastery.reader.md"
    final_diagrams = [item for item in diagrams if int(item["chapter"]) == 17]
    parts.append("# Book One — Final Mastery Laboratory\n\n")
    parts.append(prepare_fragment(
        final_dir / "final-mastery.md", final_reader,
        17, final_diagrams, level_shift=1,
    ))
    parts.append("\n\n")
    parts.append(builder.ACADEMIC_BACKMATTER.read_text(encoding="utf-8") + "\n")
    combined = "".join(parts)
    if re.search(r"ALLMIGHT(?:MATH|FIGURE|INDEX)", combined):
        raise ValueError("Unresolved markers in complete EPUB source")
    return combined


def validate_epub(path: Path) -> dict:
    """Structural EPUB3 checks; Kindle device rendering remains independent."""
    ns = {"c": "urn:oasis:names:tc:opendocument:xmlns:container",
          "o": "http://www.idpf.org/2007/opf",
          "x": "http://www.w3.org/1999/xhtml"}
    with zipfile.ZipFile(path) as z:
        bad = z.testzip()
        if bad:
            raise ValueError("Corrupt EPUB member " + bad)
        members = set(z.namelist())
        if z.namelist()[0] != "mimetype" or z.read("mimetype") != b"application/epub+zip":
            raise ValueError("EPUB must start with uncompressed mimetype")
        container = ET.fromstring(z.read("META-INF/container.xml"))
        rootfile = container.find(".//c:rootfile", ns)
        if rootfile is None:
            raise ValueError("No EPUB package document")
        opf_path = rootfile.attrib["full-path"]
        package = ET.fromstring(z.read(opf_path))
        metadata = package.find("o:metadata", ns)
        manifest = package.find("o:manifest", ns)
        spine = package.find("o:spine", ns)
        if metadata is None or manifest is None or spine is None:
            raise ValueError("Incomplete EPUB package")
        base = Path(opf_path).parent
        items = list(manifest)
        for item in items:
            if item.tag.endswith("item"):
                target = (base / item.attrib["href"].split("#")[0]).as_posix()
                if target not in members:
                    raise ValueError("Missing EPUB resource: " + target)
        if not any("nav" in item.attrib.get("properties", "").split() for item in items):
            raise ValueError("Missing Kindle navigable table of contents")
        if not any("cover-image" in item.attrib.get("properties", "").split() for item in items):
            raise ValueError("Missing embedded EPUB cover image")
        chapter_pages = [p for p in members if p.endswith(".xhtml") and "/text/" in p]
        if not chapter_pages:
            chapter_pages = [p for p in members if p.endswith(".xhtml")]
        full_text = "\n".join(z.read(p).decode("utf-8") for p in chapter_pages)
        if re.search(r"ALLMIGHT(?:MATH|FIGURE|INDEX)", full_text):
            raise ValueError("Unresolved production marker in EPUB content")
        return {"files": len(members), "xhtml_pages": len(chapter_pages),
                "bytes": path.stat().st_size,
                "mathml_elements": full_text.count("<math"),
                "embedded_images": sum(item.attrib.get("media-type","").startswith("image/")
                                       for item in items),
                "has_nav": True, "has_cover_image": True}


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cover", type=Path, required=True,
                        help="Exact cover art (JPEG/PNG). No generated substitute.")
    parser.add_argument("--output", type=Path,
                        default=OUTPUT / "Book01-From-Bits-to-Parallel-Systems-KDP.epub")
    parser.add_argument("--skip-source-verify", action="store_true",
                        help="Only for debugging; never for a KDP release")
    args = parser.parse_args()
    if not args.cover.is_file():
        parser.error("Provide the actual cover image file: " + str(args.cover))
    from PIL import Image
    with Image.open(args.cover) as image:
        w, h = image.size
        if w < 1200 or h < 1800 or not 0.56 <= w/h <= 0.75:
            parser.error(f"Cover is {w}x{h}; supply suitable portrait artwork, ideally 1600x2560")
    rows, metadata = builder.audit_sources()
    if not args.skip_source_verify:
        builder.verify_prepared(rows, metadata)
    data_path = OUTPUT / "manifest.json"
    if not data_path.exists():
        parser.error("Missing prepared reader: run --prepare first")
    data = json.loads(data_path.read_text("utf-8"))
    render.render_figures(data, OUTPUT)
    content = combine_book(data)
    work = OUTPUT / "epub-preflight"
    work.mkdir(parents=True, exist_ok=True)
    md = work / "book01.epub-source.md"
    md.write_text(content, encoding="utf-8")
    css = work / "kindle.css"
    css.write_text(
        "body{font-family:serif;line-height:1.4;}"
        "h1{page-break-before:always;break-before:page;}"
        "h1,h2,h3{line-height:1.23;page-break-after:avoid;}"
        "pre,code{font-family:monospace;overflow-wrap:anywhere;white-space:pre-wrap;}"
        "pre{font-size:.86em;line-height:1.27;padding:.5em;}"
        "img{max-width:100%;height:auto;}"
        "table{border-collapse:collapse;width:100%;font-size:.9em;}"
        "th,td{border-bottom:1px solid #aaa;padding:.22em;}"
        "figure{page-break-inside:avoid;} .math{overflow-wrap:anywhere;}",
        encoding="utf-8"
    )
    args.output.parent.mkdir(parents=True, exist_ok=True)
    run(["pandoc", "-f", "markdown+tex_math_dollars+raw_tex+pipe_tables+fenced_code_blocks",
         "-t", "epub3", "--standalone", "--toc", "--toc-depth=2",
         "--split-level=1", "--mathml", "--css", str(css),
         "--epub-cover-image", str(args.cover),
         "--metadata", "title=" + BOOK_TITLE,
         "--metadata", "subtitle=" + BOOK_SUBTITLE,
         "--metadata", "author=Swayam Singal",
         "--metadata", "lang=en-US",
         "-o", str(args.output), str(md)])
    result = validate_epub(args.output)
    report = work / "epub-preflight.json"
    report.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print("EPUB STRUCTURAL PROOF PASSED:", args.output, json.dumps(result))
    print("MANDATORY NEXT STEP: open EPUB in Kindle Previewer (tablet, phone, e-ink).")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (RuntimeError, ValueError) as exc:
        print("EPUB BUILD FAILED:", exc, file=sys.stderr)
        raise SystemExit(1)
