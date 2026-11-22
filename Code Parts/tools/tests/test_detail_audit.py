"""Detail checks cannot silently rewrite extraction or approve live teaching."""
import json
from pathlib import Path
import tempfile
import unittest
from unittest import mock

from tools import audit_engine_extraction as detail
from tools import notes_pipeline as pipeline


class DetailAuditTests(unittest.TestCase):
    def test_explicit_checks_survive_empty_model_findings_with_exact_source_spans(self):
        text = ("Study focus: Decode images. Verification\nquestion: Does a later read\n"
                "wait for the write?\nRelated conceptual chapter 12\n"
                "ENGINEERING CHECK: Reject invalid files without allocation.\n"
                "No partial handle may be published.\nB. Next exercise\nProcedure: Test more.")
        unit = {"unit_id": "u-one", "locator": {"page_number": 31}, "text": text}
        anchors = detail.required_details(unit)
        self.assertEqual(len(anchors), 2)
        self.assertIn("No partial handle", anchors[1]["source_text"])
        self.assertNotIn("Next exercise", anchors[1]["source_text"])
        for anchor in anchors:
            self.assertEqual(text[anchor["source_char_start"]:anchor["source_char_end"]].strip(),
                             anchor["source_text"])
            self.assertEqual(anchor["resolution"], "supervisor_mapping_required")

    def test_unknown_correction_and_foreign_source_citations_are_rejected(self):
        unit = {"unit_id": "u-one"}
        value = {"missing_points": [], "corrections": [
            {"point_id": "invented", "replacement_claim": "Replacement", "reason": "Reason"}],
            "review_note": "Checked."}
        with self.assertRaises(pipeline.ValidationError):
            detail.validate(value, unit, ["p-one"])
        value["corrections"] = []
        value["missing_points"] = [{"topic": "Check", "claim": "A criterion",
                                    "evidence_type": "source_statement", "source_unit_ids": ["u-other"]}]
        with self.assertRaises(pipeline.ValidationError):
            detail.validate(value, unit, ["p-one"])

    def test_missing_acceptance_criteria_are_saved_without_rewriting_or_approving(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp); source = root / "fixture.txt"; state = root / "state"
            source.write_text("Request shutdown. Accept only if every job reaches a terminal state.")
            meta = pipeline.import_source(source, state, "fixture", kind="local_document")
            _, units = pipeline.load_source(state, meta["source_id"])
            uid = units[0]["unit_id"]
            pipeline.atomic_json(state / "queue.json", {"jobs": [{"job_id": "fixture",
                "source_id": meta["source_id"], "units_sha256": meta["units_sha256"]}]})
            points_path = state / "drafts/fixture/points.json"
            pipeline.atomic_json(points_path, {"status": "extraction_complete", "points": [
                {"point_id": "p-one", "claim": "Request shutdown.",
                 "evidence_type": "source_statement", "source_unit_ids": [uid]}]})
            original = points_path.read_bytes()
            client = mock.Mock(model_digest="fixture-digest", calls_made=1)

            def chat(system, user, digest, pass_name, validator, schema):
                self.assertEqual(pass_name, "source_detail_audit")
                value = {"missing_points": [{"topic": "Shutdown completion",
                    "claim": "Accept only if every job reaches a terminal state.",
                    "evidence_type": "source_statement", "source_unit_ids": [uid]}],
                    "corrections": [], "review_note": "The acceptance condition was omitted."}
                return validator(value), False

            client.chat_json.side_effect = chat
            with mock.patch.object(pipeline, "OllamaClient", return_value=client):
                result = detail.audit(state, "fixture")
            saved = json.loads((state / "drafts/fixture/detail-audit.json").read_text())
            self.assertEqual(points_path.read_bytes(), original)
            self.assertEqual(result["missing_points"], 1)
            self.assertEqual(saved["supervisor_review"], "pending")
            self.assertFalse(saved["merged"])
            self.assertFalse((state / "drafts/fixture/draft.md").exists())


if __name__ == "__main__":
    unittest.main()
