#!/usr/bin/env python3
"""Verify Pandoc 3.x native MathML accepts *all* 36 Book Two supplements.

The earlier tests only checked that Pandoc could package source LaTeX into an
EPUB. This more stringent test builds a single JSON Pandoc document containing
every Math AST node extracted from the actual supplemental TeX, then asks
Pandoc to generate native MathML for all of them.

No silently substituted TeX spans, math screenshots or omitted formulas are
allowed. It is NOT full validation of the 16 frozen canonical chapter ASTs;
those 39k original Math nodes are checked by the full author-side builder.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path
import re
import shutil
import subprocess

from book02_epub_supplements import all_source_filenames
from book02_visuals import FIGURE_SOURCE


def traverse(x):
    if isinstance(x,dict):
        yield x
        for v in x.values():
            yield from traverse(v)
    elif isinstance(x,list):
        for v in x:
            yield from traverse(v)


def checked_pandoc(args, source):
    p=subprocess.run(["pandoc",*args],input=source,stdout=subprocess.PIPE,
                     stderr=subprocess.PIPE,text=True)
    if p.returncode:
        raise RuntimeError(f"Pandoc exited {p.returncode}: {p.stderr[-1800:]}")
    return p


def audit_supplementary_mathml():
    if not shutil.which("pandoc"):
        raise RuntimeError("Pandoc 3.x is required for supplemental MathML check")
    source_math=[]
    source_counts={}
    for name in all_source_filenames():
        raw=(FIGURE_SOURCE/name).read_text(encoding="utf-8")
        if re.search(r"\\(?:rm|bf|it)\b",raw):
            raise RuntimeError(f"Legacy math font macro in {name}: rewrite using \\mathrm{{...}}, etc.")
        parsed=checked_pandoc(["--from=latex","--to=json"],raw)
        ast=json.loads(parsed.stdout)
        elements=[node for node in traverse(ast.get("blocks",[]))
                  if node.get("t")=="Math"]
        source_counts[name]=len(elements)
        for element in elements:
            source_math.append({"file":name,"node":element})
    if len(source_math)<35:
        raise RuntimeError(f"Suspiciously few parsed supplemental formulas: {len(source_math)}")
    # Run as one document: this reports all incompatible formulas in one pass
    # rather than giving the user one failure per full, expensive EPUB build.
    paragraphs=[{"t":"Para","c":[item["node"]]} for item in source_math]
    ast={"pandoc-api-version":[1,23,1],"meta":{},"blocks":paragraphs}
    converted=checked_pandoc(["--from=json","--to=html5",
                              "--math-method=mathml"],
                             json.dumps(ast,ensure_ascii=False))
    warnings=converted.stderr
    if "Could not convert TeX math" in warnings:
        # List originating file and TeX for easier correction.
        bad=[{"source":item["file"],"tex":item["node"]["c"][1]}
             for item in source_math
             if item["node"]["c"][1] in warnings]
        raise RuntimeError("Pandoc failed strict MathML on "+json.dumps(bad[:25],
                          ensure_ascii=False,indent=2)+"\n"+warnings[-2600:])
    actual=len(re.findall(r"<math\b",converted.stdout))
    if actual!=len(source_math):
        raise RuntimeError(f"MathML result missing formulas: {actual} / {len(source_math)}")
    if re.search(r"<span[^>]+class=['\"]math\b",converted.stdout):
        raise RuntimeError("TeX math fallback HTML span found instead of MathML")
    return {"supplements_checked":len(source_counts),
            "native_mathml_formulas":actual,
            "sources_with_formulas":sum(v>0 for v in source_counts.values()),
            "source_formula_counts":source_counts,
            "pandoc_warnings":warnings[-2000:]}


if __name__=="__main__":
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("--output",type=Path)
    args=p.parse_args()
    result=audit_supplementary_mathml()
    if args.output:
        args.output.parent.mkdir(parents=True,exist_ok=True)
        args.output.write_text(json.dumps(result,indent=2)+"\n",encoding="utf-8")
    print(f"BOOK02_MATHML_OK: {result['native_mathml_formulas']} / "
          f"{result['native_mathml_formulas']} native MathML; "
          f"{result['supplements_checked']} source supplements")
