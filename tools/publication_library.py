#!/usr/bin/env python3
"""Curate the seven-volume From Bits to Research *reader* publication library.

This tool NEVER deletes source material, rewrites manuscripts, builds a book,
uses git clean, or implicitly promotes historical stage exports. It scans
candidate artifacts, then deliberately promotes EXACTLY one selected complete
PDF/EPUB per book into a local Git-ignored distribution library.

Safety is intentional:
  scan                Non-mutating candidate inventory
  promote             Dry-run by default; copies specified complete files
  verify              Check current canonical releases and SHA manifest
  tidy                Dry-run by default; quarantine exact duplicate REVIEW copies
  quarantine          Dry-run by default; archive one explicitly chosen REVIEW file

A "promotion" is an output-organization action, NOT EPUBCheck, a copyright
approval, mathematical correctness or real-device release certification.
"""
from __future__ import annotations

import argparse
import datetime as dt
import hashlib
import json
import os
from pathlib import Path
import posixpath
import re
import shutil
import subprocess
import sys
import tempfile
import urllib.parse
import xml.etree.ElementTree as ET
import zipfile

MASTER = Path(__file__).resolve().parents[1]
LIBRARY_NAME = "From Bits to Research - Publications"
BOOKS = {
    1: ("From Bits to Parallel Systems", (
        "from bits to parallel systems", "bits programs and reliable systems", "book01", "book 01",
    )),
    2: ("From Equations to Geometry", (
        "from equations to geometry", "mathematics for visual computing", "book02", "book 02",
    )),
    3: ("From Geometry to Images", (
        "from geometry to images", "geometry cameras and image formation", "book03", "book 03",
    )),
    4: ("From Light to Realism", (
        "from light to realism", "light transport and image synthesis", "book04", "book 04",
    )),
    5: ("From Code to Living Worlds", (
        "from code to living worlds", "spatial worlds engine state and simulation", "book05", "book 05",
    )),
    6: ("From Frames to Real Time", (
        "from frames to real time", "gpu architecture and real time rendering", "book06", "book 06",
    )),
    7: ("From Perception to Discovery", (
        "from perception to discovery", "vision xr and playable worlds", "book07", "book 07",
    )),
}
PDF_MAGIC = b"%PDF-"
OPF_NS = "http://www.idpf.org/2007/opf"
DC_NS = "http://purl.org/dc/elements/1.1/"
PRUNE = {".git", ".obsidian", ".venv", ".venv-book01", "node_modules",
         "__pycache__", ".playwright-cli", ".pytest_cache", ".mypy_cache",
         ".ruff_cache", ".cache", "CMakeFiles", ".gradle"}
ARCHIVE = "Archive"
MANIFEST = "Manifests/publication-manifest.json"


def norm(value: str) -> str:
    return " ".join(re.findall(r"[a-z0-9]+", value.lower()))


def identify(path: Path) -> int | None:
    # Identify by filename, not a parent that could contain multiple books.
    name = norm(path.stem)
    for number, (title, aliases) in BOOKS.items():
        if any(alias in name for alias in aliases):
            return number
        if re.search(r"\b" + f"{number:02}" + r"\b", name):
            return number
    return None


