#!/usr/bin/env python3
"""Validate Obsidian file, heading, block and asset wiki targets without writes."""
from collections import defaultdict
from pathlib import Path
import argparse
import html
import json
import re
import unicodedata
from urllib.parse import unquote
from note_integrity import INLINE_OR_WIKI, prose_lines

ROOT = Path(__file__).resolve().parents[1]


def key(value):
    return unicodedata.normalize("NFC", unquote(value)).casefold()


def heading_key(value):
    value = html.unescape(value).replace(chr(96), "")
    value = re.sub(r"\[([^\]]+)\]\([^)]*\)", r"\1", value)
    value = re.sub(r"\s+#+\s*$", "", value)
    return key(" ".join(value.strip().split()))


def audit(root):
    paths = [p for p in root.rglob("*.md") if not {".git", ".obsidian"}.intersection(p.parts)]
    files = {key(str(p.relative_to(root))): p for p in paths}
    basenames = defaultdict(set)
    content = {}
    headings = {}
    for p in paths:
        basenames[key(p.name)].add(p)
        content[p] = p.read_text(encoding="utf-8")
        headings[p] = {heading_key(m[1]) for _, line in prose_lines(content[p])
                       if (m := re.match(r"^ {0,3}#{1,6}\s+(.+?)\s*$", line))}
    errors = []
    total = assets = 0
    for p in paths:
        for number, line in prose_lines(content[p]):
            for match in INLINE_OR_WIKI.finditer(line):
                if match[2] is None:
                    continue
                original = match[2]
                target = original.split("|", 1)[0].rstrip("\\")
                path, separator, anchor = target.partition("#")
                path = unquote(path)
                total += 1
                destination = p if not path else files.get(key(path)) or files.get(key(path + ".md"))
                if destination is None:
                    choices = basenames.get(key(Path(path).name), set()) | basenames.get(key(Path(path).name + ".md"), set())
                    if len(choices) == 1:
                        destination = next(iter(choices))
                if destination is None and path:
                    direct = root / path
                    relative = p.parent / path
                    if direct.is_file():
                        destination = direct
                    elif relative.is_file():
                        destination = relative
                reason = None
                if destination is None:
                    reason = "missing file"
                elif destination.suffix != ".md":
                    assets += 1
                elif separator and anchor:
                    if anchor.startswith("^"):
                        pattern = r"(?:^|\s)" + re.escape(anchor) + r"\s*$"
                        if not re.search(pattern, content[destination], re.M):
                            reason = "missing block"
                    elif heading_key(anchor) not in headings[destination]:
                        reason = "missing heading"
                if reason:
                    errors.append({"file": str(p.relative_to(root)), "line": number,
                                   "target": original, "error": reason})
    return {"notes": len(paths), "wiki_targets": total, "asset_targets": assets,
            "error_count": len(errors), "errors": errors}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=ROOT)
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args()
    result = audit(args.root)
    if args.json:
        print(json.dumps(result, indent=2))
    else:
        print({k: v for k, v in result.items() if k != "errors"})
        for error in result["errors"][:20]:
            print(error)
    return bool(result["errors"])


if __name__ == "__main__":
    raise SystemExit(main())
