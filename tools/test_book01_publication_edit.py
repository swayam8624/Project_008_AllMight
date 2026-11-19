#!/usr/bin/env python3
"""Regression tests for publication-only editorial, code and index transforms."""
import unittest
import book01_publication_edit as editor
import book01_index as index
import book01_learning_gates as gates
import book01_render_support as render

BACKTICKS = chr(96) * 3


class PublicationTransformTests(unittest.TestCase):
    def test_canonical_title_does_not_create_a_redundant_section(self):
        sample = (
            "# Computing Foundations — From Bits to C++ and Systems APIs\n\n"
            "## From signals to meaning\n\nA bit is a state.\n\n"
            "### A register remembers state\n\nThe state is clocked.\n"
        )
        reader, report = editor.apply_reader_edits(sample, 1)
        self.assertNotIn("# Computing Foundations", reader)
        self.assertIn("# From signals to meaning", reader)
        self.assertIn("## A register remembers state", reader)
        self.assertEqual(report["heading_promotion_levels"], 1)

    def test_missing_section_level_is_corrected(self):
        sample = (
            "### GENERAL COMPUTE & PARALLEL ALGORITHMS\n\n"
            "Kernels use a grid.\n"
            "#### A workgroup is not a device-wide barrier\n\n"
            "Explain cross-group dependencies.\n"
        )
        out, report = editor.apply_reader_edits(sample, 9)
        self.assertIn("# GENERAL COMPUTE & PARALLEL ALGORITHMS", out)
        self.assertIn("## A workgroup is not a device-wide barrier", out)
        self.assertEqual(report["heading_promotion_levels"], 2)

    def test_editorial_scaffolding_is_saved_not_destroyed(self):
        source = (
            "## Ownership\n\n"
            "This addition belongs immediately after existing concepts. "
            "It does not replace earlier explanations.\n\n"
            "Stable handles name objects through a registry.\n"
        )
        out, log = editor.apply_reader_edits(source, 2)
        self.assertNotIn("This addition belongs", out)
        self.assertIn("Stable handles name objects", out)
        self.assertTrue(log["removed_editorial_notes"])

    def test_code_and_math_are_not_rewritten(self):
        code = f"{BACKTICKS}cpp\n// This addition belongs in this function\n{BACKTICKS}"
        raw_math = chr(36) * 2 + "\n\\boxed{\\text{value is 99%}}\n" + chr(36) * 2
        source = "## Meaning\n\n" + code + "\n\n" + raw_math
        transformed, _ = editor.apply_reader_edits(source, 2)
        self.assertIn(code, transformed)
        self.assertIn(raw_math, transformed)

    def test_private_obisidian_links_become_readable_text(self):
        source = "See [[Supplementary/Sources and Code Anchors#M007|verification scope]]."
        out, report = editor.apply_reader_edits(source, 2)
        self.assertIn("verification scope", out)
        self.assertNotIn("[[", out)
        self.assertEqual(report["normalized_wikilinks"], 1)

    def test_schematic_cpp_is_labeled_pseudocode_without_changing_body(self):
        source = (
            f'{BACKTICKS}cpp id="x1rcxl"\n'
            "for each item:\n    if useful:\n"
            "        output.push_back(item);\n"
            f"{BACKTICKS}\n"
        )
        out, report = editor.apply_reader_edits(source, 9)
        self.assertIn("Pseudocode (not compilable C++)", out)
        self.assertIn("output.push_back(item);", out)
        self.assertEqual(report["relabeled_pseudocode"], 1)

    def test_long_compiler_line_remains_copyable(self):
        source = (
            "Compile with " + chr(96)
            + "clang++ -std=c++23 -Wall -Wextra -Wpedantic -Werror "
            + "-fsanitize=address,undefined objects.cpp -o objects"
            + chr(96) + ", then run " + chr(96) + "./objects"
            + chr(96) + ". Repeat with -O2."
        )
        output, record = editor.apply_reader_edits(source, 4)
        self.assertIn("clang++ -std=c++23 -Wall", output)
        self.assertIn("-fsanitize=address,undefined objects.cpp", output)
        self.assertIn(f"{BACKTICKS}sh\n", output)
        self.assertIn("./objects\n", output)
        self.assertEqual(record["promoted_build_commands"], 1)

    def test_every_chapter_has_unique_reconstruction_gate(self):
        self.assertEqual(set(gates.GATES), set(range(1, 17)))
        endings = set()
        for chapter in range(1, 17):
            tex = gates.gate_latex(chapter)
            self.assertIn(r"\subsection*{Reconstruct, Test, and Transfer}", tex)
            self.assertIn(r"\begin{enumerate}", tex)
            self.assertEqual(tex.count(r"\item"), 3)
            self.assertIn(r"\end{enumerate}", tex)
            endings.add(tex)
        self.assertEqual(len(endings), 16)

    def test_duplicate_draft_figure_caption_is_removed_once(self):
        source = ("### Figure B1 - Original caption\n\n"
                  "ALLMIGHTFIGURECH01INDEX002\n\n"
                  "Explanatory prose remains.\n")
        transformed, n = editor.remove_duplicate_figure_labels(source)
        self.assertEqual(n, 1)
        self.assertNotIn("Figure B1", transformed)
        self.assertIn("Explanatory prose remains", transformed)

    def test_index_has_real_markers_only_in_prose(self):
        code = f"{BACKTICKS}cpp\n// RAII is in code\n{BACKTICKS}"
        source = ("## RAII and object lifetime\n\n"
                  "The ownership contract matters.\n\n" + code)
        converted, entries = index.insert_index_tokens(source, 3)
        self.assertGreaterEqual(len(entries), 2)
        self.assertIn(code, converted)
        self.assertEqual(converted.count(r"\index{"), len(entries))
        tex = index.replace_index_tokens(converted, entries)
        self.assertIn("\\index{RAII}", tex)
        self.assertIn("\\index{object lifetime}", tex)
        self.assertNotIn("ALLMIGHTINDEXCH", tex)

    def test_raw_index_commands_survive_pandoc_links_lists_and_tables(self):
        import shutil
        import subprocess
        from tempfile import TemporaryDirectory
        from pathlib import Path
        raw = (
            "## Ownership and object lifetime\n\n"
            "The [lifetime](https://example.org/lifetime) of a borrow matters.\n\n"
            "A " + chr(96) + "C++23" + chr(96) + " concept can appear inside inline code.\n\n"
            "| Feature | Rule |\n|:--|:--|\n"
            "| Vulkan | synchronization matters |\n\n"
            "An RAII owner retains the resource.\n"
        )
        transformed, entries = index.insert_index_tokens(raw, 3)
        self.assertGreaterEqual(len(entries), 4)
        self.assertIn("[lifetime](https://example.org/lifetime)", transformed)
        self.assertIn("C++23", transformed)
        self.assertNotIn("ALLMIGHTINDEXCH", transformed)
        self.assertEqual(transformed.count(r"\index{"), len(entries))
        if not shutil.which("pandoc"):
            self.skipTest("Pandoc not available for integration test")
        with TemporaryDirectory() as folder:
            path = Path(folder) / "reader.md"
            path.write_text(transformed, encoding="utf-8")
            result = subprocess.run(
                ["pandoc", "-f", "markdown+raw_tex", "-t", "latex",
                 "--wrap=none", "--no-highlight", str(path)],
                text=True, capture_output=True, check=True,
            )
            index.replace_index_tokens(result.stdout, entries)
            for entry in entries:
                self.assertEqual(result.stdout.count(entry["command"]), 1)

    def test_source_mathematics_and_mermaid_remain_compatible(self):
        source = (
            "# GPU memory\n\n"
            "The memory model matters.\n\n"
            f"{BACKTICKS}mermaid\nflowchart LR\nA-->B\n{BACKTICKS}\n\n"
            + chr(36) * 2 + "\n\\boxed{\\text{memory is ordered}}\n"
            + chr(36) * 2 + "\n"
        )
        reader, _ = editor.apply_reader_edits(source, 9)
        indexed, entries = index.insert_index_tokens(reader, 9)
        shielded, math = render.plan_math(indexed, 9)
        self.assertEqual(len(math), 1)
        self.assertIn(f"{BACKTICKS}mermaid", shielded)
        self.assertTrue(any(x["term"] == "memory model" for x in entries))


if __name__ == "__main__":
    unittest.main()
