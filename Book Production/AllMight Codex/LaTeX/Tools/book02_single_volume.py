#!/usr/bin/env python3
"""Book Two only: one continuous digital-master typeset source and shared front matter.

The two frozen physical-part main.tex files are treated as immutable inventories,
not separately compiled books. Original chapter/reference/attachment files remain
byte-for-byte unchanged. Only temporary build copies are synthesized.

Run the outer book02_release_candidates.py to compile, verify and export.
"""
from __future__ import annotations

from pathlib import Path
import re
import subprocess

TITLE = "From Equations to Geometry"
SUBTITLE = ("Mathematics, Geometry, and Computational Foundations "
            "for Graphics, Simulation, and Research")
AUTHOR = "Swayam Singal"
YEAR = 2026
STAGE = "02-Mathematics-for-Visual-Computing"

PREFACE = (
    "Mathematics becomes useful when an abstract statement can be followed all "
    "the way to a dependable computation. This book builds that connection "
    "through algebra, calculus, geometry, probability, numerical analysis and "
    "the algorithms used to represent and simulate visual worlds.",
    "Each chapter joins three kinds of understanding: what a mathematical "
    "object means, why the operation performed on it is justified, and "
    "what changes when finite computers implement the mathematics. A "
    "derivation is not complete until its assumptions, units, invariants and "
    "failure cases are clear.",
    "The computational companion uses C++23 for the algorithms and projects. "
    "Small Python programs may visualize results, inspect generated data, "
    "or provide an independent mathematical check; they are not substitutes "
    "for implementing the central methods in C++23.",
    "This is a cumulative study book. Read the explanations, reconstruct "
    "the equations, execute the experiments, vary the assumptions, and "
    "compare the numerical evidence with the theory. The aim is to develop "
    "judgment as well as technique: to recognize when a correct formula "
    "is not yet a correct or trustworthy program.",
)

READING_GUIDE = (
    "Start each chapter with the geometric or physical question it is trying "
    "to answer. Keep track of coordinate frames, units, shapes, domains, "
    "boundary conditions and conventions before manipulating symbols.",
    "Colored concept panels collect definitions and invariants; derivation "
    "panels expose reasoning; numerical pitfall panels identify where "
    "mathematical assumptions meet finite precision; and C++23 laboratory "
    "panels connect those ideas to executable experiments. Their role "
    "labels still distinguish them in grayscale.",
    "For the projects, compile the native C++23 workbench using CMake, run "
    "its CTest assertions and export the data before plotting anything. "
    "Record input sizes, exact formulas, seeds, tolerances and failure "
    "conditions. A successful fixture establishes only its stated contract.",
    "The reference notes preserve longer supporting discussions, and the "
    "source-code attachments preserve original project fragments. The "
    "selected primary references are a route to foundational literature, "
    "not a claim that every passage or algorithm reproduces an original "
    "paper's full experimental or mathematical results.",
)

# Checked primary sources, with permanent author / institution links.
# These are selective reading references, not invented in-text citations.
SELECTED_REFERENCES = (
    ("Shewchuk, Jonathan Richard (1997).",
     "Adaptive Precision Floating-Point Arithmetic and Fast Robust Geometric Predicates.",
     "Discrete & Computational Geometry 18(3), 305–363.",
     "https://www.cs.cmu.edu/~quake/robust.html"),
    ("Crane, Keenan; Weischedel, Clarisse; Wardetzky, Max (2013).",
     "Geodesics in Heat: A New Approach to Computing Distance Based on Heat Flow.",
     "ACM Transactions on Graphics 32(5).",
     "https://www.cs.cmu.edu/~kmcrane/Projects/HeatMethod/"),
    ("Veach, Eric (1997).",
     "Robust Monte Carlo Methods for Light Transport Simulation.",
     "PhD dissertation, Stanford University.",
     "https://graphics.stanford.edu/papers/veach_thesis/"),
)

CONTENT_RE = re.compile(r"\\input\{\.\./current/([^}\n]+/content/([^}/]+)\.tex)\}")
SOURCE_ATTACHMENT_RE = re.compile(
    r"(\\hypertarget\{source-file-[^\n]*\n"
    r"\\label\{attachmentstart:[^\n]*\n"
    r"\\section\*\{[^\n]*\}\n"
    r"\\VerbatimInput(?:\[[^\n]*\])?\{[^}\n]+\})",
)
CHAPTER_ID_RE = re.compile(
    r"^(?:01-foundations-i-ch|02-foundations-ii-ch|03-visual-computing-i-ch)\d+$"
)


