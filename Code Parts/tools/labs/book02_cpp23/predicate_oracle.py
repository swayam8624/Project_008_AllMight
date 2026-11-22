#!/usr/bin/env python3
"""Independent exact-sign verification of the C++23 binary64 orientation CSV.

The executable is the algorithm being tested. Python Fraction is used ONLY as
an independent reference oracle on its hexadecimal-float input, not to provide
Book Two's numerical implementation or exported pictures.
"""
from __future__ import annotations

import csv
from fractions import Fraction
from pathlib import Path
import subprocess
import sys


def check(executable: Path) -> None:
    run = subprocess.run(
        [str(executable.resolve()), "orientation-audit"],
        text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=True
    )
    reader = csv.DictReader(run.stdout.splitlines())
    expected = ["ax", "ay", "bx", "by", "cx", "cy", "exact_sign", "filter_sign"]
    if reader.fieldnames != expected:
        raise AssertionError(f"Unexpected native CSV schema: {reader.fieldnames!r}")
    rows = list(reader)
    if len(rows) != 506:
        raise AssertionError(f"Expected six edge fixtures + 500 random triples, got {len(rows)}")
    boundary = [1, -1, 0, 1, 1, 1]
    for index, row in enumerate(rows):
        values = [float.fromhex(row[k]) for k in expected[:6]]
        ax, ay, bx, by, cx, cy = map(Fraction.from_float, values)
        # The subtraction and products here have unlimited rational precision.
        determinant = (ax - cx) * (by - cy) - (ay - cy) * (bx - cx)
        oracle = (determinant > 0) - (determinant < 0)
        native = int(row["exact_sign"])
        if oracle != native:
            raise AssertionError(
                f"Exact C++23 mismatch on row {index}: native {native}, oracle {oracle}, "
                f"input bits {row}"
            )
        if index < len(boundary) and oracle != boundary[index]:
            raise AssertionError(f"Unexpected boundary fixture sign at row {index}")
        if int(row["filter_sign"]) not in (-1, 0, 1, 2):
            raise AssertionError(f"Invalid approximate-filter classification at row {index}")
        # A separate exact antisymmetry identity, not dependent on float math.
        swapped = (ax - cx) * (cy - by) - (ay - cy) * (cx - bx)
        if ((swapped > 0) - (swapped < 0)) != -oracle:
            raise AssertionError(f"Antisymmetry violated at row {index}")
    print(f"BOOK02_EXACT_BINARY64_ORACLE_OK {len(rows)} triples")


if __name__ == "__main__":
    if len(sys.argv) != 2:
        raise SystemExit("Usage: predicate_oracle.py path/to/book02_workbench")
    check(Path(sys.argv[1]))
