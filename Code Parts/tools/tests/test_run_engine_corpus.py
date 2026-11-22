"""State and provenance regressions for the bounded engine-corpus runner."""
from __future__ import annotations

import json
import contextlib
import io
from pathlib import Path
import tempfile
import unittest
from unittest import mock

from tools import notes_pipeline
from tools import run_engine_corpus as runner


class CorpusRunnerTests(unittest.TestCase):
    def test_nonextractable_book_pages_require_visual_review(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            state = Path(temp) / "state"
            state.mkdir()
            pdf = state / "empty.pdf"
            pdf.write_bytes(b"fixture bytes")
            metadata = {
                "source_id": "book-source", "source_sha256": "book-sha",
                "units_sha256": "empty-units-sha", "raw_file": "empty.pdf",
                "title": "Scanned book", "kind": "secondary_pdf",
                "format_metadata": {"total_pdf_pages": 2}, "extra_provenance": {},
            }
            (state / "source-index.json").write_text(json.dumps({
                "videos": [], "books": [{"source_id": "book-source", "total_pdf_pages": 2}]
            }), encoding="utf-8")

            with mock.patch.object(runner.pipeline, "load_source", return_value=(metadata, [])):
                runner.plan(state, pages_per_job=1)

            jobs = json.loads((state / "queue.json").read_text(encoding="utf-8"))["jobs"]
        self.assertEqual(len(jobs), 2)
        for job in jobs:
            self.assertEqual(job["status"], "requires_visual_review")
            self.assertEqual(job["text_pages"], [])
            self.assertEqual(job["nonextractable_pages"], [job["pdf_pages"][0]])
            self.assertEqual(job["supervisor_review"], "pending")
            self.assertFalse(job["merged"])

    def test_worker_consumes_namespaced_spans_with_all_original_cues(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            state = root / "state"
            raw = root / "captions.vtt"
            raw.write_text(
                """WEBVTT

cue-a
00:00:00.000 --> 00:00:01.000
The renderer records every submitted frame and keeps the image resource alive.

cue-b
00:00:01.000 --> 00:00:02.000
The queue waits until the graphics device signals completion before reuse.

cue-c
00:00:02.000 --> 00:00:03.000
Only then can the application safely recycle the swapchain image.
""",
                encoding="utf-8",
            )
            source = notes_pipeline.import_source(
                raw, state, "youtube:fixture:youtube_caption", title="Fixture captions",
                kind="youtube_caption", extra={"video_id": "fixture-video"},
            )
            (state / "source-index.json").write_text(json.dumps({
                "videos": [{"video_id": "fixture-video", "source_ids": [source["source_id"]],
                            "legacy_reading_status": "not reviewed"}],
                "books": [],
            }), encoding="utf-8")
            runner.plan(state)
            queue = json.loads((state / "queue.json").read_text(encoding="utf-8"))
            job = queue["jobs"][0]
            observed = {}

            def fake_build(state_dir, source_ids, **kwargs):
                observed["source_ids"] = source_ids
                observed["source_id"] = source_ids[0]
                observed["metadata"], observed["units"] = notes_pipeline.load_source(state_dir, source_ids[0])
                out = kwargs["out"]
                out.mkdir(parents=True, exist_ok=True)
                (out / "draft.md").write_text("Draft for supervisor review.\n", encoding="utf-8")
                (out / "audit.json").write_text(json.dumps({"summary": {"ok": True}}), encoding="utf-8")
                (out / "points.json").write_text(json.dumps({"uncovered_narrative_points": []}), encoding="utf-8")
                return {"uncached_model_calls": 0}

            with mock.patch.object(runner.pipeline, "build_pipeline", side_effect=fake_build):
                runner.work(state, max_jobs=1, max_calls=0, model="local-fixture",
                            output_tokens=1, chunk_chars=100)

            final_queue = json.loads((state / "queue.json").read_text(encoding="utf-8"))
            final_job = final_queue["jobs"][0]
            pack = json.loads((Path(final_job["draft_dir"]) / "supervisor-pack.json").read_text(encoding="utf-8"))
            observed["cue_ids"] = [
                unit["unit_id"] for unit in json.loads(
                    (state / "sources" / source["source_id"] / "units.json").read_text(encoding="utf-8")
                )
            ]

        self.assertEqual(observed["source_ids"], [job["source_id"]])
        derived_id = observed["source_id"]
        self.assertNotEqual(derived_id, source["source_id"])
        self.assertTrue(all(
            unit["unit_id"] == notes_pipeline.unit_identifier(derived_id, unit["locator"], unit["text"])
            for unit in observed["units"]
        ))
        self.assertEqual(observed["metadata"]["extra_provenance"]["parent_source_id"], source["source_id"])
        self.assertTrue(observed["metadata"]["extra_provenance"]["all_cue_ids_accounted"])
        consumed = [uid for span in observed["units"] for uid in span["locator"]["constituent_unit_ids"]]
        self.assertEqual(consumed, observed["cue_ids"])
        self.assertEqual(len(consumed), len(set(consumed)))
        self.assertEqual(final_job["status"], "draft_ready")
        self.assertEqual(final_job["supervisor_review"], "pending")
        self.assertFalse(final_job["merged"])
        self.assertEqual(pack["supervisor_review"], "pending")
        self.assertFalse(pack["merged"])

    def test_draft_ready_jobs_are_not_rebuilt(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            state = Path(temp)
            queued = {"jobs": [{"job_id": "already-drafted", "kind": "video", "source_id": "src",
                                "units_sha256": "units", "status": "draft_ready", "attempts": [],
                                "supervisor_review": "pending", "merged": False}]}
            (state / "queue.json").write_text(json.dumps(queued), encoding="utf-8")
            with mock.patch.object(runner.pipeline, "build_pipeline") as build:
                runner.work(state, max_jobs=1, max_calls=1, model="fixture", output_tokens=1, chunk_chars=10)
            build.assert_not_called()
            saved = json.loads((state / "queue.json").read_text(encoding="utf-8"))
        self.assertEqual(saved["jobs"][0]["status"], "draft_ready")
        self.assertEqual(saved["jobs"][0]["attempts"], [])

    def test_live_worker_lock_rejects_a_second_writer(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            state = Path(temp)
            with runner.worker_lock(state):
                with self.assertRaises(notes_pipeline.PipelineError):
                    with runner.worker_lock(state):
                        self.fail("second worker unexpectedly acquired the lock")
            self.assertFalse((state / "worker.lock").exists())

    def test_work_cli_defaults_to_extract_phase(self) -> None:
        with tempfile.TemporaryDirectory() as temp, mock.patch(
            "sys.argv", ["run_engine_corpus.py", "--state-dir", temp, "work"]
        ), mock.patch.object(runner, "work", return_value={"jobs": 0}) as work:
            with contextlib.redirect_stdout(io.StringIO()):
                self.assertEqual(runner.main(), 0)
        self.assertTrue(work.call_args.args[-1])

    def test_extract_phase_stops_at_knowledge_extracted_pending_review(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            state = Path(temp)
            meta = {"source_id": "src-fixture", "source_sha256": "source-sha",
                    "units_sha256": "units-sha", "kind": "youtube_caption"}
            units = [{"unit_id": "unit-1", "text": "A dense source unit."}]
            queue = {"jobs": [{"job_id": "job-1", "kind": "video", "source_id": "src-fixture",
                                "title": "Fixture", "units_sha256": "units-sha", "status": "queued",
                                "attempts": [], "supervisor_review": "pending", "merged": False}]}
            (state / "queue.json").write_text(json.dumps(queue), encoding="utf-8")

            def fake_build(state_dir, source_ids, **kwargs):
                self.assertTrue(kwargs["extract_only"])
                out = kwargs["out"]
                out.mkdir(parents=True, exist_ok=True)
                (out / "points.json").write_text(json.dumps({"points": [], "unit_coverage": {"unit-1": {}}}), encoding="utf-8")
                return {"uncached_model_calls": 1, "status": "knowledge_extracted"}

            with mock.patch.object(runner.pipeline, "load_source", return_value=(meta, units)), mock.patch.object(
                runner.pipeline, "build_pipeline", side_effect=fake_build
            ):
                runner.work(state, 1, 1, "fixture", 100, 10, extract_only=True)

            saved = json.loads((state / "queue.json").read_text(encoding="utf-8"))["jobs"][0]
            pack = json.loads((Path(saved["draft_dir"]) / "supervisor-pack.json").read_text(encoding="utf-8"))
        self.assertEqual(saved["status"], "knowledge_extracted")
        self.assertEqual(saved["supervisor_review"], "pending")
        self.assertFalse(saved["merged"])
        self.assertIsNone(pack["draft_sha256"])
        self.assertEqual(pack["supervisor_review"], "pending")
        self.assertFalse(pack["merged"])

    def test_extraction_reuse_requires_matching_digest_and_complete_unit_coverage(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            state = Path(temp)
            meta = {"source_id": "src-fixture", "source_sha256": "source-sha",
                    "units_sha256": "units-sha", "kind": "youtube_caption"}
            units = [{"unit_id": "unit-1", "locator": {"cue_id": "a"}, "text": "one", "text_sha256": "h1"},
                     {"unit_id": "unit-2", "locator": {"cue_id": "b"}, "text": "two", "text_sha256": "h2"}]
            digest = notes_pipeline.sha256_bytes(notes_pipeline.canonical_json([{
                "source_id": meta["source_id"], "source_sha256": meta["source_sha256"],
                "units_sha256": meta["units_sha256"],
            }]))
            job = {"source_id": "src-fixture"}
            out = state / "drafts" / "job-1"
            out.mkdir(parents=True)
            build_state = {"passes": {"extract_account": "complete"}, "source_digest": digest, "run_id": "run-1"}
            (out / "build-state.json").write_text(json.dumps(build_state), encoding="utf-8")
            (out / "points.json").write_text(json.dumps({"points": [], "unit_coverage": {"unit-1": {}, "unit-2": {}}}), encoding="utf-8")
            with mock.patch.object(runner.pipeline, "load_source", return_value=(meta, units)):
                reused = runner.reuse_extraction(state, job, out)
                self.assertEqual(reused["uncached_model_calls"], 0)
                self.assertTrue(reused["extraction_checkpoint_reused"])
                self.assertEqual(set(json.loads((out / "source-ledger.json").read_text(encoding="utf-8"))["units"][i]["unit_id"] for i in range(2)), {"unit-1", "unit-2"})

                bad_digest_state = {**build_state, "source_digest": "stale-digest"}
                (out / "build-state.json").write_text(json.dumps(bad_digest_state), encoding="utf-8")
                self.assertIsNone(runner.reuse_extraction(state, job, out))

                (out / "build-state.json").write_text(json.dumps(build_state), encoding="utf-8")
                (out / "points.json").write_text(json.dumps({"points": [], "unit_coverage": {"unit-1": {}}}), encoding="utf-8")
                self.assertIsNone(runner.reuse_extraction(state, job, out))

    def test_dense_unit_raises_effective_chunk_size_for_worker(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            state = Path(temp)
            long_text = "x" * 9100
            meta = {"source_id": "src-dense", "source_sha256": "source-sha",
                    "units_sha256": "units-sha", "kind": "secondary_pdf"}
            units = [{"unit_id": "unit-dense", "text": long_text}]
            (state / "queue.json").write_text(json.dumps({"jobs": [{
                "job_id": "dense-book", "kind": "book", "source_id": "src-dense", "title": "Dense",
                "units_sha256": "units-sha", "status": "queued", "attempts": [],
                "supervisor_review": "pending", "merged": False,
            }]}), encoding="utf-8")
            observed = {}

            def fake_build(state_dir, source_ids, **kwargs):
                observed["chunk_chars"] = kwargs["chunk_chars"]
                out = kwargs["out"]
                out.mkdir(parents=True, exist_ok=True)
                (out / "points.json").write_text(json.dumps({"points": [], "unit_coverage": {"unit-dense": {}}}), encoding="utf-8")
                return {"uncached_model_calls": 1}

            with mock.patch.object(runner.pipeline, "load_source", return_value=(meta, units)), mock.patch.object(
                runner.pipeline, "build_pipeline", side_effect=fake_build
            ):
                runner.work(state, 1, 1, "fixture", 100, 4000, extract_only=True)
        self.assertGreaterEqual(observed["chunk_chars"], len(long_text))


if __name__ == "__main__":
    unittest.main()
