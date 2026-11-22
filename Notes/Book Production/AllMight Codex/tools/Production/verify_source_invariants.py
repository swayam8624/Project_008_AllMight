#!/usr/bin/env python3
"""Read-only checks for promoted chapter source, AST, code, and XML invariants."""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import re
import sys
import xml.etree.ElementTree as ET


def sha(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def canonical_ast_sha(value: object) -> str:
    data = json.dumps(value, sort_keys=True, ensure_ascii=False, separators=(",", ":")).encode()
    return hashlib.sha256(data).hexdigest()


def walk(value):
    if isinstance(value, dict):
        yield value
        for child in value.values():
            yield from walk(child)
    elif isinstance(value, list):
        for child in value:
            yield from walk(child)


def is_mermaid(block: dict) -> bool:
    content = block.get("c")
    if not isinstance(content, list) or len(content) < 2 or not isinstance(content[0], list):
        return False
    attr = content[0]
    classes = attr[1] if len(attr) > 1 and isinstance(attr[1], list) else []
    return any("mermaid" in str(name).lower() for name in classes)


def code_blocks(ast: dict, *, mermaid: bool | None = None) -> list[str]:
    found = []
    for node in walk(ast.get("blocks", [])):
        if node.get("t") != "CodeBlock":
            continue
        is_diagram = is_mermaid(node)
        if mermaid is not None and is_diagram != mermaid:
            continue
        content = node.get("c")
        if isinstance(content, list) and len(content) > 1 and isinstance(content[1], str):
            found.append(content[1])
        else:
            found.append("__MALFORMED_CODE_BLOCK__")
    return found


def inline_code(ast: dict) -> list[str]:
    found = []
    for node in walk(ast.get("blocks", [])):
        if node.get("t") != "Code":
            continue
        content = node.get("c")
        if isinstance(content, list) and len(content) > 1 and isinstance(content[1], str):
            found.append(content[1])
        else:
            found.append("__MALFORMED_INLINE_CODE__")
    return found


def normalize(value: str) -> str:
    return value.replace("\r\n", "\n").replace("\r", "\n")


def mermaid_job_id(code: str) -> str:
    payload = json.dumps({"code": code, "kind": "mermaid", "version": 1}, sort_keys=True, ensure_ascii=False)
    return hashlib.sha256(payload.encode()).hexdigest()[:24]


def main() -> int:
    here = Path(__file__).resolve()
    default_root = here.parents[2]
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=default_root, help="production root")
    parser.add_argument("--report", type=Path, help="optional JSON report destination")
    args = parser.parse_args()
    root = args.root.resolve()
    master_root = root.parent.parent
    manifest = json.loads((root / "production-manifest.json").read_text())
    jobs = json.loads((root / "jobs.json").read_text())
    jobs_by_id = {row.get("id"): row for row in jobs.get("jobs", []) if isinstance(row, dict)}
    errors = []
    volumes = []
    mermaid_job_ids = set()
    aggregate = {"source_hashes": 0, "raw_ast_canonical_hashes": 0, "original_raw_ast_hashes": 0,
                 "reader_ast_hashes": 0, "reader_source_bindings": 0, "xml_parses": 0,
                 "fenced_code_blocks": 0, "inline_code_spans": 0, "mermaid_occurrences": 0}
    chapter_count = 0

    for volume in manifest.get("volumes", []):
        chapter_rows = volume.get("chapters", [])
        volumes.append({"id": volume.get("id"), "chapter_count": len(chapter_rows)})
        for chapter in chapter_rows:
            chapter_count += 1
            cid = chapter.get("id", "unknown")
            source = Path(chapter.get("source", ""))
            raw_path = Path(chapter.get("raw_source_ast", ""))
            original_path = Path(chapter.get("original_raw_ast", ""))
            reader_path = Path(chapter.get("normalized_reader_ast", ""))
            xml_path = Path(chapter.get("xml", ""))
            checks = {}
            try:
                checks["source_sha256"] = source.is_file() and sha(source) == chapter.get("source_sha256")
                if checks["source_sha256"]:
                    aggregate["source_hashes"] += 1
                raw = json.loads(raw_path.read_text())
                checks["raw_ast_canonical_sha256"] = canonical_ast_sha(raw) == chapter.get("raw_ast_sha256")
                if checks["raw_ast_canonical_sha256"]:
                    aggregate["raw_ast_canonical_hashes"] += 1
                checks["original_raw_ast_sha256"] = original_path.is_file() and sha(original_path) == chapter.get("original_raw_ast_sha256")
                if checks["original_raw_ast_sha256"]:
                    aggregate["original_raw_ast_hashes"] += 1
                reader = json.loads(reader_path.read_text())
                checks["normalized_reader_ast_sha256"] = sha(reader_path) == chapter.get("normalized_reader_ast_sha256")
                if checks["normalized_reader_ast_sha256"]:
                    aggregate["reader_ast_hashes"] += 1
                checks["normalized_reader_source_sha256"] = chapter.get("normalized_reader_source_sha256") == chapter.get("source_sha256")
                if checks["normalized_reader_source_sha256"]:
                    aggregate["reader_source_bindings"] += 1
                xml_root = ET.parse(xml_path).getroot()
                checks["xml_well_formed"] = True
                aggregate["xml_parses"] += 1
                blocks = reader.get("blocks", [])
                reader_fenced = [normalize(x) for x in code_blocks(reader, mermaid=False)]
                reader_inline = [normalize(x) for x in inline_code(reader)]
                xml_fenced = [normalize("".join(el.itertext())).removesuffix("\n") for el in xml_root.iter() if el.tag.rsplit("}", 1)[-1] == "Code"]
                xml_inline = [normalize("".join(el.itertext())) for el in xml_root.iter() if el.tag.rsplit("}", 1)[-1] == "InlineCode"]
                checks["fenced_code_bodies_match_xml"] = reader_fenced == xml_fenced
                checks["inline_code_bodies_match_xml"] = reader_inline == xml_inline
                aggregate["fenced_code_blocks"] += len(reader_fenced)
                aggregate["inline_code_spans"] += len(reader_inline)
                for code in code_blocks(reader, mermaid=True):
                    aggregate["mermaid_occurrences"] += 1
                    job_id = mermaid_job_id(code)
                    mermaid_job_ids.add(job_id)
                    job = jobs_by_id.get(job_id)
                    if not job or job.get("kind") != "mermaid" or job.get("code") != code:
                        errors.append({"chapter": cid, "check": "mermaid_source_to_job", "job_id": job_id})
                failed = [key for key, passed in checks.items() if not passed]
                if failed:
                    errors.append({"chapter": cid, "failed_checks": failed,
                                   "reader_fenced_count": len(reader_fenced), "xml_fenced_count": len(xml_fenced),
                                   "reader_inline_count": len(reader_inline), "xml_inline_count": len(xml_inline)})
            except Exception as exc:
                errors.append({"chapter": cid, "check": "audit_exception", "error": str(exc)})

    expected_counts = [20, 19, 21, 22, 24, 23]
    checks = {
        "six_volume_topology": len(volumes) == 6,
        "129_chapter_topology": chapter_count == 129,
        "volume_chapter_counts": [x["chapter_count"] for x in volumes] == expected_counts,
        "chapter_invariant_errors": len(errors) == 0,
        "all_95_mermaid_jobs_covered": len(mermaid_job_ids) == 95 and sum(row.get("kind") == "mermaid" for row in jobs_by_id.values()) == 95,
    }
    report = {"schema": "allmight-live-source-invariants-v1", "status": "pass" if all(checks.values()) else "fail",
              "production_manifest_sha256": sha(root / "production-manifest.json"),
              "jobs_sha256": sha(root / "jobs.json"), "chapter_count": chapter_count, "volumes": volumes,
              "counts": {**aggregate, "unique_mermaid_job_ids": len(mermaid_job_ids)}, "checks": checks, "errors": errors,
              "commands": ["python3 Tools/Production/verify_source_invariants.py --root . --report QA/source-promotion/live-source-invariants.json"],
              "limitations": ["Source Markdown hash comparison verifies the recorded source bytes; the companion source inventory is audited separately.",
                              "This checks chapter AST/XML provenance and code bodies; it does not verify page layout, EPUB conformance, or native InDesign output."]}
    if args.report:
        args.report.parent.mkdir(parents=True, exist_ok=True)
        args.report.write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n")
    print(json.dumps({"status": report["status"], "chapter_count": chapter_count, "counts": report["counts"],
                      "check_failures": [k for k, v in checks.items() if not v], "error_count": len(errors),
                      "report": str(args.report) if args.report else None}))
    return 0 if report["status"] == "pass" else 1


if __name__ == "__main__":
    raise SystemExit(main())
