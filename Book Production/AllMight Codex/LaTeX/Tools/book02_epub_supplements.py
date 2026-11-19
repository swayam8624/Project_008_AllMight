#!/usr/bin/env python3
"""Add all Book Two supplemental teaching to frozen-source EPUB composition.

IMPORTANT CONTRACT
------------------
The 568 canonical source rows and their SHA-locked Pandoc ASTs are not edited.
Each of the original 16 Book Two chapter rows remains present once and in
unchanged order. This adapter *appends* its source-accurate lesson ASTs at the
same chapter boundaries as the print-only book02_visuals.py mapping.

Unlike the print build, EPUB images are SVGs generated from reproducible
numeric data. All 35 source lessons are preserved: 5 analytic plates,
29 other lessons/research/figure fragments, and 1 cumulative project.
No PDF screenshot or raw TikZ is silently substituted for readable text.

Requires pandoc 3.x (AST API 1.23), matplotlib/numpy/scipy; no network.
Only Book Two stage and its complete-collection portion are modified.
"""
from __future__ import annotations

import hashlib
import json
import re
import shutil
import subprocess
import sys
from pathlib import Path

from book02_visuals import FIGURES, RICH_FIGURES, PRELUDE_CHAPTER, PRELUDE_FILE, FIGURE_SOURCE

STAGE = "02-Mathematics-for-Visual-Computing"
REPO = Path(__file__).resolve().parents[4]
LABS = REPO / "tools" / "labs"
ANALYTIC = {
    "01-linear-transformation.tex": "01-linear-transformation",
    "02-gaussian-uncertainty.tex": "02-gaussian-uncertainty",
    "03-optimization-landscape.tex": "03-optimization-landscape",
    "04-heat-diffusion.tex": "04-heat-diffusion",
    "05-convergence-study.tex": "05-convergence-study",
}
RICH = {
    f"{i:02d}-atlas-{subject}.tex": slug
    for i, subject, slug in (
        (28, "orientation", "orient2d-certification"),
        (29, "geodesic", "geodesic-fields"),
        (30, "geodesic-3d", "geodesic-surface-3d"),
        (31, "resolution", "geodesic-resolution"),
        (32, "fitting", "robust-fitting"),
        (33, "sampling", "mis-variance"),
        (34, "ransac", "ransac-consensus"),
    )
}
# Each scientifically meaningful image gets a text alternative distinct from
# its longer caption. Caption and body explain the experimental limitations.
ALTERNATIVES = {
    "01-linear-transformation": "Unit circle and its ellipse under an exact two-by-two shear and scale matrix, with mapped basis vectors.",
    "02-gaussian-uncertainty": "Three-dimensional normalized bivariate Gaussian probability density and top-down map, with covariance diagonal 1.44 and 0.3025.",
    "03-optimization-landscape": "Convex quadratic objective surface with five specified gradient-descent iterates and its exact minimizer.",
    "04-heat-diffusion": "Analytic heat-equation mode on a unit square at time 0.04, as a three-dimensional surface and a heatmap.",
    "05-convergence-study": "Forward-Euler absolute error against step size on logarithmic axes, compared with a first-order guide.",
    "orient2d-certification": "Clockwise and counterclockwise triangle examples plus certification and fallback regions of nearly collinear binary64 inputs.",
    "ransac-consensus": "Synthetic planar points with RANSAC consensus selection, alternative TLS line, and full-population residual histograms.",
    "geodesic-fields": "Computed mesh heat, recovered geodesic distance and signed error against Euclidean flat-domain distance.",
    "geodesic-surface-3d": "Computed geodesic distance lifted into a three-dimensional scalar surface beside a signed mesh error map.",
    "geodesic-resolution": "Mean, median and maximum absolute geodesic errors at six mesh resolutions on the same planar domain.",
    "robust-fitting": "Synthetic exponential observations with one outlier, ordinary and robust fitted curves, and all-data residual distributions.",
    "mis-variance": "Monte Carlo signed-error distributions and empirical replicate variances for equal function-evaluation budgets.",
}


def all_source_filenames() -> list[str]:
    names = [PRELUDE_FILE]
    for chapter, normal in FIGURES.items():
        names.extend(normal)
        names.extend(RICH_FIGURES.get(chapter, ()))
    if len(names) != len(set(names)):
        raise RuntimeError("Book Two supplement filename used twice")
    if len(names) != 35:
        raise RuntimeError(f"Expected 35 additive TeX lessons, got {len(names)}")
    return names


def pandoc_tex(source: str) -> list[dict]:
    exe=shutil.which("pandoc")
    if not exe:
        raise RuntimeError("Pandoc executable is required for semantic EPUB additions")
    cmd=[exe, "--from=latex", "--to=json"]
    run=subprocess.run(cmd,input=source,text=True,stdout=subprocess.PIPE,
                       stderr=subprocess.PIPE)
    if run.returncode:
        raise RuntimeError("Failed to parse Book02 supplementary TeX: "+run.stderr[-1500:])
    document=json.loads(run.stdout)
    if document.get("pandoc-api-version",[0])[0] != 1:
        raise RuntimeError("Unexpected Pandoc AST version")
    return document.get("blocks",[])


def walk(node):
    if isinstance(node, dict):
        yield node
        for part in node.values():
            yield from walk(part)
    elif isinstance(node, list):
        for part in node:
            yield from walk(part)


def as_str(text: str) -> list[dict]:
    result=[]
    for i,word in enumerate(text.split()):
        if i:
            result.append({"t":"Space"})
        result.append({"t":"Str","c":word})
    return result