def part_name(path: Path) -> bool:
    s = path.stem.lower()
    return bool(re.search(
        r"(?:^|[-_ ])p0[1-9]\b|\bpart[-_ ]?\d+\b|"
        r"\bvolume[-_ ]?(?:[ivx]+|\d+)\b", s))


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as source:
        for chunk in iter(lambda: source.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def check_epub(path: Path) -> dict:
    if not zipfile.is_zipfile(path):
        raise ValueError("Not a ZIP/EPUB: " + str(path))
    with zipfile.ZipFile(path) as z:
        names = z.namelist()
        if not names or names[0] != "mimetype":
            raise ValueError("EPUB mimetype entry not first")
        if z.getinfo("mimetype").compress_type != zipfile.ZIP_STORED:
            raise ValueError("EPUB mimetype compressed")
        if z.read("mimetype") != b"application/epub+zip":
            raise ValueError("Incorrect EPUB mimetype")
        bad = z.testzip()
        if bad is not None:
            raise ValueError("EPUB CRC failure: " + bad)
        container = ET.fromstring(z.read("META-INF/container.xml"))
        roots = [e.get("full-path") for e in container.iter()
                 if e.tag.rsplit("}", 1)[-1] == "rootfile"]
        if len(roots) != 1 or roots[0] not in names:
            raise ValueError("EPUB missing unique OPF rootfile")
        opf_path = roots[0]
        opf = ET.fromstring(z.read(opf_path))
        title = opf.find(f".//{{{DC_NS}}}title")
        book_title = title.text.strip() if title is not None and title.text else ""
        if not book_title:
            raise ValueError("EPUB missing book title metadata")
        manifest = opf.find(f"{{{OPF_NS}}}manifest")
        spine = opf.find(f"{{{OPF_NS}}}spine")
        if manifest is None or spine is None or not list(spine):
            raise ValueError("EPUB missing complete reading spine")
        items = {e.get("id"): e for e in manifest}
        for e in spine:
            if e.get("idref") not in items:
                raise ValueError("EPUB spine references undeclared resource")
        covers = [e for e in manifest
                  if "cover-image" in e.get("properties", "").split()]
        if len(covers) != 1:
            raise ValueError("EPUB should declare exactly one cover image")
        cover_path = posixpath.normpath(posixpath.join(
            posixpath.dirname(opf_path),
            urllib.parse.unquote(covers[0].get("href", ""))))
        if cover_path not in names:
            raise ValueError("EPUB cover-image entry is missing")
        return {"title": book_title, "spine_documents": len(list(spine)),
                "cover_image": cover_path, "zip_members": len(names)}


def check_pdf(path: Path) -> dict:
    with path.open("rb") as file:
        if file.read(5) != PDF_MAGIC:
            raise ValueError("Not a PDF file")
        if path.stat().st_size < 80_000:
            raise ValueError("PDF is implausibly small for a whole technical book")
        file.seek(max(0, path.stat().st_size - 4096))
        if b"%%EOF" not in file.read():
            raise ValueError("PDF trailer missing")
    return {"bytes": path.stat().st_size,
            "pages": "not assessed; visually inspect complete digital PDF"}


def validate_candidate(path: Path, number: int, fmt: str) -> dict:
    if not path.is_file() or path.is_symlink():
        raise ValueError("Candidate must be a regular file (not a symlink): " + str(path))
    if path.suffix.lower() != "." + fmt:
        raise ValueError("Candidate extension is not ." + fmt)
    if part_name(path):
        raise ValueError("Refusing a physical part as a complete single-book release: " + str(path))
    named = identify(path)
    if named is not None and named != number:
        raise ValueError(f"Candidate appears to be Book {named}, not Book {number}: {path}")
    details = check_epub(path) if fmt == "epub" else check_pdf(path)
    if fmt == "epub":
        available = norm(details["title"])
        expected = [norm(BOOKS[number][0]), *BOOKS[number][1]]
        if not any(term in available for term in expected):
            raise ValueError(
                f"EPUB dc:title {details['title']!r} does not match Book {number}. "
                "Do not silently relabel an unrelated EPUB.")
    return details


def canonical(number: int, fmt: str) -> str:
    return f"{number:02} - {BOOKS[number][0]}.{fmt}"


def layout(root: Path, create: bool) -> None:
    for dirname in ("EPUB", "PDF", ARCHIVE, "Manifests"):
        target = root / dirname
        if create:
            target.mkdir(parents=True, exist_ok=True)


def manifest_read(root: Path) -> dict:
    file = root / MANIFEST
    if not file.exists():
        return {"series": "FROM BITS TO RESEARCH", "schema_version": 1,
                "books": {f"{n:02}": {"title": info[0]} for n, info in BOOKS.items()}}
    data = json.loads(file.read_text(encoding="utf-8"))
    if data.get("series") != "FROM BITS TO RESEARCH" or data.get("schema_version") != 1:
        raise ValueError("Unknown publication manifest schema; refusing overwrite")
    return data


def manifest_write(root: Path, data: dict) -> None:
    target = root / MANIFEST
    target.parent.mkdir(parents=True, exist_ok=True)
    tmp = target.with_name(target.name + ".tmp")
    try:
        tmp.write_text(json.dumps(data, indent=2, ensure_ascii=False) + "\n",
                       encoding="utf-8")
        os.replace(tmp, target)
    finally:
        tmp.unlink(missing_ok=True)


def now() -> str:
    return dt.datetime.now(dt.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")


def archive_file(root: Path, file: Path, category: str, *, apply: bool) -> Path:
    if not file.is_file() or file.is_symlink():
        raise ValueError("Cannot archive missing/symlink file: " + str(file))
    token = sha256(file)[:12]
    archive = root / ARCHIVE / category
    archive.mkdir(parents=True, exist_ok=True) if apply else None
    dest = archive / f"{file.stem}--{token}{file.suffix}"
    if dest.exists():
        if sha256(dest) != sha256(file):
            raise ValueError("Archive-name collision with different content: " + str(dest))
        dest = archive / f"{file.stem}--{token}--{int(dt.datetime.now().timestamp())}{file.suffix}"
        if dest.exists():
            raise ValueError("Archive destination already exists: " + str(dest))
    if apply:
        shutil.move(str(file), str(dest))
    return dest


def is_tracked(master: Path, path: Path) -> bool:
    # No repo modifications, no checkout, no stash.
    try:
        relative = path.resolve().relative_to(master.resolve())
    except ValueError:
        return False
    if not (master / ".git").exists():
        return False
    p = subprocess.run(["git", "-C", str(master), "ls-files",
                        "--error-unmatch", "--", str(relative)],
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
                       check=False)
    return p.returncode == 0


def is_review_export(master: Path, source: Path) -> bool:
    try:
        rel = source.resolve().relative_to(master.resolve())
    except ValueError:
        return False
    if any(part in ("Build", "Generations", "Assets", "Continuous Notes", ".git")
           or part.startswith(".generation-") for part in rel.parts):
        return False
    return any(re.fullmatch(r"Book0?[1-7]-Review", part, re.IGNORECASE)
               for part in rel.parts[:-1])


def safe_archive_review(master: Path, root: Path, src: Path, category: str,
                        *, apply: bool) -> Path:
    if src.is_symlink() or not is_review_export(master, src):
        raise ValueError("Only original non-source BookNN-Review exports can be quarantined")
    if is_tracked(master, src):
        raise ValueError("Tracked source files cannot be quarantined")
    return archive_file(root, src, category, apply=apply)


def inventory(master: Path, root: Path, *, include_build: bool = False) -> dict:
    files = {f"{i:02}": {"epub": [], "pdf": []} for i in BOOKS}
    for base, directories, names in os.walk(master):
        current = Path(base)
        directories[:] = [d for d in directories
                          if d not in PRUNE
                          and not d.startswith(".generation-")
                          and d != LIBRARY_NAME
                          and (include_build or d not in ("Build", "Generations"))]
        if root in (current, *current.parents):
            directories[:] = []
            continue
        for name in names:
            path = current / name
            fmt = path.suffix.lower().lstrip(".")
            if fmt not in ("pdf", "epub"):
                continue
            book = identify(path)
            if book is None:
                continue
            try:
                rel = str(path.relative_to(master))
                bytes_size = path.stat().st_size
            except (OSError, ValueError):
                continue
            files[f"{book:02}"][fmt].append({
                "path": rel, "bytes": bytes_size,
                "physical_part_or_preview": part_name(path)
                or bool(re.search(r"preview|proof|sample|draft", norm(path.stem))),
                "source_category": (
                    "build output" if "Build" in path.parts or "Generations" in path.parts
                    else "legacy stage EPUB" if "Books" in path.parts and "Ebooks" in path.parts
                    else "review candidate" if "Review" in str(path.parent)
                    else "other"
                )
            })
    for entry in files.values():
        for fmt in ("epub", "pdf"):
            entry[fmt].sort(key=lambda x: x["path"].lower())
    return files


def scan(args: argparse.Namespace) -> None:
    data = inventory(args.master, args.library, include_build=args.include_build)
    print("MASTER:", args.master)
    print("PUBLICATIONS:", args.library)
    for number, (title, _) in BOOKS.items():
        row = data[f"{number:02}"]
        print(f"\nBOOK {number:02} — {title}")
        for fmt in ("epub", "pdf"):
            items = row[fmt]
            print(f"  {fmt.upper()}: {len(items)} possible source files")
            for item in items[:args.limit]:
                flag = " [PART/PREVIEW: NOT CANONICAL]" if item["physical_part_or_preview"] else ""
                print(f"    {item['path']} ({item['bytes'] // 1024} KiB, "
                      f"{item['source_category']}){flag}")
            if len(items) > args.limit:
                print(f"    ... {len(items)-args.limit} more (use --limit)")
    if args.json:
        target = args.json.expanduser()
        if target.exists() and not args.replace_report:
            raise ValueError("Inventory report already exists; use --replace-report")
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text(json.dumps(data, indent=2, ensure_ascii=False)+"\n",
                          encoding="utf-8")
        print("INVENTORY_JSON", target)


def promote(args: argparse.Namespace) -> None:
    number = args.book
    requested = {"epub": args.epub, "pdf": args.pdf}
    if not any(requested.values()):
        raise ValueError("Promote requires --epub PATH and/or --pdf PATH")
    validated = {}
    for fmt, path in requested.items():
        if path is None:
            continue
        resolved = path.expanduser().resolve()
        details = validate_candidate(resolved, number, fmt)
        dest = args.library / fmt.upper() / canonical(number, fmt)
        if resolved == dest.resolve():
            raise ValueError("Input is already the canonical target")
        new_hash = sha256(resolved)
        existing_hash = sha256(dest) if dest.exists() else None
        if dest.exists() and dest.is_symlink():
            raise ValueError("Refusing to overwrite symlinked target")
        if existing_hash and existing_hash != new_hash and not args.replace:
            raise ValueError("Canonical target has DIFFERENT bytes; "
                             "add --replace to archive the previous version: " + str(dest))
        if args.move_source and not is_review_export(args.master, resolved):
            raise ValueError("--move-source only supports non-source BookNN-Review files")
        if args.move_source and is_tracked(args.master, resolved):
            raise ValueError("--move-source is forbidden for Git-tracked inputs")
        validated[fmt] = (resolved, dest, details, new_hash, existing_hash)
    for fmt, (src, dest, details, new_hash, existing_hash) in validated.items():
        action = "UNCHANGED" if existing_hash == new_hash else ("REPLACE" if existing_hash else "CREATE")
        print(f"{'APPLY' if args.apply else 'DRY-RUN'} {action} Book {number:02} {fmt.upper()}")
        print("  source:", src)
        print("  target:", dest)
        print("  sha256:", new_hash)
        print("  structural validation:", details)
        if args.move_source:
            print("  original review copy:", "MOVED INTO ARCHIVE after verification"
                  if args.apply else "WOULD BE ARCHIVED after verification")
    if not args.apply:
        print("No files changed. Repeat with --apply after inspecting the paths.")
        return

    layout(args.library, create=True)
    manifest = manifest_read(args.library)
    for fmt, (src, dest, details, new_hash, existing_hash) in validated.items():
        if existing_hash != new_hash:
            with tempfile.NamedTemporaryFile(prefix=dest.name+".staging-",
                    suffix=".tmp", dir=dest.parent, delete=False) as f:
                staging = Path(f.name)
            backup: Path | None = None
            try:
                shutil.copy2(src, staging)
                if sha256(staging) != new_hash:
                    raise RuntimeError("Copy SHA-256 mismatch before promotion")
                if existing_hash:
                    backup = archive_file(args.library, dest,
                                          f"{fmt.upper()}/Superseded", apply=True)
                try:
                    os.replace(staging, dest)
                except Exception:
                    if backup and not dest.exists() and backup.is_file():
                        os.replace(backup, dest)
                    raise
                if sha256(dest) != new_hash:
                    raise RuntimeError("Canonical SHA-256 mismatch after promotion")
            finally:
                staging.unlink(missing_ok=True)
        record = manifest["books"].setdefault(f"{number:02}", {"title": BOOKS[number][0]})
        record[fmt] = {
            "file": str(dest.relative_to(args.library)),
            "sha256": new_hash, "bytes": dest.stat().st_size,
            "source": str(src), "promoted_utc": now(),
            "quality_state": "candidate — independent EPUBCheck and reader/visual QA not certified here",
            "structural_check": details,
        }
        manifest_write(args.library, manifest)
        if args.move_source:
            # Move source review copy to Archive, not a one-way deletion.
            archive = safe_archive_review(
                args.master, args.library, src,
                f"{fmt.upper()}/Original Review Copies", apply=True)
            print("SOURCE_REVIEW_ARCHIVED", archive)
        print("CANONICAL_PUBLICATION", dest)
    verify(args)


def verify(args: argparse.Namespace) -> None:
    root = args.library
    layout(root, create=False)
    recorded = manifest_read(root)
    errors = []
    found = 0
    for number, (title, _) in BOOKS.items():
        metadata = recorded["books"].get(f"{number:02}", {})
        for fmt in ("epub", "pdf"):
            dirpath = root / fmt.upper()
            name = canonical(number, fmt)
            path = dirpath / name
            if not path.exists():
                if metadata.get(fmt):
                    errors.append(f"Book {number:02} {fmt.upper()} missing canonical file")
                continue
            try:
                if path.is_symlink():
                    raise ValueError("Canonical item is a symlink")
                details = validate_candidate(path, number, fmt)
                actual = sha256(path)
                reported = metadata.get(fmt, {}).get("sha256")
                if not reported:
                    errors.append(f"Book {number:02} {fmt.upper()} missing manifest provenance")
                elif reported != actual:
                    errors.append(f"Book {number:02} {fmt.upper()} manifest SHA does not match")
                found += 1
                print("VERIFIED", fmt.upper(), name, "sha256", actual[:20], details)
            except Exception as exc:
                errors.append(f"Book {number:02} {fmt.upper()} validation failed: {exc}")
    allowed = {canonical(n, fmt) for n in BOOKS for fmt in ("pdf", "epub")}
    for fmt in ("epub", "pdf"):
        folder = root / fmt.upper()
        if not folder.exists():
            continue
        for file in folder.iterdir():
            if file.is_file() and file.suffix.lower() == "."+fmt and file.name not in allowed:
                errors.append("Noncanonical release placed in canonical folder: "+str(file))
    if errors:
        print("VERIFICATION_FAILED\n" + "\n".join(errors), file=sys.stderr)
        raise RuntimeError(f"{len(errors)} publication library integrity errors")
    print("PUBLICATION_LIBRARY_OK", found, "candidate files;",
          "no claim of official EPUBCheck or visual acceptance")


def tidy(args: argparse.Namespace) -> None:
    """Quarantine only EXACT duplicate review copies; preserve all other files."""
    root = args.library
    current = []
    for number in BOOKS:
        for fmt in ("epub", "pdf"):
            target = root / fmt.upper() / canonical(number, fmt)
            if target.is_file():
                current.append((number, fmt, target, sha256(target)))
    candidates = []
    for entry in args.master.iterdir():
        if not entry.is_dir() or not re.fullmatch(r"Book0?[1-7]-Review", entry.name, re.I):
            continue
        for src in entry.rglob("*"):
            if not src.is_file() or src.is_symlink() or src.suffix.lower() not in (".epub", ".pdf"):
                continue
            fmt = src.suffix.lower()[1:]
            matches = [(n, target) for n, f, target, digest in current
                       if f == fmt and sha256(src) == digest]
            if not matches or is_tracked(args.master, src):
                continue
            candidates.append((src, matches[0][0], fmt))
    if not candidates:
        print("NO_EXACT_DUPLICATE_REVIEW_EXPORTS_FOUND")
        return
    for source, number, fmt in candidates:
        dest = safe_archive_review(
            args.master, root, source,
            f"{fmt.upper()}/Duplicate Review Copies", apply=args.apply)
        print(("QUARANTINED" if args.apply else "WOULD_QUARANTINE"),
              f"Book {number:02}", source, "->", dest)
    print("TIDY_COMPLETE",len(candidates),
          "exact-byte duplicates", "quarantined" if args.apply else "dry-run only",
          "; no source/build/generated artifacts deleted")


def quarantine(args: argparse.Namespace) -> None:
    source = args.file.expanduser().resolve()
    if source.suffix.lower() not in (".epub", ".pdf"):
        raise ValueError("Only individual EPUB/PDF review outputs can be quarantined")
    dest = safe_archive_review(
        args.master, args.library, source,
        source.suffix.lower()[1:].upper()+"/Superseded Review Editions",
        apply=args.apply)
    print("QUARANTINED" if args.apply else "WOULD_QUARANTINE",source,"->",dest)
    if not args.apply:
        print("No files changed; add --apply when this specific file is safe to archive")


def main() -> int:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--master", type=Path, default=MASTER,
                   help="Master repository root (default: script's parent repository)")
    p.add_argument("--library", type=Path,
                   help="Publication output root (default: Master / From Bits to Research - Publications)")
    sub = p.add_subparsers(dest="action", required=True)
    s = sub.add_parser("scan", help="Read-only inventory: never auto-promote legacy exports")
    s.add_argument("--include-build", action="store_true",
                   help="Also search generated Build candidates, without modifying them")
    s.add_argument("--limit", type=int, default=10)
    s.add_argument("--json", type=Path)
    s.add_argument("--replace-report", action="store_true")
    t = sub.add_parser("promote", help="Promote explicitly selected whole-book files")
    t.add_argument("--book", type=int, choices=BOOKS.keys(), required=True)
    t.add_argument("--epub", type=Path)
    t.add_argument("--pdf", type=Path)
    t.add_argument("--replace", action="store_true",
                   help="Archive differing existing canonical file before replacing it")
    t.add_argument("--move-source", action="store_true",
                   help="Archive original UNTRACKED BookNN-Review input after successful copy")
    t.add_argument("--apply", action="store_true",
                   help="Actually write; omission means safe dry-run")
    sub.add_parser("verify", help="Verify canonical distribution files and SHA manifest")
    t = sub.add_parser("tidy", help="Quarantine only exact-byte duplicate untracked review exports")
    t.add_argument("--apply", action="store_true",
                   help="Actually quarantine; omission means dry-run")
    q = sub.add_parser("quarantine", help="Archive one named outdated untracked REVIEW file")
    q.add_argument("--file", type=Path, required=True)
    q.add_argument("--apply", action="store_true")
    args = p.parse_args()
    args.master = args.master.expanduser().resolve()
    args.library = (args.library or args.master / LIBRARY_NAME).expanduser().resolve()
    if not args.master.is_dir():
        p.error("Master root does not exist: "+str(args.master))
    if args.library == args.master:
        p.error("Library cannot overwrite repository root")
    try:
        if args.action == "scan":
            scan(args)
        elif args.action == "promote":
            promote(args)
        elif args.action == "verify":
            verify(args)
        elif args.action == "tidy":
            tidy(args)
        elif args.action == "quarantine":
            quarantine(args)
    except (OSError, ValueError, RuntimeError, zipfile.BadZipFile, ET.ParseError) as err:
        print("PUBLICATION_LIBRARY_ERROR:",err,file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
