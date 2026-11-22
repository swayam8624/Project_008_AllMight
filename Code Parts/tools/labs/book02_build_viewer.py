#!/usr/bin/env python3
"""Package the Book Two Numerical Geometry Atlas as one standalone offline HTML.

The viewer contains only the actual numerical CSV data and manuscript metadata
exported by book02_visual_atlas.py. No CDN, web scripts, remote assets or API.
"""
from __future__ import annotations

import argparse
import csv
import json
import math
from pathlib import Path


TEMPLATE = Path(__file__).with_name("book02_atlas_viewer_template.html")
OUTPUT = "Numerical-Geometry-Atlas.html"
SENTINEL = "__ATLAS_DATA__"


def decode_value(key: str, value: str):
    """Preserve categorical strings and parse numeric fields without guessing.

    The table schema is controlled by the figure generator. A blank field or
    nonfinite value is a failed scientific export, never silently ignored.
    """
    if value == "" or value is None:
        raise ValueError("Unexpected blank CSV field: " + key)
    if key in {"method", "decision_path"}:
        return value
    x = float(value)
    if not math.isfinite(x):
        raise ValueError("Nonfinite numeric value for: " + key)
    if key in {"k", "index", "vertex", "grid_n", "vertices", "triangles",
               "orientation_sign", "source", "generator_inlier",
               "consensus_inlier", "budget_evaluations", "seed"}:
        return int(x)
    return x


def build_viewer(folder: str | Path) -> Path:
    folder = Path(folder)
    report = json.loads((folder/"manifest.json").read_text(encoding="utf-8"))
    figures = report["figures"]
    required = {
        "orient2d-certification", "ransac-consensus", "geodesic-fields",
        "geodesic-surface-3d", "geodesic-resolution", "robust-fitting",
        "mis-variance",
    }
    # A subset export is still a valid scientific figure export, but the
    # full interactive atlas requires every mode's data to be present.
    actual = {f["name"] for f in figures}
    if not required.issubset(actual):
        # Do not emit a broken interactive app when the user requested a
        # subset of figures. Static PDF/SVG/PNG outputs still succeed.
        return folder/OUTPUT
    data = {}
    meta = {}
    for item in figures:
        name = item["name"]
        with (folder/item["data"]).open(newline="", encoding="utf-8") as f:
            records = list(csv.DictReader(f))
        if len(records) != item["rows"]:
            raise ValueError("CSV row count changed: " + name)
        data[name] = [
            {k: decode_value(k, v) for k, v in row.items()}
            for row in records
        ]
        meta[name] = {k:v for k,v in item.items()
                      if k not in ("files", "data", "data_sha256")}
    payload=json.dumps({"tables":data,"metadata":meta},
                       separators=(",",":"),sort_keys=True,allow_nan=False)
    payload=payload.replace("<","\\u003c").replace("&","\\u0026")
    template=TEMPLATE.read_text(encoding="utf-8")
    if template.count(SENTINEL)!=1:
        raise RuntimeError("Offline viewer placeholder missing or ambiguous")
    html=template.replace(SENTINEL,payload)
    if "https://" in html or "http://" in html:
        raise RuntimeError("Offline viewer contains a network dependency")
    target=folder/OUTPUT
    target.write_text(html,encoding="utf-8")
    if target.stat().st_size < 50000:
        raise RuntimeError("Generated interactive atlas unexpectedly small")
    return target


if __name__=="__main__":
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source",type=Path,required=True)
    args=parser.parse_args()
    result=build_viewer(args.source)
    print("BOOK02_INTERACTIVE_ATLAS:",result)
