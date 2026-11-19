#!/usr/bin/env python3
"""Reader-only editorial cleanup for the Book One ebook.

Source preservation is non-negotiable: this function operates on derived
reader Markdown after the exact chapter/addition provenance audit. It never
modifies the continuous master, project inserts or enrichment manuscripts.

Remove scaffolding the reader must not see; keep actual worked explanations,
failure/capability caveats, code, equations and scientific evidence.
"""
from __future__ import annotations

from collections import Counter
import re

# Both supported Markdown fence forms are protected from text transforms.
MARKDOWN_FENCE = re.compile(
    r"(?ms)^(?P<mark>[" + chr(96) + r"~]{3,})[^\n]*\n.*?^(?P=mark)[ \t]*$"
)
ADDITIVE_MARKER = re.compile(
    r"(?m)^[ \t]*<!(?:--|–)\s*ALLMIGHT-CPP23-"
    r"[A-Z0-9-]+-ADDITIVE-(?:START|END)\s*(?:--|–)>[ \t]*(?:\n|$)",
    re.I,
)
# Avoid publication-only author notes; these paragraphs describe composition,
# note counts and author-controlled build workflow rather than a teaching rule.
EDITORIAL_ONLY_STARTS = (
    r"The original Phase\s+\d+\s+(?:curriculum\s+)?assigns\b",
    r"The original curriculum sequence confirms\b",
    r"The original Chapter\s+\d+\s+(?:already|contains|is)\b",
    r"The existing Chapter\s+\d+\s+(?:already|contains|has)\b",
    r"Continue the original (?:Chapter|chapter)\b",
    r"Integration and source boundary\b",
    r"The exact Git blob revisions\b",
    r"Because this pass is committed through a connected source repository\b",
    r"This is M\d+\s*,\s*the final Phase\b",
    r"This new subject volume opens Phase \d+\b",
    r"We return immediately to\s+[“\"]REPRODUCIBILITY\b",
)
EDITORIAL_ONLY = re.compile(
    r"^(?:" + "|".join(EDITORIAL_ONLY_STARTS) + r")", re.I,
)
AUTHOR_SENTENCES = (
    re.compile(
        r"The user requested no intermediate commands or LaTeX regeneration,"
        r" so both remain deferred\.?\s*", re.I
    ),
    re.compile(
        r"It is staged in the same additive manuscript workflow and does not"
        r" shorten, rewrite, regenerate or compile the original"
        r" multi-thousand-page book\.?\s*", re.I
    ),
)
PHRASES = (
    (r"\bthe original chapter\b", "the earlier discussion"),
    (r"\bthe existing chapter\b", "the earlier discussion"),
    (r"\bthe original Chapter\s+\d+\b", "the earlier discussion"),
    (r"\bthe existing Chapter\s+\d+\b", "the earlier discussion"),
    (r"\bFor this pass\b", "For this laboratory"),
    (r"\bIn this pass\b", "In this laboratory"),
    (r"\bThis pass adds\b", "The companion laboratory includes"),
    (r"\bThis pass establishes\b", "This laboratory establishes"),
    (r"\bThis pass therefore changes\b", "This laboratory changes"),
    (r"\bThe Pass \d+ sample\b", "The following sample"),
    (r"\bThe Phase \d+ lab\b", "The corresponding lab"),
    (r"\bPhase 2 owner\b", "earlier resource owner"),
    (r"\bPass \d{1,2} application\b", "companion application"),
    (r"\bthis pass\b", "this laboratory"),
)
PHRASE_PATTERNS = [(re.compile(p, re.I), v) for p, v in PHRASES]
PRODUCTION_LEAK = re.compile(
    r"ALLMIGHT-CPP23-[A-Z0-9-]+-ADDITIVE-(?:START|END)"
    r"|the user has reserved|the user requested no intermediate"
    r"|the next engineering pass should compile",
    re.I,
)


def clean_reader_markdown(markdown: str, chapter: int) -> tuple[str, dict]:
    """Return polished reader Markdown and auditable counts.

    Only prose outside fenced blocks is edited; display-math markers and all
    original code payloads must be byte-identical after this operation.
    """
    stats: Counter[str] = Counter()
    fenced_before = [m.group(0) for m in MARKDOWN_FENCE.finditer(markdown)]
    math_before = re.findall(r"ALLMIGHTMATHCH\d+INDEX\d+", markdown)
    figure_before = re.findall(r"ALLMIGHTFIGURECH\d+INDEX\d+", markdown)

    def prose(fragment: str) -> str:
        def clear_marker(m: re.Match[str]) -> str:
            stats["authoring_markers_removed"] += 1
            return "\n"
        fragment = ADDITIVE_MARKER.sub(clear_marker, fragment)
        paragraphs = re.split(r"(\n[ \t]*\n+)", fragment)
        result = []
        for item in paragraphs:
            if not item or not item.strip():
                result.append(item)
                continue
            normalized = item.strip().lstrip("*_# ").strip()
            if (EDITORIAL_ONLY.match(normalized)
                    or (normalized.startswith("The CI jobs also do not "
                                              "generate or compile LaTeX.")
                        and "The user has reserved" in item)):
                stats["authoring_paragraphs_removed"] += 1
                result.append("")
                continue
            for rule in AUTHOR_SENTENCES:
                item, count = rule.subn("", item)
                stats["authoring_sentences_removed"] += count
            for rule, replacement in PHRASE_PATTERNS:
                def replace_with_case(match: re.Match[str]) -> str:
                    if (match.group(0)[0].isupper()
                            and replacement.startswith("the earlier")):
                        return "The" + replacement[3:]
                    return replacement
                item, count = rule.subn(replace_with_case, item)
                stats["reader_phrases_rewritten"] += count
            result.append(item)
        return "".join(result)

    fragments: list[str] = []
    cursor = 0
    for match in MARKDOWN_FENCE.finditer(markdown):
        fragments.append(prose(markdown[cursor:match.start()]))
        fragments.append(match.group(0))
        cursor = match.end()
    fragments.append(prose(markdown[cursor:]))
    cleaned = "".join(fragments)

    if [m.group(0) for m in MARKDOWN_FENCE.finditer(cleaned)] != fenced_before:
        raise ValueError(f"Book 1 chapter {chapter}: editorial cleanup changed source code")
    if re.findall(r"ALLMIGHTMATHCH\d+INDEX\d+", cleaned) != math_before:
        raise ValueError(f"Book 1 chapter {chapter}: editorial cleanup changed math")
    if re.findall(r"ALLMIGHTFIGURECH\d+INDEX\d+", cleaned) != figure_before:
        raise ValueError(f"Book 1 chapter {chapter}: editorial cleanup changed figures")
    # A literal code example may legitimately quote an authoring token.
    # The publication gate applies to visible prose, never to code payloads.
    if PRODUCTION_LEAK.search(MARKDOWN_FENCE.sub("", cleaned)):
        raise ValueError(
            f"Book 1 chapter {chapter}: development-only authoring text "
            "survived EPUB reader cleanup"
        )
    return cleaned, dict(stats)
