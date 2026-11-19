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
HEAD = re.compile(r"(?m)^(#{1,6})[ \t]+(.+?)[ \t]*$")
EDITOR_SENTENCES = (
    re.compile(r"(?im)^This addition belongs (?:immediately |directly |before |after |at ).{15,600}?(?=\n\n|\Z)", re.S),
    re.compile(r"(?im)^It does not replace those explanations.{5,500}?(?=\n\n|\Z)", re.S),
    re.compile(r"(?im)^The user has reserved the final pull.{5,700}?(?=\n\n|\Z)", re.S),
    re.compile(r"(?im)^(?:This pass (?:begins|closes)|Pass \d{1,2} (?:begins|ends|closed|completes))\b.{5,650}?(?=\n\n|\Z)", re.S),
    re.compile(r"(?im)^The original source contract calls this\b.{5,480}?(?=\n\n|\Z)", re.S),
    re.compile(r"(?im)^This is an authored reconstruction of M\d+\b.{5,460}?(?=\n\n|\Z)", re.S),
    re.compile(r"(?im)^The exact curriculum ownership is:.{5,650}?(?=\n\n|\Z)", re.S),
    re.compile(r"(?im)^This addition belongs before the chapter.{5,500}?(?=\n\n|\Z)", re.S),
)
PUBLICATION_REPHRASES = (
    (r"(?i)\bthe source attachment's central project-shaped finding\b",
     "the central language-level finding"),
    (r"(?i)\bthe source attachment\b", "the earlier source example"),
    (r"(?i)\bthe attachment's\b", "the earlier example's"),
    (r"(?i)\bthe attachment\b", "the earlier example"),
    (r"(?i)\bthe supplied source notes\b", "The cited project notes"),
    (r"(?i)\bthe supplied source\b", "the cited project source"),
    (r"(?i)\bThis pass \*\*closes Phase \d+\*\*\.", 
     "This section completes the argument established so far."),
)
MERGE_HEADING = re.compile(
    r"(?im)^#{1,6}[ \t]*(?:Next\s*[—-]\s*Pass\s+\d+|"
    r"(?:Pass|Phase)\s+\d+\s*[:—-]\s*M\d+|\+\s*M\d{3})"
    r"[^\n]*$", re.M,
)
REDUNDANT_TITLE = re.compile(
    r"(?m)^#\s+Computing Foundations\s*[—-]\s*From Bits to C\+\+ and Systems APIs\s*$"
)
WIKILINK = re.compile(r"\[\[([^\]\n]+)\]\]")
PSEUDOCODE_FENCE = re.compile(
    r'(?m)^' + re.escape(chr(96) * 3) +
    r'cpp(?P<metadata>[^\n]*)\n(?=(?:for each item:|if useful:|GPU-naive:))'
)

INLINE_BUILD_COMMAND = re.compile(
    r"Compile with " + re.escape(chr(96)) +
    r"(?P<compile>clang\+\+[^" + re.escape(chr(96)) + r"\n]{35,240})" +
    re.escape(chr(96)) + r", then run " + re.escape(chr(96)) +
    r"(?P<run>[^" + re.escape(chr(96)) + r"\n]+)" +
    re.escape(chr(96)) + r"\."
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
             "renamed_merge_headings": 0, "normalized_wikilinks": 0,
             "relabeled_pseudocode": 0, "promoted_build_commands": 0}
    source, n = REDUNDANT_TITLE.subn("", source)
    edits["removed_duplicate_title"] += n
    # A small set of author-directed remarks are not learning content.
    # These are reader-only substitutions; canonical notes remain inspectable.
    source = source.replace(
        "one of the first mental separations I want permanently burned in",
        "one of the most important conceptual distinctions"
    )

    def edit_prose(fragment: str) -> str:
        nonlocal notes
        # Names of private attachments have no meaning to a published reader.
        # These are prose-only substitutions; code and math files are retained.
        for phrase, replacement in PUBLICATION_REPHRASES:
            fragment = re.sub(phrase, replacement, fragment)
        for regex in EDITOR_SENTENCES:
            def remove(match: re.Match[str]) -> str:
                notes.append(match.group(0).strip())
                return "\n"
            fragment = regex.sub(remove, fragment)
        def simplify_head(match: re.Match[str]) -> str:
            notes.append(match.group(0).strip())
            edits["renamed_merge_headings"] += 1
            return ""  # source heading describes the authoring pass, not the concept
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

    # The printed book has its own table of contents and index. An Obsidian
    # wikilink to a private note is not a public URL; retain its visible label
    # without promising a broken hyperlink. Canonical links remain in source.
    def link_label(match: re.Match[str]) -> str:
        target = match.group(1)
        label = target.rsplit("|", 1)[-1]
        if "|" not in target:
            label = label.rsplit("/", 1)[-1].split("#", 1)[-1]
        edits["normalized_wikilinks"] += 1
        return label
    pieces, pos = [], 0
    for match in FENCE.finditer(source):
        pieces.append(WIKILINK.sub(link_label, source[pos:match.start()]))
        pieces.append(match.group(0))
        pos = match.end()
    pieces.append(WIKILINK.sub(link_label, source[pos:]))
    source = "".join(pieces)
    # Identify deliberately schematic snippets, which are useful but should
    # never be advertised as compilable C++ source.
    source, labeled = PSEUDOCODE_FENCE.subn(
        lambda match: "**Pseudocode (not compilable C++):**\n\n"
        + chr(96) * 3 + "text" + match.group("metadata") + "\n",
        source,
    )
    edits["relabeled_pseudocode"] = labeled

    # A shell command exceeding the measure of the printed page must be a
    # copyable code block, not one long unbreakable inline text run.
    def command_as_listing(match: re.Match[str]) -> str:
        edits["promoted_build_commands"] += 1
        return ("Compile and run using the following commands:\n\n"
                + chr(96) * 3 + "sh\n"
                + match.group("compile") + "\n"
                + match.group("run") + "\n"
                + chr(96) * 3 + "\n")
    command_pieces, command_pos = [], 0
    for fenced in FENCE.finditer(source):
        command_pieces.append(INLINE_BUILD_COMMAND.sub(
            command_as_listing, source[command_pos:fenced.start()]))
        command_pieces.append(fenced.group(0))
        command_pos = fenced.end()
    command_pieces.append(INLINE_BUILD_COMMAND.sub(command_as_listing, source[command_pos:]))
    source = "".join(command_pieces)

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