def latex_paragraphs(paragraphs: tuple[str, ...]) -> str:
    """Escape narrative prose, keeping print and EPUB word content aligned."""
    escape = {"\\": r"\textbackslash{}", "&": r"\&", "%": r"\%",
              "$": r"\$", "#": r"\#", "_": r"\_", "{": r"\{", "}": r"\}"}
    return "\n\n".join(r"\noindent "+ "".join(escape.get(ch, ch) for ch in p)+r"\par"
                       for p in paragraphs)


def inventory(parts: tuple[Path, Path]) -> dict[str, list[str]]:
    chapters: list[str] = []
    references: list[str] = []
    attachments: list[str] = []
    for main in parts:
        src = main.read_text(encoding="utf-8")
        for match in CONTENT_RE.finditer(src):
            inp = match.group(0)
            stem = match.group(2)
            if CHAPTER_ID_RE.match(stem):
                chapters.append(inp)
            elif stem.startswith("appendix-"):
                references.append(inp)
            else:
                raise RuntimeError(f"Unexpected Book Two input: {inp}")
        if src.count(r"\chapter{Source Attachments}") != 1:
            raise RuntimeError(f"Missing source attachment anchor in {main}")
        after = src.split(r"\chapter{Source Attachments}", 1)[1].split(r"\end{document}",1)[0]
        matched = list(SOURCE_ATTACHMENT_RE.finditer(after.strip()))
        if len(matched) != after.count(r"\VerbatimInput"):
            raise RuntimeError(f"Source attachment parsing lost code blocks in {main}")
        attachments.extend(m.group(1) for m in matched)
    if len(chapters) != 16 or len(references) != 31 or len(attachments) != 3:
        raise RuntimeError("Book Two inventory differs from expected "
                           f"16 chapters / 31 notes / 3 source files: "
                           f"{len(chapters)}/{len(references)}/{len(attachments)}")
    if len(set(chapters)) != 16 or len(set(references)) != 31:
        raise RuntimeError("Duplicate frozen input or reference found")
    return {"chapters":chapters, "references":references, "attachments":attachments}


def selected_references_latex() -> str:
    items=[]
    for author,title,publication,url in SELECTED_REFERENCES:
        items.append(r"\item "+author+" \emph{"+title+"} "+publication+
                     r" \url{"+url+"}")
    return r"\begin{enumerate}"+"\n"+"\n".join(items)+"\n"+r"\end{enumerate}"


def build_unified_tex(parts: tuple[Path, Path]) -> str:
    rows=inventory(parts)
    references="\n".join(rows["references"])
    chapters="\n".join(rows["chapters"])
    code_blocks=[]
    for source in rows["attachments"]:
        # Only the \VerbatimInput is shaded: its printed heading, label and
        # target hyperlink remain outside the breakable panel.
        source=source.replace(r"\VerbatimInput",(
            r"\begin{tcolorbox}[enhanced,breakable,sharp corners,"
            r"colback=booktwocodebg,colframe=booktwocyan!40!white,"
            r"boxrule=.4pt,borderline west={2.5pt}{0pt}{booktwocyan},"
            r"left=6pt,right=6pt,top=5pt,bottom=5pt]"
            "\n"+r"\VerbatimInput"),1)+"\n"+r"\end{tcolorbox}"
        code_blocks.append(source)
    attachments="\n\n".join(code_blocks)
    return rf"""\documentclass[11pt,twoside,openany]{{book}}
\usepackage[print]{{allmight-book}}
\providecommand{{\SetBookStage}}[2]{{}}
\SetBookStage{{2}}{{Mathematics for Visual Computing}}
\title{{Mathematics for Visual Computing}}
\author{{{AUTHOR}}}
\hypersetup{{pdftitle={{{TITLE}}},pdfauthor={{{AUTHOR}}},
  pdfsubject={{{SUBTITLE}}}}}
\begin{{document}}
\frontmatter
\begin{{titlepage}}
\thispagestyle{{empty}}
\centering
\vspace*{{.25\textheight}}
{{\fontsize{{30}}{{36}}\selectfont\bfseries\color{{booktwonavy}} {TITLE}\par}}
\vspace{{1.9em}}
{{\fontsize{{12.5}}{{18}}\selectfont\color{{booktwonavy}}
\begin{{minipage}}{{.87\textwidth}}\centering {SUBTITLE}\end{{minipage}}\par}}
\vfill
{{\fontsize{{16}}{{21}}\selectfont\color{{booktwonavy}} {AUTHOR}\par}}
\vspace*{{.08\textheight}}
\end{{titlepage}}
\clearpage
\thispagestyle{{empty}}
\null\vfill
{{\small
\noindent Copyright \textcopyright\ {YEAR} {AUTHOR}.\par
\medskip
\noindent All rights reserved.\par
\medskip
\noindent The mathematical derivations and software examples are
provided for educational study; verify the assumptions and
safety requirements before applying them to critical systems.\par
}}
\clearpage
\chapter*{{Preface}}
\phantomsection\addcontentsline{{toc}}{{chapter}}{{Preface}}
{latex_paragraphs(PREFACE)}
\chapter*{{How to Use This Book}}
\phantomsection\addcontentsline{{toc}}{{chapter}}{{How to Use This Book}}
{latex_paragraphs(READING_GUIDE)}
\tableofcontents
\mainmatter
{chapters}
\label{{stagechaptersend:{STAGE}}}
\appendix
\chapter{{Reference Notes}}
{references}
\label{{stagereferencesend:{STAGE}}}
\chapter{{Source Code Attachments}}
{attachments}
\chapter{{Selected Primary References}}
\noindent The primary sources below support the geometric,
numerical and sampling research laboratories. They are
provided for further reading; the original 31 reference
notes and all source attachments are preserved separately.\par
{selected_references_latex()}
\end{{document}}
"""


