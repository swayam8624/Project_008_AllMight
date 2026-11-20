#!/usr/bin/env python3
"""Optional plotting only: the C++23 executable creates the numerical CSV."""
import argparse
import csv
from pathlib import Path

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("csv",type=Path)
    p.add_argument("--output",type=Path,default=Path("heat.png"))
    args=p.parse_args()
    import matplotlib.pyplot as plt
    with args.csv.open(encoding="utf-8",newline="") as f:
        rows=list(csv.DictReader(f))
    if not rows or set(rows[0])!={"step","x","temperature"}:
        raise ValueError("Expected C++23 output: step,x,temperature")
    for step in (0,5,10,15,20):
        selection=[r for r in rows if int(r["step"])==step]
        plt.plot([float(r["x"]) for r in selection],
                 [float(r["temperature"]) for r in selection],
                 label=f"step {step}")
    plt.xlabel("Position (domain units)")
    plt.ylabel("Temperature (arbitrary units)")
    plt.legend()
    plt.tight_layout()
    args.output.parent.mkdir(parents=True,exist_ok=True)
    plt.savefig(args.output,dpi=180)

if __name__=="__main__":
    main()
