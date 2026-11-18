#!/usr/bin/env python3
"""Compare the exact binary32/binary64 table before and after fixed widths."""
import copy
import hashlib
import json
import os
import subprocess
import sys
from pathlib import Path

from pypdf import PdfReader

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT))
from Tools.convert import pandoc, set_binary_format_table_widths

SOURCE = Path(
    "/Users/swayamsingal/Library/Mobile Documents/iCloud~md~obsidian/Documents/Master/"
    "Chapter Edition/03 - Game Engine/01 - A game promise becomes bytes and running state.md"
)
EXPECTED_SHA256 = "fc461d5123a78ed27badcc6591db160ebd5e74b490260822566b5cfb40782392"
LATEXMK = "/usr/local/texlive/2025/bin/universal-darwin/latexmk"
OUT = ROOT / "QA" / "binary-table-regression"
PART_ID = "01-Bits-Programs-and-Reliable-Systems-P04"
AST_NAME = "05-engine-i-ch001.json"


def find_binary_table(value):
    if isinstance(value, dict):
        if value.get("t") == "Table":
            table = copy.deepcopy(value)
            if set_binary_format_table_widths(table):
                return table
        for child in value.values():
            found = find_binary_table(child)
            if found is not None:
                return found
    elif isinstance(value, list):
        for child in value:
            found = find_binary_table(child)
            if found is not None:
                return found
    return None


def compile_case(label, table):
    ast = {"pandoc-api-version": [1, 23, 1, 2], "meta": {}, "blocks": [table]}
    table_tex = pandoc(json.dumps(ast, ensure_ascii=False).encode(), "json", "latex").decode()
    tex = "\\documentclass[11pt]{book}\n\\usepackage[print]{allmight-book}\n\\begin{document}\n" + table_tex + "\n\\end{document}\n"
    case = OUT / label
    case.mkdir(parents=True, exist_ok=True)
    assets = case / "Assets"
    if assets.is_symlink() or assets.exists():
        if assets.is_symlink(): assets.unlink()
    assets.symlink_to(ROOT / "Assets", target_is_directory=True)
    (case / "table.tex").write_text(tex)
    env = dict(os.environ)
    env["PATH"] = str(Path(LATEXMK).parent) + os.pathsep + env.get("PATH", "")
    env["TEXINPUTS"] = str(ROOT) + ":" + env.get("TEXINPUTS", "")
    env["TEXMFHOME"] = "/tmp/allmight-local-texmf"
    run = subprocess.run(
        [LATEXMK, "-g", "-lualatex", "-interaction=nonstopmode", "-halt-on-error",
         "-file-line-error", "-outdir=build", "table.tex"],
        cwd=case, env=env, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
    )
    (case / "compile.log").write_text(run.stdout)
    return run.returncode, case / "build" / "table.pdf", run.stdout


def main():
    if hashlib.sha256(SOURCE.read_bytes()).hexdigest() != EXPECTED_SHA256:
        raise SystemExit("Source changed; review the hash-bound table fixture")
    generation = json.loads((ROOT / "CURRENT.json").read_text())["generation_path"]
    ast_path = ROOT / generation / PART_ID / "AST" / AST_NAME
    document = json.loads(ast_path.read_text())
    table = find_binary_table(document)
    if table is None:
        raise SystemExit("Could not find exact Property / binary32 / binary64 table")
    baseline = copy.deepcopy(table)
    baseline["c"][2] = [[{"t": "AlignDefault"}, {"t": "ColWidthDefault"}] for _ in range(3)]
    baseline_rc, baseline_pdf, baseline_log = compile_case("baseline", baseline)
    fixed_rc, fixed_pdf, fixed_log = compile_case("fixed-width", table)
    count = lambda log: sum("Overfull \\hbox" in line for line in log.splitlines())
    baseline_overfull, fixed_overfull = count(baseline_log), count(fixed_log)
    if baseline_rc or fixed_rc or not baseline_pdf.is_file() or not fixed_pdf.is_file():
        raise SystemExit(f"baseline rc={baseline_rc}, fixed rc={fixed_rc}")
    if baseline_overfull == 0 or fixed_overfull:
        raise SystemExit(f"expected baseline overflow and no fixed overflow; got {baseline_overfull} and {fixed_overfull}")
    text = " ".join(" ".join(page.extract_text() or "" for page in PdfReader(str(fixed_pdf)).pages).split())
    for token in ("Property", "binary32", "binary64", "Sign position", "Exponent positions", "−1022 through 1023"):
        if token not in text:
            raise SystemExit(f"Fixed table lost source text: {token}")
    print(f"PASS: baseline has {baseline_overfull} overfull hboxes; fixed-width table has 0; content retained in {fixed_pdf}")


if __name__ == "__main__":
    main()
