#!/usr/bin/env python3
"""Audit Mermaid source SVG dimensions and projected print scaling.

This is deliberately an audit, not an asset renderer or source editor. It
joins a selected conversion generation to the canonical and supplemental
print-asset manifests, then records exact source SVG/PDF evidence and the
minimum font size projected through the current style cap.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import re
import sys
from collections import defaultdict
from datetime import datetime, timezone
from pathlib import Path
from typing import Any
from xml.etree import ElementTree as ET

from pypdf import PdfReader


ACTIVE = Path(
    "/Users/swayamsingal/Library/Mobile Documents/iCloud~md~obsidian/Documents/Master/Book Production/AllMight Codex"
)
CACHE = Path("/Users/swayamsingal/.codex/source-cache/allmight-book-production-2026-11-15")
QA = ACTIVE / "LaTeX" / "QA"


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def svg_pt(value: str | None, unit: str | None) -> float | None:
    if value is None:
        return None
    number = float(value)
    unit = (unit or "px").lower()
    if unit == "px":
        return number * 0.75
    if unit == "pt":
        return number
    return None


def svg_evidence(path: Path) -> dict[str, Any]:
    raw = path.read_text(encoding="utf-8", errors="replace")
    root = ET.fromstring(raw)
    diagram_id = root.attrib.get("id", "")
    base_match = re.search(
        r"#" + re.escape(diagram_id) + r"\s*\{[^}]*font-size\s*:\s*([0-9.]+)\s*(px|pt|em|rem|%)?",
        raw,
        re.IGNORECASE,
    )
    base_font = svg_pt(base_match.group(1), base_match.group(2)) if base_match else None
    explicit_fonts = [
        value
        for match in re.finditer(
            r"font-size\s*:\s*([0-9.]+)\s*(px|pt|em|rem|%)?", raw, re.IGNORECASE
        )
        if (value := svg_pt(match.group(1), match.group(2))) is not None
    ]

    def text_of(element: ET.Element) -> str:
        return " ".join(" ".join(element.itertext()).split())

    nodes = []
    for element in root.iter():
        classes = element.attrib.get("class", "").split()
        if "node" not in classes:
            continue
        nodes.append(
            {
                "svg_id": element.attrib.get("id"),
                "data_id": element.attrib.get("data-id"),
                "label": text_of(element),
            }
        )
    edges = [
        {
            "svg_id": element.attrib.get("id"),
            "data_id": element.attrib.get("data-id"),
        }
        for element in root.iter()
        if "flowchart-link" in element.attrib.get("class", "").split()
    ]
    edge_labels = {}
    for element in root.iter():
        if "edgeLabel" in element.attrib.get("class", "").split():
            label_node = next(
                (child for child in element.iter() if child.attrib.get("data-id")), None
            )
            if label_node is not None:
                edge_labels[label_node.attrib["data-id"]] = text_of(element)

    return {
        "svg_id": diagram_id,
        "root_width": root.attrib.get("width"),
        "root_height": root.attrib.get("height"),
        "view_box": root.attrib.get("viewBox"),
        "base_font_pt": base_font,
        "minimum_explicit_font_pt": min(explicit_fonts) if explicit_fonts else base_font,
        "nodes": nodes,
        "edges": edges,
        "edge_labels": edge_labels,
        "label_texts": sorted(node["label"] for node in nodes if node["label"])
        + sorted(value for value in edge_labels.values() if value),
    }


def manifest_asset_map() -> dict[str, dict[str, Any]]:
    files = [
        CACHE / "print-asset-manifest.json",
        ACTIVE / "QA" / "print-reference-dedup" / "supplemental-mermaid-print-assets.json",
    ]
    assets: dict[str, dict[str, Any]] = {}
    for manifest_path in files:
        if not manifest_path.is_file():
            continue
        data = json.loads(manifest_path.read_text(encoding="utf-8"))
        for asset in data.get("assets", []):
            if "mermaid" in asset.get("kinds", []) and asset.get("pdf"):
                assets[asset["pdf"]] = {
                    **asset,
                    "asset_manifest": str(manifest_path),
                    "supplemental": "supplemental-mermaid-print-assets.json" in manifest_path.name,
                }
    return assets


def style_caps(style_path: Path) -> dict[str, float]:
    text = style_path.read_text(encoding="utf-8", errors="replace")
    # Read exact cap factors where available. Page geometry is currently the
    # print profile in allmight-book.sty; all dimensions below are points.
    width_factor = float(re.search(r"max width\s*=\s*([0-9.]+)\\linewidth", text).group(1))
    height_factor = float(re.search(r"max height\s*=\s*([0-9.]+)\\textheight", text).group(1))
    page_w, page_h = 8 * 72.0, 10 * 72.0
    text_w = page_w - (1.0 + 0.72 + 0.06) * 72.0
    text_h = page_h - (0.78 + 0.80) * 72.0
    return {
        "max_width_factor": width_factor,
        "max_height_factor": height_factor,
        "text_width_pt": text_w,
        "text_height_pt": text_h,
        "cap_width_pt": width_factor * text_w,
        "cap_height_pt": height_factor * text_h,
    }


def run(manifest_path: Path, output_path: Path) -> dict[str, Any]:
    conversion = json.loads(manifest_path.read_text(encoding="utf-8"))
    assets = manifest_asset_map()
    style_path = ACTIVE / "LaTeX" / "allmight-book.sty"
    caps = style_caps(style_path)
    by_pdf: dict[str, list[dict[str, Any]]] = defaultdict(list)
    missing_asset_rows = []
    for row in conversion.get("rows", []):
        for diagram in row.get("diagrams", []):
            pdf = diagram.get("pdf")
            if pdf not in assets:
                missing_asset_rows.append({"id": diagram.get("id"), "pdf": pdf, "source": row.get("source")})
            else:
                by_pdf[pdf].append({"row": row, "diagram": diagram})

    records = []
    for pdf_path, occurrences in sorted(by_pdf.items()):
        asset = assets[pdf_path]
        svg_path = Path(asset["source_svg"])
        pdf_file = Path(pdf_path)
        svg = svg_evidence(svg_path)
        page = PdfReader(str(pdf_file)).pages[0]
        width_pt = float(page.mediabox.width)
        height_pt = float(page.mediabox.height)
        scale = min(1.0, caps["cap_width_pt"] / width_pt, caps["cap_height_pt"] / height_pt)
        minimum = svg["minimum_explicit_font_pt"]
        effective_minimum = minimum * scale if minimum is not None else None
        effective_base = svg["base_font_pt"] * scale if svg["base_font_pt"] is not None else None
        node_count = len(svg["nodes"])
        sources = [
            {
                "volume": occ["row"].get("volume"),
                "kind": occ["row"].get("kind"),
                "source": occ["row"].get("source"),
                "code_sha256": occ["diagram"].get("code_sha256"),
            }
            for occ in occurrences
        ]
        record = {
            "source_id": svg_path.stem,
            "source_svg": str(svg_path),
            "source_svg_sha256": asset.get("source_svg_sha256") or sha256(svg_path),
            "source_svg_sha256_actual": sha256(svg_path),
            "source_code_sha256": sorted({x["code_sha256"] for x in sources if x["code_sha256"]}),
            "source_rows": sources,
            "print_pdf": str(pdf_file),
            "print_pdf_sha256_manifest": asset.get("pdf_sha256"),
            "print_pdf_sha256_actual": sha256(pdf_file),
            "print_pdf_media_box_pt": {"width": width_pt, "height": height_pt},
            "source_svg_dimensions": {
                "width": svg["root_width"],
                "height": svg["root_height"],
                "view_box": svg["view_box"],
            },
            "source_font_pt": {
                "root_base": svg["base_font_pt"],
                "minimum_explicit": svg["minimum_explicit_font_pt"],
            },
            "current_print_style_projection": {
                "shrink_only_scale": scale,
                "effective_base_font_pt": effective_base,
                "effective_minimum_explicit_font_pt": effective_minimum,
                "effective_height_in": height_pt * scale / 72.0,
                "below_8pt": effective_minimum is not None and effective_minimum < 8.0,
                "simple_over_3in": node_count <= 6 and height_pt * scale > 3.0 * 72.0,
            },
            "svg_structure": svg,
            "occurrence_count": len(occurrences),
            "supplemental_reference_asset": asset["supplemental"],
        }
        records.append(record)

    all_occurrences = sum(record["occurrence_count"] for record in records)
    below_8 = [record for record in records if record["current_print_style_projection"]["below_8pt"]]
    simple_tall = [record for record in records if record["current_print_style_projection"]["simple_over_3in"]]
    supplemental = [record for record in records if record["supplemental_reference_asset"]]
    report = {
        "schema": "allmight-figure-layout-audit-v1",
        "status": "preliminary" if ".generation-" in manifest_path.as_posix() else "audited",
        "created_at_utc": datetime.now(timezone.utc).isoformat(),
        "conversion_manifest": str(manifest_path),
        "conversion_manifest_sha256": sha256(manifest_path),
        "style_file": str(style_path),
        "style_file_sha256": sha256(style_path),
        "style_caps": caps,
        "method": {
            "source_svg_root_font": "root-scoped SVG CSS font-size converted at 0.75 pt per px",
            "minimum_explicit_font": "minimum font-size declaration in SVG CSS/inline styles, projected conservatively",
            "print_pdf_dimensions": "first-page MediaBox read with pypdf",
            "fit_model": "min(1, cap_width / natural_width, cap_height / natural_height)",
            "thresholds": {"minimum_label_font_pt": 8.0, "simple_node_max": 6, "simple_height_in": 3.0},
            "limitations": [
                "This predicts source-label scale through the current LaTeX cap; it does not substitute for inspection of each compiled PDF.",
                "Minimum explicit CSS size can be an icon or edge-label style rather than the main node label; individual variants need render review.",
                "The active stage may be replaced before promotion; regenerate from the promoted conversion-manifest.json for final metrics.",
            ],
        },
        "summary": {
            "mermaid_occurrences": all_occurrences,
            "unique_mermaid_pdfs": len(records),
            "unique_supplemental_reference_assets": len(supplemental),
            "unique_below_8pt": len(below_8),
            "below_8pt_canonical": sum(not r["supplemental_reference_asset"] for r in below_8),
            "below_8pt_supplemental": sum(r["supplemental_reference_asset"] for r in below_8),
            "simple_over_3in": len(simple_tall),
            "simple_over_3in_supplemental": sum(r["supplemental_reference_asset"] for r in simple_tall),
            "missing_asset_rows": len(missing_asset_rows),
            "effective_min_font_pt_range": [
                min(r["current_print_style_projection"]["effective_minimum_explicit_font_pt"] for r in records if r["current_print_style_projection"]["effective_minimum_explicit_font_pt"] is not None),
                max(r["current_print_style_projection"]["effective_minimum_explicit_font_pt"] for r in records if r["current_print_style_projection"]["effective_minimum_explicit_font_pt"] is not None),
            ],
        },
        "missing_asset_rows": missing_asset_rows,
        "assets": records,
    }
    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(json.dumps(report, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    return report


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--manifest", type=Path, help="conversion manifest to audit; defaults to active promoted manifest")
    parser.add_argument("--output", type=Path, default=QA / "figure-layout-audit.json")
    args = parser.parse_args()
    manifest = args.manifest or ACTIVE / "LaTeX" / "conversion-manifest.json"
    report = run(manifest, args.output)
    print(
        json.dumps(
            {
                "status": report["status"],
                "manifest": report["conversion_manifest"],
                "summary": report["summary"],
                "output": str(args.output),
            },
            indent=2,
        )
    )
    return 0 if not report["missing_asset_rows"] else 1


if __name__ == "__main__":
    sys.exit(main())
