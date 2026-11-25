#!/usr/bin/env python3
"""Stage and optionally build one complete Book 03 proof in a NEW build directory.

The two formats derive from the same Pandoc AST. Never promote releases, mutate
manuscripts, or touch the existing PDF/EPUB library. Pandoc 3+, CairoSVG and
Pillow are required; full builds also require LuaLaTeX and the listed fonts.
"""
from __future__ import annotations

import argparse
import copy
import hashlib
import json
import re
import shutil
import subprocess
import xml.etree.ElementTree as ET
import zipfile
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
BOOK = Path("Notes/Book Production/Seven-Book Curriculum/Book03")
TITLE = "From Geometry to Images"
SUBTITLE = "3D Geometry, Transformations, Cameras, Projection, and Image Formation"
READER = "markdown+tex_math_single_backslash+tex_math_double_backslash"
TAG = re.compile(r"\\tag\{([^{}]+)\}")
NS = "{http://www.w3.org/2000/svg}"

CSS = """
body { font-family: serif; line-height: 1.5; margin: 5%; color: #252a32; }
h1, h2, h3 { color: #233e50; line-height: 1.23; break-after: avoid; }
h1 { border-bottom: 2px solid #335e78; padding-bottom: .35em; }
pre { background: #eef4f7; border-left: .22em solid #335e78; padding: .65em;
      white-space: pre-wrap; overflow-wrap: anywhere; font-size: .86em; }
code { font-family: monospace; overflow-wrap: anywhere; }
blockquote { margin: 1em 0; padding: .5em .9em; background: #f2f6f4;
             border-left: .2em solid #47817b; }
figure { margin: 1.2em 0; text-align: center; }
img { max-width: 100%; height: auto; }
figcaption { font-size: .9em; text-align: left; }
table { width: 100%; border-collapse: collapse; font-size: .9em; }
th,td { border-bottom: 1px solid #ccd5da; padding: .35em; vertical-align: top; }
math[display="block"] { margin: .75em 0; }
.eq-label { display: inline; font-size: .9em; }
a { color: #335e78; }
"""
TEX = r"""
\usepackage{fvextra}
\usepackage[most]{tcolorbox}
\usepackage{etoolbox}
\usepackage{fancyhdr}
\usepackage{titlesec}
\definecolor{BookInk}{HTML}{252A32}
\definecolor{BookAccent}{HTML}{335E78}
\definecolor{BookWash}{HTML}{EEF4F7}
\definecolor{BookNote}{HTML}{F2F6F4}
\setlength{\parindent}{1em}
\setlength{\parskip}{0.15em}
\setlength{\emergencystretch}{2em}
\setlength{\headheight}{14pt}
\pagestyle{fancy}
\fancyhf{}
\fancyhead[LE,RO]{\small\thepage}
\fancyhead[LO,RE]{\small\nouppercase{\leftmark}}
\renewcommand{\headrulewidth}{0.3pt}
\titleformat{\chapter}[display]{\normalfont\bfseries\color{BookInk}}{}{0pt}{\Huge}[\vspace{.5ex}{\color{BookAccent}\titlerule[1.5pt]}]
\titlespacing*{\chapter}{0pt}{-1ex}{2ex}
\titleformat{\section}{\Large\bfseries\color{BookInk}}{}{0pt}{}
\titleformat{\subsection}{\large\bfseries\color{BookAccent}}{}{0pt}{}
\AtBeginDocument{%
  \fvset{fontsize=\fontsize{8.3}{10.5}\selectfont,breaklines=true,breakanywhere=true}%
  \RecustomVerbatimEnvironment{verbatim}{Verbatim}{fontsize=\fontsize{8.3}{10.5}\selectfont,breaklines=true,breakanywhere=true}%
  \ifcsdef{Shaded}{\renewenvironment{Shaded}{\begin{tcolorbox}[enhanced,breakable,colback=BookWash,colframe=BookAccent,boxrule=.25pt,arc=1pt,left=5pt,right=5pt,top=3pt,bottom=3pt]}{\end{tcolorbox}}}{}%
}
\AtBeginEnvironment{quote}{\small\color{BookAccent}}
"""


