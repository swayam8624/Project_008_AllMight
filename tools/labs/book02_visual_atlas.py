#!/usr/bin/env python3
"""Book Two, Pass 3 — reproducible scientific visual atlas.

All figures come from the tested Book Two numerical labs, not generated-looking
decorations or fabricated speedups. Synthetic experiments and analytical
references are labeled. Nothing here reproduces an original paper's published
timing or dataset figures.

Run:
    python tools/labs/book02_visual_atlas.py --output path/to/atlas
Outputs:
    figures/{name}.{pdf,svg,png}  print-vector, web-vector and screen preview
    data/{name}.csv              numerical observations behind each figure
    manifest.json                versions, seeds, exact/approximate truth and units

Requires numpy, scipy and matplotlib. Uses an isolated Agg backend; no
internet, API keys, GUI or external assets are involved.
"""
from __future__ import annotations

import argparse
import csv
from datetime import datetime, timezone
import hashlib
import json
import math
from pathlib import Path
import sys

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib import colors as mplcolors
from matplotlib.ticker import MaxNLocator
from mpl_toolkits.mplot3d import Axes3D  # noqa: F401
import numpy as np
import scipy
from scipy.optimize import least_squares

from book02_papers_geometry import (
    orient2d, synthetic_line_data, ransac_line, total_least_squares,
)
from book02_papers_numeric import (
    planar_grid, heat_method, damped_huber_fit, synthetic_curve,
    scipy_fit_reference, curve_model, curve_residual, curve_jacobian,
)
from book02_papers_sampling import inverse_q2

# Ink + cyan + gold matches the book cover but stays legible on white paper.
INK = "#14273D"
TEAL = "#087D94"
GOLD = "#C78623"
BLUE = "#4464B0"
RED = "#BB5764"
GREY = "#788395"
PAPER = "#FFFFFF"
GRID = "#E9EDF2"
FIGURES = (
    "orient2d-certification",
    "ransac-consensus",
    "geodesic-fields",
    "geodesic-surface-3d",
    "geodesic-resolution",
    "robust-fitting",
    "mis-variance",
)


def style() -> None:
    plt.rcParams.update({
        "font.family": "DejaVu Sans",
        "font.size": 9,
        "axes.titlesize": 10.5,
        "axes.titleweight": "semibold",
        "axes.labelsize": 9,
        "axes.edgecolor": "#B8C2CD",
        "axes.labelcolor": INK,
        "text.color": INK,
        "xtick.color": "#42546B",
        "ytick.color": "#42546B",
        "legend.fontsize": 8,
        "legend.frameon": False,
        "figure.facecolor": PAPER,
        "axes.facecolor": PAPER,
        "savefig.facecolor": PAPER,
        "svg.fonttype": "none",
        "svg.hashsalt": "book02-numerical-geometry-atlas",
        "pdf.fonttype": 42,
        "grid.color": GRID,
        "grid.linewidth": 0.6,
        "axes.spines.top": False,
        "axes.spines.right": False,
    })


def decorate(ax, xlabel: str = "", ylabel: str = "") -> None:
    ax.grid(True, alpha=.9, zorder=0)
    if xlabel:
        ax.set_xlabel(xlabel)
    if ylabel:
        ax.set_ylabel(ylabel)
    ax.set_axisbelow(True)


