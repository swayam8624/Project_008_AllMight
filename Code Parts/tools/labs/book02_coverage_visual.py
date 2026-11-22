#!/usr/bin/env python3
"""Source-generated Book Two figure: exact coverage and sampling error.

Creates vector PDF/SVG, PNG and 60-replicate CSV using the exact rational
oracle from book02_coverage_oracle.py. Synthetic, not a GPU benchmark.
"""
from __future__ import annotations

import argparse
import csv
from fractions import Fraction as F
import json
import math
from pathlib import Path
from random import Random

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.patches import Polygon
import numpy as np

from book02_coverage_oracle import (
    EXAMPLE, clipped_triangle, exact_coverage, inside_triangle,
    sampled_coverage,
)

INK="#18304A"
CYAN="#087D94"
GOLD="#D38C26"
RED="#C15863"
NEUTRAL="#8290A0"


def render(output: Path) -> dict:
    output=Path(output)
    output.mkdir(parents=True,exist_ok=True)
    p=exact_coverage(EXAMPLE)
    area=float(p)
    original=np.array([[float(x),float(y)] for x,y in EXAMPLE])
    clipped=np.array([[float(x),float(y)] for x,y in clipped_triangle(EXAMPLE)])
    rng=Random(73013)
    samples=[(F.from_float(rng.random()),F.from_float(rng.random()))
             for _ in range(256)]
    hit=np.array([inside_triangle(EXAMPLE,pt) for pt in samples])
    pts=np.array([[float(x),float(y)] for x,y in samples])
    observed=hit.mean()
    n=256
    reps=60
    seed_first=140901
    estimates=np.array([float(sampled_coverage(EXAMPLE,n,seed_first+i*104729))
                        for i in range(reps)])
    errs=estimates-area
    theoretical=math.sqrt(area*(1-area)/n)
    empirical=float(errs.std(ddof=1))
    fig,ax=plt.subplots(1,2,figsize=(9.1,3.8))
    for a in ax:
        a.spines[["top","right"]].set_visible(False)
        a.tick_params(labelsize=8)
        a.grid(color="#E1E7EE",lw=.6,zorder=0)
    ax[0].add_patch(Polygon(original,closed=True,fill=False,
                            linestyle="--",lw=1.6,edgecolor=GOLD,
                            label="original triangle",zorder=3))
    ax[0].add_patch(Polygon(clipped,closed=True,facecolor=CYAN,alpha=.18,
                            edgecolor=CYAN,lw=1.7,label="exact intersection",
                            zorder=3))
    ax[0].scatter(pts[~hit,0],pts[~hit,1],s=8,c=RED,alpha=.65,
                  label="miss",zorder=4)
    ax[0].scatter(pts[hit,0],pts[hit,1],s=8,c=CYAN,alpha=.7,
                  label="hit",zorder=4)
    ax[0].set(xlim=(-.35,1.35),ylim=(-.05,1.35),
              title="Triangle and one pixel • exact geometry",
              xlabel="pixel x (unit-square)",ylabel="pixel y (unit-square)")
    ax[0].set_aspect("equal",adjustable="box")
    ax[0].legend(fontsize=7,loc="upper right")
    ax[0].text(.02,-.18,
        f"Exact area = 119/192 ≈ {area:.6f}; 256-point sample = {observed:.6f}",
        transform=ax[0].transAxes,fontsize=8.0,color=INK)
    ax[1].hist(errs,bins=14,color=CYAN,edgecolor="white",alpha=.85)
    ax[1].axvline(0,color=INK,linestyle="--",lw=1.2,label="exact area")
    for v in (-theoretical,theoretical):
        ax[1].axvline(v,color=GOLD,ls=":",lw=1.3)
    ax[1].set(title="Signed coverage error • 60 independent seeds",
              xlabel="sampled coverage − 119/192",ylabel="replication count")
    ax[1].legend(fontsize=7)
    ax[1].text(.02,-.18,
        f"Theoretical σ = {theoretical:.5f}; measured σ = {empirical:.5f}",
        transform=ax[1].transAxes,fontsize=8,color=INK)
    fig.suptitle("From exact triangle geometry to a stochastic pixel estimate",
                 fontsize=11.2,fontweight="bold",color=INK)
    fig.subplots_adjust(bottom=.18,wspace=.24,top=.83)
    for fmt in ("pdf","svg","png"):
        kwargs={"bbox_inches":"tight","pad_inches":.12}
        if fmt=="png":
            kwargs["dpi"]=200
        fig.savefig(output/("triangle-pixel-coverage."+fmt),**kwargs)
    plt.close(fig)
    with (output/"coverage-replicates.csv").open("w",encoding="utf-8",newline="") as f:
        writer=csv.writer(f)
        writer.writerow(["seed","samples","exact_area","estimated_area","signed_error"])
        for i,value in enumerate(estimates):
            writer.writerow([seed_first+i*104729,n,format(area,".17g"),
                            format(float(value),".17g"),
                            format(float(value-area),".17g")])
    result={"schema":"book02-pixel-coverage-content-1",
            "kind":"synthetic exact geometry vs Bernoulli sampling",
            "exact_fraction":str(p),"exact_decimal":area,
            "source_triangle":[[str(x),str(y)] for x,y in EXAMPLE],
            "clipped_vertices":[[str(x),str(y)] for x,y in clipped_triangle(EXAMPLE)],
            "single_draw_seed":73013,"single_draw_n":256,
            "single_draw_estimate":float(observed),
            "replication_seed_first":seed_first,"replicate_count":reps,
            "n_per_replication":n,
            "theoretical_stddev":theoretical,"empirical_stddev":empirical,
            "caveat":"No hardware timing, no production MSAA and no raster edge-ownership guarantee."}
    (output/"coverage-provenance.json").write_text(
        json.dumps(result,indent=2)+"\n",encoding="utf-8")
    return result


if __name__=="__main__":
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("--output",type=Path,required=True)
    args=p.parse_args()
    result=render(args.output)
    print("BOOK02_COVERAGE_FIGURE_OK",result["exact_fraction"],
          "replicates",result["replicate_count"])
