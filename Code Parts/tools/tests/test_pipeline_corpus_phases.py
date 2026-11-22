"""Regression tests for extraction-only corpus review and accounting fixes."""
from __future__ import annotations

import json
from pathlib import Path
import tempfile
import unittest
from unittest import mock

from tools import notes_pipeline as pipeline


class NarrativeAccountingTests(unittest.TestCase):
    def test_repeated_citations_are_allowed_and_redundant_omission_is_removed(self) -> None:
        result = {
            "title": "Tool arguments",
            "sections": [{"heading": "Use and validate", "paragraphs": [
                {"text": "The mode selects the operation.", "evidence_type": "source_statement",
                 "point_ids": ["p-mode"], "source_unit_ids": ["u-mode"]},
                {"text": "Check that mode before parsing its arguments.", "evidence_type": "inference",
                 "point_ids": ["p-mode"], "source_unit_ids": ["u-mode"]},
            ]}],
            "uncovered_points": [{"point_id": "p-mode", "reason": "Also used above by mistake."}],
        }
        clean = pipeline.validate_narrative(result, ["p-mode"], {"p-mode": {"u-mode"}})
        self.assertEqual(len(clean["sections"][0]["paragraphs"]), 2)
        self.assertEqual(clean["sections"][0]["paragraphs"][0]["point_ids"], ["p-mode"])
        self.assertEqual(clean["uncovered_points"], [])

    def test_truly_missing_points_still_fail_coverage(self) -> None:
        result = {"title": "Partial", "sections": [],
                  "uncovered_points": [{"point_id": "p-one", "reason": "Not in this passage."}]}
        with self.assertRaisesRegex(pipeline.ValidationError, "coverage differs"):
            pipeline.validate_narrative(result, ["p-one", "p-two"],
                                        {"p-one": {"u-one"}, "p-two": {"u-two"}})


class PromptPackingTests(unittest.TestCase):
    def test_raw_caption_text_is_not_model_preview_or_chunk_budget(self) -> None:
        units = [
            {"unit_id": "u-one", "text": "123456", "raw_text": "R" * 5000},
            {"unit_id": "u-two", "text": "abcdef", "raw_text": "S" * 5000},
        ]
        chunks = pipeline._pack_units(units, char_limit=10)
        self.assertEqual([[u["unit_id"] for u in chunk] for chunk in chunks], [["u-one"], ["u-two"]])
        preview = pipeline._unit_preview(units[0])
        self.assertNotIn("raw_text", preview)
        self.assertEqual(preview["text"], "123456")


class ExtractionOnlyTests(unittest.TestCase):
    def test_extract_only_writes_complete_source_accounting_without_composing(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp); state = root / "state"; source = root / "source.txt"
            source.write_text("First source paragraph.\n\nSecond source paragraph.", encoding="utf-8")
            meta = pipeline.import_source(source, state, "fixture", kind="local_document")
            _meta, units = pipeline.load_source(state, meta["source_id"])
            calls = []
            client = mock.Mock()
            client.model_digest = "test-model-digest"
            client.options = {"temperature": 0}
            client.calls_made = 1

            def chat(system, user, digest, pass_name, validator=None, schema=None):
                calls.append(pass_name)
                self.assertEqual(pass_name, "extract_account")
                value = {"units": [
                    {"unit_id": unit["unit_id"], "disposition": "used", "ignored_reason": None,
                     "points": [{"topic": "source paragraph", "claim": f"Claim from {unit['unit_id']}",
                                 "evidence_type": "source_statement", "source_unit_ids": [unit["unit_id"]]}]}
                    for unit in units
                ]}
                return validator(value), False

            client.chat_json.side_effect = chat
            out = root / "extraction-only"
            with mock.patch.object(pipeline, "OllamaClient", return_value=client):
                result = pipeline.build_pipeline(state, [meta["source_id"]], out=out,
                                                 chunk_chars=1000, output_tokens=1024,
                                                 extract_only=True)
            build = json.loads((out / "build-state.json").read_text(encoding="utf-8"))
            ledger = json.loads((out / "source-ledger.json").read_text(encoding="utf-8"))
            points = json.loads((out / "points.json").read_text(encoding="utf-8"))

        self.assertEqual(result["status"], "knowledge_extracted_supervisor_review_required")
        self.assertEqual(build["status"], "knowledge_extracted_supervisor_review_required")
        self.assertEqual(build["passes"], {"extract_account": "complete", "compose": "pending", "audit": "pending"})
        self.assertEqual(calls, ["extract_account"])
        self.assertEqual(points["status"], "extraction_complete")
        self.assertEqual(len(points["points"]), len(units))
        self.assertEqual(len(ledger["units"]), len(units))
        self.assertEqual({row["unit_id"] for row in ledger["units"]}, {unit["unit_id"] for unit in units})
        self.assertFalse((out / "draft.md").exists())
        self.assertFalse((out / "audit.json").exists())
        self.assertIn("not supervisor approval", ledger["note"])


if __name__ == "__main__":
    unittest.main()
