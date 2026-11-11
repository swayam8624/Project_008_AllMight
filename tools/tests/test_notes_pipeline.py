"""Regression tests for the local, evidence-bounded notes pipeline."""
from __future__ import annotations

import json
from pathlib import Path
import subprocess
import tempfile
import types
import unittest
from unittest import mock

from tools import notes_pipeline as pipeline


class SubtitleTests(unittest.TestCase):
    def test_vtt_deduplicates_only_overlapping_rolling_prefix(self) -> None:
        text = """WEBVTT

00:00:00.000 --> 00:00:01.500
Hello

00:00:01.000 --> 00:00:02.500
Hello world

00:00:04.000 --> 00:00:05.000
Warning!

00:00:06.000 --> 00:00:07.000
Warning!
"""
        cues = pipeline.parse_vtt(text)
        self.assertEqual([cue["text"] for cue in cues], ["Hello", "world", "Warning!", "Warning!"])
        self.assertEqual(cues[1]["overlap_removed_words"], 1)
        self.assertEqual(cues[1]["raw_text"], "Hello world")
        self.assertEqual(cues[2]["overlap_removed_words"], 0)
        self.assertEqual(cues[3]["overlap_removed_words"], 0)


class TechnicalSubtitleTests(unittest.TestCase):
    def test_template_types_survive_markup_and_entity_cleanup(self) -> None:
        cues = pipeline.parse_vtt("WEBVTT\n\n00:00:00.000 --> 00:00:02.000\n<c.green>std::vector&lt;T&gt; C++</c>\n")
        self.assertEqual(cues[0]["text"], "std::vector<T> C++")

    def test_rolling_corrections_keep_distinct_language_and_identifier_tokens(self) -> None:
        cues = pipeline.parse_vtt("WEBVTT\n\n00:00:00.000 --> 00:00:02.000\nC Node\n\n00:00:01.000 --> 00:00:03.000\nC++ node\n")
        self.assertEqual([cue["text"] for cue in cues], ["C Node", "C++ node"])


