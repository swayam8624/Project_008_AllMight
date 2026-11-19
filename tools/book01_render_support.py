#!/usr/bin/env python3
"""Source-preserving figure, math, and KDP checks for Book 1."""
from __future__ import annotations
from hashlib import sha256
from pathlib import Path
import re
import shutil
import subprocess

MERMAID = re.compile(r"(?m)^```mermaid(?:[ \t]+[^\n]*)?\n(?P<body>[\s\S]*?)^```[ \t]*$")
DISPLAY_MATH = re.compile(r"(?s)\$\$(.*?)\$\$")
EMPTY_BOX = re.compile(r"\\boxed\s*\{\s*\}", re.MULTILINE)
KDP_MAX_WHITE_BW = 828


def digest(payload: bytes) -> str:
    return sha256(payload).hexdigest()


def math_preflight(source: str, label: str) -> dict:
    no_code = re.sub(r"(?ms)^```[^\n]*\n.*?^```[ \t]*$", "", source)
    blocks = DISPLAY_MATH.findall(no_code)
    empty = [i for i, b in enumerate(blocks, 1) if not b.strip()]
    broken = [i for i, b in enumerate(blocks, 1) if EMPTY_BOX.search(b)]
    if empty or broken:
        raise ValueError(f"{label}: empty math {empty}; empty boxed expressions {broken}")
    return {"display_math": len(blocks),
            "boxed_math": sum(len(re.findall(r"\\boxed\s*\{", b)) for b in blocks)}


def plan_figures(source: str, chapter: int, part: int, directory: Path, output: Path):
    """Plan every Mermaid figure without changing the canonical chapter."""
    figure_dir = directory / "content" / "figures"
    figure_dir.mkdir(parents=True, exist_ok=True)
    entries = []

    def translate(match):
        body = match.group("body").strip() + "\n"
        if not body.strip():
            raise ValueError(f"Chapter {chapter}: empty Mermaid code")
        idx = len(entries) + 1
        basename = f"chapter-{chapter:02d}-diagram-{idx:02d}"
        mmd = figure_dir / (basename + ".mmd")
        png = figure_dir / (basename + ".png")
        token = f"ALLMIGHTFIGURECH{chapter:02d}INDEX{idx:03d}"
        headings = re.findall(r"(?m)^#{2,5} +([^\n]+)$", source[:match.start()])
        lead = headings[-1] if headings else "Computational data flow"
        lead = re.sub(r"[`*_\[\]]", "", lead).strip()
        lead = re.sub(r"\s+", " ", lead)[:96]
        mmd.write_text(body, encoding="utf-8")
        entries.append({
            "chapter": chapter, "part": part, "index": idx, "token": token,
            "diagram_sha256": digest(body.encode("utf-8")),
            "source": mmd.relative_to(output).as_posix(),
            "image": png.relative_to(output).as_posix(),
            "caption": f"Figure {chapter}.{idx}: {lead}",
        })
        return "\n\n" + token + "\n\n"

    revised, matched = MERMAID.subn(translate, source)
    declared = len(re.findall(r"(?m)^```mermaid(?:[ \t]|$)", source))
    if matched != declared:
        raise ValueError(f"Chapter {chapter}: {declared} Mermaid fences; {matched} matched")
    return revised, entries


def replace_figure_tokens(tex: str, figures: list[dict]) -> str:
    for item in figures:
        token = item["token"]
        path = f'../book01-expanded-p{item["part"]:02d}/content/figures/' + Path(item["image"]).name
        caption = item["caption"].replace("\\", "").replace("&", r"\&").replace("%", r"\%").replace("#", r"\#").replace("_", r"\_")
        latex = (
            "\\par\\begin{center}\n"
            "\\includegraphics[width=.94\\linewidth,height=.36\\textheight,"
            "keepaspectratio]{" + path + "}\n"
            "\\par{\\footnotesize\\itshape " + caption +
            "}\\par\\end{center}\\smallskip\n"
        )
        pattern = r"(?m)^" + re.escape(token) + r"$"
        if len(re.findall(pattern, tex)) != 1:
            raise ValueError(f"Missing/duplicate diagram marker: {token}")
        tex = re.sub(pattern, lambda _: latex, tex, count=1)
    if "ALLMIGHTFIGURECH" in tex:
        raise ValueError("Unresolved Mermaid marker in TeX")
    return tex


def check_math_rendered(source_stats: dict, tex: str, chapter: int):
    generated = len(re.findall(r"\\boxed\s*\{", tex))
    if generated < source_stats["boxed_math"]:
        raise ValueError(
            f"Chapter {chapter}: source has {source_stats['boxed_math']} boxed "
            f"equations but generated TeX only {generated}"
        )


def render_figures(manifest: dict, output: Path):
    figures = manifest.get("diagrams", [])
    if not figures:
        print("No Mermaid diagrams declared")
        return
    binary = shutil.which("mmdc")
    if not binary:
        raise RuntimeError(
            f"{len(figures)} diagrams must be rendered. Install Mermaid CLI: "
            "brew install node; npm install -g @mermaid-js/mermaid-cli"
        )
    for item in figures:
        src, dst = output / item["source"], output / item["image"]
        if not src.is_file() or digest(src.read_bytes()) != item["diagram_sha256"]:
            raise ValueError("Changed or missing diagram source: " + str(src))
        if dst.is_file() and dst.stat().st_size > 500 and item.get("render_sha256") == digest(dst.read_bytes()):
            continue
        result = subprocess.run([binary, "-i", str(src), "-o", str(dst),
                                 "-s", "3", "-t", "neutral", "-b", "white"],
                                capture_output=True, text=True)
        if result.returncode or not dst.is_file() or dst.stat().st_size < 500:
            raise RuntimeError(f"Mermaid failed: chapter {item['chapter']} "
                               f"diagram {item['index']}\n" +
                               (result.stderr or result.stdout)[-4000:])
        item["render_sha256"] = digest(dst.read_bytes())
    print("Verified rendered diagrams:", len(figures))


def pdf_preflight(pdf: Path, *, paperback: bool) -> int:
    try:
        from pypdf import PdfReader
    except ImportError as exc:
        raise RuntimeError("PDF preflight needs: python3 -m pip install pypdf") from exc
    pages = PdfReader(str(pdf)).pages
    count = len(pages)
    if count < 1:
        raise RuntimeError("PDF has no pages")
    if paperback:
        if count > KDP_MAX_WHITE_BW:
            raise RuntimeError(f"KDP paperback REFUSED: {count} > {KDP_MAX_WHITE_BW} pages")
        for index, page in enumerate(pages, 1):
            width, height = float(page.mediabox.width), float(page.mediabox.height)
            if abs(width - 504) > .5 or abs(height - 720) > .5:
                raise RuntimeError(f"KDP trim mismatch at PDF page {index}")
    return count
