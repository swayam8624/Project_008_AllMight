#!/usr/bin/env python3
"""Stage the author's exact cover art for the next complete digital PDF.

Use a supplied PNG/JPEG, not an AI recreation. This is the *interior* title
page of the full digital PDF; a KDP paperback needs a separate print wrap
(back, spine, front) and should not use this as its cover submission.

Example:
  python tools/build_book01_expanded_reader.py --prepare
  python tools/stage_book01_cover.py --cover "/path/to/your/cover.jpg"
  python tools/build_book01_expanded_reader.py --compile
  python tools/build_book01_kdp_epub.py --cover "/path/to/your/cover.jpg"
"""
from __future__ import annotations

import argparse
import hashlib
from pathlib import Path
import sys

from PIL import Image
import fitz

ROOT = Path(__file__).resolve().parents[2]
OUTPUT = ROOT / "Code Parts/build/book01-expanded-reader"
DIGITAL = OUTPUT / "book01-complete"


def stage(cover: Path) -> dict:
    if not cover.is_file():
        raise FileNotFoundError(cover)
    with Image.open(cover) as art:
        art.load()
        width, height = art.size
        if art.mode not in ("RGB", "RGBA"):
            art = art.convert("RGB")
        # Digital PDF cover art does not share the 300 dpi print-wrap limit.
        # The author-supplied 992x1586 cover already used in the EPUB is
        # acceptable in a 7x10 inch on-screen reading edition. Never upscale
        # or crop the original merely to satisfy a paperback rule.
        if width < 800 or height < 1200:
            raise ValueError(f"Cover {width}x{height} is too small for a readable digital title page (minimum 800x1200)")
        if not (0.56 <= width / height <= 0.75):
            raise ValueError("Expected a portrait cover with approximately 1:1.6 aspect ratio")
    if not (DIGITAL / "main.tex").is_file():
        raise FileNotFoundError("Run the Book 1 --prepare step before staging cover art")
    pdf = fitz.open()
    page = pdf.new_page(width=504, height=720)
    # Center-fit without cropping or distorting the supplied art. Marginal
    # letterboxing on a 7x10 PDF page is preferable to cutting off cover text.
    page.draw_rect(page.rect, color=(0, 0, 0), fill=(0, 0, 0))
    img_ratio = width / height
    page_ratio = 504 / 720
    if img_ratio > page_ratio:
        dw = 504
        dh = dw / img_ratio
    else:
        dh = 720
        dw = dh * img_ratio
    bounds = fitz.Rect((504 - dw)/2, (720-dh)/2,
                       (504+dw)/2, (720+dh)/2)
    page.insert_image(bounds, filename=str(cover), keep_proportion=True)
    out = DIGITAL / "book01-cover.pdf"
    pdf.save(out, garbage=4, deflate=True)
    pdf.close()
    digest = hashlib.sha256(cover.read_bytes()).hexdigest()
    print(f"Exact cover staged: {out} ({width}x{height}, source SHA256 {digest})")
    return {"source": str(cover), "staged_pdf": str(out),
            "width": width, "height": height, "sha256": digest}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cover", type=Path, required=True)
    args = parser.parse_args()
    stage(args.cover)


if __name__ == "__main__":
    try:
        main()
    except (ValueError, FileNotFoundError) as exc:
        print("COVER ERROR:", exc, file=sys.stderr)
        raise SystemExit(1)
