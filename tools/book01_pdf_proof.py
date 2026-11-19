#!/usr/bin/env python3
"""Independent PDF publication proof. Does not edit the PDF or manuscript.

Usage:
  python tools/book01_pdf_proof.py FILE.pdf [--paperback] [--expect-figures N]
Needs PyMuPDF: python -m pip install pymupdf
The report distinguishes release-blocking geometry from editorial warnings.
"""
from __future__ import annotations
import argparse
import json
from pathlib import Path
import re
import sys

KDP_BW_WHITE_PAGE_LIMIT = 828
SKIP_FRONT_TEXT = 100
FATAL = {"trim", "page_limit", "missing_metadata", "duplicate_footers",
         "overlapping_headers", "clipped_text", "tiny_code", "missing_figures"}
LEAKS = (
    "This addition belongs", "the user has reserved", "Supplementary/Manual Integration",
    "source attachment's central", "the supplied source attachment",
)


def inspect(path: Path, paperback: bool, expect_figures: int = 0) -> dict:
    try:
        import fitz
    except ImportError as exc:
        raise RuntimeError("Install PyMuPDF: python3 -m pip install pymupdf") from exc
    doc = fitz.open(path)
    report = {"filename": path.name, "pages": len(doc), "problems": [],
              "sample_pages": [], "meta": {}}
    meta = doc.metadata or {}
    report["meta"] = {name: meta.get(name, "") for name in ("title", "author", "subject")}
    def flag(code: str, page: int | None, detail: str) -> None:
        report["problems"].append({"code": code, "page": page, "detail": detail,
                                    "fatal": code in FATAL})
    if not meta.get("title") or not meta.get("author"):
        flag("missing_metadata", None, "PDF document info must name title and author")
    if paperback and len(doc) > KDP_BW_WHITE_PAGE_LIMIT:
        flag("page_limit", None, f"Exceeded {KDP_BW_WHITE_PAGE_LIMIT} pages")
    image_count = 0
    for i, page in enumerate(doc, 1):
        rect = page.rect
        if paperback and (abs(rect.width-504)>0.5 or abs(rect.height-720)>0.5):
            flag("trim", i, f"Page is {rect.width/72:.3f} x {rect.height/72:.3f} inches")
        image_count += len(page.get_images(full=True))
        lines, bottom_nums, header_lines = [], [], []
        for block in page.get_text("dict")["blocks"]:
            for line in block.get("lines", []):
                txt = "".join(span["text"] for span in line["spans"]).strip()
                if not txt: continue
                bbox = fitz.Rect(line["bbox"])
                lines.append((bbox, txt, line))
                if bbox.y0 > rect.height-75 and re.fullmatch(r"[ivxlcdm]+|\d+", txt, re.I):
                    bottom_nums.append((bbox, txt))
                if 25 < bbox.y0 < 95 and len(txt) >= 8:
                    header_lines.append((bbox, txt))
                if i > 1 and (bbox.x0 < -1.0 or bbox.x1 > rect.width + 1.0 or bbox.y0 < -1.0 or bbox.y1 > rect.height + 1.0):
                    flag("clipped_text", i, f"{txt[:70]!r} at {tuple(round(t) for t in bbox)}")
                if "Mono" in str(line["spans"][0].get("font", "")) or "Menlo" in str(line["spans"][0].get("font", "")):
                    if min(x["size"] for x in line["spans"]) < 7.95:
                        flag("tiny_code", i, f"Code <8 pt: {txt[:70]!r}")
        if len(bottom_nums) > 1:
            # Duplicate footers, not a page number repeated inside a table.
            matched = [v for _,v in bottom_nums]
            if len(matched) != len(set(matched)):
                flag("duplicate_footers", i, "Footer contains duplicate page numbers")
        for a in range(len(header_lines)):
            for b in range(a+1,len(header_lines)):
                ra,ta=header_lines[a]; rb,tb=header_lines[b]
                intersection=ra & rb
                if intersection.is_empty: continue
                if intersection.width>8 and intersection.height>4 and ta!=tb:
                    flag("overlapping_headers", i, f"{ta[:40]!r} collides with {tb[:40]!r}")
        if i <= SKIP_FRONT_TEXT: continue
        text = page.get_text()
        for leak in LEAKS:
            if leak.casefold() in text.casefold():
                flag("editorial_leak", i, leak)
        if re.search(r"\b(?:[1-9]|1[0-7])\.0\.[1-9]\d*\b", text):
            flag("bad_section_numbering", i, "Zero-section numbering found")
    if image_count < expect_figures:
        flag("missing_figures", None, f"Only {image_count} PDF image instances; expected at least {expect_figures}")
    report["image_instances"] = image_count
    tallies={}
    for item in report["problems"]:
        tallies[item["code"]]=tallies.get(item["code"],0)+1
    report["counts"]=tallies
    report["fatal_count"]=sum(p["fatal"] for p in report["problems"])
    return report


def main() -> int:
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("pdf", type=Path)
    parser.add_argument("--paperback", action="store_true")
    parser.add_argument("--expect-figures", type=int, default=0)
    parser.add_argument("--report", type=Path)
    args=parser.parse_args()
    result=inspect(args.pdf, args.paperback, args.expect_figures)
    output=json.dumps(result,ensure_ascii=False,indent=2)
    if args.report:
        args.report.parent.mkdir(parents=True,exist_ok=True)
        args.report.write_text(output+"\n",encoding="utf-8")
    print(json.dumps({k:v for k,v in result.items() if k not in ("problems","sample_pages")},ensure_ascii=False,indent=2))
    for issue in result["problems"][:25]:
        print("FATAL" if issue["fatal"] else "REVIEW", issue["page"], issue["code"],issue["detail"])
    if len(result["problems"])>25:
        print("Plus",len(result["problems"])-25,"more; see JSON report.")
    return 1 if result["fatal_count"] else 0


if __name__=="__main__":
    sys.exit(main())
