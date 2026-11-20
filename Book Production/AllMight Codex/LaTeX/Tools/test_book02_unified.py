#!/usr/bin/env python3
"""Read-only Book Two unified-publication structural contracts.

No LuaLaTeX, Pandoc or PDF rendering is performed. The author's Mac performs
the expensive final proof; this catches accidental split/omission before that.
"""
from __future__ import annotations

import ast
import importlib.util
import re
import sys
from pathlib import Path

TOOLS=Path(__file__).resolve().parent
ROOT=TOOLS.parent
REPO=ROOT.parents[2]
sys.path.insert(0,str(TOOLS))
from book02_single_volume import (TITLE,SUBTITLE,AUTHOR,STAGE,PREFACE,
                                   READING_GUIDE,build_unified_tex,inventory,
                                   epub_frontmatter,epub_references)

def test() -> None:
    generation=ROOT/"Generations"/".generation-n3azv4es"
    parts=tuple(generation/(STAGE+suffix)/"main.tex"
                for suffix in ("-P01","-P02"))
    assert all(p.is_file() for p in parts), parts
    rows=inventory(parts)
    assert {key:len(value) for key,value in rows.items()} == {
        "chapters":16,"references":31,"attachments":3}
    tex=build_unified_tex(parts)
    assert tex.count(r"\documentclass")==1
    assert tex.count(r"\begin{document}")==1
    assert tex.count(r"\end{document}")==1
    assert tex.count(r"\tableofcontents")==1
    assert tex.count(r"\frontmatter")==1
    assert tex.count(r"\mainmatter")==1
    assert tex.count(r"\appendix")==1
    assert tex.count(r"\begin{titlepage}")==1
    assert tex.count(r"\chapter*{Preface}")==1
    assert tex.count(r"\chapter*{How to Use This Book}")==1
    assert tex.count(r"\VerbatimInput")==3
    assert tex.count(r"\begin{tcolorbox}[")==3
    assert tex.count(r"\end{tcolorbox}")==3
    assert tex.count("frame=none")==3  # no redundant nested code frames
    original_code_count=0
    for original_input in rows["chapters"]:
        fragment=re.search(r"\\input\{\.\./current/([^}]+)\}",original_input)
        assert fragment is not None
        chapter=(generation/fragment.group(1)).read_text(encoding="utf-8")
        original_code_count += chapter.count(r"\begin{verbatim}")
    assert original_code_count==112, original_code_count
    visual_preamble=(ROOT/"Assets"/"Book02"/"preamble.tex").read_text(encoding="utf-8")
    assert r"\fvset{frame=single" in visual_preamble
    assert "fillcolor=\\color{booktwocodebg}" in visual_preamble
    epub_css=(TOOLS/"book02_epub_supplements.py").read_text(encoding="utf-8")
    assert "pre { background: #f0f4f6; border-left:" in epub_css
    assert tex.count(r"\input{../current/")==47  # 16 original + 31 source notes
    assert all(tex.count(row)==1 for group in ("chapters","references")
               for row in rows[group])
    assert "Physical part" not in tex
    assert "Next part" not in tex
    assert "Previous part" not in tex
    assert r"\maketitle" not in tex
    assert tex.index(r"\frontmatter") < tex.index(r"\chapter*{Preface}") < tex.index(r"\tableofcontents") < tex.index(r"\mainmatter")
    assert tex.index(r"\mainmatter") < tex.index(rows["chapters"][0]) < tex.index(rows["chapters"][-1]) < tex.index(r"\appendix")
    assert tex.index(r"\appendix") < tex.index(rows["references"][0]) < tex.index(rows["references"][-1]) < tex.index(r"\chapter{Source Code Attachments}")
    for field in (TITLE,SUBTITLE,AUTHOR):
        assert field in tex
    assert "STAGE 2" not in tex.split(r"\end{titlepage}")[0]
    for para in PREFACE+READING_GUIDE:
        assert para[:30] in tex
    front=epub_frontmatter()
    headings=[b for b in front if b["t"]=="Header"]
    assert len(headings)==3
    assert ["".join(x.get("c"," ") if x.get("t")=="Str" else " "
                    for x in h["c"][2]).strip() for h in headings] == [
        "Copyright","Preface","How to Use This Book"]
    assert len([b for b in front if b["t"]=="Para"])==1+len(PREFACE)+len(READING_GUIDE)
    refs=epub_references()
    assert refs[-1]["t"]=="BulletList"
    assert len(refs[-1]["c"])==3

    for script in ("book02_single_volume.py","book02_release_candidates.py","build_epubs.py"):
        code=(TOOLS/script).read_text(encoding="utf-8")
        ast.parse(code,filename=script)
    release=(TOOLS/"book02_release_candidates.py").read_text()
    assert "single_pdf_candidate(generation,output)" in release
    assert "generate_minimal_jpeg_cover" in release
    assert "print_only=args.print_only" in release
    assert "book02-candidate-verification-stale-" in release
    assert 'files.append((location,"From Equations to Geometry.pdf"))' in release
    epub=(TOOLS/"build_epubs.py").read_text()
    assert "blocks[:0]=epub_frontmatter()" in epub
    assert "blocks.extend(epub_references())" in epub
    print("BOOK02_UNIFIED_DIGITAL_BOOK_SOURCE_OK",
          "16 chapters","112 original code frames","31 reference notes",
          "3 boxed C++ attachments",
          "single title page","single TOC","preface","copyright",
          "book reading guide","selected primary references")

if __name__=="__main__":
    test()
