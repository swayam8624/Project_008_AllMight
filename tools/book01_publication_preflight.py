#!/usr/bin/env python3
"""Inspect finished Book 1 PDFs for publication-blocking layout defects.

Requires PyMuPDF: python -m pip install pymupdf
This complements the source-preservation, Mermaid and formula integrity gates.
It cannot certify technical correctness or replace KDP Print Previewer.
"""
from __future__ import annotations
import argparse
from collections import Counter
from pathlib import Path
import json
import re
import sys

REQUIRED_METADATA = ("title", "author")
MIN_CODE_POINT_SIZE = 8.0


def inspect_pdf(path: Path) -> dict:
    try:
        import fitz
    except ImportError as e:
        raise RuntimeError("Install PyMuPDF: python -m pip install pymupdf") from e

    doc = fitz.open(path)
    errors = []
    warnings = []
    summaries = Counter()
    if not doc.page_count:
        errors.append("The PDF has zero pages")
    for key in REQUIRED_METADATA:
        if not (doc.metadata or {}).get(key, "").strip():
            errors.append(f"Missing embedded PDF {key} metadata")
    if path.name.lower().find("kdp") >= 0 and doc.page_count > 828:
        errors.append(f"Paperback has {doc.page_count} pages, beyond 828-page interior limit")

    font_chars = Counter()
    for page_no, page in enumerate(doc, 1):
        width, height = page.rect.width, page.rect.height
        footer_numbers = []
        header_spans = []
        text_character_count = 0
        for block in page.get_text("dict")["blocks"]:
            if "lines" not in block:
                continue
            for line in block["lines"]:
                for span in line["spans"]:
                    value = span["text"].strip()
                    if not value:
                        continue
                    rect = span["bbox"]
                    text_character_count += len(value)
                    if rect[0] < -1 or rect[2] > width + 1 or rect[1] < -1 or rect[3] > height + 1:
                        errors.append(
                            f"page {page_no}: clipped text {value[:70]!r} at {tuple(round(x,1) for x in rect)}"
                        )
                    if rect[1] > height - 77 and value.isdigit() and len(value) < 6:
                        footer_numbers.append((value, rect))
                    if rect[1] < 80 and rect[3] > 28:
                        header_spans.append((value, rect))
                    font = span.get("font", "").casefold()
                    if any(k in font for k in ("menlo", "mono", "courier", "code")):
                        font_chars[(round(span["size"], 1), "code")] += len(value)
                        if span["size"] < MIN_CODE_POINT_SIZE - .05:
                            summaries["undersized_code_characters"] += len(value)

        # Ignore obvious chapter pages and contents: a sparse page can be correct.
        if text_character_count < 750 and 10 < page_no < doc.page_count - 2:
            summaries["sparse_pages"] += 1
        if len(footer_numbers) > 1:
            values = [x[0] for x in footer_numbers]
            if len(set(values)) == 1:
                errors.append(f"page {page_no}: duplicate footer page number {values[0]}")
        # Two long strings whose rectangles overlap in the top running-head band.
        for i, (a, ra) in enumerate(header_spans):
            for b, rb in header_spans[i+1:]:
                ix = max(0, min(ra[2], rb[2]) - max(ra[0], rb[0]))
                iy = max(0, min(ra[3], rb[3]) - max(ra[1], rb[1]))
                if ix > 7 and iy > 3 and len(a) > 8 and len(b) > 8:
                    errors.append(
                        f"page {page_no}: overlapping running head {a[:44]!r} / {b[:44]!r}"
                    )

    if summaries["undersized_code_characters"]:
        warnings.append(
            f"{summaries['undersized_code_characters']} code characters below "
            f"{MIN_CODE_POINT_SIZE} pt; assess print legibility"
        )
    if summaries["sparse_pages"]:
        warnings.append(
            f"{summaries['sparse_pages']} pages contain fewer than 750 characters; "
            "check chapter breaks, floats, and large listings visually"
        )
    if not getattr(doc, "is_pdf", False):
        errors.append("The input is not a PDF")
    return {
        "file": str(path),
        "pages": doc.page_count,
        "page_size_inches": (round(doc[0].rect.width / 72, 3),
                             round(doc[0].rect.height / 72, 3)) if doc.page_count else None,
        "metadata": {x: doc.metadata.get(x) for x in REQUIRED_METADATA},
        "errors": errors,
        "warnings": warnings,
        "counts": dict(summaries),
        "code_font_distribution": [
            {"size_pt": key[0], "characters": n} for key, n
            in sorted(font_chars.items(), key=lambda item: item[0][0])
        ],
        "pass": not errors,
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("pdf", type=Path, nargs="+")
    parser.add_argument("--strict", action="store_true",
                        help="Return nonzero for any detected publication blocker")
    parser.add_argument("--output", type=Path, help="Optional JSON report path")
    args = parser.parse_args()
    reports = [inspect_pdf(path) for path in args.pdf]
    for report in reports:
        print(f"{report['file']} — {report['pages']} pages")
        print("  Blocking errors:", len(report["errors"]))
        for e in report["errors"][:25]:
            print("   -", e)
        if len(report["errors"]) > 25:
            print("   ... plus", len(report["errors"]) - 25, "additional errors")
        for w in report["warnings"]:
            print("  Warning:", w)
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(json.dumps(reports, indent=2) + "\n", encoding="utf-8")
    return 1 if args.strict and any(r["errors"] for r in reports) else 0


if __name__ == "__main__":
    sys.exit(main())
