#!/usr/bin/env python3
"""Compile three selected M018 note blocks, including their saved spelling.

This checks the displayed vector declaration, expression kernel and local
materialization example. It does not claim to compile every book fragment.
"""
import argparse
from pathlib import Path
import re
import subprocess
import tempfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--compiler", default="clang++")
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[2]
    note = root / "Continuous Notes" / (
        "Computing Foundations - From Bits to C++ and Systems APIs.md"
    )
    text = note.read_text(encoding="utf-8")
    title = "## A registry crosses its last C++ boundary:"
    start = text.index(title)
    end = text.find("\n## ", start + len(title))
    chapter = text[start:] if end == -1 else text[start:end]
    blocks = re.findall(r"~~~cpp\n(.*?)\n~~~", chapter, re.DOTALL)

    def select(predicate, label):
        matches = [block for block in blocks if predicate(block)]
        if len(matches) != 1:
            raise RuntimeError(f"Expected one saved {label} block, got {len(matches)}")
        return matches[0]

    vector = select(lambda b: "struct WorldSpace" in b and "struct Vector" in b,
                    "vector declaration")
    kernel = select(lambda b: "concept VectorExpression" in b, "expression kernel")
    # Accept either spelling so reintroducing the old constexpr error is
    # caught by the compiler rather than merely by a text comparison.
    example = select(lambda b: re.match(r"(?:const|constexpr) auto inner =", b),
                     "local materialization")
    source = (
        "#include <array>\n#include <concepts>\n"
        "#include <cstddef>\n#include <type_traits>\n"
        + vector + "\n" + kernel
        + "\nint main() {\n"
        + "constexpr W3 a{{1,2,3}}, b{{4,5,6}}, c{{7,8,9}};\n"
        + example
        + "\nreturn lazy.coordinates == std::array<float,3>{12,15,18} ? 0 : 1;\n}\n"
    )
    with tempfile.TemporaryDirectory(prefix="allmight-m018-note-") as directory:
        path = Path(directory)
        cpp, executable = path / "saved.cpp", path / "saved-example"
        cpp.write_text(source, encoding="utf-8")
        subprocess.run([
            args.compiler, "-std=c++20", "-Wall", "-Wextra", "-Wpedantic", "-Werror",
            str(cpp), "-o", str(executable),
        ], check=True)
        subprocess.run([str(executable)], check=True)
    print("m018: three selected saved note blocks compile and evaluate: PASS")


if __name__ == "__main__":
    main()
