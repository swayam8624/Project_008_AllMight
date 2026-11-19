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
# Supplemental teaching stays separate from the original chapters and source
# generation. It is inserted into build copies at stable source boundaries.
FIGURES = {
    "01-foundations-i-ch017": ("12-array-contract-case.tex",),
    "01-foundations-i-ch018": ("13-refraction-contract-case.tex", "21-robust-predicates-research.tex"),
    "01-foundations-i-ch019": (
        "01-linear-transformation.tex", "07-metric-gradient-case.tex",
    ),
    "01-foundations-i-ch020": ("08-conditioning-case.tex",),
    "02-foundations-ii-ch021": ("14-projection-jacobian-case.tex", "22-cotangent-operators-research.tex"),
    "02-foundations-ii-ch022": ("15-differentiation-quadrature-case.tex",),
    "02-foundations-ii-ch023": ("16-bayes-base-rate-case.tex",),
    "02-foundations-ii-ch024": ("02-gaussian-uncertainty.tex",),
    "02-foundations-ii-ch025": (
        "04-heat-diffusion.tex", "09-heat-stability-case.tex",
        "23-heat-method-paper-lab.tex",
    ),
    "02-foundations-ii-ch026": ("17-matrix-exponential-case.tex",),
    "02-foundations-ii-ch027": (
        "03-optimization-landscape.tex", "10-gauge-nullspace-case.tex",
        "24-robust-least-squares-paper-lab.tex",
    ),
    "02-foundations-ii-ch028": ("18-bracketed-root-case.tex",),
    "02-foundations-ii-ch029": ("05-convergence-study.tex",),
    "02-foundations-ii-ch030": ("19-clustered-inference-case.tex", "27-multiple-importance-sampling.tex"),
    "02-foundations-ii-ch031": ("11-sampling-support-case.tex", "25-ransac-paper-lab.tex"),
    "03-visual-computing-i-ch012": ("20-sampling-measure-case.tex", "26-library-parity-and-limits.tex"),
}
PRELUDE_CHAPTER = "01-foundations-i-ch017"
PRELUDE_FILE = "06-reader-contract.tex"
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

    # The new entry must occur before the original first chapter, so the
    # mathematical objects/units contract precedes Python performance detail.
    prelude_input = re.compile(
        r"(\\input\{\.\./current/[^}\n]+/content/" +
        re.escape(PRELUDE_CHAPTER) + r"\.tex\})"
    )
    prelude_matches = list(prelude_input.finditer(source))
    if len(prelude_matches) > 1:
        raise RuntimeError("Ambiguous Book02 prelude anchor")
    if prelude_matches:
        source = prelude_input.sub(
            lambda m: rf"\input{{{FIGURE_DESTINATION}/{PRELUDE_FILE}}}" +
            "\n" + m.group(1), source
        )
        installed.insert(0, PRELUDE_FILE)

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