def require(ok, message):
    if not ok:
        raise ValueError(message)


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def run(args, cwd, log=None, input_text=None):
    p = subprocess.run(args, cwd=cwd, input=input_text, text=True,
                       stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if log is not None:
        log.write_text(p.stdout + "\n--- STDERR ---\n" + p.stderr, encoding="utf-8")
    if p.returncode:
        raise RuntimeError(f"{args[0]} exit {p.returncode}: {p.stderr[-4000:]}")
    return p


def nodes(value):
    if isinstance(value, dict):
        if "t" in value:
            yield value
        for v in value.values():
            yield from nodes(v)
    elif isinstance(value, list):
        for v in value:
            yield from nodes(v)


def plain(inlines):
    return "".join(n["c"] if n["t"] == "Str" else " "
                   for n in nodes(inlines) if n["t"] in {"Str", "Space", "SoftBreak"})


def render_assets(book, out):
    import cairosvg
    from PIL import Image, ImageChops, ImageDraw
    assets = sorted((book / "Assets").glob("*.svg"), key=lambda p: int(p.name.split("-")[0]))
    require(len(assets) == 142, "expected 142 source SVGs")
    target = out / "figures"
    target.mkdir()
    report = {}
    thumbnails = []
    for number, source in enumerate(assets, 1):
        require(int(source.name.split("-")[0]) == number, f"figure order: {source.name}")
        tree = ET.parse(source).getroot()
        require(tree.tag == NS+"svg", f"not SVG: {source}")
        require(tree.find(NS+"title") is not None and tree.find(NS+"desc") is not None,
                f"missing accessible metadata: {source}")
        # Prevent network access and external file dependencies in illustration conversion.
        for element in tree.iter():
            for key, val in element.attrib.items():
                if key.split("}")[-1] == "href":
                    require(val.startswith(("#", "data:")), f"external SVG resource: {source}")
            require(element.tag != NS+"script", f"script in SVG: {source}")
        raw = source.read_text(encoding="utf-8")
        require(not re.search(r"url\(\s*['\"]?(?!#|data:)[^)]", raw),
                f"external SVG URL: {source}")
        png = target / (source.stem + ".png")
        cairosvg.svg2png(bytestring=source.read_bytes(), write_to=str(png), output_width=1680)
        with Image.open(png) as image:
            rgba = image.convert("RGBA")
            white = Image.new("RGBA", rgba.size, "white")
            white.alpha_composite(rgba)
            rgb = white.convert("RGB")
            require(rgb.width >= 800 and rgb.height >= 100, f"tiny image: {source}")
            require(ImageChops.difference(rgb, Image.new("RGB", rgb.size, rgb.getpixel((0,0)))).getbbox(),
                    f"blank SVG render: {source}")
            report[source.name] = {"source_sha256": digest(source), "png_sha256": digest(png),
                                   "width": rgb.width, "height": rgb.height}
            rgb.thumbnail((390, 214))
            thumbnails.append((source.name, rgb.copy()))
    for start in range(0, len(thumbnails), 12):
        sheet = Image.new("RGB", (1240, 4*260), "#eceff2")
        draw = ImageDraw.Draw(sheet)
        for i, (name, thumb) in enumerate(thumbnails[start:start+12]):
            x, y = 8+(i%3)*412, 8+(i//3)*260
            sheet.paste(thumb, (x+(396-thumb.width)//2, y))
            draw.text((x, y+220), name[:54], fill="#182c3a")
        sheet.save(out / f"contact-{start//12+1:02d}.png")
    return report


def rewrite_chapter(doc, n, figures):
    blocks = doc["blocks"]
    require(blocks and blocks[0]["t"] == "Header", f"chapter {n}: missing title")
    head = blocks[0]["c"]
    require(head[0] == 1 and plain(head[2]).startswith(f"Chapter {n:02d}"),
            f"chapter {n}: heading mismatch")
    ids = {}
    for item in nodes(blocks):
        if item["t"] == "Header":
            attr = item["c"][1]
            old = attr[0]
            new = f"chapter-{n:02d}" if item is blocks[0] else f"c{n:02d}-{old}"
            require(old not in ids, f"chapter {n}: duplicate heading ID {old}")
            ids[old] = new
            attr[0] = new
    used = []
    for item in nodes(blocks):
        if item["t"] == "Image":
            target = item["c"][2][0]
            require(target.startswith("../Assets/"), f"unrecognized figure path {target}")
            name = target.split("/")[-1]
            require(name in figures and plain(item["c"][1]).strip(), f"missing figure/alt: {target}")
            used.append(name)
            item["c"][2][0] = "figures/" + Path(name).stem + ".png"
        elif item["t"] == "Link":
            url = item["c"][2][0]
            if url.startswith("#"):
                require(url[1:] in ids, f"unresolved chapter anchor: {url}")
                item["c"][2][0] = "#" + ids[url[1:]]
            else:
                require(url.startswith(("https://", "http://", "mailto:")),
                        f"unrecognized link: {url}")
    # Pandoc 3 treats standalone image alt text as a caption. Prefer the
    # explicit authored numbered caption; preserve the actual image alt text.
    result = []
    i = 0
    while i < len(blocks):
        block = blocks[i]
        if block["t"] == "Figure" and i+1 < len(blocks):
            follow = blocks[i+1]
            if follow["t"] in ("Para", "Plain") and plain(follow["c"]).startswith("Figure "):
                block["c"][1] = [None, [{"t": "Plain", "c": follow["c"]}]]
                i += 1
        result.append(block)
        i += 1
    doc["blocks"] = result
    return used


def epub_equations(value):
    """Retain explicit authored equation labels without passing \\tag to MathML."""
    if isinstance(value, list):
        result = []
        for v in value:
            if isinstance(v, dict) and v.get("t") == "Math":
                mode, formula = v["c"]
                tags = TAG.findall(formula)
                if tags:
                    require(mode["t"] == "DisplayMath" and len(tags) == 1,
                            f"unsupported equation tags: {tags}")
                    result.append({"t":"Math", "c":[mode,TAG.sub("",formula)]})
                    result.append({"t":"Space"})
                    result.append({"t":"Span", "c":[["",["eq-label"],[]],
                                   [{"t":"Str","c":"("+tags[0]+")"}]]})
                    continue
            result.append(epub_equations(v))
        return result
    if isinstance(value, dict):
        return {k:epub_equations(v) for k,v in value.items()}
    return value


def prepare(root, out):
    book = root / BOOK
    chapters = sorted((book / "Chapters").glob("*.md"))
    require(len(chapters) == 32, "expected 32 chapters")
    figures = render_assets(book, out)
    report = {"schema": 1, "title": TITLE, "chapters": [], "figures": figures}
    combined, api, all_used = [], None, []
    for n, chapter in enumerate(chapters, 1):
        require(chapter.name.startswith(f"{n:02d} - "), f"chapter order: {chapter}")
        source = chapter.read_text(encoding="utf-8")
        p = run(["pandoc", "-f", READER, "-t", "json"], out, input_text=source)
        require(not p.stderr.strip(), f"Pandoc input warning {chapter.name}: {p.stderr}")
        doc = json.loads(p.stdout)
        api = api or doc["pandoc-api-version"]
        require(doc["pandoc-api-version"] == api, "mixed Pandoc APIs")
        counts = Counter(v["t"] for v in nodes(doc["blocks"]))
        require(counts["Math"] and counts["Image"], f"chapter lacks parsed math/figures: {chapter}")
        codes = [v["c"][1] for v in nodes(doc["blocks"]) if v["t"]=="CodeBlock"]
        used = rewrite_chapter(doc, n, figures)
        after = [v["c"][1] for v in nodes(doc["blocks"]) if v["t"]=="CodeBlock"]
        require(codes == after, f"code changed in staging: {chapter}")
        require(len(used)==counts["Image"], f"image count changed: {chapter}")
        all_used.extend(used)
        report["chapters"].append({"number":n,"file":chapter.name,"sha256":digest(chapter),
                                   "math":counts["Math"],"code":counts["CodeBlock"],
                                   "images":used,"blocks":len(doc["blocks"])})
        combined.extend(doc["blocks"])
    require(set(all_used)==set(figures), "unreferenced source illustrations")
    # Both formats share the same front/back matter and stable chapter anchors.
    matter = []
    for name in ("FRONT_MATTER.md", "BACK_MATTER.md"):
        file = book / name
        doc = json.loads(run(["pandoc","-f",READER,"-t","json"],out,
                             input_text=file.read_text(encoding="utf-8")).stdout)
        matter.append(doc["blocks"])
        report.setdefault("matter", {})[name] = digest(file)
    combined = matter[0] + combined + matter[1]
    ids = [v["c"][1][0] for v in nodes(combined) if v["t"]=="Header"]
    require(len(ids)==len(set(ids)), "duplicate combined reader header IDs")
    for v in nodes(combined):
        if v["t"]=="Link" and v["c"][2][0].startswith("#"):
            require(v["c"][2][0][1:] in ids, f"broken reader link: {v['c'][2][0]}")
    meta = {"title":{"t":"MetaString","c":TITLE},
            "subtitle":{"t":"MetaString","c":SUBTITLE},
            "lang":{"t":"MetaString","c":"en"}}
    doc = {"pandoc-api-version":api,"meta":meta,"blocks":combined}
    (out/"reader-print.json").write_text(json.dumps(doc,ensure_ascii=False),encoding="utf-8")
    ebook = copy.deepcopy(doc)
    ebook["blocks"] = epub_equations(ebook["blocks"])
    require(sum(1 for v in nodes(ebook["blocks"]) if v["t"]=="Math") ==
            sum(1 for v in nodes(doc["blocks"]) if v["t"]=="Math"), "math count changed for EPUB")
    (out/"reader-epub.json").write_text(json.dumps(ebook,ensure_ascii=False),encoding="utf-8")
    (out/"reader.css").write_text(CSS,encoding="utf-8")
    (out/"reader.tex").write_text(TEX,encoding="utf-8")
    report["ast_sha256"] = {f:digest(out/f) for f in ("reader-print.json","reader-epub.json")}
    # HTML preview also exercises the actual MathML writer before PDF/EPUB.
    p = run(["pandoc","-f","json","-t","html5","--standalone","--mathml","--toc",
             "--css=reader.css","reader-epub.json","-o","reader-preview.html"],
            out,out/"html-preview.log")
    report["html_warnings"] = p.stderr.splitlines()
    (out/"source-proof.json").write_text(json.dumps(report,indent=2,ensure_ascii=False)+"\n",encoding="utf-8")
    print(f"Prepared {len(chapters)} chapters, {len(figures)} rendered figures, "
          f"{sum(x['math'] for x in report['chapters'])} parsed math nodes, "
          f"{sum(x['code'] for x in report['chapters'])} code blocks")
    return report


def build(out, cover, author):
    epub = ["pandoc","-f","json","-t","epub3","--mathml","--toc","--toc-depth=2",
            "--css=reader.css","--metadata","author="+author,
            "reader-epub.json","-o","Book03-From-Geometry-to-Images-PROOF.epub"]
    if cover:
        epub.extend(["--epub-cover-image",str(cover.resolve())])
    e = run(epub,out,out/"epub-build.log")
    pdf = run(["pandoc","-f","json","--pdf-engine=lualatex","--top-level-division=chapter",
               "--toc","--toc-depth=2","--highlight-style=tango","--include-in-header=reader.tex",
               "-V","documentclass=book","-V","classoption=openany","-V","fontsize=10pt",
               "-V","geometry=paperwidth=8in,paperheight=10in,inner=0.85in,outer=0.70in,top=0.72in,bottom=0.75in",
               "-V","mainfont=TeX Gyre Pagella","-V","mathfont=TeX Gyre Pagella Math",
               "-V","monofont=DejaVu Sans Mono","--metadata","author="+author,
               "reader-print.json","-o","Book03-From-Geometry-to-Images-PROOF.pdf"],
              out,out/"pdf-build.log")
    file = out/"Book03-From-Geometry-to-Images-PROOF.epub"
    with zipfile.ZipFile(file) as z:
        require(z.testzip() is None, "EPUB ZIP integrity")
        require(z.namelist()[0]=="mimetype" and z.read("mimetype")==b"application/epub+zip",
                "EPUB mimetype contract")
        require(z.getinfo("mimetype").compress_type==zipfile.ZIP_STORED,"EPUB compressed mimetype")
        xhtml = [name for name in z.namelist() if name.endswith(".xhtml")]
        ids = set()
        for name in xhtml:
            tree = ET.fromstring(z.read(name))
            ids.update(el.attrib.get("id") for el in tree.iter())
        require(all(f"chapter-{n:02d}" in ids for n in range(1,33)), "EPUB chapter IDs incomplete")
    check = shutil.which("epubcheck")
    if check:
        run([check,str(file)],out,out/"epubcheck.log")
    print("Proof PDF and EPUB built. No promotion; formal EPUBCheck:",bool(check))
    proof = {"epub_sha256":digest(file),"pdf_sha256":digest(out/"Book03-From-Geometry-to-Images-PROOF.pdf"),
             "epubcheck_ran":bool(check),"epub_warnings":e.stderr.splitlines(),
             "pdf_warnings":pdf.stderr.splitlines(),"cover_epub_only":bool(cover),
             "accepted":False}
    (out/"build-proof.json").write_text(json.dumps(proof,indent=2)+"\n",encoding="utf-8")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root",type=Path,default=ROOT)
    parser.add_argument("--output",type=Path,help="new directory beneath Code Parts/build")
    parser.add_argument("--build",action="store_true",help="compile proof PDF and EPUB; never promote")
    parser.add_argument("--cover",type=Path,help="optional approved EPUB cover, not a PDF cover")
    parser.add_argument("--author",default="Swayam Singal")
    a = parser.parse_args()
    root = a.root.resolve()
    base = (root/"Code Parts/build").resolve()
    out = (a.output or base/"book03-reader-proof").resolve()
    require(out.is_relative_to(base) and out!=base,"output must be under existing Code Parts/build layout")
    require(not out.exists(),"proof directory must be new")
    version = run(["pandoc","--version"],root).stdout.splitlines()[0]
    require(re.search(r"pandoc\s+([3-9]|\d{2,})\.",version),f"Pandoc 3+ required: {version}")
    out.mkdir(parents=True)
    prepare(root,out)
    if a.build:
        build(out,a.cover,a.author)
    print("Proof directory:",out)


if __name__=="__main__":
    main()
