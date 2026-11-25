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
from book03_editorial import edit_chapter, format_math, subject_index, INDEX

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
a { color: #087fa0; }
.index-reference { white-space: nowrap; }
#subject-index { break-before: page; }
.book-callout.concept { break-inside: avoid; }
h1,h2,h3 {color:#087fa0;}
pre {background:#f0f7fa;border-left:0.19em solid #087fa0;}
.book-callout {background:#eaf5f7;border-left:0.19em solid #087fa0;padding:.65em .8em;margin:.8em 0;}
.book-callout.example {background:#f7f4e9;border-color:#9f7629;}
.book-callout.warning {background:#f9eeee;border-color:#a13b3b;}
"""
TEX = r"""
\usepackage{fvextra}
\usepackage[most]{tcolorbox}
\usepackage{etoolbox}
\usepackage{fancyhdr}
\usepackage{titlesec}
\definecolor{BookInk}{HTML}{252A32}
\definecolor{BookAccent}{HTML}{087FA0}
\definecolor{BookWash}{HTML}{F0F7FA}
\definecolor{BookNote}{HTML}{EAF5F7}
\setlength{\parindent}{1em}
\setlength{\parskip}{0.15em}
\setlength{\emergencystretch}{2em}
\setlength{\headheight}{14pt}
\pagestyle{fancy}
\fancyhf{}
\fancyhead[R]{\footnotesize From Geometry to Images}
\fancyhead[L]{\footnotesize\color{BookAccent} FROM BITS TO RESEARCH / BOOK THREE}
\fancyfoot[R]{\footnotesize\thepage}
\renewcommand{\headrulewidth}{0.3pt}
\titleformat{\chapter}[display]{\normalfont\bfseries\color{BookInk}}{}{0pt}{\raggedright\huge}[\vspace{.5ex}{\color{BookAccent}\titlerule[1.5pt]}]
\titlespacing*{\chapter}{0pt}{-1ex}{2ex}
\titleformat{\section}{\large\bfseries\color{BookAccent}}{}{0pt}{}
\titleformat{\subsection}{\large\bfseries\color{BookAccent}}{}{0pt}{}
\AtBeginDocument{%
  \fvset{fontsize=\fontsize{8.3}{10.5}\selectfont,breaklines=true,breakanywhere=true}%
  \RecustomVerbatimEnvironment{verbatim}{Verbatim}{fontsize=\fontsize{8.3}{10.5}\selectfont,breaklines=true,breakanywhere=true}%
  \ifcsdef{Highlighting}{\RecustomVerbatimEnvironment{Highlighting}{Verbatim}{commandchars=\\\{\},fontsize=\fontsize{8.3}{10.5}\selectfont,breaklines=true,breakanywhere=true}}{}%
  \ifcsdef{Shaded}{\renewenvironment{Shaded}{}{}}{}%
  \let\BookOriginalTexttt\texttt
  \renewcommand{\texttt}[1]{{\fontsize{8.5}{10.5}\selectfont\BookOriginalTexttt{#1}}}%
}
\AtBeginEnvironment{quote}{\small\color{BookAccent}}
\usepackage{caption}
\usepackage{float}
\captionsetup[figure]{labelformat=empty,font=small}
\makeatletter
\def\fps@figure{H}
\renewcommand{\@pnumwidth}{3em}
\renewcommand{\@tocrmarg}{3.5em}
\makeatother
\usepackage{adjustbox}
\providecommand{\maxwidth}{\linewidth}
\setkeys{Gin}{width=\maxwidth,height=.42\textheight,keepaspectratio}
% Newer Pandoc templates additionally wrap figures in pandocbounded.
\providecommand{\pandocbounded}[1]{#1}
\renewcommand{\pandocbounded}[1]{\adjustbox{max width=\linewidth,max totalheight=.42\textheight}{#1}}
\raggedbottom
\setlength{\emergencystretch}{4em}
\clubpenalty=10000
\widowpenalty=10000
\displaywidowpenalty=10000
\makeatletter
\AtBeginDocument{%
\renewcommand{\maketitle}{%
  \begin{titlepage}\thispagestyle{empty}\raggedright
  {\color{BookAccent}\rule{\textwidth}{1.5pt}\par}
  \vspace{9pt}{\sffamily\large\bfseries\color{BookAccent} FROM BITS TO RESEARCH\par}
  \vspace{4pt}{\sffamily\small BOOK THREE OF SEVEN\par}
  \vspace{0.16\textheight}
  {\fontsize{32}{36}\selectfont\bfseries From Geometry\par to Images\par}
  \vspace{10pt}{\color{BookAccent}\rule{.6\textwidth}{1.1pt}\par}
  \vspace{10pt}{\large 3D Geometry, Transformations, Cameras,\par Projection, and Image Formation\par}
  \vfill
  {\Large\bfseries\@author\par}
  \vspace{6pt}{\small Digital reader edition \textbullet\ November 2026\par}
  \vspace{12pt}{\color{BookAccent}\rule{\textwidth}{1.5pt}\par}
  \end{titlepage}}}
\makeatother
\newtcolorbox{BookCode}{enhanced,breakable,frame hidden,boxrule=0pt,arc=0pt,colback=BookWash,borderline west={1.4pt}{0pt}{BookAccent},left=6pt,right=5pt,top=4pt,bottom=4pt,before skip=5pt,after skip=5pt}
\newtcolorbox{BookConcept}{enhanced,breakable,frame hidden,boxrule=0pt,arc=0pt,colback=BookNote,borderline west={1.4pt}{0pt}{BookAccent},left=7pt,right=6pt,top=5pt,bottom=5pt}
\definecolor{ExampleWash}{HTML}{F7F4E9}
\definecolor{ExampleEdge}{HTML}{9F7629}
\newtcolorbox{BookExample}{enhanced,breakable,frame hidden,boxrule=0pt,arc=0pt,colback=ExampleWash,borderline west={1.4pt}{0pt}{ExampleEdge},left=7pt,right=6pt,top=5pt,bottom=5pt}
\definecolor{WarningWash}{HTML}{F9EEEE}
\newtcolorbox{BookWarning}{enhanced,breakable,frame hidden,boxrule=0pt,arc=0pt,colback=WarningWash,borderline west={1.4pt}{0pt}{red!55!black},left=7pt,right=6pt,top=5pt,bottom=5pt}
"""

PRINT_BREAKS = r"""
function Code(el)
  if #el.text < 16 then return nil end
  local result = pandoc.List()
  local start = 1
  for i=1,#el.text do
    if el.text:sub(i,i):match('[/_:%-]') and i<#el.text then
      result:insert(pandoc.Code(el.text:sub(start,i)))
      result:insert(pandoc.RawInline('latex','\\allowbreak{}'))
      start=i+1
    end
  end
  if start==1 then return nil end
  if start<=#el.text then result:insert(pandoc.Code(el.text:sub(start))) end
  return result
end
function Math(el)
  if el.mathtype ~= 'InlineMath' then return nil end
  local depth, out = 0, {}
  for i=1,#el.text do
    local c=el.text:sub(i,i)
    if c=='{' then depth=depth+1 elseif c=='}' then depth=depth-1 end
    out[#out+1]=c
    if c==',' and depth==0 then out[#out+1]='\\allowbreak{}' end
  end
  el.text=table.concat(out)
  return el
end
function CodeBlock(el)
  return {pandoc.RawBlock('latex','\\begin{BookCode}'), el, pandoc.RawBlock('latex','\\end{BookCode}')}
end
function Link(el)
  if el.classes:includes('index-reference') then
    local id=el.target:sub(2)
    local chapter=el.attributes['data-chapter']
    return pandoc.RawInline('latex','\\hyperref['..id..']{Ch. '..chapter..', p.~\\pageref*{'..id..'}}')
  end
end
function Div(el)
  if el.classes:includes('book-callout') then
    local box = 'BookConcept'
    if el.classes:includes('example') then box='BookExample' end
    if el.classes:includes('warning') then box='BookWarning' end
    local blocks = pandoc.List({pandoc.RawBlock('latex','\\begin{'..box..'}')})
    blocks:extend(el.content); blocks:insert(pandoc.RawBlock('latex','\\end{'..box..'}')); return blocks
  end
end
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
    import sys
    from PIL import Image, ImageChops, ImageDraw
    # Pango supplies per-glyph fallback for scientific labels on macOS.
    mac_renderer = shutil.which("rsvg-convert") if sys.platform == "darwin" else None
    if sys.platform == "darwin":
        require(mac_renderer, "rsvg-convert is required for macOS SVG font fallback")
    else:
        import cairosvg
    assets = sorted((book / "Assets").glob("*.svg"), key=lambda p: int(p.name.split("-")[0]))
    require(len(assets) == 172, "expected 172 source SVGs")
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
        if mac_renderer:
            run([mac_renderer, "--width", "1680", "--output", str(png), str(source)], out)
        else:
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
    captions = []
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
                captions.append(plain(follow["c"]))
                block["c"][1] = [None, [{"t": "Plain", "c": follow["c"]}]]
                i += 1
        result.append(block)
        i += 1
    doc["blocks"] = result
    require(len(captions)==len(used), f"chapter {n}: authored captions not attached to all figures")
    for index, caption in enumerate(captions,1):
        require(re.match(rf"Figure {n}\.{index}\s*[—–:-]",caption),
                f"chapter {n}: caption out of reader order: {caption}")
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


def semantic_box(block):
    # Wrap only explicit, short authored examples and notes. No text changes.
    if block["t"] != "Para" or not block["c"] or block["c"][0]["t"] != "Strong":
        return block
    label = plain(block["c"][0])
    role = None
    if re.match(r"^(Example\b|Worked example\b|Try it\b|Check your intuition\b|Tiny test\b)",label,re.I):
        role = "example"
    elif re.match(r"^(Warning\b|Common mistake\b|Caution\b)",label,re.I):
        role = "warning"
    elif re.match(r"^(Note\b|Key idea\b|The important promise\b|Remember\b)",label,re.I):
        role = "concept"
    if role and len(plain(block)) < 1400:
        return {"t":"Div","c":[["",["book-callout",role],[]],[block]]}
    return block


def prepare(root, out):
    book = root / BOOK
    chapters = sorted((book / "Chapters").glob("*.md"))
    require(len(chapters) == 32, "expected 32 chapters")
    figures = render_assets(book, out)
    report = {"schema": 1, "title": TITLE, "chapters": [], "figures": figures}
    combined, api, all_used = [], None, []
    provenance = []
    for n, chapter in enumerate(chapters, 1):
        require(chapter.name.startswith(f"{n:02d} - "), f"chapter order: {chapter}")
        source = chapter.read_text(encoding="utf-8")
        source, math_edits = format_math(source, n)
        p = run(["pandoc", "-f", READER, "-t", "json"], out, input_text=source)
        require(not p.stderr.strip(), f"Pandoc input warning {chapter.name}: {p.stderr}")
        doc = json.loads(p.stdout)
        api = api or doc["pandoc-api-version"]
        require(doc["pandoc-api-version"] == api, "mixed Pandoc APIs")
        before = doc["blocks"]
        doc["blocks"], relocated = edit_chapter(before, n)
        for kind in ("CodeBlock", "Image"):
            original = [v["c"] for v in nodes(before) if v["t"] == kind]
            edited = [v["c"] for v in nodes(doc["blocks"]) if v["t"] == kind]
            require(original == edited, f"editorial pass changed {kind} in chapter {n}")
        tagged_before = [v["c"] for v in nodes(before) if v["t"]=="Math" and TAG.search(v["c"][1])]
        tagged_after = [v["c"] for v in nodes(doc["blocks"]) if v["t"]=="Math" and TAG.search(v["c"][1])]
        require(tagged_before == tagged_after, f"editorial pass changed tagged equations in chapter {n}")
        provenance.append({"chapter":n,"relocated":relocated,"inline_math_edits":math_edits})
        counts = Counter(v["t"] for v in nodes(doc["blocks"]))
        # Check the actual parsed equations, not merely the Markdown delimiters.
        labels = [label for v in nodes(doc["blocks"]) if v["t"]=="Math"
                  for label in TAG.findall(v["c"][1])]
        require(len(labels)==len(set(labels)), f"chapter {n}: duplicate equation labels")
        # Match explicit prose references in the parsed paragraph text only;
        # code and displayed mathematics have no Str nodes.
        refs = re.findall(r"\b(?:[Ee]quation|eq\.)\s*\(?(\d{1,2}\.[A-Za-z]?\d+)\)?",
                          plain(doc["blocks"]))
        for ref in refs:
            if int(ref.split(".")[0])==n:
                require(ref in labels, f"chapter {n}: unresolved equation {ref}")
        require(counts["Math"] and counts["Image"], f"chapter lacks parsed math/figures: {chapter}")
        codes = [v["c"][1] for v in nodes(doc["blocks"]) if v["t"]=="CodeBlock"]
        used = rewrite_chapter(doc, n, figures)
        after = [v["c"][1] for v in nodes(doc["blocks"]) if v["t"]=="CodeBlock"]
        require(codes == after, f"code changed in staging: {chapter}")
        require(len(used)==counts["Image"], f"image count changed: {chapter}")
        all_used.extend(used)
        report["chapters"].append({"number":n,"file":chapter.name,"sha256":digest(chapter),
                                   "math":counts["Math"],"equation_labels":labels,"code":counts["CodeBlock"],
                                   "images":used,"blocks":len(doc["blocks"])})
        combined.extend(doc["blocks"])
    require(set(all_used)==set(figures), "unreferenced source illustrations")
    # Both formats share the same front/back matter and stable chapter anchors.
    matter = []
    for name in ("COPYRIGHT.md", "FRONT_MATTER.md", "BACK_MATTER.md", "BIBLIOGRAPHY.md"):
        file = book / name
        doc = json.loads(run(["pandoc","-f",READER,"-t","json"],out,
                             input_text=file.read_text(encoding="utf-8")).stdout)
        matter.append(doc["blocks"])
        report.setdefault("matter", {})[name] = digest(file)
    combined = matter[0] + matter[1] + [semantic_box(b) for b in combined] + matter[2] + matter[3]
    combined.extend(subject_index(combined))
    report["subject_index_entries"] = len(INDEX)
    report["editorial_paragraphs_relocated"] = sum(len(v["relocated"]) for v in provenance)
    report["inline_math_edits"] = sum(e["count"] for v in provenance for e in v["inline_math_edits"])
    (out/"reader-editorial-provenance.json").write_text(json.dumps(provenance,ensure_ascii=False,indent=2)+"\n",encoding="utf-8")
    ids = [v["c"][1][0] for v in nodes(combined) if v["t"]=="Header"]
    require(len(ids)==len(set(ids)), "duplicate combined reader header IDs")
    for v in nodes(combined):
        if v["t"]=="Link" and v["c"][2][0].startswith("#"):
            require(v["c"][2][0][1:] in ids, f"broken reader link: {v['c'][2][0]}")
    meta = {"title":{"t":"MetaString","c":TITLE},
            "subtitle":{"t":"MetaString","c":SUBTITLE},
            "lang":{"t":"MetaString","c":"en"},
            "date":{"t":"MetaString","c":"2026-11-25"}}
    doc = {"pandoc-api-version":api,"meta":meta,"blocks":combined}
    (out/"reader-print.json").write_text(json.dumps(doc,ensure_ascii=False),encoding="utf-8")
    ebook = copy.deepcopy(doc)
    ebook["blocks"] = epub_equations(ebook["blocks"])
    require(sum(1 for v in nodes(ebook["blocks"]) if v["t"]=="Math") ==
            sum(1 for v in nodes(doc["blocks"]) if v["t"]=="Math"), "math count changed for EPUB")
    (out/"reader-epub.json").write_text(json.dumps(ebook,ensure_ascii=False),encoding="utf-8")
    (out/"reader.css").write_text(CSS,encoding="utf-8")
    (out/"reader.tex").write_text(TEX,encoding="utf-8")
    (out/"print-breaks.lua").write_text(PRINT_BREAKS,encoding="utf-8")
    report["ast_sha256"] = {f:digest(out/f) for f in ("reader-print.json","reader-epub.json")}
    # HTML preview also exercises the actual MathML writer before PDF/EPUB.
    p = run(["pandoc","-f","json","-t","html5","--standalone","--mathml","--toc",
             "--css=reader.css","reader-epub.json","-o","reader-preview.html"],
            out,out/"html-preview.log")
    report["html_warnings"] = p.stderr.splitlines()
    require(not re.search(r"Could not convert TeX math|Could not render math",p.stderr,re.I),
            "HTML MathML writer reported conversion failures; see html-preview.log")
    (out/"source-proof.json").write_text(json.dumps(report,indent=2,ensure_ascii=False)+"\n",encoding="utf-8")
    print(f"Prepared {len(chapters)} chapters, {len(figures)} rendered figures, "
          f"{sum(x['math'] for x in report['chapters'])} parsed math nodes, "
          f"{sum(x['code'] for x in report['chapters'])} code blocks")
    return report


def build(out, cover, author):
    import sys
    math_font = "TeX Gyre DejaVu Math" if sys.platform == "darwin" else "DejaVu Math TeX Gyre"
    check = shutil.which("epubcheck")
    require(check, "EPUBCheck 5.4.0 is required for a complete proof; use the official distribution")
    version = run([check,"--version"],out,out/"epubcheck-version.log")
    require(re.search(r"(?<!\d)5\.4\.0(?!\d)",version.stdout+"\n"+version.stderr), "EPUBCheck version must be 5.4.0")
    cover_args = []
    cover_sha256 = None
    if cover:
        from PIL import Image
        cover = cover.resolve()
        require(cover.is_file(),"approved cover file not found")
        with Image.open(cover) as image:
            suffix = {"PNG":".png","JPEG":".jpg"}.get(image.format)
            require(suffix and image.height>image.width,
                    "approved cover must be a portrait PNG or JPEG")
            image.verify()
        staged_cover = out / ("approved-cover"+suffix)
        shutil.copyfile(cover,staged_cover)
        cover_sha256 = digest(staged_cover)
        require(cover_sha256=="69d65d13f5b28695685b8ab42c580c17705d6021e1e05ac158c95e918313ce08", "cover differs from the approved Book03 JPEG")
        cover_tex = r"""
\usepackage{eso-pic}
\AtBeginDocument{%
  \hypersetup{pageanchor=false}%
  \thispagestyle{empty}%
  \AddToShipoutPictureBG*{\AtPageLowerLeft{\color[rgb]{.018,.040,.075}\rule{\paperwidth}{\paperheight}}}%
  \AddToShipoutPictureFG*{\AtPageCenter{\makebox(0,0){\includegraphics[width=\paperwidth,height=\paperheight,keepaspectratio]{COVERFILE}}}}%
  \null\clearpage\hypersetup{pageanchor=true}%
}
""".replace("COVERFILE",staged_cover.name)
        (out/"approved-cover.tex").write_text(cover_tex,encoding="utf-8")
        cover_args = ["--include-in-header=approved-cover.tex"]
    epub = ["pandoc","-f","json","-t","epub3","--mathml","--toc","--toc-depth=2",
            "--css=reader.css","--metadata","author="+author,
            "reader-epub.json","-o","Book03-From-Geometry-to-Images-PROOF.epub"]
    if cover:
        epub.extend(["--epub-cover-image",str(staged_cover)])
    e = run(epub,out,out/"epub-build.log")
    file = out/"Book03-From-Geometry-to-Images-PROOF.epub"
    run([check,str(file)],out,out/"epubcheck.log")
    # Probe real fontspec family names rather than assuming every OS exposes STIX identically.
    font_probe = r"\documentclass{article}\usepackage{fontspec}\begin{document}" + "\n"
    for name in ("STIX", "STIX Two Text", "STIXGeneral"):
        font_probe += r"\IfFontExistsTF{"+name+r"}{\typeout{BOOKFONT="+name+r"}}{}"+"\n"
    font_probe += r"\end{document}"
    (out/"font-probe.tex").write_text(font_probe,encoding="utf-8")
    probe = run(["lualatex","-interaction=nonstopmode","-halt-on-error","font-probe.tex"],out,out/"font-probe.log")
    families = re.findall(r"(?m)^BOOKFONT=(.+)$",probe.stdout)
    require(families,"Install STIX, STIX Two Text, or STIXGeneral before the print build")
    main_font = families[0].strip()
    conversion = run(["pandoc","-f","json","-t","latex","--standalone","--top-level-division=chapter",
               "--toc","--toc-depth=2","--highlight-style=tango","--include-in-header=reader.tex",
               "--lua-filter=print-breaks.lua",*cover_args,
               "-V","documentclass=book","-V","classoption=openany","-V","classoption=oneside","-V","fontsize=10pt",
               "-V","geometry=paperwidth=8in,paperheight=10in,inner=0.85in,outer=0.70in,top=0.72in,bottom=0.75in",
               "-V","mainfont="+main_font,"-V","mathfont="+math_font,
               "-V","monofont=DejaVu Sans Mono","-V","monofontoptions=Scale=1","--metadata","author="+author,
               "reader-print.json","-o","reader-proof.tex"],
              out,out/"pdf-conversion.log")
    # Make copyright/preface genuine front matter; TOC precedes Chapter 01.
    tex_path = out/"reader-proof.tex"
    tex = tex_path.read_text(encoding="utf-8")
    marker = r"\chapter{Chapter 01"
    require(marker in tex and tex.count(r"\tableofcontents")==1, "unexpected print template")
    tex = tex.replace(r"\tableofcontents","",1).replace(r"\mainmatter","",1)
    tex = tex.replace(marker, r"\clearpage"+"\n"+r"{\setcounter{tocdepth}{1}\tableofcontents}"+"\n"+r"\mainmatter"+"\n"+marker,1)
    index_heading = r"\chapter{Subject index}\label{subject-index}"
    require(tex.count(index_heading)==1, "unexpected print subject index heading")
    tex = tex.replace(index_heading, index_heading+"\n"+r"\setlength{\parskip}{0pt}", 1)
    tex_path.write_text(tex,encoding="utf-8")
    # Keep the real compiler logs; Pandoc's temporary PDF engine directory
    # otherwise disappears before the output can be inspected.
    passes = []
    for number in range(1,4):
        run(["lualatex","-interaction=nonstopmode","-halt-on-error","-file-line-error",
             "reader-proof.tex"],out,out/f"pdf-pass-{number}.log")
        log = (out/"reader-proof.log").read_text(encoding="utf-8",errors="replace")
        passes.append(log)
    final_log = passes[-1]
    issues = [line for line in final_log.splitlines()
              if re.search(r"Overfull \\[hv]box|Missing character:|undefined references",line,re.I)]
    require(not issues,"Print preflight: unresolved layout/glyph/reference issues:\n"+"\n".join(issues[:30]))
    (out/"reader-proof.pdf").rename(out/"Book03-From-Geometry-to-Images-PROOF.pdf")
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
    print("Proof PDF and EPUB built. No promotion; formal EPUBCheck:",bool(check))
    proof = {"epub_sha256":digest(file),"pdf_sha256":digest(out/"Book03-From-Geometry-to-Images-PROOF.pdf"),
             "epubcheck_ran":True,"epubcheck_version":"5.4.0","epub_warnings":e.stderr.splitlines(),
             "pdf_warnings":conversion.stderr.splitlines(),"main_font":main_font,"code_size_tex_pt":8.3,"final_layout_issues":issues,"latex_passes":len(passes),"cover_in_both_formats":bool(cover),"approved_cover_sha256":cover_sha256,
             "accepted":False}
    (out/"build-proof.json").write_text(json.dumps(proof,indent=2)+"\n",encoding="utf-8")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root",type=Path,default=ROOT)
    parser.add_argument("--output",type=Path,help="new directory beneath Code Parts/build")
    parser.add_argument("--build",action="store_true",help="compile proof PDF and EPUB; never promote")
    parser.add_argument("--cover",type=Path,help="optional explicitly approved portrait PNG/JPEG for both formats")
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
