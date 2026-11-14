"""Shared, dependency-free Markdown and approved-route checks for the books."""
from dataclasses import dataclass
import json
from pathlib import Path
import re


BT = chr(96)
FENCE = re.compile(r"^ {0,3}(" + BT + r"{3,}|~{3,})(.*)$")
INLINE_OR_WIKI = re.compile(r"(" + BT + r"+).*?\1|\[\[([^\]]+)\]\]")
LEGACY_ROUTES = {
    "Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing":
        "Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs",
    "Continuous Notes/XR - From Light to Perception - Seeing and Representing Reality":
        "Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography",
    "Continuous Notes/Systems - From Cartridge Bytes to Reproducible State":
        "Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs",
    "Rigid Bodies - From Extended Motion to Contact Constraints":
        "Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#Physical Foundations — Rigid Bodies, Extended Motion, and Contact Constraints",
    "Mass properties and inertia tensor":
        "Continuous Notes/Game Engine - From First Frame to Living Worlds#Part 1637 — Composite-body mass properties",
    "Persistent contact manifolds":
        "Continuous Notes/Game Engine - From First Frame to Living Worlds#Complete persistent-contact implementation and cache policy",
    "AABB overlap and containment":
        "Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Axis-Aligned Bounding Boxes",
    "Complexity, asymptotics, and amortized analysis":
        "Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#Why vectors usually grow geometrically: derive the cost instead of repeating “amortized O(1)”",
}


@dataclass(frozen=True)
class CodeBlock:
    language: str
    body: str
    start: int
    end: int


def code_blocks(text):
    """Parse matching fence character/length, retaining exact program bytes.

    A shorter fence inside a longer example is content. A closing fence cannot
    carry a language suffix. Tilde and backtick blocks obey the same rules.
    """
    opened = None
    body = []
    blocks = []
    for number, line in enumerate(text.splitlines(keepends=True), 1):
        match = FENCE.match(line.rstrip("\r\n"))
        if opened is not None:
            character, length, start, language = opened
            if (match and match[1][0] == character
                    and len(match[1]) >= length and not match[2].strip()):
                blocks.append(CodeBlock(language, "".join(body).rstrip("\r\n"),
                                        start, number))
                opened = None
                body = []
            else:
                body.append(line)
        elif match:
            marker, info = match.groups()
            if marker[0] == BT and BT in info:
                continue
            language = info.strip().split()[0] if info.strip() else ""
            opened = (marker[0], len(marker), number, language)
    if opened is not None:
        raise ValueError(f"unclosed code fence at line {opened[2]}")
    return blocks


def prose_lines(text):
    """Return numbered lines outside code, so examples cannot invent links."""
    blocks = iter(code_blocks(text))
    block = next(blocks, None)
    for number, line in enumerate(text.splitlines(keepends=True), 1):
        if block and number > block.end:
            block = next(blocks, None)
        if block is None or number < block.start:
            yield number, line


def migrate_legacy_links(text):
    """Retarget actual wiki links; preserve labels, anchors and literal code."""
    visible = {number for number, _ in prose_lines(text)}
    lines = text.splitlines(keepends=True)

    def replace(match):
        if match[2] is None:  # An inline-code span is literal evidence.
            return match[0]
        raw = match[2]
        target, separator, label = raw.partition("|")
        path, hashmark, anchor = target.partition("#")
        replacement = LEGACY_ROUTES.get(path)
        if replacement is None:
            return match[0]
        replacement, default_mark, default_anchor = replacement.partition("#")
        if not hashmark and default_mark:
            hashmark, anchor = default_mark, default_anchor
        if not separator and "/" not in path:
            separator, label = "|", path
        return "[[" + replacement + hashmark + anchor + separator + label + "]]"

    return "".join(INLINE_OR_WIKI.sub(replace, line) if n in visible else line
                   for n, line in enumerate(lines, 1))


def normalized_baseline(text, volume_name):
    """Only the user's specific corrections are exempted from preservation.

    Link destinations change; explanatory text stays. The Tape repair removes
    one premature close. The false Phase-18 completion sentence is removed,
    while the remaining original source narrative must still be preserved.
    """
    if volume_name.startswith("Computing Foundations"):
        premature = "#include <vector>\n" + BT * 3 + "\n\nclass Tape {"
        text = text.replace(premature, "#include <vector>\n\nclass Tape {", 1)
    if volume_name.startswith("Game Engine"):
        text = text.replace("That closes the **entire 18-pass run**.\n", "", 1)
    text = migrate_legacy_links(text)
    if volume_name.startswith("Game Engine"):
        text = text.replace('Read a concept where it first helps solve a problem. If a word is unfamiliar, its first useful appearance should supply enough explanation to continue. The mathematics volume and C++ volume are deeper companions, but a link does not replace the explanation needed here: [[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs]] and [[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs]].', 'Read a concept where it first helps solve a problem. If a word is unfamiliar, its first useful appearance should supply enough explanation to continue. Mathematics, C++ and systems now share [[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs|the Computing Foundations book]]; the explanation needed to follow this Engine narrative still belongs here.', 1)
    corrections = Path(__file__).parent / "book_completion/approved_text_corrections.json"
    if corrections.is_file():
        for correction in json.loads(corrections.read_text()):
            if Path(correction["file"]).name == volume_name:
                text = text.replace(correction["before"], correction["after"], 1)
    return text


def ordered_lines_preserved(previous, current):
    stream = iter(current.splitlines(keepends=True))
    for wanted in previous.splitlines(keepends=True):
        if not any(candidate == wanted for candidate in stream):
            return False
    return True
