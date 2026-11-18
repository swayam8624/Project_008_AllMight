#!/usr/bin/env python3
"""Validate the approved special Mermaid post-render chain before native proof."""
from __future__ import annotations

import argparse
import hashlib
import json
import sys
import xml.etree.ElementTree as ET
from pathlib import Path
from typing import Any


SPECIAL_IDS = (
    "12f60b7ab1e672dd10535591",
    "8e675126df1a1b1854a18b23",
    "fcbea06ba67e4dcc48784ddf",
)
COMPOSITE_ID = "fcbea06ba67e4dcc48784ddf"
PRIVATE_CACHE_MARKER = ".codex/" + "source-cache/"


def sha(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def read_json(path: Path) -> dict[str, Any]:
    value = json.loads(path.read_text())
    if not isinstance(value, dict):
        raise ValueError(f"expected JSON object: {path}")
    return value


def one(rows: list[dict[str, Any]], key: str, value: str, label: str) -> dict[str, Any]:
    hits = [row for row in rows if row.get(key) == value]
    if len(hits) != 1:
        raise ValueError(f"expected one {label} matching {key}={value!r}; found {len(hits)}")
    return hits[0]


def main() -> int:
    p = argparse.ArgumentParser()
    p.add_argument("--production-root", type=Path, required=True)
    p.add_argument("--print-assets", type=Path, required=True)
    p.add_argument("--font-metadata", type=Path, required=True)
    p.add_argument("--report", type=Path, required=True)
    a = p.parse_args()
    R = a.production_root.resolve()
    errors: list[str] = []
    evidence: dict[str, Any] = {"schema": "postrender-asset-verification-v1", "special_asset_ids": list(SPECIAL_IDS), "assets": []}
    try:
        jobs_path = R / "composite-jobs.json"
        composite_report_path = R / "composite-render-report.json"
        complete_path = R / "asset-render-complete-report.json"
        norm_path = R / "layout-normalizations.json"
        jobs = read_json(jobs_path)
        composite_report = read_json(composite_report_path)
        complete = read_json(complete_path)
        normalizations = read_json(norm_path)
        print_manifest = read_json(a.print_assets.resolve())
        font_metadata = read_json(a.font_metadata.resolve())

        if jobs.get("schema") != "allmight-postrender-composite-jobs-v1":
            raise ValueError("composite jobs manifest schema is not the approved versioned post-render schema")
        if jobs.get("manifest_version") != 1 or jobs.get("compositor_version") != "allmight-fcbea-composite-v2":
            raise ValueError("composite jobs manifest version/renderer is not the approved v2 version")
        if "before font" not in jobs.get("required_stage", "").lower() or "print pdf" not in jobs.get("required_stage", "").lower():
            raise ValueError("composite manifest does not require the stage before font and print-PDF validation")
        if composite_report.get("schema") != "allmight-postrender-composite-report-v1" or composite_report.get("status") != "published":
            raise ValueError("composite render report is not a published versioned post-render result")
        if PRIVATE_CACHE_MARKER in json.dumps(jobs, ensure_ascii=False) or PRIVATE_CACHE_MARKER in json.dumps(composite_report, ensure_ascii=False):
            raise ValueError("published composite manifest/report contains a private source-cache reference")

        def production_file(relative: Any, label: str) -> Path:
            if not isinstance(relative, str) or not relative or Path(relative).is_absolute():
                raise ValueError(f"{label} must be a nonempty production-relative path")
            if PRIVATE_CACHE_MARKER in relative.replace("\\", "/"):
                raise ValueError(f"{label} references a private source cache")
            target = (R / relative).resolve()
            if not target.is_relative_to(R):
                raise ValueError(f"{label} escapes the production root")
            if not target.is_file():
                raise ValueError(f"{label} is missing: {target}")
            return target

        def report_path(value: Any, label: str) -> Path:
            if not isinstance(value, str) or not value:
                raise ValueError(f"{label} must record a path")
            raw = Path(value)
            target = (R / raw).resolve() if not raw.is_absolute() else raw.resolve()
            if not target.is_relative_to(R):
                raise ValueError(f"{label} escapes the production root")
            if PRIVATE_CACHE_MARKER in value.replace("\\", "/"):
                raise ValueError(f"{label} references a private source cache")
            return target

        recovery = jobs.get("recovery_sources", [])
        required_recovery_roles = {"original-pre-composite", "panel-a", "panel-b"}
        recovery_roles = {row.get("role") for row in recovery if isinstance(row, dict)}
        if len(recovery) != 3 or recovery_roles != required_recovery_roles:
            raise ValueError("composite manifest must list exactly the original and both panel recovery SVGs")
        recovery_evidence = []
        for row in recovery:
            path = production_file(row.get("path"), f"recovery source {row.get('role')}")
            actual_sha = sha(path)
            if actual_sha != row.get("sha256"):
                raise ValueError(f"recovery source hash mismatch for {row.get('role')}: {path}")
            recovery_evidence.append({"role": row["role"], "path": row["path"], "sha256": actual_sha})

        job = one(jobs.get("jobs", []), "id", COMPOSITE_ID, "composite job")
        if job.get("kind") != "svg-composite":
            raise ValueError("required fcbea job is not an SVG composite")
        component_evidence = []
        components = job.get("components", [])
        if len(components) != 2:
            raise ValueError("fcbea composite must contain exactly two packaged Mermaid panels")
        for component in components:
            component_svg = production_file(component.get("packaged_source_svg"), f"component {component.get('id')} SVG")
            component_sha = sha(component_svg)
            if component_sha != component.get("source_svg_sha256"):
                raise ValueError(f"packaged component SVG hash mismatch for {component.get('id')}")
            component_evidence.append({"id": component.get("id"), "path": component.get("packaged_source_svg"), "sha256": component_sha})
        expected_manifest_sha = sha(jobs_path)
        if composite_report.get("manifest_sha256") != expected_manifest_sha:
            raise ValueError("composite report was generated from a different jobs manifest")
        if composite_report.get("manifest_version") != jobs.get("manifest_version"):
            raise ValueError("composite report manifest version does not match jobs manifest")
        if composite_report.get("job_id") != COMPOSITE_ID:
            raise ValueError("composite report job id mismatch")
        if composite_report.get("composite_renderer_version") != jobs.get("compositor_version"):
            raise ValueError("composite renderer version mismatch")
        if composite_report.get("all_nodes_edges_labels_pass") is not True:
            raise ValueError("composite semantic preservation check did not pass")
        compositor_script = production_file(jobs.get("compositor", {}).get("script"), "compositor implementation")
        renderer_script = production_file(jobs.get("renderer", {}).get("script"), "component renderer implementation")
        if sha(compositor_script) != composite_report.get("compositor_script_sha256"):
            raise ValueError("compositor implementation hash does not match the published report")
        if sha(renderer_script) != composite_report.get("component_renderer_script_sha256"):
            raise ValueError("component renderer hash does not match the published report")

        output = Path(composite_report.get("output_path", "")).resolve()
        declared_output = Path(job.get("output_path", "")).resolve()
        if output != declared_output or output != Path(job.get("canonical_source_asset_output", "")).resolve():
            raise ValueError("composite report output does not match the declared canonical asset path")
        output_sha = sha(output)
        if composite_report.get("output_sha256") != output_sha:
            raise ValueError("composite report output hash does not match the published SVG")
        if composite_report.get("source_svg_before_composition_sha256") != next(row["sha256"] for row in recovery_evidence if row["role"] == "original-pre-composite"):
            raise ValueError("composite source recovery fingerprint does not match the job manifest")
        if composite_report.get("source_code_sha256") != job.get("source_code_sha256"):
            raise ValueError("composite source code fingerprint does not match the job manifest")
        transform_report = composite_report.get("transform_report", {})
        if transform_report.get("renderer_version") != jobs.get("compositor_version"):
            raise ValueError("composite transform report renderer version mismatch")
        report_path(transform_report.get("svg"), "transformed composite SVG report path")
        if transform_report.get("svg_sha256") != output_sha:
            raise ValueError("transformed composite report hash differs from the published SVG")
        panel_paths = transform_report.get("source_panels", [])
        panel_hashes = transform_report.get("source_panel_svg_sha256", [])
        if len(panel_paths) != 2 or len(panel_hashes) != 2:
            raise ValueError("transform report must fingerprint both component panel SVGs")
        for index, (panel_path, panel_hash) in enumerate(zip(panel_paths, panel_hashes)):
            report_path(panel_path, f"transformed panel {index + 1} report path")
            if panel_hash != component_evidence[index]["sha256"]:
                raise ValueError(f"transform report panel {index + 1} hash differs from packaged component source")
        if int(transform_report.get("prefixed_id_counts", {}).get("total_unique", 0)) != 57:
            raise ValueError("composite should preserve exactly 57 namespaced SVG IDs")
        if transform_report.get("unresolved_local_references") != []:
            raise ValueError("composite contains unresolved local SVG references")
        if transform_report.get("source_file_untouched") is not True or transform_report.get("source_job_untouched") is not True:
            raise ValueError("composite transform did not certify source/job preservation")

        complete_row = one(complete.get("assets", []), "id", COMPOSITE_ID, "complete-render record")
        if complete_row.get("post_render_transform_version") != jobs.get("compositor_version"):
            raise ValueError("complete-render record lacks the required composite post-render version")
        if complete_row.get("svg_sha256") != output_sha or Path(complete_row.get("outfile", "")).resolve() != output:
            raise ValueError("complete-render record does not match the final composite SVG")
        if complete_row.get("composite_job_manifest_sha256") != expected_manifest_sha:
            raise ValueError("complete-render record does not fingerprint the current composite manifest")
        if complete_row.get("composite_report_sha256") != sha(composite_report_path):
            raise ValueError("complete-render record does not fingerprint the current composite report")

        norm_row = one(normalizations.get("candidates", []), "id", COMPOSITE_ID, "layout normalization record")
        if norm_row.get("production_status") != "published_single_svg_two_panel_composite":
            raise ValueError("normalization ledger does not mark the two-panel composite as published")
        if norm_row.get("production_render", {}).get("svg_sha256") != output_sha:
            raise ValueError("normalization ledger does not match the final composite bytes")

        font_row = font_metadata.get("by_svg_path", {}).get(str(output))
        if not font_row or font_row.get("svg_sha256") != output_sha:
            raise ValueError("computed font metadata is missing or stale for the final composite")
        effective_font = float(font_row.get("effective_min_label_pt_at_page_fit", 0))
        if effective_font < 9.75:
            raise ValueError(f"composite effective minimum label font is below 9.75 pt: {effective_font}")

        print_rows = print_manifest.get("assets", [])
        for asset_id in SPECIAL_IDS:
            candidate_rows = [row for row in print_rows if row.get("job_ids") and asset_id in row.get("job_ids", [])]
            if len(candidate_rows) != 1:
                raise ValueError(f"expected one current print-asset row for {asset_id}; found {len(candidate_rows)}")
            row = candidate_rows[0]
            svg = Path(row.get("source_svg", "")).resolve()
            if not svg.is_file() or row.get("input_sha256") != sha(svg) or row.get("source_svg_sha256") != sha(svg):
                raise ValueError(f"print row source SVG hash is stale for {asset_id}")
            pdf = Path(row.get("pdf", "")).resolve()
            if row.get("status") != "complete" or not pdf.is_file() or row.get("pdf_sha256") != sha(pdf):
                raise ValueError(f"print PDF is missing, incomplete or stale for {asset_id}")
            if "mermaid" not in row.get("kinds", []):
                raise ValueError(f"print row is not marked as Mermaid for {asset_id}")
            if asset_id == COMPOSITE_ID and svg != output:
                raise ValueError("print row does not target the published post-render composite SVG")
            evidence["assets"].append({"id": asset_id, "svg": str(svg), "svg_sha256": sha(svg), "pdf": str(pdf), "pdf_sha256": sha(pdf), "print_transform_version": row.get("print_transform_version")})

        evidence.update({
            "status": "pass",
            "post_render_stage_version": jobs.get("compositor_version"),
            "composite_manifest": str(jobs_path),
            "composite_manifest_sha256": expected_manifest_sha,
            "composite_report": str(composite_report_path),
            "composite_report_sha256": sha(composite_report_path),
            "composite_svg": str(output),
            "composite_svg_sha256": output_sha,
            "effective_min_label_pt_at_page_fit": effective_font,
            "semantic_validation": "all nodes, edges and labels pass; 57 unique IDs; zero unresolved local refs",
            "recovery_sources": recovery_evidence,
            "packaged_components": component_evidence,
        })
    except Exception as exc:  # Report every failure as a hard gate for the caller.
        errors.append(str(exc))
        evidence["status"] = "fail"
    evidence["errors"] = errors
    report = a.report.resolve()
    report.parent.mkdir(parents=True, exist_ok=True)
    report.write_text(json.dumps(evidence, ensure_ascii=False, indent=2) + "\n")
    print(json.dumps({"status": evidence["status"], "report": str(report), "errors": errors}, ensure_ascii=False))
    return 0 if not errors else 1


if __name__ == "__main__":
    raise SystemExit(main())
