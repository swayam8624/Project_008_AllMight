#!/usr/bin/env python3
"""Compile the exact source table's first rows with invisible wrap points."""
import hashlib
import json
import os
import subprocess
import sys
from pathlib import Path

from pypdf import PdfReader

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT))
from Tools.convert import add_channel_membership_breakpoints

SOURCE = Path(
    "/Users/swayamsingal/Library/Mobile Documents/iCloud~md~obsidian/Documents/Master/"
    "Supplementary/Game Engine - Sources and Coverage.md"
)
EXPECTED_SHA256 = "83bd2d031c9151c9e16ac081659315fb8fcae88305963e6604aafdfa3c51e7db"
STYLE = ROOT / "allmight-book.sty"
LATEXMK = "/usr/local/texlive/2025/bin/universal-darwin/latexmk"
OUT = ROOT / "QA" / "table-breakpoint-regression"


def main():
    if hashlib.sha256(SOURCE.read_bytes()).hexdigest() != EXPECTED_SHA256:
        raise SystemExit("Source changed; the hash-bound table fixture must be reviewed")
    generation = json.loads((ROOT / "CURRENT.json").read_text())["generation_path"]
    generated = ROOT / generation / "01-Bits-Programs-and-Reliable-Systems-P04" / "content" / "appendix-877738f6f4d2.tex"
    adjusted, row_count = add_channel_membership_breakpoints(generated.read_text())
    rows = [line for line in adjusted.splitlines() if r"\{``feed'':" in line and " & " in line]
    if row_count != len(rows) or row_count < 3:
        raise SystemExit(f"Expected multiple metadata rows, got {row_count}")
    if not all(r"\allowbreak{}" in line for line in rows[:3]):
        raise SystemExit("Breakpoint insertion missing from representative rows")

    table_start = adjusted.rfind(r"\begin{longtable}", 0, adjusted.index("Channel / membership"))
    header_end = adjusted.index(r"\endhead", table_start) + len(r"\endhead")
    table_header = adjusted[table_start:header_end]
    document = (
        r"\documentclass[11pt]{book}" + "\n"
        + r"\usepackage[print]{allmight-book}" + "\n"
        + r"\begin{document}" + "\n"
        + table_header + "\n"
        + "\n".join(rows[:3]) + "\n"
        + r"\end{longtable}" + "\n"
        + r"\end{document}" + "\n"
    )
    OUT.mkdir(parents=True, exist_ok=True)
    tex = OUT / "table.tex"
    tex.write_text(document)
    env = dict(os.environ)
    env["PATH"] = str(Path(LATEXMK).parent) + os.pathsep + env.get("PATH", "")
    env["TEXINPUTS"] = str(ROOT) + ":" + env.get("TEXINPUTS", "")
    env["TEXMFHOME"] = "/tmp/allmight-local-texmf"
    run = subprocess.run(
        [LATEXMK, "-g", "-lualatex", "-interaction=nonstopmode", "-halt-on-error",
         "-file-line-error", "-outdir=build", "table.tex"],
        cwd=OUT, env=env, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
    )
    log = run.stdout
    (OUT / "compile.log").write_text(log)
    pdf = OUT / "build" / "table.pdf"
    overfull = [line for line in log.splitlines() if "Overfull \\hbox" in line]
    if run.returncode or not pdf.is_file() or overfull:
        raise SystemExit(f"compile returncode={run.returncode}, overfull={len(overfull)}; see {OUT / 'compile.log'}")
    extracted = "\n".join(page.extract_text() or "" for page in PdfReader(str(pdf)).pages)
    normalized = " ".join(extracted.split())
    if "core_playlist" not in normalized or "Kohi tutorial series" not in normalized:
        raise SystemExit("Rendered table text lost source content")
    print(f"PASS: {row_count} exact-source rows receive zero-width breakpoints; fixture has no overfull hboxes; {pdf}")


if __name__ == "__main__":
    main()
