"""Book01 expanded edition: fixture-level source preservation and routing.

These tests never invoke LaTeX, change the original master, or require Pandoc.
"""
from __future__ import annotations
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

TOOLS = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(TOOLS))
import build_book01_expanded_reader as reader  # noqa: E402
import apply_cpp23_project_inserts as inserts  # noqa: E402


class BookOneExpandedReaderTests(unittest.TestCase):
    def test_all_16_chapters_and_18_original_lessons_are_placed_once(self):
        chapters, metadata = reader.audit_sources()
        self.assertEqual(len(chapters), 16)
        self.assertEqual(metadata["chapter_count"], 16)
        self.assertEqual(metadata["addition_count"], 18)
        assigned = [record["id"] for row in chapters for record in row["lesson_receipts"]]
        expected = [spec.name for spec in inserts.INSERTS]
        self.assertEqual(len(assigned), len(set(assigned)))
        self.assertEqual(set(assigned), set(expected))
        for item in chapters:
            self.assertGreater(item["original_body_bytes"], 0)
            self.assertGreater(item["expanded_body_bytes"], item["original_body_bytes"])
            self.assertEqual(reader.sha256(item["_original"]), item["original_body_sha256"])
            self.assertEqual(reader.sha256(item["_expanded"]), item["expanded_body_sha256"])

    def test_extra_gap_lessons_are_not_moved_to_the_end_of_book(self):
        routing = reader.specs_by_chapter()
        self.assertEqual([x.name for x in routing[6]], ["NES-06G", "CPP-18A"])
        self.assertEqual([x.name for x in routing[10]], ["NES-10R", "CPP-19E"])
        self.assertEqual(sum(len(x) for x in routing.values()), 18)

    def test_original_derived_source_markers_are_strict(self):
        raw = b"Nav\n" + reader.SOURCE_BEGIN + b"original text\n" + reader.SOURCE_END + b"\n"
        prelude, body, suffix = reader.chapter_regions(raw)
        self.assertEqual(prelude, b"Nav\n")
        self.assertEqual(body, b"original text\n")
        self.assertEqual(suffix.strip(), b"")
        for invalid in (b"original without markers",
                        raw + reader.SOURCE_BEGIN,
                        raw.replace(reader.SOURCE_END, b"")):
            with self.assertRaises(ValueError):
                reader.chapter_regions(invalid)

    def test_exact_additive_splice_and_reverse_with_two_shared_anchors(self):
        with tempfile.TemporaryDirectory() as d, \
             patch.object(inserts, "ROOT", Path(d)), \
             patch.object(reader, "ROOT", Path(d)):
            root = Path(d)
            (root / "A.md").write_text("### ABC — Test A\nDeep material.\n", encoding="utf-8")
            (root / "B.md").write_text("### DEF — Test B\nSecond addition.\n", encoding="utf-8")
            specs = [inserts.Insert("ABC", "A.md", b"\n## Common anchor\n"),
                     inserts.Insert("DEF", "B.md", b"\n## Common anchor\n")]
            original = b"# Untouched original\nOne paragraph.\n## Common anchor\nAfter.\n"
            expanded, receipts = reader.insert_and_prove(original, specs)
            self.assertEqual([x["id"] for x in receipts], ["ABC", "DEF"])
            self.assertEqual(expanded.count(b"## Common anchor"), 1)
            self.assertLess(expanded.index(b"### ABC"), expanded.index(b"### DEF"))
            self.assertLess(expanded.index(b"### DEF"), expanded.index(b"## Common anchor"))
            stripped = expanded
            for spec in reversed(specs):
                stripped = stripped.replace(inserts.get_block(spec)[2], b"", 1)
            self.assertEqual(stripped, original)
            with self.assertRaises(ValueError):
                reader.insert_and_prove(b"\n## Common anchor\n## Common anchor\n", specs)

    def test_reader_does_not_reroute_generated_navigation_as_teaching(self):
        preface = (
            "# 01 — Chapter title\n\n"
            "[[Chapter Edition/01 - Computing Foundations/00 - Reading Map|Map]]\n\n"
            "Real chapter bridge.\n\n"
            "**By the end:** demonstrate a contract.\n"
            "<!-- Derived from original; preserve original below. -->\n"
        ).encode()
        result = reader.original_prelude(preface).decode()
        self.assertIn("Real chapter bridge.", result)
        self.assertIn("**By the end:**", result)
        self.assertNotIn("Reading Map|Map", result)
        self.assertNotIn("# 01", result)

    def test_tex_special_characters_are_not_passed_raw_into_titles(self):
        escaped = reader.tex_escape(r"R&D_100% $ # ~ ^ {ok}")
        for part in (r"\&", r"\_", r"\%", r"\$", r"\#", r"\{", r"\}"):
            self.assertIn(part, escaped)


if __name__ == "__main__":
    unittest.main()
