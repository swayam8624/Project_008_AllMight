#!/usr/bin/env python3
"""Inject reproducible Book 02 vector figures into a temporary LaTeX build copy.

The canonical Markdown masters and generation snapshots are never modified.
The figure placement is tied to stable source chapter IDs, not page numbers.
"""
from __future__ import annotations

import re
import shutil
from pathlib import Path

FIGURE_SOURCE = Path(__file__).resolve().parents[1] / "Assets" / "Book02"
FIGURE_DESTINATION = "book02-figures"

# Source chapter -> figure(s) presented immediately after that chapter's text.
FIGURES = {
    "01-foundations-i-ch019": ("01-linear-transformation.tex",),
    "02-foundations-ii-ch024": ("02-gaussian-uncertainty.tex",),
    "02-foundations-ii-ch025": ("04-heat-diffusion.tex",),
    "02-foundations-ii-ch027": ("03-optimization-landscape.tex",),
    "02-foundations-ii-ch029": ("05-convergence-study.tex",),
}
PACKAGE_MARKER = r"\usepackage[print]{allmight-book}"
STAGE_MARKER = r"\SetBookStage{2}{Mathematics for Visual Computing}"
INJECTION_MARKER = "% AllMight Book02 reproducible scientific figures"


def install_book02_visuals(tex_path: str | Path) -> list[str]:
    """Install figures without changing the authorial source or generated stage.

    Operates only on a copied main.tex in a proof/build directory.
    Returns installed figure filenames. A second invocation is idempotent.
    Other books are unaffected. Missing required source files are fatal.
    """
    tex_path = Path(tex_path)
    source = tex_path.read_text(encoding="utf-8")
    if STAGE_MARKER not in source:
        return []
    if INJECTION_MARKER in source:
        return []
    if source.count(PACKAGE_MARKER) != 1:
        raise RuntimeError("Book02 preamble anchor missing or ambiguous: " + str(tex_path))

    installed: list[str] = []
    for chapter, figure_names in FIGURES.items():
        # Generated stage-proofs and printed parts both use these content IDs.
        chapter_input = re.compile(
            r"(\\input\{\.\./current/[^}\n]+/content/" +
            re.escape(chapter) +
            r"\.tex\})"
        )
        matches = list(chapter_input.finditer(source))
        if len(matches) > 1:
            raise RuntimeError("Ambiguous chapter input for Book02 figure: " + chapter)
        if not matches:
            continue
        lines = "\n".join(
            rf"\input{{{FIGURE_DESTINATION}/{name}}}" for name in figure_names
        )
        source = chapter_input.sub(lambda m: m.group(1) + "\n" + lines, source)
        installed.extend(figure_names)

    if not installed:
        raise RuntimeError("Book02 has no matching chapter anchors in " + str(tex_path))
    root = tex_path.parent / FIGURE_DESTINATION
    root.mkdir(parents=True, exist_ok=True)
    for name in ("preamble.tex", *installed):
        source_file = FIGURE_SOURCE / name
        if not source_file.is_file():
            raise FileNotFoundError("Missing source-scientific figure: " + str(source_file))
        shutil.copy2(source_file, root / name)

    source = source.replace(
        PACKAGE_MARKER,
        PACKAGE_MARKER + "\n" + INJECTION_MARKER +
        "\n" + rf"\input{{{FIGURE_DESTINATION}/preamble.tex}}",
        1,
    )
    tex_path.write_text(source, encoding="utf-8")
    return installed
