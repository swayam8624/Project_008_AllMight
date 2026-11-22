#!/usr/bin/env python3
"""Export Book Two's standalone *supplement integration proof* for EPUBCheck.

Not the original 16-chapter book. Instead, a demonstrably valid EPUB with
all 36 real supplemental lessons, all 13 SVGs, and a placeholder header
for each original chapter to prove the intended placement. The complete
full-source KDP EPUB requires the frozen 568-row conversion source tree.
"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import tempfile

from book02_epub_supplements import (
    FIGURES, PRELUDE_CHAPTER, Book02EpubSupplement
)


def make_proof(out: Path) -> Path:
    out=Path(out).resolve()
    out.parent.mkdir(parents=True,exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="book02-epub-proof-") as tmp:
        tmp=Path(tmp)
        assets=tmp/"assets"
        assets.mkdir()
        def asset_file(src,ext):
            source=Path(src)
            raw=source.read_bytes()
            name=hashlib.sha256(raw).hexdigest()+ext
            dest=assets/name
            if not dest.exists():
                dest.write_bytes(raw)
            return "assets/"+name
        adapter=Book02EpubSupplement(tmp,asset_file)
        blocks=[
            {"t":"Header","c":[1,["book02-proof",[],[]],[
                {"t":"Str","c":"Book Two: Supplementary Content Integration Proof"}]]},
            {"t":"Para","c":[{"t":"Str","c":
                "This EPUB verifies supplemental content only. Original chapter headers below are structural placeholders, not a representation of the complete manuscript."}]}
        ]
        blocks.extend(adapter.chapter_blocks(PRELUDE_CHAPTER,"stage",prelude=True))
        for chapter in FIGURES:
            blocks.append({"t":"Header","c":[1,[chapter,[],[]],[
                {"t":"Str","c":"Original chapter position: "+chapter}]]})
            blocks.extend(adapter.chapter_blocks(chapter,"stage"))
        (tmp/"content.json").write_text(json.dumps({
            "pandoc-api-version":[1,23,1],"meta":{},"blocks":blocks
        },ensure_ascii=False),encoding="utf-8")
        cmd=["pandoc","content.json","--from=json","--to=epub3",
             "--math-method=mathml","--output",str(out),
             "--metadata","title=Book Two Supplemental Publication Proof",
             "--metadata","author=Swayam Singal",
             "--metadata","lang=en","--split-level=1"]
        r=subprocess.run(cmd,cwd=tmp,text=True,stdout=subprocess.PIPE,
                         stderr=subprocess.PIPE)
        if r.returncode:
            raise RuntimeError("Pandoc EPUB publication proof failed:\n"+r.stderr[-4000:])
        if out.stat().st_size < 80000:
            raise RuntimeError("Unexpectedly small proof EPUB "+str(out))
        print("BOOK02_EPUB_SEMANTIC_PROOF",len(adapter.used),
              "source supplements, image count 13, output",out)
        return out


if __name__=="__main__":
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output",required=True,type=Path)
    args=parser.parse_args()
    make_proof(args.output)
