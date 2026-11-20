#!/usr/bin/env python3
"""Optional book-figure renderer. ALL scientific rows come from C++23 CSV.

This file never solves an equation, refits a model, or silently fabricates
numerical values. It only validates and plots exported workbench results.
"""
import argparse
import csv
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("csv", type=Path, help="A book02_workbench CSV export")
    parser.add_argument("--output", type=Path, default=Path("book02-figure.png"))
    args = parser.parse_args()

    import matplotlib.pyplot as plt

    with args.csv.open(encoding="utf-8", newline="") as data:
        reader = csv.DictReader(data)
        fields = set(reader.fieldnames or [])
        rows = list(reader)
    if not rows:
        raise ValueError("Empty numerical export: refusing an empty figure.")

    navy, cyan, gold = "#12253d", "#0081a3", "#c28017"
    plt.rcParams.update({"font.size": 10, "axes.spines.top": False,
                         "axes.spines.right": False, "savefig.dpi": 190})
    fig, ax = plt.subplots(figsize=(7.2, 4.4), layout="constrained")

    if fields == {"step", "x", "temperature"}:
        for step in (0, 5, 10, 15, 20):
            selection = [r for r in rows if int(r["step"]) == step]
            if not selection:
                raise ValueError(f"Missing native heat step {step}")
            ax.plot([float(r["x"]) for r in selection],
                    [float(r["temperature"]) for r in selection],
                    marker=".", label=f"step {step}")
        ax.set_xlabel("Position (domain units)")
        ax.set_ylabel("Temperature (arbitrary units)")
        ax.set_title("C++23 explicit 1D heat diffusion")

    elif fields == {"x", "observed", "truth", "fit_huber",
                    "fit_least_squares", "is_outlier"}:
        x = [float(r["x"]) for r in rows]
        inliers = [r for r in rows if int(r["is_outlier"]) == 0]
        outliers = [r for r in rows if int(r["is_outlier"]) == 1]
        if len(outliers) != 1:
            raise ValueError("Expected one explicitly labeled synthetic outlier")
        ax.scatter([float(r["x"]) for r in inliers],
                   [float(r["observed"]) for r in inliers],
                   color=navy, marker="o", facecolors="none", label="Observations")
        ax.scatter([float(r["x"]) for r in outliers],
                   [float(r["observed"]) for r in outliers],
                   color="#a54b33", marker="x", s=68, linewidths=2,
                   label="Declared gross outlier")
        ax.plot(x, [float(r["truth"]) for r in rows],
                color=navy, linestyle=":", linewidth=2, label="Generating curve")
        ax.plot(x, [float(r["fit_huber"]) for r in rows],
                color=cyan, linewidth=2, label="Huber / C++23")
        ax.plot(x, [float(r["fit_least_squares"]) for r in rows],
                color=gold, linewidth=1.8, label="Ordinary LS / C++23")
        ax.set_xlabel("x (dimensionless)")
        ax.set_ylabel("Measurement y (arbitrary units)")
        ax.set_title("C++23 robust exponential fit under one known outlier")

    elif fields == {"delta", "coefficient_error",
                    "residual_rms", "qr_diagonal_ratio"}:
        x = [float(r["delta"]) for r in rows]
        errors = [float(r["coefficient_error"]) for r in rows]
        residuals = [float(r["residual_rms"]) for r in rows]
        ax.loglog(x, errors, color=cyan, marker="o",
                  label="Coefficient error vs known truth")
        ax.loglog(x, residuals, color=gold, marker="s",
                  label="Residual RMS (different quantity)")
        ax.invert_xaxis()
        ax.set_xlabel("Column separation delta (dimensionless)")
        ax.set_ylabel("Absolute error / residual, log scale")
        ax.set_title("C++23 pivoted QR: residual is not forward accuracy")
        ax.text(.02, .02, "Fixed +1e-10 observation perturbation",
                transform=ax.transAxes, fontsize=8, color=navy)
    else:
        raise ValueError(f"Unsupported C++23 CSV schema: {sorted(fields)}")

    ax.grid(alpha=.16)
    ax.legend(fontsize=8, frameon=False)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(args.output)
    plt.close(fig)


if __name__ == "__main__":
    main()