def save_record(fig, name: str, output: Path, headings: list[str],
                records: list[list[object]], metadata: dict) -> dict:
    """Save one figure and its exact rows, refusing nonfinite numeric entries."""
    if name not in FIGURES:
        raise ValueError("Unregistered figure name " + name)
    assert headings and records
    for row in records:
        if len(row) != len(headings):
            raise ValueError(f"{name}: row/header mismatch")
        for value in row:
            if isinstance(value, (float, np.floating)) and not math.isfinite(float(value)):
                raise ValueError(f"{name}: nonfinite data")
    (output / "figures").mkdir(parents=True, exist_ok=True)
    (output / "data").mkdir(parents=True, exist_ok=True)
    csv_path = output / "data" / (name + ".csv")
    with csv_path.open("w", newline="", encoding="utf-8") as f:
        writer = csv.writer(f)
        writer.writerow(headings)
        writer.writerows(records)
    # Identical CSV on a rerun is the core deterministic-data contract.
    data_sha = hashlib.sha256(csv_path.read_bytes()).hexdigest()
    fig.tight_layout(pad=1.25)
    paths = {}
    for fmt in ("pdf", "svg", "png"):
        destination = output / "figures" / (name + "." + fmt)
        kw = {"bbox_inches": "tight", "pad_inches": .10}
        if fmt == "png":
            kw["dpi"] = 190
        if fmt == "pdf":
            kw["metadata"] = {
                "Creator": "Book02 numerical atlas (source-driven)",
                "Title": name,
                "CreationDate": datetime(2020, 1, 1, tzinfo=timezone.utc),
            }
        fig.savefig(destination, format=fmt, **kw)
        if destination.stat().st_size < 600:
            raise RuntimeError(f"Empty or incomplete {name}.{fmt}")
        paths[fmt] = "figures/" + destination.name
    plt.close(fig)
    return {
        "name": name,
        "files": paths,
        "data": "data/" + csv_path.name,
        "data_sha256": data_sha,
        "rows": len(records),
        **metadata,
    }


def orientation(output: Path) -> dict:
    fig, axs = plt.subplots(1, 2, figsize=(8.0, 3.25))
    cases = [
        ((0., 0.), (1., 0.), (.42, .90), TEAL, "CCW"),
        ((0., 0.), (1., 0.), (.42, -.90), GOLD, "CW"),
        ((0., 0.), (1., 0.), (.42, 0.), GREY, "collinear"),
    ]
    for a, b, c, color, label in cases:
        xshift = {"CCW": -1.25, "CW": .25, "collinear": 1.75}[label]
        pts = np.array([a, b, c], dtype=float)
        pts[:, 0] += xshift
        sign = orient2d(a, b, c).sign
        axs[0].plot([pts[0, 0], pts[1, 0], pts[2, 0], pts[0, 0]],
                    [pts[0, 1], pts[1, 1], pts[2, 1], pts[0, 1]],
                    color=color, lw=1.9, marker="o", markersize=3.7)
        axs[0].text(xshift + .5, 1.13, f"{label}: {sign:+d}",
                    ha="center", fontsize=8, color=color, fontweight="bold")
    axs[0].set_xlim(-1.5, 3.1)
    axs[0].set_ylim(-1.05, 1.38)
    axs[0].set_aspect("equal")
    axs[0].axis("off")
    axs[0].set_title("A sign changes topology", loc="left")

    ks = list(range(2, 54))
    records = []
    for k in ks:
        # Stored binary64 values, not arbitrary precision real coordinates.
        third = float(2.0 + 2.0 ** (-k))
        result = orient2d((0., 0.), (1., 1.), (2., third))
        signed = third - 2.0
        records.append([k, third, signed, result.sign, result.path])
    safe = [r for r in records if r[-1] == "filtered-float" and r[2] > 0]
    fallback = [r for r in records if r[-1] == "exact-rational" and r[2] > 0]
    rounded = [r for r in records if r[2] == 0]
    for rows, color, label, marker in (
        (safe, TEAL, "certified fast sign", "o"),
        (fallback, GOLD, "exact rational fallback", "D"),
        (rounded, RED, "perturbation rounded away", "x"),
    ):
        if rows:
            axs[1].scatter([r[0] for r in rows],
                           [max(r[2], 2.0 ** -54) for r in rows],
                           c=color, label=label, s=19, marker=marker, zorder=3)
    axs[1].set_yscale("log", base=2)
    axs[1].set_title("Near-collinearity • stored binary64", loc="left")
    decorate(axs[1], "Perturbation exponent k in 2⁻ᵏ",
             "|determinant| (input units²)")
    axs[1].legend(loc="lower left", fontsize=7)
    axs[1].set_xlim(0, 55)
    return save_record(fig, FIGURES[0], output,
                       ["k", "third_y_binary64", "determinant_exact_stored",
                        "orientation_sign", "decision_path"], records, {
                           "type": "analytic exact-sign witness",
                           "units": "arbitrary 2D length units",
                           "seed": None,
                           "truth": "sign of stored binary64 coordinates",
                           "caveat": "Exact fallback is not Shewchuk expansion arithmetic; data rounded on input cannot be recovered.",
                       })