class EvidenceValidationTests(unittest.TestCase):
    def test_pdf_cannot_be_mislabelled_as_a_youtube_caption(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            path = root / "companion.pdf"
            path.write_bytes(b"PDF fixture rejected before parsing")
            with self.assertRaises(pipeline.ValidationError):
                pipeline.import_source(path, root / "state", "supplied", kind="youtube_auto_caption")

    def setUp(self) -> None:
        self.units = [
            {"unit_id": "u-one", "text": "first"},
            {"unit_id": "u-two", "text": "second"},
        ]

    def test_unit_accounting_rejects_omitted_unit_and_unknown_citation(self) -> None:
        good = {
            "units": [
                {"unit_id": "u-one", "disposition": "used", "points": [{"topic": "x", "claim": "claim", "evidence_type": "source_statement", "source_unit_ids": ["u-one"]}]},
                {"unit_id": "u-two", "disposition": "ignored", "ignored_reason": "duplicate detail", "points": []},
            ]
        }
        self.assertEqual(len(pipeline.validate_unit_accounting(good, self.units)["units"]), 2)
        with self.assertRaises(pipeline.ValidationError):
            pipeline.validate_unit_accounting({"units": good["units"][:1]}, self.units)
        bad_citation = json.loads(json.dumps(good))
        bad_citation["units"][0]["points"][0]["source_unit_ids"] = ["u-unknown"]
        with self.assertRaises(pipeline.ValidationError):
            pipeline.validate_unit_accounting(bad_citation, self.units)

    def test_narrative_rejects_missing_point_and_mismatched_source_union(self) -> None:
        points = ["p-one", "p-two"]
        sources = {"p-one": {"u-one"}, "p-two": {"u-two"}}
        valid = {
            "title": "Topic",
            "sections": [{"heading": "Section", "paragraphs": [
                {"text": "One.", "evidence_type": "source_statement", "point_ids": ["p-one"], "source_unit_ids": ["u-one"]},
                {"text": "Two.", "evidence_type": "inference", "point_ids": ["p-two"], "source_unit_ids": ["u-two"]},
            ]}],
            "uncovered_points": [],
        }
        self.assertEqual(len(pipeline.validate_narrative(valid, points, sources)["sections"]), 1)
        omitted = json.loads(json.dumps(valid))
        omitted["sections"][0]["paragraphs"].pop()
        with self.assertRaises(pipeline.ValidationError):
            pipeline.validate_narrative(omitted, points, sources)
        mismatched = json.loads(json.dumps(valid))
        mismatched["sections"][0]["paragraphs"][0]["source_unit_ids"] = ["u-two"]
        with self.assertRaises(pipeline.ValidationError):
            pipeline.validate_narrative(mismatched, points, sources)


class ImportProvenanceTests(unittest.TestCase):
    def test_pdf_units_keep_page_locator_and_secondary_source_kind(self) -> None:
        fake_pdf = types.ModuleType("pypdf")

        class Page:
            def __init__(self, text: str) -> None:
                self.text = text

            def extract_text(self) -> str:
                return self.text

        class Reader:
            def __init__(self, _path: str) -> None:
                self.is_encrypted = False
                self.pages = [Page("Page one evidence."), Page("Page two evidence.")]

        fake_pdf.PdfReader = Reader
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            source = root / "companion.pdf"
            source.write_bytes(b"test PDF bytes")
            with mock.patch.dict("sys.modules", {"pypdf": fake_pdf}):
                meta = pipeline.import_source(source, root / "cache", "supplied-pdf", kind="secondary_document", pages="2")
            loaded_meta, units = pipeline.load_source(root / "cache", meta["source_id"])
        self.assertEqual(loaded_meta["kind"], "secondary_document")
        self.assertEqual(units[0]["locator"]["kind"], "pdf_page")
        self.assertEqual(units[0]["locator"]["page_number"], 2)
        self.assertEqual(loaded_meta["format_metadata"]["selected_original_page_numbers"], [2])

    def test_pdf_subset_and_full_imports_have_distinct_ids_and_preserve_units(self) -> None:
        fake_pdf = types.ModuleType("pypdf")

        class Page:
            def __init__(self, text: str) -> None:
                self.text = text

            def extract_text(self) -> str:
                return self.text

        class Reader:
            def __init__(self, _path: str) -> None:
                self.is_encrypted = False
                self.pages = [Page("Page one fact."), Page("Page two fact."), Page("Page three fact.")]

        fake_pdf.PdfReader = Reader
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            source = root / "book.pdf"
            source.write_bytes(b"same stable PDF bytes")
            state = root / "cache"
            with mock.patch.dict("sys.modules", {"pypdf": fake_pdf}):
                subset = pipeline.import_source(source, state, "supplied-pdf", kind="secondary_document", pages="1")
                subset_meta, subset_units_before = pipeline.load_source(state, subset["source_id"])
                full = pipeline.import_source(source, state, "supplied-pdf", kind="secondary_document")
                subset_meta_after, subset_units_after = pipeline.load_source(state, subset["source_id"])
                full_meta, full_units = pipeline.load_source(state, full["source_id"])
        self.assertNotEqual(subset["source_id"], full["source_id"])
        self.assertEqual(subset_meta["source_id"], subset_meta_after["source_id"])
        self.assertEqual(subset_units_before, subset_units_after)
        self.assertEqual(subset_units_after[0]["locator"]["page_number"], 1)
        self.assertEqual(len(full_units), 3)
        self.assertEqual(full_meta["format_metadata"]["selected_original_page_numbers"], [1, 2, 3])

    def test_source_footnotes_link_original_pdf_page_and_video_timestamp(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            state = root / "cache"
            run = root / "draft"
            pdf_meta = {"source_id": "src-pdf", "raw_file": "original.pdf", "title": "Book", "kind": "secondary_document", "extra_provenance": {}}
            pdf_unit = {"unit_id": "u-pdf", "locator": {"kind": "pdf_page", "page_number": 7}}
            yt_meta = {"source_id": "src-yt", "raw_file": "original.vtt", "title": "Video", "kind": "youtube_caption", "extra_provenance": {"video_url": "https://www.youtube.com/watch?v=abcdefghijk"}}
            yt_unit = {"unit_id": "u-yt", "locator": {"kind": "subtitle_cue", "start_timestamp": "00:02:03.500", "end_timestamp": "00:02:05.000", "start_seconds": 123}}
            pdf_footnote = pipeline.source_footnote(pdf_unit, pdf_meta, state, run)
            yt_footnote = pipeline.source_footnote(yt_unit, yt_meta, state, run)
        self.assertIn("PDF page 7", pdf_footnote)
        self.assertIn("#page=7", pdf_footnote)
        self.assertIn("https://www.youtube.com/watch?v=abcdefghijk&t=123", yt_footnote)
        self.assertIn("00:02:03.500–00:02:05.000", yt_footnote)

    def test_identical_caption_bytes_for_different_video_ids_do_not_collide(self) -> None:
        caption = "WEBVTT\n\n00:00:00.000 --> 00:00:01.000\nSame words.\n"
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            source = root / "caption.vtt"
            source.write_text(caption, encoding="utf-8")
            one = pipeline.import_source(source, root / "cache", "youtube:abcdefghijk:manual:en", "Video one", kind="youtube_manual_caption", extra={"video_id": "abcdefghijk"})
            two = pipeline.import_source(source, root / "cache", "youtube:lmnopqrstuv:manual:en", "Video two", kind="youtube_manual_caption", extra={"video_id": "lmnopqrstuv"})
        self.assertNotEqual(one["source_id"], two["source_id"])
        self.assertEqual(one["extra_provenance"]["video_id"], "abcdefghijk")
        self.assertEqual(two["extra_provenance"]["video_id"], "lmnopqrstuv")


class OllamaCacheTests(unittest.TestCase):
    def _client(self, root: Path, responses: list[dict]) -> pipeline.OllamaClient:
        client = pipeline.OllamaClient.__new__(pipeline.OllamaClient)
        client.state_dir = root
        client.model = "local-test:1b"
        client.model_digest = "digest-a"
        client.max_calls = 10
        client.calls_made = 0
        client.options = {"temperature": 0}
        client._http_json = mock.Mock(side_effect=responses)
        return client

    def test_length_terminated_response_is_rejected_and_not_cached(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            client = self._client(root, [{"done": False, "done_reason": "length", "message": {"content": '{"ok": true}'}}])
            with self.assertRaises(pipeline.ValidationError):
                client.chat_json("sys", "user", "source", "extract")
            self.assertEqual(list((root / "model-cache").glob("*.json")), [])

    def test_utf8_context_guard_rejects_before_any_api_request(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            client = self._client(root, [])
            client.options = {"num_ctx": 4096, "num_predict": 2048}
            with self.assertRaises(pipeline.ValidationError):
                client.chat_json("system", "é" * 4096, "source", "extract")
            client._http_json.assert_not_called()

    def test_invalid_model_response_gets_diagnostic_not_valid_cache(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            client = self._client(root, [{"done": True, "message": {"content": "not JSON"}}])
            client.max_calls = 1  # No budget remains for repair.
            with self.assertRaises(pipeline.ValidationError):
                client.chat_json("sys", "user", "source", "extract")
            self.assertEqual(list((root / "model-cache").glob("*.json")), [])
            diagnostics = list((root / "failed-responses").glob("*.json"))
            self.assertEqual(len(diagnostics), 1)
            self.assertEqual(json.loads(diagnostics[0].read_text(encoding="utf-8"))["response"], "not JSON")

    def test_one_bounded_repair_then_valid_result_is_cached_and_reused(self) -> None:
        responses = [
            {"done": True, "message": {"content": "{invalid"}},
            {"done": True, "message": {"content": '{"ok": true}'}},
        ]
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            client = self._client(root, responses)
            result, cached = client.chat_json("sys", "user", "source", "extract")
            self.assertEqual(result, {"ok": True})
            self.assertFalse(cached)
            self.assertEqual(client._http_json.call_count, 2)
            repair_prompt = client._http_json.call_args_list[1].args[1]["messages"][1]["content"]
            self.assertIn("VALIDATION REPAIR", repair_prompt)
            self.assertEqual(len(list((root / "failed-responses").glob("*.json"))), 1)
            self.assertEqual(len(list((root / "model-cache").glob("*.json"))), 1)
            reused, was_cached = client.chat_json("sys", "user", "source", "extract")
            self.assertEqual(reused, {"ok": True})
            self.assertTrue(was_cached)
            self.assertEqual(client._http_json.call_count, 2)

    def test_model_and_source_digests_partition_cached_responses(self) -> None:
        response = {"done": True, "done_reason": "stop", "message": {"content": '{"ok": true}'}}
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            client = self._client(root, [response, response, response])
            self.assertEqual(client.chat_json("sys", "user", "source-a", "extract")[1], False)
            self.assertEqual(client.chat_json("sys", "user", "source-a", "extract")[1], True)
            self.assertEqual(client.chat_json("sys", "user", "source-b", "extract")[1], False)
            client.model_digest = "digest-b"
            self.assertEqual(client.chat_json("sys", "user", "source-a", "extract")[1], False)
            self.assertEqual(client._http_json.call_count, 3)


class RateLimitTests(unittest.TestCase):
    def test_http_429_cooldown_persists_and_prevents_next_network_call(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            state = root / "state"
            pipeline.ensure_state_dirs(state)
            pipeline.atomic_json(state / "inventory" / "PL12345.json", {
                "playlist_id": "PL12345", "entries": [{"id": "abcdefghijk", "title": "A", "url": "https://www.youtube.com/watch?v=abcdefghijk", "index": 1}],
            })
            limited = subprocess.CompletedProcess([], 1, "", "ERROR: HTTP Error 429: Too Many Requests\nRetry-After: 3600")
            with mock.patch.object(pipeline, "_ytdlp", return_value=limited) as ytdlp:
                first = pipeline.fetch_captions(state, playlist_id="PL12345")
                second = pipeline.fetch_captions(state, playlist_id="PL12345")
            self.assertEqual(first["status"], "rate_limited")
            self.assertEqual(second["status"], "cooldown_active")
            self.assertEqual(ytdlp.call_count, 1)
            self.assertTrue((state / "cooldown.json").exists())

    def test_cached_first_entry_does_not_block_next_uncached_video(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            state = root / "state"
            pipeline.ensure_state_dirs(state)
            entries = [
                {"id": "abcdefghijk", "title": "Cached", "url": "https://www.youtube.com/watch?v=abcdefghijk", "index": 1},
                {"id": "lmnopqrstuv", "title": "Next", "url": "https://www.youtube.com/watch?v=lmnopqrstuv", "index": 2},
            ]
            pipeline.atomic_json(state / "inventory" / "PL12345.json", {"playlist_id": "PL12345", "entries": entries})
            pipeline.atomic_json(state / "fetch" / "abcdefghijk.json", {"video_id": "abcdefghijk", "status": "caption_imported", "source_id": "src-existing"})
            no_tracks = subprocess.CompletedProcess([], 0, json.dumps({"id": "lmnopqrstuv", "subtitles": {}, "automatic_captions": {}}), "")
            with mock.patch.object(pipeline, "_ytdlp", return_value=no_tracks) as ytdlp:
                result = pipeline.fetch_captions(state, limit=1, playlist_id="PL12345")
            self.assertEqual(ytdlp.call_count, 1)
            self.assertIn("lmnopqrstuv", ytdlp.call_args.args[0][-1])
            self.assertEqual(result["requested"], 1)
            self.assertEqual([row["status"] for row in result["results"]], ["skipped_cached", "no_english_track"])


class BuildSafetyTests(unittest.TestCase):
    def test_failed_build_does_not_replace_previously_successful_draft(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            state = root / "cache"
            source = root / "input.txt"
            source.write_text("A source paragraph.", encoding="utf-8")
            meta = pipeline.import_source(source, state, "local-asr", kind="local_asr")
            out = root / "draft-output"
            out.mkdir()
            draft = out / "draft.md"
            original = "# Prior complete draft\n\nKeep this intact.\n"
            draft.write_text(original, encoding="utf-8")
            fake_client = mock.Mock()
            fake_client.model_digest = "digest"
            fake_client.calls_made = 0
            fake_client.options = {}
            fake_client.chat_json.side_effect = pipeline.ValidationError("invalid model output")
            with mock.patch.object(pipeline, "OllamaClient", return_value=fake_client):
                with self.assertRaises(pipeline.ValidationError):
                    pipeline.build_pipeline(state, [meta["source_id"]], out=out)
            self.assertEqual(draft.read_text(encoding="utf-8"), original)

    def test_successful_build_formats_json_prompts_and_writes_only_to_draft_output(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            state = root / "cache"
            source = root / "input.txt"
            source.write_text("A source paragraph.", encoding="utf-8")
            meta = pipeline.import_source(source, state, "local-asr", kind="local_asr")
            _source_meta, units = pipeline.load_source(state, meta["source_id"])
            unit_id = units[0]["unit_id"]
            calls: list[tuple[str, str]] = []
            fake_client = mock.Mock()
            fake_client.model_digest = "digest"
            fake_client.calls_made = 3
            fake_client.options = {}

            def chat(_system: str, user: str, _digest: str, pass_name: str, validator=None, schema=None):
                calls.append((pass_name, user))
                if pass_name == "extract_account":
                    value = {"units": [{"unit_id": unit_id, "disposition": "used", "points": [{"topic": "Mechanics", "claim": "A supported claim.", "evidence_type": "source_statement", "source_unit_ids": [unit_id]}]}]}
                elif pass_name == "compose_topic":
                    value = {"title": "Mechanics", "sections": [{"heading": "Evidence", "paragraphs": [{"text": "A {braced} example.", "evidence_type": "source_statement", "point_ids": ["p-000001"], "source_unit_ids": [unit_id]}]}], "uncovered_points": []}
                else:
                    value = {"status": "locally_supported", "issues": [], "suggested_checks": []}
                return (validator(value) if validator else value), False

            fake_client.chat_json.side_effect = chat
            out = root / "review-draft"
            with mock.patch.object(pipeline, "OllamaClient", return_value=fake_client) as client_class:
                result = pipeline.build_pipeline(state, [meta["source_id"]], out=out, output_tokens=3072)
            self.assertEqual(client_class.call_args.kwargs["num_predict"], 3072)
            self.assertEqual(result["status"], "complete_local_draft_human_review_required")
            self.assertEqual([name for name, _ in calls], ["extract_account", "compose_topic", "independent_local_audit"])
            self.assertIn("POINTS_JSON", calls[1][1])
            self.assertIn("{braced}", (out / "draft.md").read_text(encoding="utf-8"))
            self.assertTrue((out / "source-ledger.json").is_file())
            self.assertFalse((state / "draft.md").exists())


if __name__ == "__main__":
    unittest.main()
