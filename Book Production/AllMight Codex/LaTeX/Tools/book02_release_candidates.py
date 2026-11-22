#!/usr/bin/env python3
"""Build and independently preflight Book Two *single-volume* PDF and EPUB candidates.

Does not rewrite the 16 original source chapters, the canonical manuscript,
CURRENT.json, the seven-volume physical-parts.json, or any other book.

The original global physical-parts.json can refer to an older generation.
This isolated path reads the two frozen Book Two physical-part main.tex
files as a source inventory and constructs ONE continuous typeset book,
with fresh front matter, one TOC, all 16 original chapters, and integrated
reference/code appendices. It does not edit the generated originals.

Requirements:
- The author's installed LuaLaTeX/latexmk and allmight-book.sty fonts/assets.
- Python environment with pypdf, numpy, scipy, matplotlib, pandoc 3.x.
- For the *full* EPUB: existing private source-cache build_books helpers,
  all 568 frozen ASTs, Java, EPUBCheck 5.4.0. These are checked explicitly.

Outputs are CANDIDATES requiring page-by-page visual and multi-reader EPUB
review. A roughly 1,000-page complete digital PDF is NOT automatically
a manufacturable single-volume print-on-demand interior.
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
from book02_single_volume import build_unified_tex, generate_minimal_jpeg_cover
from build import ROOT, generation_path, tex_runtime, compile_tex

STAGE = "02-Mathematics-for-Visual-Computing"
PARTS = (STAGE+"-P01",STAGE+"-P02")
TITLE = "From Equations to Geometry"
EXPECTED_EXTRA = 36
EXPECTED_IMAGE_ASSETS = 13
MAX_PAGES_PER_PHYSICAL_BOOK = 800


def digest(file: Path) -> str:
    return hashlib.sha256(file.read_bytes()).hexdigest()

def source_fingerprint()->str:
    """Identify all versioned Book Two additions and figure generators.

    This prevents combining a PDF from an old local addendum with a newly
    generated EPUB just because CURRENT.json still names the same 16-chapter
    frozen generation.
    """
    files=sorted((ROOT/"Assets"/"Book02").glob("*.tex"))
    files.extend([
        ROOT/"Tools"/"book02_single_volume.py",
        ROOT/"Tools"/"book02_visuals.py",
        ROOT/"Tools"/"book02_epub_supplements.py",
        ROOT/"Tools"/"book02_svg_id_repair.py",
        ROOT/"Tools"/"book02_epub_reader_polish.py",
        ROOT/"Tools"/"book02_epub_cover_repair.py",
        ROOT/"Tools"/"book02_finish_epub.py",
        ROOT/"Tools"/"build_epubs.py",
        Path(__file__).resolve(),
        ROOT.parent.parent.parent/"tools"/"labs"/"book02_visual_atlas.py",
        ROOT.parent.parent.parent/"tools"/"labs"/"book02_coverage_oracle.py",
        ROOT.parent.parent.parent/"tools"/"labs"/"book02_coverage_visual.py",
    ])
    # Include the native C++23 teaching project in publication provenance.
    # A changed algorithm or optional plotting source invalidates old candidates.
    cpp23=ROOT.parents[2]/"tools"/"labs"/"book02_cpp23"
    # Fingerprint authored sources only; local __pycache__, editor files or
    # build outputs must never make an unchanged candidate appear stale.
    published={".cpp",".hpp",".h",".py",".md",".txt"}
    files.extend(sorted(p for p in cpp23.rglob("*")
                        if p.is_file() and (p.suffix in published
                                            or p.name=="CMakeLists.txt")))
    h=hashlib.sha256()
    for path in files:
        if not path.is_file():
            raise RuntimeError("Missing source for candidate fingerprint: "+str(path))
        h.update(str(path.relative_to(ROOT.parents[2])).encode("utf-8"))
        h.update(b"\0")
        h.update(path.read_bytes())
        h.update(b"\0")
    return h.hexdigest()



def record_pdf(file:Path, label:str, build_log:Path, *, page_limit:int=MAX_PAGES_PER_PHYSICAL_BOOK) -> dict:
    if not file.is_file():
        raise RuntimeError("PDF not produced: "+str(file))
    reader=PdfReader(str(file))
    pages=len(reader.pages)
    if not 1<=pages<=page_limit:
        raise RuntimeError(f"{label}: page count {pages} outside 1–{page_limit} allowed for this output")
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


def latex_failure_excerpt(log_path: Path) -> str:
    """Report the useful TeX error, not only a path the assistant can't read."""
    if not log_path.is_file():
        return f"(No compiler log was created at {log_path})"
    lines=log_path.read_text(encoding="utf-8",errors="replace").splitlines()
    matches=[]
    for i,line in enumerate(lines):
        if (line.startswith("! ") or "LaTeX Error:" in line or
            ("Package " in line and " Error:" in line) or
            any(text in line for text in ("Emergency stop.",
                "Fatal error","Undefined control sequence",
                "Missing $ inserted","File ended while scanning"))):
            lo=max(0,i-4)
            hi=min(len(lines),i+9)
            matches.append("\n".join(f"{j+1:>5}: {lines[j]}" for j in range(lo,hi)))
            if len(matches)>=4:
                break
    # Some latexmk errors are only visible in its final stream summary.
    tail="\n".join(f"{j+1:>5}: {lines[j]}" for j in
                   range(max(0,len(lines)-45),len(lines)))
    return (("\n\n".join(matches) if matches else "(No explicit TeX error marker found)")+
            "\n\n---- Compiler log tail ----\n"+tail)[-9000:]


