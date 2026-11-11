"""Concurrency and resume regressions for the parallel engine runner."""
from __future__ import annotations

import json
from pathlib import Path
import tempfile
import threading
import unittest
from unittest import mock

from tools import notes_pipeline
from tools import run_engine_parallel as parallel


def _job(job_id: str, status: str = "queued") -> dict:
    return {"job_id": job_id, "kind": "video", "source_id": f"src-{job_id}",
            "title": job_id, "source_sha256": "source", "units_sha256": "units",
            "status": status, "attempts": [], "supervisor_review": "pending", "merged": False}


class ParallelEngineTests(unittest.TestCase):
    def _state(self, jobs: list[dict]) -> Path:
        temp = tempfile.TemporaryDirectory()
        self.addCleanup(temp.cleanup)
        state = Path(temp.name)
        (state / "queue.json").write_text(json.dumps({"jobs": jobs}), encoding="utf-8")
        return state

    def test_jobs_overlap_and_coordinator_alone_writes_shared_state(self) -> None:
        state = self._state([_job("a"), _job("b")])
        barrier = threading.Barrier(2)
        active = 0
        peak = 0
        guard = threading.Lock()

        def extract(_state, job, *_args):
            nonlocal active, peak
            with guard:
                active += 1
                peak = max(peak, active)
            barrier.wait(timeout=5)
            with guard:
                active -= 1
            return {"status": "knowledge_extracted", "attempt": {"status": "knowledge_extracted", "job": job["job_id"]}}

        atomic_json = notes_pipeline.atomic_json
        writes = []
        def recording_write(path, value):
            if Path(path).name in {"queue.json", "worker-status.json"}:
                writes.append((Path(path).name, threading.current_thread().name))
            return atomic_json(path, value)

        with mock.patch.object(parallel, "extract_job", side_effect=extract), \
             mock.patch.object(notes_pipeline, "atomic_json", side_effect=recording_write):
            parallel.work(state, workers=2)

        saved = json.loads((state / "queue.json").read_text(encoding="utf-8"))
        self.assertEqual(peak, 2)
        self.assertEqual({job["status"] for job in saved["jobs"]}, {"knowledge_extracted"})
        self.assertEqual({len(job["attempts"]) for job in saved["jobs"]}, {1})
        self.assertTrue(writes)
        self.assertTrue(all(name == "MainThread" for _, name in writes))

    def test_duplicate_ids_are_rejected_before_dispatch(self) -> None:
        state = self._state([_job("same"), _job("same")])
        with mock.patch.object(parallel, "extract_job") as extract:
            with self.assertRaisesRegex(notes_pipeline.ValidationError, "Duplicate queue job IDs"):
                parallel.work(state)
        extract.assert_not_called()

    def test_only_and_max_jobs_preserve_completed_jobs(self) -> None:
        state = self._state([_job("done", "knowledge_extracted"), _job("a"), _job("b"), _job("c")])
        called = []
        def extract(_state, job, *_args):
            called.append(job["job_id"])
            return {"status": "knowledge_extracted", "attempt": {"status": "knowledge_extracted"}}
        with mock.patch.object(parallel, "extract_job", side_effect=extract):
            parallel.work(state, only=["b", "c"], max_jobs=1, workers=1)
        saved = json.loads((state / "queue.json").read_text(encoding="utf-8"))
        by_id = {job["job_id"]: job for job in saved["jobs"]}
        self.assertEqual(called, ["b"])
        self.assertEqual(by_id["done"]["status"], "knowledge_extracted")
        self.assertEqual(by_id["done"]["attempts"], [])
        self.assertEqual(by_id["a"]["status"], "queued")
        self.assertEqual(by_id["b"]["status"], "knowledge_extracted")
        self.assertEqual(by_id["c"]["status"], "queued")

    def test_keyboard_interrupt_stops_active_work_and_does_not_start_remaining(self) -> None:
        state = self._state([_job(name) for name in "abcd"])
        started = []
        def extract(_state, job, _model, _calls, _tokens, _chars, stop):
            started.append(job["job_id"])
            self.assertTrue(stop.wait(timeout=5))
            return {"status": "paused_by_user", "pause_reason": "interrupted",
                    "attempt": {"status": "paused_by_user"}}

        with mock.patch.object(parallel, "extract_job", side_effect=extract), \
             mock.patch.object(parallel.futures, "wait", side_effect=KeyboardInterrupt):
            with self.assertRaises(KeyboardInterrupt):
                parallel.work(state, workers=2)
        saved = json.loads((state / "queue.json").read_text(encoding="utf-8"))
        by_id = {job["job_id"]: job for job in saved["jobs"]}
        self.assertCountEqual(started, ["a", "b"])
        self.assertTrue(all(by_id[name]["status"] == "paused_by_user" for name in started))
        self.assertTrue(all(len(by_id[name]["attempts"]) == 1 for name in started))
        self.assertTrue(all(by_id[name]["status"] == "queued" for name in "cd"))
        self.assertTrue(all(not by_id[name]["attempts"] for name in "cd"))

    def test_extract_job_rejects_changed_foreign_source_hashes(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            state = Path(temp)
            job = {**_job("foreign"), "started_at": "now"}
            meta = {"source_sha256": "foreign-source", "units_sha256": "units"}
            with mock.patch.object(parallel.pipeline, "load_source", return_value=(meta, [])), \
                 mock.patch.object(parallel.runner, "reuse_extraction", side_effect=AssertionError("must reject before reuse")):
                result = parallel.extract_job(state, job, "fixture", 1, 128, 1000, threading.Event())
        self.assertEqual(result["status"], "failed")
        self.assertIn("Source changed", result["last_error"])


if __name__ == "__main__":
    unittest.main()
