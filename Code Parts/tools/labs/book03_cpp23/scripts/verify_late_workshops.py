#!/usr/bin/env python3
"""Check exported P24–P32 fixture tables independently of the native code.

These schema and numerical witnesses do not replicate every stochastic fit.
"""
import csv
import math
from pathlib import Path
import sys


def require(condition, message):
    if not condition:
        raise ValueError(message)


def read(root, name, expected):
    with (root / name).open(newline="", encoding="utf-8") as stream:
        raw = list(csv.DictReader(stream))
    require(len(raw) == expected, f"{name}: expected {expected} rows, got {len(raw)}")
    rows = [{key: float(value) for key, value in row.items()} for row in raw]
    require(all(math.isfinite(v) for row in rows for v in row.values()), f"{name}: nonfinite")
    return rows


def verify(root):
    sensor = read(root, "sensor.csv", 17)
    for stop, row in zip(range(-8, 9), sensor):
        require(row["incident"] == 1000 * 2.0**stop, "sensor exposure sequence")
        require(row["variance"] >= 0 and row["snr"] >= 0, "sensor moments")
        require(0 <= row["well_clip_fraction"] <= 1 and
                0 <= row["adc_top_fraction"] <= 1, "sensor saturation fractions")

    color = read(root, "color.csv", 729)
    for row in color:
        for ch in "rgb":
            x = row["linear_" + ch]
            encoded = 12.92*x if x <= .0031308 else 1.055*x**(1/2.4)-.055
            require(0 <= x <= 1 and abs(row["srgb_encoded_" + ch]-encoded) < 1e-12,
                    "color transfer formula")

    calibration = read(root, "calibration.csv", 100)
    epipolar = read(root, "epipolar.csv", 100)
    reconstruction = read(root, "reconstruction.csv", 100)
    for name, rows in (("calibration", calibration), ("epipolar", epipolar),
                       ("reconstruction", reconstruction)):
        require({(r["sigma"],r["trial"]) for r in rows} ==
                {(s,t) for s in (0,.1,.25,.5,1) for t in range(20)},
                f"{name}: missing or repeated trial")
    for r in calibration:
        require(r["train_rms"] >= 0 and r["heldout_rms"] >= 0 and
                r["fx"] > 0 and r["fy"] > 0, "calibration residual/intrinsics")
    for r in epipolar:
        require(r["train_sampson_rms"] >= 0 and r["heldout_sampson_rms"] >= 0,
                "epipolar residual")
    for r in reconstruction:
        require(r["initial_rms"] >= 0 and r["refined_rms"] >= 0 and
                r["refined_rms"] <= r["initial_rms"] + 1e-12,
                "fixed-pose reprojection descent")

    matches = read(root, "matching.csv", 50)
    for r in matches:
        require(r["found"] == 1 and r["tp"] + r["fn"] == 100-r["outliers"],
                "matching labels")
        require(r["tp"] >= 0 and 0 <= r["fp"] <= r["outliers"] and r["fn"] >= 0,
                "matching count bounds")
        require(abs(r["precision"]-r["tp"]/(r["tp"]+r["fp"])) < 1e-12 and
                abs(r["recall"]-r["tp"]/(r["tp"]+r["fn"])) < 1e-12,
                "matching precision and recall")

    fusion = read(root, "fusion.csv", 100)
    require({(r["frames"],r["trial"]) for r in fusion} ==
            {(f,t) for f in (1,2,4,8,16) for t in range(20)}, "fusion trials")
    for r in fusion:
        require(r["observed_columns"] == 219 and r["missing_columns"] == 37,
                "fusion support counts")
        require(r["first_rmse"] >= 0 and r["fused_rmse"] >= 0, "fusion residuals")
        if r["frames"] == 1:
            require(abs(r["first_rmse"]-r["fused_rmse"]) < 1e-12,
                    "single-frame equality")

    capstone = read(root, "capstone.csv", 100)
    require({(r["baseline_m"],r["trial"]) for r in capstone} ==
            {(b,t) for b in (.05,.1,.2,.4,.8) for t in range(20)}, "capstone trials")
    for r in capstone:
        require(r["attempted"] == r["accepted"] == 200, "capstone support")
        expected = 16 * math.sqrt(2) * r["sigma_px"] / (800*r["baseline_m"])
        require(abs(r["predicted_depth_sigma_m"]-expected) < 1e-12,
                "first-order disparity uncertainty")
        require(r["depth_rmse_m"] >= 0 and r["position_rmse_m"] >= 0 and
                r["pixel_rmse_px"] >= 0, "capstone residuals")
    print("P24–P32 independent CSV witnesses passed (P31 separate export verifier).")


if __name__ == "__main__":
    if len(sys.argv) != 2:
        raise SystemExit("usage: verify_late_workshops.py BUILD_OUTPUT_DIRECTORY")
    verify(Path(sys.argv[1]))
