#!/usr/bin/env python3
"""Book Two: analytic figure equivalents for the reflowable EPUB.

Print uses the five original equation-driven PGFPlots illustrations. This
generator evaluates *the same source equations* into accessible SVG/PNG
variants that EPUB readers can reflow. Each plot is analytical, not empirical.
The original TeX plots and source paragraphs are never edited.
"""
from __future__ import annotations

import argparse
import json
import math
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np

NAMES = [
    "01-linear-transformation",
    "02-gaussian-uncertainty",
    "03-optimization-landscape",
    "04-heat-diffusion",
    "05-convergence-study",
]
INK = "#14273D"
CYAN = "#087D94"
GOLD = "#C78623"
MUTED = "#63758C"


def save(fig, folder: Path, name: str, statement: str) -> dict:
    folder.mkdir(parents=True, exist_ok=True)
    fig.tight_layout(pad=1.3)
    for ext in ("svg", "png"):
        path=folder/(name+"."+ext)
        if ext == "svg":
            fig.savefig(path, format=ext, bbox_inches="tight", pad_inches=.12)
        else:
            fig.savefig(path, format=ext, bbox_inches="tight", pad_inches=.12, dpi=180)
        if path.stat().st_size < 750:
            raise RuntimeError("Unusable exported illustration "+str(path))
    plt.close(fig)
    return {"name":name, "equations":statement, "format":["svg","png"],
            "kind":"exact analytical plot (not measured data)"}