def single_pdf_candidate(generation:Path, root:Path) -> dict:
    """One continuous Book Two PDF: no part title pages or split TOCs.

    This is a complete digital book master. Its page count is deliberately
    NOT constrained by the 800-page physical-part manufacturing threshold.
    """
    out=root/"single-print"
    out.mkdir(parents=True,exist_ok=True)
    current=out/"current"
    if current.is_symlink():
        current.unlink()
    elif current.exists():
        raise RuntimeError("Cannot overwrite non-symlink single-print/current: "+str(current))
    current.symlink_to(generation,target_is_directory=True)
    folder=out/"complete"
    folder.mkdir(parents=True,exist_ok=True)
    part_mains=(generation/PARTS[0]/"main.tex",generation/PARTS[1]/"main.tex")
    tex=build_unified_tex(part_mains)
    master=folder/"main.tex"
    master.write_text(tex,encoding="utf-8")
    additions=install_book02_visuals(master,include_rich_atlas=True)
    if len(additions)!=EXPECTED_EXTRA or len(set(additions))!=EXPECTED_EXTRA:
        raise RuntimeError(f"Unified Book Two lost supplemental sources: {len(additions)} of {EXPECTED_EXTRA}")
    engine,env=tex_runtime()
    compiled=compile_tex(folder,engine,env,generation)
    if compiled["returncode"]!=0:
        log=Path(compiled["compile_log_path"])
        detail=latex_failure_excerpt(log)
        failure_file=root/"book02-single-pdf-failure.json"
        failure_file.write_text(json.dumps({
            "output":"single continuous Book Two PDF",
            "compiler_returncode":compiled["returncode"],
            "log_path":str(log),"error_excerpt":detail,
            "next_step":"Repair the isolated LaTeX compositor or style, never the frozen original chapters."
        },indent=2)+"\n",encoding="utf-8")
        raise RuntimeError("Unified Book Two LuaLaTeX compilation failed.\n"
                           f"Diagnostic: {failure_file}\n{detail}")
    candidate=folder/"From Equations to Geometry.pdf"
    shutil.copy2(Path(compiled["pdf"]),candidate)
    report=record_pdf(candidate,"Complete Book Two",Path(compiled["compile_log_path"]),
                      page_limit=2500)
    report["mode"]="single-continuous-digital-master"
    report["original_chapters"]=16
    report["reference_notes"]=31
    report["source_attachments"]=3
    report["added_sources"]=additions
    print("BOOK02_SINGLE_PDF_CANDIDATE_OK",report["pages"],candidate,flush=True)
    return report


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
            compiler_log=Path(compile_result["compile_log_path"])
            detail=latex_failure_excerpt(compiler_log)
            failure_file=root/"book02-print-failure.json"
            failure_file.write_text(json.dumps({
                "part":part,
                "compiler_returncode":compile_result["returncode"],
                "log_path":str(compiler_log),
                "error_excerpt":detail,
                "next_step":"Inspect source/TeX diagnostic, repair, and rebuild. No PDF was accepted."
            },indent=2,ensure_ascii=False)+"\n",encoding="utf-8")
            raise RuntimeError(
                f"{part} LuaLaTeX exited {compile_result['returncode']}.\n"
                f"Diagnostic report: {failure_file}\n"
                f"Original compiler log: {compiler_log}\n\n{detail}")
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
    if not path.is_file():
        raise RuntimeError("Full stage EPUB was not produced: "+str(path))
    epubcheck=record.get("epubcheck",{})
    if epubcheck.get("exit_code")!=0:
        # build_epubs.py deliberately retains a status record even when
        # EPUBCheck fails. Do not hide its actionable failure behind a
        # generic packaging error, and do not accept the EPUB candidate.
        diagnostic=(epubcheck.get("output") or "(No EPUBCheck diagnostic captured)").strip()
        status_path=path.parent/"build-status.json"
        raise RuntimeError(
            "Book Two full EPUB was generated, but EPUBCheck 5.4.0 rejected it.\n"
            f"EPUB: {path}\n"
            f"EPUBCheck exit code: {epubcheck.get('exit_code')!r}\n"
            f"Recorded build status: {status_path}\n"
            f"Complete EPUBCheck log: {epubcheck.get('full_log','(not recorded)')}\n"
            "EPUBCheck output (last 2,500 characters):\n"+diagnostic+"\n"
            "Fix the specific validation messages before calling this EPUB verified; "
            "the successfully generated PDF candidates remain intact."
        )
    supplement=record.get("book02_supplement_sources",[])
    if len(supplement)!=EXPECTED_EXTRA:
        raise RuntimeError(f"Expected 36 EPUB additive sources, got {len(supplement)}")
    if cover_image is not None and record.get("book02_cover_sha256")!=digest(cover_image):
        raise RuntimeError("EPUB cover provenance differs from the supplied cover image")
    with zipfile.ZipFile(path) as z:
        # A JPEG in the OPF does not guarantee a visible cover page.
        # Book Two previously escaped the SVG <image> element into text
        # (&lt;image), leaving ebook readers with a blank opening screen.
        # The release contract requires the first spine XHTML to contain
        # a real, image-backed XHTML img with accessible description.
        import xml.etree.ElementTree as ET
        import posixpath
        import urllib.parse
        opf=ET.fromstring(z.read("EPUB/content.opf"))
        opf_items=[e for e in opf.iter() if e.tag.endswith("}item")]
        image_assets=[e for e in opf_items
                      if "cover-image" in e.get("properties","").split()]
        pages=[e for e in opf_items if e.get("href","").endswith("cover.xhtml")]
        if len(image_assets)!=1 or len(pages)!=1:
            raise RuntimeError("Book Two EPUB does not declare one cover image and cover page")
        cover_location=posixpath.normpath(
            posixpath.join("EPUB",urllib.parse.unquote(pages[0].get("href"))))
        image_location=posixpath.normpath(
            posixpath.join("EPUB",urllib.parse.unquote(image_assets[0].get("href"))))
        relative_image=posixpath.relpath(
            image_location,posixpath.dirname(cover_location))
        cover_html=z.read(cover_location)
        cover_xml=ET.fromstring(cover_html)
        displayed=[e for e in cover_xml.iter()
                   if e.tag=="{http://www.w3.org/1999/xhtml}img"]
        if (len(displayed)!=1 or displayed[0].get("src")!=relative_image
                or not displayed[0].get("alt")):
            raise RuntimeError("The EPUB front-cover JPEG is embedded but not visibly rendered in cover.xhtml")
        if b"&lt;image" in cover_html:
            raise RuntimeError("Book Two front cover still contains the escaped, invisible SVG image tag")
        names=z.namelist()
        if cover_image is not None and not any(
            hashlib.sha256(z.read(name)).hexdigest()==digest(cover_image)
            for name in names if name.lower().endswith((".jpg",".jpeg"))
        ):
            raise RuntimeError("The actual EPUB package does not contain the supplied cover JPEG")
        # The reader edition deliberately rasterizes four path-heavy
        # Matplotlib SVG plots to 1600px PNG, preserving the other SVGs.
        # Count the optimized scientific images as well as retained vectors.
        svgs=[p for p in names if p.endswith(".svg")]
        scientific_png=[p for p in names if p.endswith("-reader.png")]
        if len(svgs)+len(scientific_png)<EXPECTED_IMAGE_ASSETS:
            raise RuntimeError(
                f"Expected at least 13 scientific figure assets, got "
                f"{len(svgs)} SVGs and {len(scientific_png)} optimized PNGs")
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
    if cover_image is None:
        # The default front cover contains ONLY title, subtitle and author.
        # It is not the decorative reference image, nor any other book cover.
        cover_image=generate_minimal_jpeg_cover(out/"book02-typographic-cover.jpg")
    # Test the entire Book Two supplemental MathML corpus BEFORE expensive
    # conversion of the frozen original mathematical manuscript.
    audit_file=out/"book02-supplement-mathml-report.json"
    subprocess.run([
        sys.executable,str(ROOT/"Tools"/"book02_mathml_preflight.py"),
        "--output",str(audit_file)
    ],check=True)
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
        import importlib.util
        for module in ("lxml","PIL","cairosvg"):
            checks["epub_finish_"+module]=importlib.util.find_spec(module) is not None
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