def figure_caption(fragment: str) -> str:
    # A figure may contain nested math braces in caption. Preserve the
    # complete original TeX caption by anchoring its closing brace to
    # the immediately following \label; don't stop at the first '}'.
    found=re.search(r"\\caption\{(.*)\}\s*\\label\{([^}]+)\}",fragment,re.S)
    if not found:
        raise RuntimeError("Image figure is missing the source caption/label")
    return found.group(1)


class Book02EpubSupplement:
    def __init__(self, output: Path, asset_callback):
        self.output=Path(output)
        self.asset_callback=asset_callback
        self.figure_paths:dict[str,Path]={}
        self.lesson_cache:dict[tuple[str,int],list[dict]]={}
        self.used:list[dict]=[]
        self.prepared=False

    def prepare(self):
        if self.prepared:
            return
        root=self.output/"book02-epub-visual-sources"
        rich=root/"rich"
        analytic=root/"analytical"
        # Rebuilding on each clean production run intentionally avoids
        # borrowing cached plots from a different code revision.
        rich_script=LABS/"book02_visual_atlas.py"
        analytic_script=LABS/"book02_analytical_epub_figures.py"
        for script,dest in ((rich_script,rich),(analytic_script,analytic)):
            if not script.is_file():
                raise FileNotFoundError(script)
            run=subprocess.run([sys.executable,str(script),"--output",str(dest)],
                               text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
            if run.returncode:
                raise RuntimeError("Book Two EPUB figure generation failed: "+
                                   run.stdout[-2500:])
        for name,slug in ANALYTIC.items():
            self.figure_paths[name]=analytic/(slug+".svg")
        for name,slug in RICH.items():
            self.figure_paths[name]=rich/"figures"/(slug+".svg")
        for name,path in self.figure_paths.items():
            if not path.is_file() or path.stat().st_size<600:
                raise RuntimeError("Book Two EPUB source image missing: "+str(path))
        self.prepared=True

    def lesson(self,name:str,base_level:int)->list[dict]:
        if name not in all_source_filenames():
            raise ValueError("Unexpected Book Two source "+name)
        key=(name,base_level)
        if key in self.lesson_cache:
            # Explicitly avoid sharing mutable AST nodes across the stage
            # volume and the all-seven-books collection.
            import copy
            return copy.deepcopy(self.lesson_cache[key])
        raw=(FIGURE_SOURCE/name).read_text(encoding="utf-8")
        if name in self.figure_paths or name in ANALYTIC or name in RICH:
            self.prepare()
            if raw.count(r"\begin{figure}")!=1 or raw.count(r"\end{figure}")!=1:
                raise RuntimeError(f"Expected one source plot in {name}")
            fragment=re.search(r"\\begin\{figure\}.*?\\end\{figure\}",raw,re.S)
            if fragment is None:
                raise RuntimeError(f"Missing figure block in {name}")
            caption=figure_caption(fragment.group())
            src=self.figure_paths[name]
            target=self.asset_callback(src,".svg")
            # Generate semantic Pandoc Figure node from an explicit
            # \includegraphics target, preserving the *original caption*
            # and the source section prose before/after that figure.
            replacement=(r"\begin{figure}[!htbp]"+"\n"+r"\centering"+"\n"+
                         r"\includegraphics[width=0.9\linewidth]{"+target+"}\n"+
                         r"\caption{"+caption+"}\n"+r"\end{figure}")
            raw=raw[:fragment.start()]+replacement+raw[fragment.end():]
        blocks=pandoc_tex(raw)
        if not blocks:
            raise RuntimeError("Empty Pandoc conversion for "+name)
        headings=[n for n in walk(blocks) if n.get("t")=="Header"]
        if not headings:
            raise RuntimeError("Source has no reader-visible heading: "+name)
        prefix="book02-"+Path(name).stem
        for node in walk(blocks):
            if node.get("t")=="Header":
                node["c"][0]=min(6,int(node["c"][0])+base_level)
                node["c"][1][0]=prefix+"-"+str(node["c"][1][0])
            elif node.get("t")=="Figure":
                node["c"][0][0]=prefix+"-figure"
            elif node.get("t")=="Image":
                raw_target=node["c"][2][0]
                if not raw_target.startswith("assets/"):
                    raise RuntimeError("Book02 image target not packaged: "+raw_target)
                slug=ANALYTIC.get(name,RICH.get(name))
                node["c"][0]=["",[],[["width","90%"]]]
                node["c"][1]=as_str(ALTERNATIVES[slug])
                node["c"][2][1]=ALTERNATIVES[slug]
        # A Pandoc fallback to unrendered LaTeX/PGFPlots is a hard failure.
        for node in walk(blocks):
            if node.get("t") in ("RawBlock","RawInline") and "tikz" in str(node.get("c","")).lower():
                raise RuntimeError("Raw TikZ leaked into EPUB for "+name)
        import copy
        self.lesson_cache[key]=copy.deepcopy(blocks)
        self.used.append({"source":name,"sha256":hashlib.sha256(
            (FIGURE_SOURCE/name).read_bytes()).hexdigest(),
            "math_nodes":sum(1 for n in walk(blocks) if n.get("t")=="Math"),
            "images":sum(1 for n in walk(blocks) if n.get("t")=="Image")})
        return blocks

    def chapter_blocks(self,chapter_id:str,mode:str,prelude=False) -> list[dict]:
        if chapter_id not in FIGURES:
            return []
        base_level=1 if mode=="stage" else 2
        if prelude:
            return self.lesson(PRELUDE_FILE,base_level) if chapter_id==PRELUDE_CHAPTER else []
        blocks=[]
        for name in (*FIGURES[chapter_id],*RICH_FIGURES.get(chapter_id,())):
            blocks.extend(self.lesson(name,base_level))
        return blocks