def render(folder: Path) -> dict:
    folder=Path(folder)
    plt.rcParams.update({"font.family":"DejaVu Sans","font.size":9,
                         "axes.titlesize":11,"axes.labelcolor":INK,
                         "text.color":INK,"axes.edgecolor":"#BFCAD7",
                         "grid.color":"#E3E9EF","svg.fonttype":"none"})
    results=[]
    theta=np.linspace(0,2*np.pi,361)
    circle=np.vstack((np.cos(theta),np.sin(theta)))
    A=np.array([[2.0,.6],[0.0,.45]])
    elliptical=A@circle
    fig,axes=plt.subplots(1,2,figsize=(8,3.35))
    for ax,coords,title,basis in [
        (axes[0],circle,"Unit circle",np.eye(2)),
        (axes[1],elliptical,"Transformed circle: A u",A),
    ]:
        ax.plot(*coords,c=CYAN,lw=2)
        ax.arrow(0,0,*basis[:,0],color=GOLD,width=.012,head_width=.11,length_includes_head=True)
        ax.arrow(0,0,*basis[:,1],color=INK,width=.012,head_width=.11,length_includes_head=True)
        ax.set(xlim=(-2.8,2.8),ylim=(-2.0,2.0),xlabel="x",ylabel="y",title=title)
        ax.set_aspect("equal");ax.grid(alpha=.8)
    results.append(save(fig,folder,NAMES[0],"u=(cos theta,sin theta), A=[[2,.6],[0,.45]]"))

    x=np.linspace(-3,3,81);y=np.linspace(-2,2,75)
    X,Y=np.meshgrid(x,y); sigma_x=1.2;sigma_y=.55
    P=np.exp(-.5*((X/sigma_x)**2+(Y/sigma_y)**2))/(2*np.pi*sigma_x*sigma_y)
    fig=plt.figure(figsize=(8.3,3.4))
    ax=fig.add_subplot(121,projection="3d")
    ax.plot_surface(X,Y,P,cmap="viridis",linewidth=0,antialiased=False)
    ax.set(xlabel="x",ylabel="y",zlabel="density",title="Exact Gaussian density")
    ax.view_init(elev=28,azim=-52)
    ax2=fig.add_subplot(122)
    image=ax2.pcolormesh(X,Y,P,cmap="viridis",shading="auto")
    ax2.set(xlabel="x",ylabel="y",title="Same density, top down")
    ax2.set_aspect("equal")
    fig.colorbar(image,ax=ax2,label="Density (unit⁻²)",fraction=.045,pad=.04)
    results.append(save(fig,folder,NAMES[1],"p=exp(-.5[(x/1.2)^2+(y/.55)^2])/(2*pi*1.2*.55)"))

    x=np.linspace(-1.2,2.2,81);y=np.linspace(-1.3,1.2,71)
    X,Y=np.meshgrid(x,y);Z=(X-1)**2+4*(Y+.5)**2
    px,py=-1.0,1.0;points=[]
    for _ in range(5):
        points.append((px,py,(px-1)**2+4*(py+.5)**2))
        px-=.1*2*(px-1);py-=.1*8*(py+.5)
    pts=np.array(points)
    fig=plt.figure(figsize=(7.0,4.0));ax=fig.add_subplot(111,projection="3d")
    ax.plot_surface(X,Y,Z,cmap="viridis",alpha=.76,linewidth=0)
    ax.plot(pts[:,0],pts[:,1],pts[:,2],c=INK,marker="o",lw=1.8,ms=4,label="GD iterates")
    ax.scatter([1],[-.5],[0],marker="*",s=80,c=GOLD,label="exact minimizer")
    ax.set(xlabel="x",ylabel="y",zlabel="f(x,y)",title="Exact objective and deterministic descent")
    ax.view_init(elev=31,azim=-57)
    ax.legend(loc="upper right",fontsize=7)
    results.append(save(fig,folder,NAMES[2],"f=(x-1)^2+4(y+.5)^2, alpha=.1, start=(-1,1)"))

    x=np.linspace(0,1,80);y=np.linspace(0,1,80);X,Y=np.meshgrid(x,y)
    U=np.exp(-2*np.pi**2*.04)*np.sin(np.pi*X)*np.sin(np.pi*Y)
    fig=plt.figure(figsize=(8.3,3.6));ax=fig.add_subplot(121,projection="3d")
    ax.plot_surface(X,Y,U,cmap="viridis",linewidth=0)
    ax.set(xlabel="x",ylabel="y",zlabel="u",title="Exact heat mode at t=0.04")
    ax.view_init(elev=30,azim=-55)
    ax2=fig.add_subplot(122);image=ax2.pcolormesh(X,Y,U,cmap="viridis",vmin=0,vmax=float(np.max(U)))
    ax2.set(xlabel="x",ylabel="y",title="Temperature heatmap")
    ax2.set_aspect("equal")
    fig.colorbar(image,ax=ax2,label="u (dimensionless)",fraction=.046,pad=.04)
    results.append(save(fig,folder,NAMES[3],"u=exp(-2*pi^2*t)*sin(pi*x)*sin(pi*y), t=.04"))

    counts=np.array([2**i for i in range(1,9)]);h=1/counts
    errs=np.abs((1-h)**counts-np.exp(-1))
    fig,ax=plt.subplots(figsize=(7,3.6))
    ax.loglog(h,errs,"o-",c=CYAN,label="Forward-Euler analytic error")
    ax.loglog(h,.184*h,"--",c=GOLD,label="First-order guide: 0.184h")
    ax.set(xlabel="h = 1/N",ylabel="Absolute error at T=1",
           title="Exact convergence: forward Euler on y′ = −y")
    ax.grid(which="both",alpha=.6);ax.legend(fontsize=8)
    results.append(save(fig,folder,NAMES[4],"y_N=(1-1/N)^N, y(1)=exp(-1), N=2,4,...,256"))

    report={"schema":"book02-analytic-epub-1","figures":results}
    (folder/"analytical-manifest.json").write_text(
        json.dumps(report,indent=2,ensure_ascii=False)+"\n",encoding="utf-8")
    return report


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("--output",type=Path,required=True)
    args=p.parse_args()
    result=render(args.output)
    print(f"BOOK02_ANALYTIC_FIGURES_OK {len(result['figures'])} {args.output}")


if __name__=="__main__":
    main()