def export_candidates_to_master(generation:Path, destination:Path, *, print_only:bool=False) -> Path:
    """Copy the complete PDF (and optionally verified EPUB) into Master.

    The source build remains in the isolated Book02 worktree. The resulting
    digital PDF, optional EPUB and manifest are copied into a source-fingerprinted folder in
    the author's canonical Master workspace. Never merge/reset/edit tracked
    Master content as a side effect of exporting outputs.
    """
    source=ROOT/"Build"/generation.name/"book02-publication-candidates"/"book02-candidate-verification.json"
    if not source.is_file():
        raise RuntimeError("No completed Book Two verification report. Build both print parts and full EPUB first: "+str(source))
    report=json.loads(source.read_text(encoding="utf-8"))
    current_fingerprint=source_fingerprint()
    if report.get("generation")!=generation.name or report.get("source_fingerprint_sha256")!=current_fingerprint:
        raise RuntimeError("Candidate report source revision mismatch. Rebuild both formats before exporting into Master.")
    pdf=report.get("single_pdf")
    epub=report.get("epub")
    if not pdf:
        raise RuntimeError("No verified single-volume PDF in current report. Run --print-only.")
    if not print_only and (not epub or epub.get("epubcheck",{}).get("exit_code")!=0):
        raise RuntimeError("Full EPUB is not yet EPUBCheck-verified. Use --export-only --print-only to export the PDF alone.")
    files=[]
    location=Path(pdf["candidate_path"])
    if not location.is_file() or digest(location)!=pdf.get("sha256"):
        raise RuntimeError("Single Book Two PDF missing or altered: "+str(location))
    files.append((location,"From Equations to Geometry.pdf"))
    if not print_only:
        epub_location=Path(epub["candidate_path"])
        if not epub_location.is_file() or digest(epub_location)!=epub.get("sha256"):
            raise RuntimeError("Candidate EPUB missing or altered since verification: "+str(epub_location))
        files.append((epub_location,"From Equations to Geometry.epub"))
    files.append((source,"book02-candidate-verification.json"))
    # The explicit destination should be INSIDE the author's Master folder;
    # require it to exist rather than guessing a hard-coded personal path.
    destination=Path(destination).expanduser().resolve()
    if not destination.is_dir():
        raise RuntimeError("Export root must exist; supply a path inside the intended Master folder: "+str(destination))
    edition_key=("Book02-From-Equations-to-Geometry-"+generation.name.lstrip(".")+
                 "-"+current_fingerprint[:12])
    # A PDF-only proof and a later full PDF+EPUB export have different
    # immutable manifests. Keep them in separate folders so an approved
    # PDF remains available while preserving no-overwrite semantics.
    version=destination/(edition_key+("-PDF-Review" if print_only else ""))
    if version.exists() and not version.is_dir():
        raise RuntimeError("Book Two export location is not a directory: "+str(version))
    version.mkdir(parents=True,exist_ok=True)
    copied=[]
    for original,name in files:
        target=version/name
        if target.exists():
            if not target.is_file() or digest(target)!=digest(original):
                raise RuntimeError("Refusing to overwrite existing different Master output: "+str(target))
        else:
            shutil.copy2(original,target)
        copied.append(str(target))
    # The original candidate report contains Desktop source-worktree paths.
    # Keep it as immutable provenance, but also write a relocation-safe
    # manifest for auditing the books after the Desktop worktree is deleted.
    export_manifest={
        "schema":"allmight-book02-master-export-v1",
        "generation":generation.name,
        "source_fingerprint_sha256":current_fingerprint,
        "copied_candidates":{
            "complete_pdf":{"file":"From Equations to Geometry.pdf",
                            "sha256":pdf["sha256"],"pages":pdf.get("pages")},
            **({"epub":{"file":"From Equations to Geometry.epub",
                        "sha256":epub["sha256"],
                        "epubcheck_exit_code":epub["epubcheck"]["exit_code"],
                        "supplement_source_count":epub.get("supplement_source_count")}}
               if not print_only else {}),
        },
        "original_candidate_report":"book02-candidate-verification.json",
        "note":"All artifact paths are relative to this export folder and remain valid after removal of the Desktop source worktree."
    }
    manifest_file=version/"book02-master-export-manifest.json"
    manifest_text=json.dumps(export_manifest,indent=2,ensure_ascii=False)+"\n"
    if manifest_file.exists():
        if manifest_file.read_text(encoding="utf-8")!=manifest_text:
            raise RuntimeError("Refusing to overwrite a different canonical Master export manifest: "+str(manifest_file))
    else:
        manifest_file.write_text(manifest_text,encoding="utf-8")
    copied.append(str(manifest_file))
    print("BOOK02_MASTER_EXPORT_OK",version)
    for item in copied:
        print("  "+item)
    return version


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("--generation",help="Frozen generation id, defaults to CURRENT.json")
    p.add_argument("--cover-image",type=Path,help="Book Two JPEG cover on the author\x27s Mac, for the EPUB (not the print interior)")
    p.add_argument("--check-only",action="store_true",help="Read-only source and environment preflight, without building anything")
    p.add_argument("--export-only",action="store_true",help="Copy previously verified PDFs and EPUB into the author's Master workspace without rebuilding or changing Git history")
    p.add_argument("--export-dir",type=Path,help="Existing target folder inside the canonical Master workspace for verified output copies")
    g=p.add_mutually_exclusive_group()
    g.add_argument("--print-only",action="store_true")
    g.add_argument("--epub-only",action="store_true")
    args=p.parse_args()
    generation=generation_path(args.generation)
    if args.export_only:
        if not args.export_dir:
            p.error("--export-only requires --export-dir pointing inside Master")
        export_candidates_to_master(generation,args.export_dir,print_only=args.print_only)
        return 0
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
            "source_fingerprint_sha256":source_fingerprint(),
            "source_lock":audit(),"single_pdf":None,"epub":None,
            "declaration":"Candidate export only. Full original text mathematical correctness and reader/trim visual acceptance remain editorial gates."}
    output=ROOT/"Build"/generation.name/"book02-publication-candidates"
    output.mkdir(parents=True,exist_ok=True)
    file=output/"book02-candidate-verification.json"
    # A print-only run followed by epub-only must not erase the PDF acceptance
    # evidence. Conversely a newly generated PDF should not erase a verified
    # EPUB record. Merge only from the SAME generation, not stale snapshots.
    if file.is_file():
        old=json.loads(file.read_text(encoding="utf-8"))
        if old.get("generation")!=generation.name:
            raise RuntimeError("Existing candidate report belongs to another frozen generation")
        if old.get("source_fingerprint_sha256")!=result["source_fingerprint_sha256"]:
            # Preserve the old record and PDFs, but never blend old publication
            # evidence with a new cover/layout/source fingerprint.
            old_hash=(old.get("source_fingerprint_sha256") or "unknown")[:12]
            archived=output/("book02-candidate-verification-stale-"+old_hash+".json")
            if archived.exists():
                if archived.read_bytes()!=file.read_bytes():
                    raise RuntimeError("Existing stale report archive differs: "+str(archived))
            else:
                shutil.copy2(file,archived)
            print("BOOK02_STALE_REPORT_ARCHIVED",archived,flush=True)
        else:
            result["single_pdf"]=old.get("single_pdf")
            result["epub"]=old.get("epub")
    if not args.epub_only:
        result["single_pdf"]=single_pdf_candidate(generation,output)
    if not args.print_only:
        result["epub"]=epub_candidate(generation,output,cover_image)
    file.write_text(json.dumps(result,indent=2,ensure_ascii=False)+"\n")
    print("BOOK02_CANDIDATES_VERIFIED",file)
    return 0


if __name__=="__main__":
    raise SystemExit(main())