def ransac(output: Path) -> dict:
    points, truth = synthetic_line_data()
    fit = ransac_line(points, threshold=.08, seed=73013)
    naive = total_least_squares(points)
    P = np.asarray(points)
    inliers = set(fit.inlier_indices)
    truthset = set(truth)
    xs = np.linspace(-4.1, 4.1, 200)
    fity = (-fit.line.a * xs - fit.line.c) / fit.line.b
    naivey = (-naive.a * xs - naive.c) / naive.b
    fig, axs = plt.subplots(1, 2, figsize=(8.2, 3.4),
                            gridspec_kw={"width_ratios": [1.6, 1]})
    ax = axs[0]
    mask = np.array([k in inliers for k in range(len(P))])
    ax.scatter(P[~mask, 0], P[~mask, 1], color=RED, s=16,
               alpha=.85, label="rejected by RANSAC", zorder=3)
    ax.scatter(P[mask, 0], P[mask, 1], color=TEAL, s=15,
               alpha=.85, label="selected inliers", zorder=3)
    ax.plot(xs, 2*xs+1, color=INK, lw=1.3, ls=":", label="known generating line")
    ax.plot(xs, fity, color=TEAL, lw=1.7, label="RANSAC + TLS")
    ax.plot(xs, naivey, color=GOLD, lw=1.55, ls="--", label="all-point TLS")
    ax.set_xlim(-4.3, 4.3)
    ax.set_ylim(-13, 11)
    ax.set_title("Consensus is not all-point least squares", loc="left")
    decorate(ax, "x (synthetic units)", "y (synthetic units)")
    ax.legend(loc="upper left", fontsize=7.1)

    resid_fit = np.array([fit.line.distance(tuple(p)) for p in P])
    resid_tls = np.array([naive.distance(tuple(p)) for p in P])
    ax = axs[1]
    bins = np.linspace(0, max(np.percentile(resid_tls, 90), 4), 19)
    ax.hist(resid_tls, bins=bins, color=GOLD, alpha=.66, label="all-point TLS")
    ax.hist(resid_fit, bins=bins, color=TEAL, alpha=.62, label="RANSAC fit")
    ax.axvline(.08, color=RED, lw=1.4, ls="--", label="0.08 threshold")
    ax.set_title("Residuals over all 110 points", loc="left")
    decorate(ax, "Orthogonal point-to-line distance", "Point count")
    ax.legend(loc="upper right", fontsize=7.0)
    fig.suptitle(f"80 generated inliers • 30 generated outliers • seed 73013 • "
                 f"{fit.iterations} hypotheses evaluated", y=1.025,
                 fontsize=9.0, color=GREY)
    rows = [[i, float(P[i, 0]), float(P[i, 1]), int(i in truthset),
             int(i in inliers), float(resid_fit[i]), float(resid_tls[i])]
            for i in range(len(P))]
    return save_record(fig, FIGURES[1], output,
                       ["index", "x", "y", "generator_inlier",
                        "consensus_inlier", "ransac_residual",
                        "allpoint_tls_residual"], rows, {
                            "type": "synthetic RANSAC experiment",
                            "units": "arbitrary planar distance",
                            "seed": 73013,
                            "num_points": len(P),
                            "selected_inliers": len(inliers),
                            "iterations": fit.iterations,
                            "fit_line": {"a": float(fit.line.a),
                                         "b": float(fit.line.b),
                                         "c": float(fit.line.c)},
                            "caveat": "Single synthetic scene, not published RANSAC paper camera-localization performance.",
                        })