def _pstr(value: str) -> list[dict]:
    # Pandoc Str should contain no whitespace, hence make explicit Space nodes.
    tokens=re.findall(r"\S+|\s+",value)
    return ([{"t":"Space"} if w.isspace() else {"t":"Str","c":w}
             for w in tokens])


def epub_frontmatter() -> list[dict]:
    blocks=[]
    for title,paragraphs in (
        ("Copyright",(f"Copyright © {YEAR} {AUTHOR}. All rights reserved.",)),
        ("Preface",PREFACE),
        ("How to Use This Book",READING_GUIDE),
    ):
        blocks.append({"t":"Header","c":[1,["book02-"+title.lower().replace(" ","-"),[],[]],_pstr(title)]})
        blocks.extend({"t":"Para","c":_pstr(p)} for p in paragraphs)
    return blocks


def epub_references() -> list[dict]:
    blocks=[{"t":"Header","c":[1,["book02-selected-primary-references",[],[]],
                                   _pstr("Selected Primary References")]}]
    blocks.append({"t":"Para","c":_pstr("The original reference notes and source-code attachments are preserved separately. These selected primary references provide further reading.")})
    items=[]
    for author,title,publication,url in SELECTED_REFERENCES:
        inline=_pstr(author+" "+title+" "+publication+" ")
        inline.append({"t":"Link","c":[["",[],[]],_pstr("Original source"),[url,""]]})
        items.append([{"t":"Para","c":inline}])
    blocks.append({"t":"BulletList","c":items})
    return blocks


def generate_minimal_jpeg_cover(destination: Path) -> Path:
    """Exactly three pieces of visible text, with no badge, series title or logo.

    The JPEG is an EPUB front-cover asset; the unified PDF title page uses
    the same content but keeps the type fully vector-rendered by LuaLaTeX.
    """
    from PIL import Image, ImageDraw, ImageFont
    destination.parent.mkdir(parents=True,exist_ok=True)
    if destination.is_file():
        destination.unlink()  # generated build output only
    find = subprocess.run(["kpsewhich","texgyrepagella-regular.otf"],
                          text=True,capture_output=True)
    regular=find.stdout.strip()
    find_bold = subprocess.run(["kpsewhich","texgyrepagella-bold.otf"],
                               text=True,capture_output=True)
    bold=find_bold.stdout.strip()
    if not regular or not Path(regular).is_file() or not bold or not Path(bold).is_file():
        raise RuntimeError("TeX Gyre Pagella OTF fonts not found with kpsewhich. "
                           "Install the TeX fonts (already required by PDF), "
                           "or supply a cover JPEG explicitly.")
    image=Image.new("RGB",(1600,2560),(17,35,59))
    draw=ImageDraw.Draw(image)
    ink=(244,240,224)
    accent=(201,183,140)
    titlefont=ImageFont.truetype(bold,106)
    subfont=ImageFont.truetype(regular,49)
    authorfont=ImageFont.truetype(regular,61)

    def centered(lines: list[str], font, middle: int, spacing: int, fill):
        bbox=[draw.textbbox((0,0),line,font=font) for line in lines]
        heights=[box[3]-box[1] for box in bbox]
        total=sum(heights)+spacing*(len(lines)-1)
        y=middle-total//2
        for line,height,bb in zip(lines,heights,bbox):
            width=bb[2]-bb[0]
            draw.text(((1600-width)//2-bb[0],y-bb[1]),line,font=font,fill=fill)
            y+=height+spacing

    centered(["From Equations","to Geometry"],titlefont,860,32,ink)
    centered(["Mathematics, Geometry, and Computational",
              "Foundations for Graphics, Simulation,",
              "and Research"],subfont,1290,19,accent)
    centered([AUTHOR],authorfont,2140,0,ink)
    image.save(destination,"JPEG",quality=94,optimize=True,subsampling=0)
    return destination
