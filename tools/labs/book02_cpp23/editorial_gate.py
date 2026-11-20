#!/usr/bin/env python3
"""Cheap repo-local guard: no TeX compilation, no false claims about full proof."""
from __future__ import annotations
from pathlib import Path

ROOT=Path(__file__).resolve().parents[3]
LATEX=ROOT/"Book Production"/"AllMight Codex"/"LaTeX"
ASSETS=LATEX/"Assets"/"Book02"
LAB=ROOT/"tools"/"labs"/"book02_cpp23"

def check() -> None:
    supplements=sorted(ASSETS.glob("[0-9][0-9]-*.tex"))
    assert len(supplements)==36, "Original 36-supplement contract changed"
    assert (LAB/"CMakeLists.txt").exists()
    cpp=(LAB/"src"/"main.cpp").read_text(encoding="utf-8")
    cmake=(LAB/"CMakeLists.txt").read_text(encoding="utf-8")
    assert "cxx_std_23" in cmake and "CXX_EXTENSIONS OFF" in cmake
    advanced=(LAB/"src"/"advanced.hpp").read_text(encoding="utf-8")
    for symbol in ("qr_least_squares", "fit_exponential_huber",
                   "residual_and_jacobian", "huber_objective",
                   "qr_diagonal_ratio"):
        assert symbol in advanced, f"Missing native advanced solver: {symbol}"
    geometry=(LAB/"src"/"geometry.hpp").read_text(encoding="utf-8")
    for symbol in ("ransac_line", "total_least_squares", "ideal_trials",
                   "synthetic_line_data", "inlier_rms"):
        assert symbol in geometry, f"Missing native geometry: {symbol}"
    for command in ('"qr-sensitivity"', '"robust-fit"', '"robust-data"',
                    '"ransac-summary"', '"ransac-data"'):
        assert command in cpp, f"Missing CLI exercise: {command}"
    for symbol in ("orient_exact_grid","orient_filtered","fit_line",
                   "heat_step","integrate","self_test"):
        assert symbol in cpp, f"Missing C++23 capability: {symbol}"
    style=(ASSETS/"preamble.tex").read_text(encoding="utf-8")
    for role in ("booktwoconcept","booktwoproof","booktwopitfall",
                 "booktwolab","renewenvironment{Shaded}"):
        assert role in style, f"Missing semantic visual style: {role}"
    checkpoint_names=(
        "06-reader-contract.tex","08-conditioning-case.tex",
        "09-heat-stability-case.tex","21-robust-predicates-research.tex",
        "24-robust-least-squares-paper-lab.tex",
        "27-multiple-importance-sampling.tex",
        "35-cumulative-numerical-geometry-atlas.tex")
    for name in checkpoint_names:
        content=(ASSETS/name).read_text(encoding="utf-8")
        assert "C++23" in content and "book02" in content, name
        for role in ("booktwoconcept","booktwopitfall","booktwolab"):
            assert content.count("\\begin{"+role+"}")==content.count(
                "\\end{"+role+"}"), f"Unbalanced {role} in {name}"
    assert "Householder QR instead of squaring" in (ASSETS/"08-conditioning-case.tex").read_text(encoding="utf-8")
    assert "An augmented QR derivation" in (ASSETS/"24-robust-least-squares-paper-lab.tex").read_text(encoding="utf-8")
    assert "Native extension: from a line" in (ASSETS/"35-cumulative-numerical-geometry-atlas.tex").read_text(encoding="utf-8")
    assert "Native consensus and orthogonal" in (ASSETS/"25-ransac-paper-lab.tex").read_text(encoding="utf-8")
    builder=(LATEX/"Tools"/"book02_release_candidates.py").read_text(encoding="utf-8")
    assert 'rglob("*")' in builder and '"book02_cpp23"' in builder
    print("BOOK02_CPP23_EDITORIAL_GATE_OK",
          len(checkpoint_names),"checkpoints",len(supplements),"supplements")

if __name__=="__main__":
    check()
