#!/usr/bin/env python3
"""Create genuine page-level LaTeX index entries in source-preserving reader prose.

Raw LaTeX index commands are inserted as isolated paragraph blocks outside
code and display math; Pandoc preserves them and makeindex assigns page numbers.
A reader's underlying C++/math text is unchanged.
"""
from __future__ import annotations
import re

TERMS = (
    "abstract machine", "address space", "alignment", "atomic operation",
    "atomics", "bandwidth", "barrier", "binary representation", "bit mask",
    "borrowing", "bounding volume hierarchy", "buffer lifetime", "cache",
    "cache locality", "compaction", "concepts", "const correctness",
    "coroutine", "C++23", "data race", "descriptor", "divergence",
    "endianness", "error handling", "expected", "floating point",
    "frame in flight", "generational handle", "generic programming",
    "GPU profiling", "instruction set", "interface", "invariant",
    "lifetime", "memory coalescing", "memory model", "memory ordering",
    "move semantics", "object lifetime", "object representation",
    "ownership", "pipeline barrier", "prefix scan", "quantization",
    "RAII", "radix sort", "resource management", "serialization",
    "SIMD", "SIMT", "subgroup", "synchronization", "threadgroup",
    "undefined behavior", "value category", "virtual memory",
    "Vulkan", "Metal", "workgroup",
)
FENCE = re.compile(r"(?ms)^(?P<marker>`{3,}|~{3,})[^\n]*\n"
                   r".*?^(?P=marker)[ \t]*$")
DISPLAY = re.compile(r"(?s)\$\$(.*?)\$\$")


def index_command(term: str) -> str:
    """Create the exact no-visible-text TeX index command for a term."""
    escaped = (term.replace("\\", r"\textbackslash{}")
                    .replace("&", r"\&").replace("#", r"\#")
                    .replace("%", r"\%").replace("_", r"\_")
                    .replace("{", r"\{").replace("}", r"\}"))
    return r"\index{" + escaped + "}"


def insert_index_tokens(source: str, chapter: int) -> tuple[str, list[dict]]:
    """Put one page-index command near each term's first prose paragraph.

    Pandoc can consume a marker inserted inside a link, emphasis or Markdown
    table cell without ever emitting it in LaTeX. We therefore emit a raw TeX
    index *block* before the containing prose paragraph. Pandoc preserves
    raw TeX blocks without modifying the original text's markup or words.
    Never index occurrences inside code fences or display mathematics.
    """
    entries: list[dict] = []
    indexed_terms: set[str] = set()
    terms = sorted(TERMS, key=len, reverse=True)

    def mark_prose(text: str) -> str:
        if not text:
            return text
        paragraphs = re.split(r"(\n[ \t]*\n+)", text)
        emitted: list[str] = []
        for fragment in paragraphs:
            if not fragment or not fragment.strip():
                emitted.append(fragment)
                continue
            # Ignore comment-only authoring sentinels, which do not appear
            # in a print edition and should not anchor an index entry.
            if fragment.lstrip().startswith("<!--"):
                emitted.append(fragment)
                continue
            # Markdown's four-space (or tab) indented code blocks are not
            # fenced. Never insert an index macro into their source lines.
            # Pandoc would print the literal "\\index{term}" inside code.
            nonblank = [line for line in fragment.splitlines() if line.strip()]
            if nonblank and all(re.match(r"^(?: {4,}|\t)", line)
                                for line in nonblank):
                emitted.append(fragment)
                continue
            page_terms: list[str] = []
            for term in terms:
                if term.casefold() in indexed_terms:
                    continue
                pattern = re.compile(r"(?i)(?<![A-Za-z])"
                                     + re.escape(term) + r"(?![A-Za-z])")
                if pattern.search(fragment):
                    indexed_terms.add(term.casefold())
                    page_terms.append(term)
                    entries.append({
                        "term": term, "chapter": chapter,
                        "command": index_command(term),
                    })
            if page_terms:
                # A separate raw-TeX block is preserved regardless of
                # Markdown's inline syntax in the following paragraph.
                emitted.append(
                    "\n\n"
                    + "\n\n".join(index_command(t) for t in page_terms)
                    + "\n\n" + fragment
                )
            else:
                emitted.append(fragment)
        return "".join(emitted)

    def preserve_math(text: str) -> str:
        parts: list[str] = []
        offset = 0
        for match in DISPLAY.finditer(text):
            parts.append(mark_prose(text[offset:match.start()]))
            parts.append(match.group(0))
            offset = match.end()
        parts.append(mark_prose(text[offset:]))
        return "".join(parts)

    pieces: list[str] = []
    offset = 0
    for match in FENCE.finditer(source):
        pieces.append(preserve_math(source[offset:match.start()]))
        pieces.append(match.group(0))
        offset = match.end()
    pieces.append(preserve_math(source[offset:]))
    return "".join(pieces), entries


def replace_index_tokens(tex: str, entries: list[dict]) -> str:
    """Verify Pandoc preserved every explicit TeX index entry once.

    The historical name is retained for the book builder's API. The new
    indexer no longer relies on fragile textual placeholders.
    """
    # Defense in depth: a corrupted or missed Markdown fence/indent must not
    # put a raw LaTeX indexing command inside a printable code example.
    verbatim = re.compile(r"\\begin\{verbatim\}.*?\\end\{verbatim\}", re.DOTALL)
    for listing in verbatim.finditer(tex):
        if re.search(r"\\index\{[^}]+\}", listing.group(0)):
            raise ValueError("Raw index macro leaked into a verbatim listing")
    for item in entries:
        command = item["command"]
        count = tex.count(command)
        if count != 1:
            raise ValueError(
                f"Index term {item['term']!r} in chapter {item['chapter']} "
                f"was emitted {count} times in LaTeX; expected exactly one"
            )
    if "ALLMIGHTINDEXCH" in tex:
        raise ValueError("Unresolved legacy index marker in TeX")
    return tex
