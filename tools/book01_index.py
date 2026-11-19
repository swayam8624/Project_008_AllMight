#!/usr/bin/env python3
"""Create genuine page-level LaTeX index entries in source-preserving reader prose.

Markers are inserted only into non-code, non-display-math Markdown and are
resolved to \index{...} after Pandoc. makeindex then assigns actual PDF pages.
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
FENCE = re.compile(r"(?ms)^" + re.escape(chr(96) * 3) +
                   r"[^\n]*\n.*?^" + re.escape(chr(96) * 3) + r"[ \t]*$")
DISPLAY = re.compile(r"(?s)\$\$(.*?)\$\$")


def insert_index_tokens(source: str, chapter: int) -> tuple[str, list[dict]]:
    """Mark the first meaningful prose occurrence of each term per chapter."""
    entries = []
    terms = sorted(TERMS, key=len, reverse=True)
    pieces = []
    offset = 0
    occupied = set()

    def mark_nonmath(text: str) -> str:
        for term in terms:
            if term.casefold() in occupied:
                continue
            pattern = re.compile(r"(?i)(?<![A-Za-z])" +
                                 re.escape(term) + r"(?![A-Za-z])")
            match = pattern.search(text)
            if not match:
                continue
            index = len(entries) + 1
            token = f"ALLMIGHTINDEXCH{chapter:02d}TERM{index:03d}"
            # Space-before marker makes Pandoc retain it as ordinary text.
            text = text[:match.end()] + " " + token + text[match.end():]
            entries.append({"term": term, "token": token, "chapter": chapter})
            occupied.add(term.casefold())
        return text

    def prose(text: str) -> str:
        segments, start = [], 0
        for m in DISPLAY.finditer(text):
            segments.append(mark_nonmath(text[start:m.start()]))
            segments.append(m.group(0))
            start = m.end()
        segments.append(mark_nonmath(text[start:]))
        return "".join(segments)

    for match in FENCE.finditer(source):
        pieces.append(prose(source[offset:match.start()]))
        pieces.append(match.group(0))
        offset = match.end()
    pieces.append(prose(source[offset:]))
    return "".join(pieces), entries


def replace_index_tokens(tex: str, entries: list[dict]) -> str:
    for item in entries:
        count = tex.count(item["token"])
        if count != 1:
            raise ValueError("Index marker missing or duplicated: " + item["token"])
        tex = tex.replace(item["token"], r"\index{" + item["term"] + "}", 1)
    if "ALLMIGHTINDEXCH" in tex:
        raise ValueError("Unresolved index marker in TeX")
    return tex
