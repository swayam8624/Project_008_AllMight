#!/usr/bin/env python3
from pathlib import Path
import re
import sys

path = Path(sys.argv[1])
lines = path.read_text(encoding="utf-8").splitlines()

paragraphs = []
buf = []
start = None
in_fence = False
fence = None
in_math = False
BACKTICK_FENCE = chr(96) * 3

def flush():
    global buf, start
    if not buf:
        return
    text = " ".join(x.strip() for x in buf).strip()
    if text:
        words = re.findall(r"\b[\w'+-]+\b", text)
        sentences = [s.strip() for s in re.split(r"(?<=[.!?])\s+", text) if s.strip()]
        longest = max((len(re.findall(r"\b[\w'+-]+\b", s)) for s in sentences), default=0)
        if len(words) >= 90 or longest >= 46:
            paragraphs.append((len(words), longest, start, text[:260]))
    buf = []
    start = None

for i, line in enumerate(lines, 1):
    stripped = line.strip()
    if stripped.startswith(BACKTICK_FENCE) or stripped.startswith("~~~"):
        flush()
        mark = stripped[:3]
        if not in_fence:
            in_fence, fence = True, mark
        elif mark == fence:
            in_fence, fence = False, None
        continue
    if in_fence:
        continue
    if stripped == "$$":
        flush()
        in_math = not in_math
        continue
    if in_math:
        continue
    if (not stripped or stripped.startswith("#") or stripped.startswith("|")
        or stripped.startswith("<!--") or stripped.startswith("> ")
        or re.match(r"^\s*[-*]\s+", line) or re.match(r"^\s*\d+[.)]\s+", line)):
        flush()
        continue
    if start is None:
        start = i
    buf.append(line)

flush()
paragraphs.sort(reverse=True)
print(f"dense prose candidates: {len(paragraphs)}")
for words, longest, line, sample in paragraphs[:60]:
    print(f"line {line}: paragraph_words={words}, longest_sentence_words={longest} :: {sample}")
