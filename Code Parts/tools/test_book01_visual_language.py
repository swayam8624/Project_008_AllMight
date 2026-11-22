#!/usr/bin/env python3
"""Book One's source-preserving PDF and EPUB visual-language integration tests."""
from __future__ import annotations

from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest
import zipfile

ROOT = Path(__file__).resolve().parents[2]
BOOK = ROOT / "Notes/Book Production" / "Seven-Book Curriculum" / "Book01"
FILTER = BOOK / "book01-code-boxes.lua"
STYLE = BOOK / "book01-reader.sty"
EPUB_CSS = BOOK / "book01-ebook.css"


class VisualLanguageTests(unittest.TestCase):
    def test_style_contracts(self) -> None:
        source = STYLE.read_text(encoding="utf-8")
        self.assertIn(r"\newtcblisting{BookCode}[1][]", source)
        for environment in ("BookLearningNote", "BookPractice", "BookQuote"):
            self.assertIn(r"\newtcolorbox{" + environment + "}", source)
        for language in ("BOneShell", "BOneShader", "BOneCMake", "BOneJSON"):
            self.assertIn(r"\lstdefinelanguage{" + language + "}", source)

    def test_epub_actual_pandoc_token_roles(self) -> None:
        css = EPUB_CSS.read_text(encoding="utf-8")
        for item in ("span.kw", "span.co", "span.st", "span.dv",
                     "blockquote", "pre.sourceCode", "border-left"):
            self.assertIn(item, css)

    @unittest.skipUnless(shutil.which("pandoc"), "Pandoc needed for Lua filter test")
    def test_pandoc_source_code_roundtrip(self) -> None:
        fence = chr(96) * 3
        tick = chr(96)
        sample = (
            "A " + tick + "std::vector<int>" + tick + " can retain data.\n\n"
            + fence + "cpp\n#include <vector>\n"
            "// Retain the texture until the GPU completes.\n"
            "std::vector<int> pixels{1, 2, 3};\n" + fence + "\n\n"
            + fence + "bash\ncmake --build build --parallel 4\n" + fence + "\n\n"
            + fence + "metal\nkernel void main0(device float *p [[buffer(0)]]) {}\n" + fence + "\n\n"
            "> Encoding is the mapping from meaning to representation.\n\n"
            "By the end: explain why one address is not one object.\n\n"
            "Recall question: who owns the buffer?\n"
        )
        with tempfile.TemporaryDirectory() as tmp:
            source = Path(tmp) / "sample.md"
            source.write_text(sample, encoding="utf-8")
            result = subprocess.run(
                ["pandoc", "-f", "markdown", "-t", "latex",
                 "--no-highlight", "--lua-filter", str(FILTER), str(source)],
                capture_output=True, text=True
            )
        self.assertEqual(result.returncode, 0, result.stderr)
        out = result.stdout
        self.assertIn(r"\begin{BookCode}[language={C++}]", out)
        self.assertIn(r"\begin{BookCode}[language={BOneShell}]", out)
        self.assertIn(r"\begin{BookCode}[language={BOneShader}]", out)
        self.assertIn("std::vector<int> pixels{1, 2, 3};", out)
        self.assertIn("// Retain the texture until the GPU completes.", out)
        self.assertIn(r"\begin{BookQuote}", out)
        self.assertIn(r"\begin{BookLearningNote}[Learning objective]", out)
        self.assertIn(r"\begin{BookLearningNote}[Check your understanding]", out)
        self.assertIn(r"\texttt{std::vector", out)
        self.assertNotIn(r"\begin{lstlisting}", out)
        self.assertNotIn(r"\begin{Shaded}", out)


    @unittest.skipUnless(shutil.which("pandoc"), "Pandoc needed for EPUB proof")
    def test_epub_renders_semantic_cards_without_flattening_code(self) -> None:
        fence = chr(96) * 3
        sample = (
            "# Chapter\n\n"
            "By the end: explain the invariant.\n\n"
            "Recall question: who owns the object?\n\n"
            "> Definition: a contract states a valid set of operations.\n\n"
            "> Checkpoint: demonstrate the failure case.\n\n"
            + fence + "cpp\nint main() { return 0; }\n" + fence + "\n"
        )
        with tempfile.TemporaryDirectory() as tmp:
            source = Path(tmp) / "source.md"
            result_file = Path(tmp) / "proof.epub"
            source.write_text(sample, encoding="utf-8")
            result = subprocess.run([
                "pandoc", "-f", "markdown", "-t", "epub3",
                "--lua-filter", str(FILTER),
                "-o", str(result_file), str(source)
            ], capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            with zipfile.ZipFile(result_file) as archive:
                markup = "".join(
                    archive.read(file).decode("utf-8")
                    for file in archive.namelist() if file.endswith(".xhtml")
                )
        self.assertIn("book-learning-note", markup)
        self.assertIn("book-practice", markup)
        # Pandoc syntax-highlights the source into nested XHTML spans;
        # compare the reader-visible text, not the raw tag-interleaved XML.
        import html
        import re
        visible_code = html.unescape(re.sub(r"<[^>]+>", "", markup))
        self.assertIn("int main()", visible_code)
        self.assertIn('class="sourceCode cpp"', markup)


if __name__ == "__main__":
    unittest.main()
