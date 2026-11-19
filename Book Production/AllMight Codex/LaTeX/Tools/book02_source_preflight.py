#!/usr/bin/env python3
"""Book Two release gate: 16 original source locks and additive inventory.

This is a strict source-custody and layout-input integrity check, NOT a proof
that all 1.75 MB of original mathematical claims have been peer reviewed.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import re
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
LOCK=ROOT/"Tools"/"book02_original_source_lock.json"
CATALOG=ROOT/"Tools"/"stage-catalog.json"
EXTRA=ROOT/"Assets"/"Book02"


def git_blob_sha(data:bytes)->str:
    return hashlib.sha1(b"blob "+str(len(data)).encode()+b"\0"+data).hexdigest()


def audit()->dict:
    lock=json.loads(LOCK.read_text(encoding="utf-8"))
    catalogue=json.loads(CATALOG.read_text(encoding="utf-8"))
    stage=next(s for s in catalogue["stages"] if s["number"]==2)
    expected=stage["chapters"]
    actual=[r["chapter"] for r in lock["chapters"]]
    if actual!=expected or len(actual)!=16:
        raise RuntimeError("Book Two chapter inventory or order changed")
    chapter_results=[]
    for record in lock["chapters"]:
        path=Path(__file__).resolve().parents[4]/record["relative_path"]
        raw=path.read_bytes()
        if len(raw)!=record["bytes"]:
            raise RuntimeError("Original Book Two source byte-size changed: "+str(path))
        sha=git_blob_sha(raw)
        if sha!=record["git_blob_sha1"]:
            raise RuntimeError("Original Book Two source contents changed: "+str(path))
        text=raw.decode("utf-8")
        if "\\chapter{" not in text or len(text)<30000:
            raise RuntimeError("Original chapter appears incomplete: "+str(path))
        if not re.search(r"\\hypertarget\{"+re.escape(record["chapter"])+r"\}\{\}",text):
            raise RuntimeError("Original chapter anchor is missing: "+str(path))
        chapter_results.append({
            "chapter":record["chapter"],
            "source_bytes":len(raw),
            "github_blob_sha1":sha,
            "section_count":len(re.findall(r"\\(?:sub)*section\{",text)),
            "source_code_blocks":len(re.findall(r"\\begin\{(?:Verbatim|Shaded|lstlisting|Highlighting)\}",text)),
            "figure_references":len(re.findall(r"\\(?:includegraphics|begin\{tikzpicture\})",text)),
            "citations_or_urls":len(re.findall(r"\\(?:cite|href|url)\{",text)),
        })
    supplemental=[x.name for x in EXTRA.glob("??-*.tex")]
    if len(supplemental)!=35:
        raise RuntimeError(f"Expected 35 additive Book Two TeX sources, got {len(supplemental)}")
    if len(set(supplemental))!=35:
        raise RuntimeError("Duplicate supplement filenames")
    report={
        "pass":"source custody preflight",
        "original_chapters":len(chapter_results),
        "original_bytes":sum(x["source_bytes"] for x in chapter_results),
        "unchanged_github_sha1":True,
        "additive_sources":len(supplemental),
        "notes":[
            "All original chapters are checked byte-for-byte against the frozen-generation blob SHA1.",
            "This report checks source identity and document shape, not full mathematical correctness.",
            "All original text plus 35 chapter-boundary supplements still require full PDF and EPUB rendering acceptance.",
        ],
        "chapters":chapter_results,
    }
    return report


if __name__=="__main__":
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("--output",type=Path)
    args=p.parse_args()
    result=audit()
    raw=json.dumps(result,indent=2,ensure_ascii=False)+"\n"
    if args.output:
        args.output.parent.mkdir(parents=True,exist_ok=True)
        args.output.write_text(raw,encoding="utf-8")
    print("BOOK02_SOURCE_LOCK_OK",result["original_chapters"],
          "original chapters, bytes",result["original_bytes"],
          "supplements",result["additive_sources"])
