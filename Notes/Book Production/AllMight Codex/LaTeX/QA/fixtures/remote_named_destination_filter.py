#!/usr/bin/env python3
"""Compile and inspect a two-PDF LaTeX remote named-destination link."""
import hashlib
import json
import os
import subprocess
import sys
from pathlib import Path

from pypdf import PdfReader

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "QA" / "remote-destination-proof"
TEX_BIN = Path("/usr/local/texlive/2025/bin/universal-darwin")


def run(command, *, cwd, env, input_bytes=None):
    result = subprocess.run(command, cwd=cwd, env=env, input=input_bytes,
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    if result.returncode:
        raise RuntimeError(f"command failed ({result.returncode}): {command}\n"
                           + result.stdout.decode(errors="replace")[-5000:])
    return result.stdout.decode(errors="replace")


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def destination_name(value):
    try:
        value = value.get_object()
    except AttributeError:
        pass
    if isinstance(value, (list, tuple)) and value:
        value = value[0]
        try:
            value = value.get_object()
        except AttributeError:
            pass
    name = str(value).lstrip("/#")
    return name.removeprefix("nameddest=")


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    env = os.environ.copy()
    env["PATH"] = str(TEX_BIN) + os.pathsep + env.get("PATH", "")
    env.setdefault("TEXMFHOME", "/tmp/allmight-local-texmf")
    env["TEXINPUTS"] = str(ROOT) + "//:" + env.get("TEXINPUTS", "")
    filter_path = ROOT / "Tools" / "semantic-callouts.lua"

    target_md = b"# Geometry Basics {#geometry-basics}\n\nNamed target.\n"
    target_tex_body = run(["pandoc", "-f", "markdown", "-t", "latex",
                           "--lua-filter=" + str(filter_path)],
                          cwd=OUT, env=env, input_bytes=target_md)
    target_tex = OUT / "stage2.tex"
    target_tex.write_text("\\documentclass{article}\n\\usepackage{hyperref}\n"
                          "\\begin{document}\n" + target_tex_body
                          + "\\end{document}\n", encoding="utf-8")
    run(["lualatex", "-interaction=nonstopmode", "-halt-on-error", "-file-line-error",
         "-output-directory=.", "stage2.tex"], cwd=OUT, env=env)

    source_md = b"# Source Book {#source-book}\n\n[Jump to geometry](stage2.pdf#nameddest=geometry-basics).\n"
    source_tex_body = run(["pandoc", "-f", "markdown", "-t", "latex",
                           "--lua-filter=" + str(filter_path)],
                          cwd=OUT, env=env, input_bytes=source_md)
    html = run(["pandoc", "-f", "markdown", "-t", "html",
                "--lua-filter=" + str(filter_path)],
               cwd=OUT, env=env, input_bytes=source_md)
    source_tex = OUT / "stage1.tex"
    source_tex.write_text("\\documentclass{article}\n\\usepackage{hyperref}\n"
                          "\\begin{document}\n" + source_tex_body
                          + "\\end{document}\n", encoding="utf-8")
    run(["lualatex", "-interaction=nonstopmode", "-halt-on-error", "-file-line-error",
         "-output-directory=.", "stage1.tex"], cwd=OUT, env=env)

    source_pdf = PdfReader(str(OUT / "stage1.pdf"), strict=False)
    target_pdf = PdfReader(str(OUT / "stage2.pdf"), strict=False)
    target_names = {str(name).lstrip("/#") for name in target_pdf.named_destinations}
    actions = []
    for page in source_pdf.pages:
        for annotation_ref in page.get("/Annots", []):
            annotation = annotation_ref.get_object()
            action = annotation.get("/A", {}).get_object()
            if action.get("/S") == "/GoToR":
                raw_destination = action.get("/D")
                name = destination_name(raw_destination)
                actions.append({"file": str(action.get("/F")),
                                "destination_raw": str(raw_destination),
                                "destination_name": name})
    assert "geometry-basics" in target_names, target_names
    assert len(actions) == 1, actions
    assert "stage2.pdf" in actions[0]["file"], actions
    assert actions[0]["destination_name"] == "geometry-basics", actions
    assert "nameddest=" not in actions[0]["destination_raw"], actions
    assert "stage2.pdf#nameddest=geometry-basics" in html, html

    report = {
        "schema": "allmight-remote-named-destination-proof-v2",
        "generation_format": "LaTeX filter rewrites remote-PDF fragment only for LaTeX; HTML retains the semantic nameddest URI",
        "remote_pdf_count": 2,
        "destination_name": "geometry-basics",
        "target_page_count": len(target_pdf.pages),
        "source_page_count": len(source_pdf.pages),
        "destination_in_target_name_tree": True,
        "remote_gotor_actions": actions,
        "html_link_preserves_semantic_target": True,
        "pdf_sha256": {"stage1.pdf": sha(OUT / "stage1.pdf"),
                       "stage2.pdf": sha(OUT / "stage2.pdf")},
        "status": "pass",
    }
    (OUT / "verification.json").write_text(json.dumps(report, indent=2) + "\n",
                                            encoding="utf-8")
    print(json.dumps(report, indent=2))


if __name__ == "__main__":
    main()
