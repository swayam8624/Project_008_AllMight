#!/usr/bin/env python3
"""One-command, non-destructive Book One release on the author's Mac.

From Master (after pulling the branch), run:
    .venv-book01/bin/python tools/book01_release_once.py

The driver automatically:
  1. Identifies the original Book One cover (or accepts --cover PATH).
  2. Checks editor/provenance and the actual Pandoc PDF+EPUB visual pipeline.
  3. Prepares and verifies all 16 chapters, 18 lessons, diagrams and TeX.
  4. Compiles ONE complete PDF, with the supplied cover, no KDP page ceiling.
  5. Builds and structurally verifies the reflowable EPUB, running EPUBCheck
     too when installed on the machine.
  6. Publishes PDF and EPUB in Master/build, writes a checksum/proof manifest
     under Master/build/book01-expanded-reader, and opens final documents.

The original continuous manuscript, all Git changes, the other books and
previous releases are not removed, rewritten, or reset. Failed builds stop
immediately and preserve a detailed local log under the generated build tree.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
from datetime import datetime, timezone
import zipfile

ROOT = Path(__file__).resolve().parent.parent
OUTPUT = ROOT / "build" / "book01-expanded-reader"
RELEASE = ROOT / "build"
NOTE = ROOT / "Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md"
BOOK_BUILDER = ROOT / "tools" / "build_book01_expanded_reader.py"
EBOOK_BUILDER = ROOT / "tools" / "build_book01_kdp_epub.py"
PDF = RELEASE / "Book01-From-Bits-to-Parallel-Systems-COMPLETE.pdf"
EPUB = RELEASE / "Book01-From-Bits-to-Parallel-Systems-EBOOK.epub"
PROOF = OUTPUT / "book01-complete" / "build" / "publication-proof.json"
LOG = OUTPUT / "book01-release-once.log"
RECEIPT = OUTPUT / "book01-release-once.json"


def sha256(path: Path) -> str:
    with path.open("rb") as stream:
        return hashlib.file_digest(stream, "sha256").hexdigest()


def qualified_cover(path: Path) -> bool:
    if not path.is_file() or path.suffix.casefold() not in (".png", ".jpg", ".jpeg"):
        return False
    try:
        from PIL import Image
        with Image.open(path) as image:
            width, height = image.size
            return width >= 800 and height >= 1200 and 0.56 <= width / height <= 0.75
    except (OSError, ImportError, ValueError):
        return False


def find_cover(explicit: Path | None) -> Path:
    if explicit is not None:
        candidate = explicit.expanduser().resolve()
        if not qualified_cover(candidate):
            raise RuntimeError(
                "Cover must be an existing, readable portrait PNG/JPEG "
                "(minimum 800x1200): " + str(candidate)
            )
        return candidate

    home = Path.home()
    search_dirs = [
        RELEASE, home / "Downloads", home / "Pictures",
        ROOT / "Book Production/Seven-Book Curriculum/Book01",
        home / "Desktop",  # read-only discovery; never write or stage here
    ]
    candidates: list[tuple[int, Path]] = []
    for directory in search_dirs:
        if not directory.is_dir():
            continue
        # Deliberately shallow: do not walk the giant iCloud Obsidian vault.
        for path in directory.iterdir():
            if not path.is_file():
                continue
            name = path.stem.casefold().replace("_", " ").replace("-", " ")
            if not ("bits" in name and ("parallel" in name or "book01" in name)):
                continue
            if not qualified_cover(path):
                continue
            score = (
                10 * ("from bits to parallel systems" in name)
                + 5 * ("cover" in name)
                + 2 * ("book01" in name)
            )
            candidates.append((score, path.resolve()))
    if not candidates:
        raise RuntimeError(
            "Book One cover not found. Save the original PNG/JPEG as "
            "Master/build/Book01-cover.png, or pass --cover /absolute/path/to/cover.png. "
            "No arbitrary image or PDF title substitute will be used."
        )
    candidates.sort(key=lambda pair: (-pair[0], str(pair[1])))
    best = [path for score, path in candidates if score == candidates[0][0]]
    # The same image in two places is safe. Different plausible covers are not.
    hashes = {sha256(path) for path in best}
    if len(hashes) > 1:
        raise RuntimeError(
            "Multiple distinct, equally plausible Book One covers found; "
            "choose the intended art with --cover PATH:\n" +
            "\n".join("  " + str(path) for path in best)
        )
    return best[0]


def tool_environment() -> dict[str, str]:
    env = os.environ.copy()
    path_dirs = ["/Library/TeX/texbin", "/opt/homebrew/bin", "/usr/local/bin"]
    brew = shutil.which("brew")
    if brew:
        probe = subprocess.run([brew, "--prefix", "node"], capture_output=True, text=True)
        if probe.returncode == 0 and probe.stdout.strip():
            path_dirs.insert(0, str(Path(probe.stdout.strip()) / "bin"))
    env["PATH"] = os.pathsep.join(path_dirs + [env.get("PATH", "")])
    env["PYTHONUNBUFFERED"] = "1"
    return env


def require_tools(env: dict[str, str]) -> None:
    for executable in ("pandoc", "lualatex", "mmdc"):
        if not shutil.which(executable, path=env["PATH"]):
            raise RuntimeError(
                "Required Mac publishing tool not found: " + executable +
                ". Existing generated files were not changed."
            )
    for module in ("fitz", "PIL", "pypdf"):
        try:
            __import__(module)
        except ImportError as exc:
            raise RuntimeError(
                "Missing Python dependency " + module + " in the selected "
                "Book One venv. No build was started."
            ) from exc


def run_step(label: str, command: list[str], log_file, env: dict[str, str]) -> None:
    banner = "\n=== " + label + " ===\n"
    print(banner, flush=True)
    log_file.write(banner + "$ " + " ".join(command) + "\n")
    log_file.flush()
    with subprocess.Popen(
        command, cwd=ROOT, env=env, stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT, text=True, errors="replace", bufsize=1
    ) as process:
        assert process.stdout is not None
        for line in process.stdout:
            print(line, end="", flush=True)
            log_file.write(line)
        status = process.wait()
    log_file.flush()
    if status:
        raise RuntimeError(
            label + " failed (exit " + str(status) + "). "
            "Full diagnostics saved to " + str(LOG)
        )


def verify_results(cover: Path, source_digest: str, epubcheck: bool) -> dict:
    if sha256(NOTE) != source_digest:
        raise RuntimeError("Original manuscript checksum changed unexpectedly; stop.")

    if not PDF.is_file() or PDF.stat().st_size < 100_000:
        raise RuntimeError("Complete digital PDF is missing or unexpectedly small.")
    with PDF.open("rb") as stream:
        if stream.read(5) != b"%PDF-":
            raise RuntimeError("Complete PDF lacks a valid PDF signature.")
    if not EPUB.is_file() or not zipfile.is_zipfile(EPUB):
        raise RuntimeError("Final EPUB is missing or not a ZIP/EPUB file.")
    with zipfile.ZipFile(EPUB) as archive:
        if archive.testzip() is not None:
            raise RuntimeError("EPUB ZIP contains a corrupt member.")
        if archive.namelist()[0] != "mimetype" or archive.read("mimetype") != b"application/epub+zip":
            raise RuntimeError("EPUB container/mimetype structure invalid.")

    if not PROOF.is_file():
        raise RuntimeError("Missing complete-PDF publication geometry/figure proof.")
    proof = json.loads(PROOF.read_text(encoding="utf-8"))
    if proof.get("fatal_count") != 0:
        raise RuntimeError("Complete PDF publication proof contains fatal errors.")

    import fitz
    with fitz.open(PDF) as pdf_document:
        page_count = len(pdf_document)
        title = (pdf_document.metadata or {}).get("title", "")
        if page_count < 100 or not title:
            raise RuntimeError("Book PDF has too few pages or missing title metadata.")

    receipt = {
        "status": "complete-digital-candidates",
        "created_utc": datetime.now(timezone.utc).isoformat(),
        "source_sha256_before_after": source_digest,
        "cover": str(cover),
        "cover_sha256": sha256(cover),
        "pdf": {"file": str(PDF), "sha256": sha256(PDF), "pages": page_count},
        "epub": {"file": str(EPUB), "sha256": sha256(EPUB), "bytes": EPUB.stat().st_size},
        "pdf_geometry_proof": str(PROOF),
        "epubcheck_ran": epubcheck,
        "note": "Source/technical proof only; actual target-reader visual inspection is still necessary.",
    }
    RECEIPT.write_text(json.dumps(receipt, indent=2) + "\n", encoding="utf-8")
    return receipt


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cover", type=Path, help="Exact author-approved Book One JPG/PNG cover; auto-detected if omitted.")
    parser.add_argument("--no-open", action="store_true", help="Do not open the final PDF/EPUB.")
    args = parser.parse_args()

    if not NOTE.is_file() or not BOOK_BUILDER.is_file() or not EBOOK_BUILDER.is_file():
        print("ERROR: Run from the checked-out AllMight Master repository.", file=sys.stderr)
        return 1

    try:
        cover = find_cover(args.cover)
        env = tool_environment()
        require_tools(env)
        original_hash = sha256(NOTE)
        OUTPUT.mkdir(parents=True, exist_ok=True)
        RELEASE.mkdir(parents=True, exist_ok=True)
        py = sys.executable
        with LOG.open("w", encoding="utf-8") as log:
            print("Using cover:", cover)
            print("PDF and EPUB will be published directly in:", RELEASE)
            print("Manuscript checksum:", original_hash)
            print("Full run log:", LOG)
            run_step("Check PDF and EPUB presentation", [py, "tools/test_book01_visual_language.py"], log, env)
            run_step("Test figures/math preservation", [py, "tools/test_book01_render_support.py"], log, env)
            run_step("Test reader editorial transformations", [py, "tools/test_book01_publication_edit.py"], log, env)
            run_step("Test PDF proof rules", [py, "tools/test_book01_pdf_proof.py"], log, env)
            run_step("Audit 16 chapters and 18 additions", [py, str(BOOK_BUILDER), "--check"], log, env)
            run_step("Prepare original complete book", [py, str(BOOK_BUILDER), "--prepare"], log, env)
            run_step("Verify every prepared source", [py, str(BOOK_BUILDER), "--verify-prepared"], log, env)
            if sha256(NOTE) != original_hash:
                raise RuntimeError("Manuscript checksum changed during preparation.")
            run_step("Compile ONE continuous PDF with the actual cover",
                     [py, str(BOOK_BUILDER), "--compile", "--cover", str(cover)], log, env)

            # Validate EPUB before placing it in the final release location.
            staging = OUTPUT / "epub-preflight" / EPUB.name
            staging.parent.mkdir(parents=True, exist_ok=True)
            run_step("Build and structurally validate complete EPUB",
                     [py, str(EBOOK_BUILDER), "--cover", str(cover), "--output", str(staging)], log, env)

            epubcheck = shutil.which("epubcheck", path=env["PATH"])
            epubcheck_ran = bool(epubcheck)
            if epubcheck:
                run_step("EPUBCheck formal validation", [epubcheck, str(staging)], log, env)
            else:
                warning = "EPUBCheck not installed: EPUB structural validation passed, formal EPUBCheck remains outstanding."
                print("WARNING:", warning)
                log.write("WARNING: " + warning + "\n")

            # atomic rename, staging and final destination are on same filesystem.
            if not zipfile.is_zipfile(staging):
                raise RuntimeError("Staged EPUB failed ZIP integrity validation.")
            os.replace(staging, EPUB)
            receipt = verify_results(cover, original_hash, epubcheck_ran)
            print("\nSUCCESS — complete reader files:\n  " + str(PDF) + "\n  " + str(EPUB))
            print("PDF pages:", receipt["pdf"]["pages"])
            print("SHA-256 verification report:", RECEIPT)
            log.write("\nSUCCESS " + str(RECEIPT) + "\n")
        if not args.no_open and sys.platform == "darwin":
            opener = shutil.which("open")
            if opener:
                subprocess.run([opener, "-a", "Preview", str(PDF)], check=False)
                subprocess.run([opener, "-a", "Books", str(EPUB)], check=False)
                subprocess.run([opener, str(RELEASE)], check=False)
        return 0
    except (OSError, RuntimeError, ValueError, ImportError) as exc:
        print("\nBOOK ONE RELEASE STOPPED: " + str(exc), file=sys.stderr)
        print("The manuscript and Book Two were not deliberately modified. Check the log for which Book One outputs succeeded.", file=sys.stderr)
        if LOG.exists():
            print("Full log:", LOG, file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
