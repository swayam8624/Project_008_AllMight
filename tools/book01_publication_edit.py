#!/usr/bin/env python3
"""Conservative reader-only editorial normalization for the academic edition.

The Chapter Edition and continuous master are never mutated. Every removed
production instruction is recorded in the generated provenance manifest.
This is deliberately not a blanket 'remove internal project terms' filter:
legitimate worked examples, evidence limitations and source claims remain.
"""
from __future__ import annotations
import re
from typing import Any

FENCE = re.compile(r"(?ms)^" + re.escape(chr(96) * 3) +
                   r"[^\n]*\n.*?^" + re.escape(chr(96) * 3) + r"[ \t]*$")
HEAD = re.compile(r"(?m)^(#{1,6})[ \t]+(.+?)\s*$")
EDITOR_SENTENCES = (
    re.compile(r"(?im)^This addition belongs (?:immediately |directly |before |after |at ).{15,600}?(?=\n\n|\Z)", re.S),
    re.compile(r"(?im)^It does not replace those explanations.{5,500}?(?=\n\n|\Z)", re.S),
    re.compile(r"(?im)^The user has reserved the final pull.{5,700}?(?=\n\n|\Z)", re.S),
)
MERGE_HEADING = re.compile(
    r"(?im)^#{1,6}\s*(?:Next\s*[—-]\s*Pass\s+\d+|"
    r"(?:Pass|Phase)\s+\d+\s*[:—-]\s*M\d+)"
    r"[^\n]*$", re.M,
)
REDUNDANT_TITLE = re.compile(
    r"(?m)^#\s+Computing Foundations\s*[—-]\s*From Bits to C\+\+ and Systems APIs\s*$"
)
FIGURE_LABEL = re.compile(
    r"(?m)^(?:#{1,5}\s*)?Figure\s+(?:B\d+|CPP[- ]\d+)[^\n]*\n"
    r"(?=\s*ALLMIGHTFIGURECH)"
)


def apply_reader_edits(source: str, chapter: int) -> tuple[str, dict[str, Any]]:
    """Normalize top-level headings, set aside unequivocal production notes."""
    notes: list[str] = []
    edits = {"chapter": chapter, "removed_editorial_notes": notes,
             "heading_level_adjustments": 0, "removed_duplicate_title": 0,
             "renamed_merge_headings": 0}
    source, n = REDUNDANT_TITLE.subn("", source)
    edits["removed_duplicate_title"] += n

    def edit_prose(fragment: str) -> str:
        nonlocal notes
        for regex in EDITOR_SENTENCES:
            def remove(match: re.Match[str]) -> str:
                notes.append(match.group(0).strip())
                return "\n"
            fragment = regex.sub(remove, fragment)
        def simplify_head(match: re.Match[str]) -> str:
            notes.append(match.group(0).strip())
            edits["renamed_merge_headings"] += 1
            return "### Continuation and technical validation"
        fragment = MERGE_HEADING.sub(simplify_head, fragment)
        return fragment

    # No edited prose regex may ever touch a literal fenced code block.
    parts: list[str] = []
    pos = 0
    for match in FENCE.finditer(source):
        parts.append(edit_prose(source[pos:match.start()]))
        parts.append(match.group(0))
        pos = match.end()
    parts.append(edit_prose(source[pos:]))
    source = "".join(parts)

    # Pandoc converts '# heading' to LaTeX section in a chapter fragment.
    # In the raw chapter edition the content often begins at ## or ###,
    # creating invalid 2.0.x section numbers. Shift the minimum encountered
    # heading level to '#'; preserve hierarchy and code fences.
    non_code = FENCE.sub("", source)
    depths = [len(m.group(1)) for m in HEAD.finditer(non_code)]
    shift = (min(depths) - 1) if depths else 0
    if shift:
        def promote(fragment: str) -> str:
            def fix(match: re.Match[str]) -> str:
                edits["heading_level_adjustments"] += 1
                return "#" * max(1, len(match.group(1)) - shift) + " " + match.group(2)
            return HEAD.sub(fix, fragment)
        parts, pos = [], 0
        for match in FENCE.finditer(source):
            parts.append(promote(source[pos:match.start()]))
            parts.append(match.group(0))
            pos = match.end()
        parts.append(promote(source[pos:]))
        source = "".join(parts)
    edits["heading_promotion_levels"] = shift
    return source, edits


def remove_duplicate_figure_labels(source: str) -> tuple[str, int]:
    """Remove duplicated public caption only immediately before a figure token.

    The token renderer includes the figure's visible numbered caption.
    A preceding draft figure label is redundant, not a separate explanation.
    """
    return FIGURE_LABEL.subn("", source)


def editorial_quality_report(source: str) -> list[str]:
    """Flag remaining potentially public-facing merge residue for human review."""
    patterns = [
        r"\bthe user has\b",
        r"\bthe supplied attachment\b",
        r"\bthe source attachment\b",
        r"\bSupplementary/Manual Integration\b",
        r"\bSources and Code Anchors\b",
        r"\bthe final pull\b",
        r"\bPass\s+\d{1,3}\b",
        r"\bPart\s+\d{3,4}\b",
        r"\bM\d{3}\b",
    ]
    out = []
    for pattern in patterns:
        count = len(re.findall(pattern, source, flags=re.I))
        if count:
            out.append(f"{pattern}: {count}")
    return out