def field_data(n=25):
    V, F = planar_grid(n, n, 2.0/(n-1))
    source = (n//2)*n + (n//2)
    fit = heat_method(V, F, source, time_factor=1.0)
    oracle = np.linalg.norm(V - V[source], axis=1)
    return V, F, fit, oracle, source


def geodesic(output: Path) -> dict:
    V, F, result, truth, source = field_data()
    err = result.distances-truth
    fig, axs = plt.subplots(1, 3, figsize=(9.1, 3.2))
    panels = [
        ("Implicit heat field", result.heat, "viridis", "u (dimensionless)"),
        ("Recovered distance", result.distances, "viridis", "Distance (m)"),
        ("Signed distance error", err, "coolwarm", "Estimate − exact (m)"),
    ]
    for i, (name, values, cmap, label) in enumerate(panels):
        a = axs[i]
        if i == 2:
            lim = max(float(np.max(np.abs(values))), 1e-9)
            norm = mplcolors.TwoSlopeNorm(vmin=-lim, vcenter=0, vmax=lim)
            t = a.tripcolor(V[:, 0], V[:, 1], F, values,
                            cmap=cmap, norm=norm, shading="gouraud")
        else:
            t = a.tripcolor(V[:, 0], V[:, 1], F, values,
                            cmap=cmap, shading="gouraud")
        a.scatter([V[source, 0]], [V[source, 1]], marker="*",
                  edgecolors="white", linewidths=.5, color=GOLD,
                  s=65, zorder=5)
        a.set_title(name, loc="left")
        a.set_aspect("equal")
        a.set_xlabel("x (m)")
        if i == 0:
            a.set_ylabel("y (m)")
        fig.colorbar(t, ax=a, fraction=.045, pad=.03, label=label)
    fig.suptitle("25×25 triangular grid • centre source • exact straight-line oracle",
                 y=1.035, fontsize=9, color=GREY)
    rows = [[int(i), float(V[i, 0]), float(V[i, 1]),
             float(result.heat[i]), float(result.distances[i]),
             float(truth[i]), float(err[i])] for i in range(len(V))]
    return save_record(fig, FIGURES[2], output,
                       ["vertex", "x_m", "y_m", "heat",
                        "distance_estimated_m", "distance_exact_m",
                        "signed_error_m"], rows, {
                            "type": "computed triangular FEM geodesic distance",
                            "units": "metres",
                            "num_vertices": len(V),
                            "num_triangles": len(F),
                            "source": int(source),
                            "mae_m": float(np.mean(np.abs(err))),
                            "max_absolute_error_m": float(np.max(np.abs(err))),
                            "caveat": "Planar convex domain only; numerical estimate, not exact field.",
                        })


def surface3d(output: Path) -> dict:
    V, F, result, truth, source = field_data()
    err = result.distances-truth
    fig = plt.figure(figsize=(8.3, 3.9))
    ax1 = fig.add_subplot(121, projection="3d")
    surf = ax1.plot_trisurf(V[:, 0], V[:, 1], result.distances, triangles=F,
                            cmap="viridis", linewidth=.0, antialiased=True)
    ax1.set_title("Reconstructed geodesic surface", pad=8)
    ax1.set(xlabel="x (m)", ylabel="y (m)", zlabel="Distance (m)")
    ax1.view_init(elev=30, azim=-57)
    ax1.tick_params(labelsize=7)
    ax2 = fig.add_subplot(122)
    lim = max(float(np.max(np.abs(err))), 1e-8)
    div = mplcolors.TwoSlopeNorm(vmin=-lim, vcenter=0, vmax=lim)
    t = ax2.tripcolor(V[:, 0], V[:, 1], F, err,
                      cmap="coolwarm", norm=div, shading="gouraud")
    ax2.triplot(V[:, 0], V[:, 1], F, color=INK, lw=.12, alpha=.15)
    ax2.scatter([V[source, 0]], [V[source, 1]], marker="*",
                color=GOLD, s=70, edgecolor="white", linewidth=.5)
    ax2.set_title("Signed error and mesh triangles", loc="left")
    ax2.set_aspect("equal")
    decorate(ax2, "x (m)", "y (m)")
    fig.colorbar(t, ax=ax2, fraction=.047, pad=.04, label="Distance error (m)")
    fig.suptitle("Source-derived heat method • numerical surface, not a measured physical landscape",
                 y=1.0, fontsize=9.0, color=GREY)
    rows = [[int(i), float(V[i, 0]), float(V[i, 1]),
             float(result.distances[i]), float(truth[i]), float(err[i])]
            for i in range(len(V))]
    return save_record(fig, FIGURES[3], output,
                       ["vertex", "x_m", "y_m", "estimated_m", "exact_m", "error_m"],
                       rows, {
                           "type": "3D visualization of computed geodesic field",
                           "units": "metres",
                           "source": int(source),
                           "caveat": "Third visual axis represents computed distance, not mesh elevation.",
                       })


def resolution(output: Path) -> dict:
    cases = [9, 13, 17, 21, 25, 33]
    rows = []
    for n in cases:
        V,F,result,truth,source = field_data(n)
        err = np.abs(result.distances-truth)
        rows.append([n, len(V), len(F), 2.0/(n-1),
                     float(np.mean(err)), float(np.median(err)),
                     float(np.max(err))])
    fig, ax = plt.subplots(figsize=(6.75, 3.65))
    h = np.array([r[3] for r in rows])
    for column, color, label, marker in (
        (4, TEAL, "mean absolute error", "o"),
        (5, BLUE, "median absolute error", "s"),
        (6, GOLD, "maximum absolute error", "^"),
    ):
        values = np.array([r[column] for r in rows])
        ax.plot(h, values, color=color, lw=1.75, marker=marker,
                ms=5, label=label)
    ax.invert_xaxis()
    ax.set_title("Refining the grid: observed error, not a promised order", loc="left")
    decorate(ax, "Grid spacing h (m)", "Absolute geodesic error (m)")
    ax.legend(loc="best")
    for r in rows:
        ax.annotate(f"{r[0]}²", (r[3], r[4]), xytext=(0, 7),
                    textcoords="offset points", ha="center", fontsize=7.2)
    fig.text(.12, .005, "Same 2 m × 2 m planar domain; t = mean(edge length)²; "
             "edge-size dependency is shown, not runtime.",
             fontsize=7.6, color=GREY)
    return save_record(fig, FIGURES[4], output,
                       ["grid_n", "vertices", "triangles", "grid_spacing_m",
                        "mean_absolute_error_m", "median_absolute_error_m",
                        "max_absolute_error_m"], rows, {
                            "type": "computed resolution sensitivity study",
                            "units": "metres",
                            "grid_sizes": cases,
                            "caveat": "No guaranteed monotonic convergence order is inferred from a finite grid sweep.",
                        })


def robust_fit(output: Path) -> dict:
    x, y, true = synthetic_curve()
    ours = damped_huber_fit(x, y)
    ref = scipy_fit_reference(x, y)
    standard = least_squares(curve_residual, np.array([1., -.2, .1]),
                             jac=curve_jacobian, args=(x, y), max_nfev=5000)
    xx = np.linspace(x.min(), x.max(), 250)
    fig, axs = plt.subplots(1, 2, figsize=(8.6, 3.3),
                            gridspec_kw={"width_ratios": [1.4, 1.]})
    axs[0].scatter(x, y, s=17, c=BLUE, label="35 generated observations", zorder=3)
    axs[0].scatter([x[13]], [y[13]], s=65, marker="x", c=RED,
                   lw=1.6, label="declared injected outlier", zorder=6)
    for p, color, lab, ls in (
        (true, INK, "generating curve", ":"),
        (standard.x, GOLD, "ordinary least squares", "--"),
        (ours.parameters, TEAL, "custom Huber/LM-like", "-"),
        (ref.x, BLUE, "SciPy Huber reference", "-."),
    ):
        axs[0].plot(xx, curve_model(p, xx), lw=1.6, c=color, ls=ls, label=lab)
    axs[0].set_title("One gross outlier changes the objective", loc="left")
    decorate(axs[0], "x (synthetic units)", "y (synthetic units)")
    axs[0].legend(loc="upper right", fontsize=6.9)
    residuals = [np.abs(curve_residual(p, x, y)) for p in (
        standard.x, ours.parameters, ref.x)]
    axs[1].boxplot(residuals, tick_labels=["ordinary", "custom robust", "SciPy robust"],
                   patch_artist=True, showfliers=True,
                   boxprops={"facecolor": "#E9EDF2", "edgecolor": INK},
                   medianprops={"color": GOLD},
                   whiskerprops={"color": INK},
                   capprops={"color": INK},
                   flierprops={"markeredgecolor": RED, "markersize": 5})
    axs[1].set_yscale("symlog", linthresh=.025)
    axs[1].set_title("Absolute residuals over all observations", loc="left")
    decorate(axs[1], "", "|predicted − observed|")
    axs[1].tick_params(axis="x", labelrotation=14)
    rows = [[int(i), float(x[i]), float(y[i]), float(curve_model(true,x[i])),
             float(curve_model(standard.x,x[i])), float(curve_model(ours.parameters,x[i])),
             float(curve_model(ref.x,x[i]))] for i in range(len(x))]
    return save_record(fig, FIGURES[5], output,
                       ["index", "x", "observed", "generator_truth",
                        "ordinary_fit", "custom_huber_fit", "scipy_huber_fit"], rows, {
                            "type": "synthetic nonlinear fit with single outlier",
                            "units": "synthetic observation units",
                            "seed": None,
                            "true_parameters": list(map(float, true)),
                            "custom_parameters": list(map(float, ours.parameters)),
                            "scipy_reference_parameters": list(map(float, ref.x)),
                            "caveat": "Synthetic model fit; not physical measurements or Ceres parity.",
                        })


def equal_budget_estimates(total: int, seed: int) -> tuple[float, float, float]:
    """Same function evaluation budget for each of three estimators.

    Uniform: total uniforms; q2: total tilted samples; MIS: total/2
    samples from each. Distinct local RNG streams, same declared seed.
    """
    if total < 4 or total % 2:
        raise ValueError("budget must be even and >=4")
    rng = np.random.default_rng(seed)
    u = rng.random(total)
    q = (np.sqrt(1 + 8*rng.random(total)) - 1)/2
    uniform = float(np.mean(np.exp(u)))
    tilted = float(np.mean(np.exp(q) / (.5+q)))
    half = total // 2
    v = np.concatenate([u[:half], q[:half]])
    balanced = float(np.mean(np.exp(v) / (.75+.5*v)))
    return uniform, tilted, balanced


def mis(output: Path) -> dict:
    budgets = [40, 80, 160, 320, 640]
    reps = 90
    key_names = ["uniform", "tilted", "balanced"]
    labels = ["uniform", "tilted proposal", "balance MIS"]
    color_values = [GOLD, BLUE, TEAL]
    fig, axs = plt.subplots(1, 2, figsize=(8.3, 3.35))
    rows = []
    rates = {key: [] for key in key_names}
    truth = math.e - 1
    for n in budgets:
        observed = np.array([equal_budget_estimates(n, 92000+i)
                             for i in range(reps)])
        for j, key in enumerate(key_names):
            vals = observed[:, j]
            rates[key].append((n, float(np.var(vals, ddof=1))))
            for i, v in enumerate(vals):
                rows.append([n, 92000+i, key, float(v), float(v-truth)])
    for key, label, color in zip(key_names, labels, color_values):
        vals = [r[4] for r in rows if r[0] == 160 and r[2] == key]
        axs[0].hist(vals, bins=22, histtype="step", lw=1.65,
                    color=color, label=label)
        xdata, ydata = zip(*rates[key])
        axs[1].plot(xdata, ydata, color=color, lw=1.7, marker="o",
                    ms=4.5, label=label)
    axs[0].axvline(0, color=INK, lw=1, ls=":")
    axs[0].set_title("Signed error distribution • 160 evaluations", loc="left")
    decorate(axs[0], "Estimate − (e − 1)", "Independent replicates")
    axs[1].set_xscale("log", base=2)
    axs[1].set_yscale("log")
    # Explicit ticks are essential on a log budget axis: MaxNLocator can\n    # produce unreadable crowded scientific notation after log scaling.\n    axs[1].set_xticks(budgets)\n    axs[1].set_xticklabels([str(x) for x in budgets], fontsize=7.4)\n    axs[1].set_xlim(35, 720)
    axs[1].set_title("Variance vs equal sample budget", loc="left")
    decorate(axs[1], "Integrand evaluations per replicate", "Sample variance of estimates")
    axs[1].legend(loc="upper right", fontsize=7.6)
    fig.suptitle("90 independent pseudorandom streams per budget • no timing claims",
                 y=1.025, fontsize=8.7, color=GREY)
    return save_record(fig, FIGURES[6], output,
                       ["budget_evaluations", "seed", "method",
                        "estimate", "signed_error"], rows, {
                            "type": "synthetic Monte Carlo replicate distributions",
                            "units": "integral of exp(x) over [0,1]",
                            "seed_first": 92000,
                            "independent_replicates": reps,
                            "true_integral": truth,
                            "budgets": budgets,
                            "caveat": "Equal function-evaluation counts; not computational time, renderer performance, or original MIS paper benchmark.",
                        })


PLOTS = {
    FIGURES[0]: orientation,
    FIGURES[1]: ransac,
    FIGURES[2]: geodesic,
    FIGURES[3]: surface3d,
    FIGURES[4]: resolution,
    FIGURES[5]: robust_fit,
    FIGURES[6]: mis,
}


def generate(output: Path, selected: tuple[str, ...] = FIGURES) -> dict:
    if not selected or len(set(selected)) != len(selected) or any(x not in PLOTS for x in selected):
        raise ValueError("Requested figure list must contain unique known names.")
    output = Path(output).resolve()
    style()
    records = [PLOTS[name](output) for name in selected]
    # This eighth *content bridge* plate is analytically checked against
    # exact rational clipping rather than any GPU rasterization backend.
    # Generate only for the complete book atlas; subset experiments retain
    # their established seven-figure CLI semantics.
    coverage_info = None
    if set(selected) == set(FIGURES):
        from book02_coverage_visual import render as render_coverage
        coverage_info = render_coverage(output / "coverage")
        if coverage_info["exact_fraction"] != "119/192":
            raise RuntimeError("Book Two pixel-coverage reference drifted")
    manifest = {
        "schema": "allmight-book02-atlas-1",
        "provenance": "deterministic synthetic/analytic research-method teaching labs",
        "numpy": np.__version__,
        "scipy": scipy.__version__,
        "matplotlib": matplotlib.__version__,
        "figures": records,
        "coverage_bridge": coverage_info,
        "limitations": [
            "Figures reproduce tested tutorial algorithms, not published paper benchmarks.",
            "No claims of GPU/CPU speedup, original dataset reproduction or physical measurements.",
            "Heat/geodesic errors use a flat convex rectangular Euclidean-distance oracle.",
            "Seeds reproduce pseudorandom streams; publication-level inference needs independent validation.",
        ],
    }
    file = output / "manifest.json"
    file.write_text(json.dumps(manifest, indent=2, sort_keys=True,
                               allow_nan=False) + "\n", encoding="utf-8")
    # Standalone offline interactive atlas is built from those same exact
    # exported rows, never from a separately invented visualization dataset.
    from book02_build_viewer import build_viewer
    build_viewer(output)
    return manifest


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", required=True, type=Path,
                        help="Destination folder for PDF/SVG/PNG, CSV and manifest.")
    parser.add_argument("--figures", nargs="*", choices=FIGURES,
                        default=None,
                        help="Optional subset; default renders all figures.")
    args = parser.parse_args()
    selected = tuple(args.figures) if args.figures else FIGURES
    report = generate(args.output, selected)
    print(f"BOOK02_ATLAS_OK figures={len(report['figures'])} "
          f"output={args.output.resolve()}")
    for item in report["figures"]:
        print(f"{item['name']}: {item['rows']} data rows "
              f"sha256={item['data_sha256'][:14]}")


if __name__ == "__main__":
    main()
