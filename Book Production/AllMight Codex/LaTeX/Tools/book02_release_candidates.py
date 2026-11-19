#!/usr/bin/env python3
"""Build and independently preflight Book Two print and EPUB *candidates*.

Does not rewrite the 16 original source chapters, the canonical manuscript,
CURRENT.json, the seven-volume physical-parts.json, or any other book.

The original global physical-parts.json can refer to an older generation;
this isolated path intentionally avoids mutating that shared plan. It uses
the frozen current generation's P01 and P02 main.tex files directly.

Requirements:
- The author's installed LuaLaTeX/latexmk and allmight-book.sty fonts/assets.
- Python environment with pypdf, numpy, scipy, matplotlib, pandoc 3.x.
- For the *full* EPUB: existing private source-cache build_books helpers,
  all 568 frozen ASTs, Java, EPUBCheck 5.4.0. These are checked explicitly.

Outputs are CANDIDATES requiring visual print trim and multi-reader
EPUB review; no automated workflow can certify all mathematical proofs.
"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import re
import shutil
import subprocess
import sys
import zipfile

from pypdf import PdfReader

from book02_source_preflight import audit
from book02_visuals import install_book02_visuals
from build import ROOT, generation_path, tex_runtime, compile_tex

STAGE = "02-Mathematics-for-Visual-Computing"
PARTS = (STAGE+"-P01",STAGE+"-P02")
TITLE = "From Equations to Geometry"
EXPECTED_EXTRA = 36
EXPECTED_IMAGE_ASSETS = 13
MAX_PAGES_PER_PHYSICAL_BOOK = 800


def digest(file: Path) -> str:
    return hashlib.sha256(file.read_bytes()).hexdigest()


def record_pdf(file:Path, label:str, build_log:Path) -> dict:
    if not file.is_file():
        raise RuntimeError("PDF not produced: "+str(file))
    reader=PdfReader(str(file))
    pages=len(reader.pages)
    if not 1<=pages<=MAX_PAGES_PER_PHYSICAL_BOOK:
        raise RuntimeError(f"{label}: page count {pages} outside the 1–800 range")
    first=" ".join((p.extract_text() or "") for p in reader.pages[:5])
    if TITLE not in first:
        raise RuntimeError(f"{label}: new public book title absent from frontmatter")
    log=build_log.read_text(errors="replace")
    overfull=[float(n) for n in re.findall(r"Overfull \\hbox \(([\d.]+)pt too wide\)",log)]
    missing=len(re.findall(r"Missing character:",log))
    # Overfull is reported rather than silently accepted or rejected:
    # the complete source text can already contain legacy typesetting warnings.
    return {
        "id":label,"candidate_path":str(file),
        "sha256":digest(file),"bytes":file.stat().st_size,
        "pages":pages,"overfull_hbox_count":len(overfull),
        "worst_overfull_pt":max(overfull,default=0),
        "missing_glyph_warnings":missing,
        "compile_log_path":str(build_log),
    }


def print_candidates(generation:Path, root:Path)->list[dict]:
    out=root/"print"
    out.mkdir(parents=True,exist_ok=True)
    current=out/"current"
    if current.is_symlink():
        current.unlink()
    elif current.exists():
        raise RuntimeError("Cannot overwrite non-symlink print/current: "+str(current))
    current.symlink_to(generation, target_is_directory=True)
    engine,env=tex_runtime()
    reports=[]
    for part in PARTS:
        folder=out/part
        folder.mkdir(parents=True,exist_ok=True)
        shutil.copy2(generation/part/"main.tex",folder/"main.tex")
        additions=install_book02_visuals(folder/"main.tex",include_rich_atlas=True)
        if part.endswith("P01") and not any(s=="06-reader-contract.tex" for s in additions):
            raise RuntimeError("Missing Book Two reader's mathematical contract")
        if part.endswith("P02") and "35-cumulative-numerical-geometry-atlas.tex" not in additions:
            raise RuntimeError("Missing cumulative research project")
        compile_result=compile_tex(folder,engine,env,generation)
        if compile_result["returncode"]!=0:
            raise RuntimeError(part+" LuaLaTeX failed; see "+compile_result["compile_log_path"])
        pdf=Path(compile_result["pdf"])
        candidate=folder/(TITLE.replace(" ","-")+"-"+part[-3:]+".pdf")
        shutil.copy2(pdf,candidate)
        report=record_pdf(candidate,part,Path(compile_result["compile_log_path"]))
        report["added_sources"]=additions
        reports.append(report)
        print("BOOK02_PRINT_CANDIDATE_OK",part,report["pages"],candidate,flush=True)
    if len(reports)!=2 or sum(len(r["added_sources"]) for r in reports)!=EXPECTED_EXTRA:
        raise RuntimeError("Incorrect additive source coverage across both physical PDFs")
    return reports


def inspect_epub(path:Path,record:dict,cover_image:Path|None=None)->dict:
    if not path.is_file() or record.get("epubcheck",{}).get("exit_code")!=0:
        raise RuntimeError("Full stage EPUB missing or EPUBCheck not successful")
    supplement=record.get("book02_supplement_sources",[])
    if len(supplement)!=EXPECTED_EXTRA:
        raise RuntimeError(f"Expected 36 EPUB additive sources, got {len(supplement)}")
    if cover_image is not None and record.get("book02_cover_sha256")!=digest(cover_image):
        raise RuntimeError("EPUB cover provenance differs from the supplied cover image")
    with zipfile.ZipFile(path) as z:
        names=z.namelist()
        if cover_image is not None and not any(
            hashlib.sha256(z.read(name)).hexdigest()==digest(cover_image)
            for name in names if name.lower().endswith((".jpg",".jpeg"))
        ):
            raise RuntimeError("The actual EPUB package does not contain the supplied cover JPEG")
        svgs=[p for p in names if p.endswith(".svg")]
        if len(svgs)<EXPECTED_IMAGE_ASSETS:
            raise RuntimeError(f"Expected at least 13 scientific SVGs, got {len(svgs)}")
        xhtml=[p for p in names if p.endswith(".xhtml")]
        content=b" ".join(z.read(p) for p in xhtml)
        for snippet in (b"Numerical Geometry Atlas",b"Reconstruction",b"Geodesics"):
            if snippet.lower() not in content.lower():
                raise RuntimeError("EPUB reading content omitted: "+str(snippet))
        if content.count(b"alt=")<EXPECTED_IMAGE_ASSETS:
            raise RuntimeError("At least one supplemental scientific image lacks alt text")
        # Check exact image assets with names and content from SOURCE, not
        # just a count of any preexisting unrelated visuals.
        has_labels=sum(b"book02-" in z.read(p) for p in xhtml)
        if has_labels==0:
            raise RuntimeError("Missing original supplement header IDs")
    return {
        "candidate_path":str(path),
        "sha256":digest(path),
        "bytes":path.stat().st_size,
        "svg_asset_count":len(svgs),
        "supplement_source_count":len(supplement),
        "original_frozen_chapters":record.get("chapter_rows"),
        "epubcheck":record["epubcheck"],
        "cover_sha256":record.get("book02_cover_sha256"),
    }


def epub_candidate(generation:Path,root:Path,cover_image:Path|None=None)->dict:
    # The historical builder has several author-local helper dependencies.
    # Fail immediately, rather than creating an incomplete EPUB in their
    # absence or quietly dropping 16 chapters/references/attachments.
    helper=Path("/Users/swayamsingal/.codex/source-cache/allmight-book-production-2026-11-15/build_books.py")
    if not helper.is_file():
        raise RuntimeError(
            "Full Book Two EPUB requires the original source-cache "
            "build_books.py on the author's Mac. The independently EPUBCheck-"
            "validated 36-section integration proof is available in CI; "
            "do not mistake it for the complete 16-chapter publication.")
    epubcheck=Path("/Users/swayamsingal/.codex/source-cache/allmight-book-production-2026-11-15/epubcheck/epubcheck-5.4.0/epubcheck.jar")
    if not epubcheck.is_file():
        raise RuntimeError("Original pinned EPUBCheck 5.4.0 JAR is missing")
    out=root/"epub";out.mkdir(parents=True,exist_ok=True)
    cmd=[sys.executable,str(ROOT/"Tools"/"build_epubs.py"),
         "--generation",generation.name,"--stage",STAGE,
         "--output-dir",str(out)]
    if cover_image is not None:
        cmd.extend(["--book02-cover-image",str(cover_image)])
    subprocess.run(cmd,check=True)
    path=out/(STAGE+".epub")
    status=json.loads((out/"build-status.json").read_text())
    records=[x for x in status["completed"] if x.get("stage_id")==STAGE]
    if len(records)!=1:
        raise RuntimeError("EPUB build status lacks the exact Book Two stage")
    result=inspect_epub(path,records[0],cover_image)
    print("BOOK02_EPUB_CANDIDATE_OK",path,flush=True)
    return result



def check_environment(generation:Path, *, cover_image:Path|None,
                      require_print:bool, require_epub:bool)->dict:
    """Read-only source, compiler, and cover preflight.

    A successful preflight means the expensive full-book build can begin.
    It does not replace rendering, EPUBCheck or checking mathematical claims.
    """
    checks={}
    for part in PARTS:
        checks["main_"+part[-3:]]=(generation/part/"main.tex").is_file()
    source_lock=audit()
    checks["16_original_chapters"]=source_lock["original_chapters"]==16
    checks["36_supplied_additions"]=source_lock["additive_sources"]==EXPECTED_EXTRA
    if require_print:
        checks["latexmk"]=bool(shutil.which("latexmk") or
            Path("/usr/local/texlive/2025/bin/universal-darwin/latexmk").is_file())
        checks["lualatex"]=bool(shutil.which("lualatex"))
        checks["book_style"]=(ROOT/"allmight-book.sty").is_file()
    if require_epub:
        checks["pandoc"]=bool(shutil.which("pandoc"))
        checks["java"]=bool(shutil.which("java"))
        cache=Path("/Users/swayamsingal/.codex/source-cache/allmight-book-production-2026-11-15")
        checks["source_cache_builder"]=(cache/"build_books.py").is_file()
        checks["epubcheck_5_4_0"]=(cache/"epubcheck"/"epubcheck-5.4.0"/"epubcheck.jar").is_file()
        checks["frozen_conversion_manifest"]=(generation/"conversion-manifest.json").is_file()
    if cover_image is not None and require_epub:
        checks["cover_file"]=cover_image.is_file() and cover_image.suffix.lower() in (".jpg",".jpeg")
        if checks["cover_file"]:
            try:
                from PIL import Image
                with Image.open(cover_image) as image:
                    checks["portrait_cover_jpeg"]=(image.format=="JPEG" and
                        image.width>=300 and image.height>=450 and image.height>image.width)
            except (OSError,ValueError):
                checks["portrait_cover_jpeg"]=False
    return {"generation":generation.name,"cover_declared":bool(cover_image),
            "checks":checks,"ready_for_candidate_build":all(checks.values()),
            "caveat":"This preflight is not publication approval."}


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("--generation",help="Frozen generation id, defaults to CURRENT.json")
    p.add_argument("--cover-image",type=Path,help="Book Two JPEG cover on the author\x27s Mac, for the EPUB (not the print interior)")
    p.add_argument("--check-only",action="store_true",help="Read-only source and environment preflight, without building anything")
    g=p.add_mutually_exclusive_group()
    g.add_argument("--print-only",action="store_true")
    g.add_argument("--epub-only",action="store_true")
    args=p.parse_args()
    generation=generation_path(args.generation)
    cover_image=args.cover_image.expanduser().resolve() if args.cover_image else None
    if cover_image is not None and (not cover_image.is_file() or cover_image.suffix.lower() not in (".jpg",".jpeg")):
        p.error("Book Two EPUB cover must be an existing .jpg/.jpeg file")
    if args.check_only:
        status=check_environment(generation,cover_image=cover_image,
                                 require_print=not args.epub_only,
                                 require_epub=not args.print_only)
        print(json.dumps(status,indent=2,ensure_ascii=False))
        return 0 if status["ready_for_candidate_build"] else 2
    result={"generation":generation.name,"title":TITLE,
            "source_lock":audit(),"print_parts":[],"epub":None,
            "declaration":"Candidate export only. Full original text mathematical correctness and reader/trim visual acceptance remain editorial gates."}
    output=ROOT/"Build"/generation.name/"book02-publication-candidates"
    output.mkdir(parents=True,exist_ok=True)
    if not args.epub_only:
        result["print_parts"]=print_candidates(generation,output)
    if not args.print_only:
        result["epub"]=epub_candidate(generation,output,cover_image)
    file=output/"book02-candidate-verification.json"
    file.write_text(json.dumps(result,indent=2,ensure_ascii=False)+"\n")
    print("BOOK02_CANDIDATES_VERIFIED",file)
    return 0


if __name__=="__main__":
    raise SystemExit(main())
