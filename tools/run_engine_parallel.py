#!/usr/bin/env python3
"""Concurrent source extraction with one queue writer and cancellable children."""
from __future__ import annotations

import argparse
import concurrent.futures as futures
import json
import os
from pathlib import Path
import signal
import subprocess
import sys
import threading
import time

try:
    from . import notes_pipeline as pipeline
    from . import run_engine_corpus as runner
except ImportError:
    import notes_pipeline as pipeline
    import run_engine_corpus as runner


def terminate_child(child: subprocess.Popen) -> None:
    """Only stop the process group created for this source job."""
    if child.poll() is not None:
        return
    try:
        os.killpg(child.pid, signal.SIGINT)
    except ProcessLookupError:
        return
    try:
        child.wait(timeout=5)
    except subprocess.TimeoutExpired:
        try:
            os.killpg(child.pid, signal.SIGTERM)
        except ProcessLookupError:
            pass
        try:
            child.wait(timeout=5)
        except subprocess.TimeoutExpired:
            os.killpg(child.pid, signal.SIGKILL)
            child.wait()


def extract_job(state: Path, job: dict, model: str, max_calls: int,
                output_tokens: int, chunk_chars: int, stop: threading.Event) -> dict:
    """Workers write only this job's output directory, never the shared queue."""
    started = time.monotonic()
    attempt = {"started_at": job["started_at"], "model": model}
    out = state / "drafts" / job["job_id"]
    out.mkdir(parents=True, exist_ok=True)
    update = {"draft_dir": str(out)}
    try:
        if stop.is_set():
            return {"status": "paused_by_user", "attempt": {**attempt, "status": "paused_by_user"}}
        meta, units = pipeline.load_source(state, job["source_id"])
        if (meta["units_sha256"] != job["units_sha256"] or
                meta["source_sha256"] != job["source_sha256"]):
            raise pipeline.ValidationError("Source changed since queue planning")
        result = runner.reuse_extraction(state, job, out)
        if result is None:
            effective_chars = max(chunk_chars, max(len(unit["text"]) for unit in units))
            command = [sys.executable, str(Path(pipeline.__file__).resolve()),
                       "--state-dir", str(state), "build", "--source", job["source_id"],
                       "--model", model, "--max-calls", str(max_calls),
                       "--output-tokens", str(output_tokens), "--chunk-chars", str(effective_chars),
                       "--out", str(out), "--extract-only"]
            with (out / "parallel-worker.log").open("ab") as log:
                child = subprocess.Popen(command, stdout=log, stderr=log, start_new_session=True)
                try:
                    while child.poll() is None:
                        if stop.wait(0.2):
                            terminate_child(child)
                            break
                finally:
                    if child.poll() is None:
                        terminate_child(child)
            if stop.is_set():
                update.update(status="paused_by_user", pause_reason="Parallel extraction interrupted")
                attempt["status"] = "paused_by_user"
                return {**update, "attempt": {**attempt, "elapsed_seconds": round(time.monotonic() - started, 2),
                                               "finished_at": pipeline.iso_utc()}}
            build = runner.read(out / "build-state.json") if (out / "build-state.json").exists() else {}
            if child.returncode:
                if build.get("status") == "paused_call_budget":
                    raise pipeline.CallBudgetExceeded("Source reached its per-job call allowance")
                raise pipeline.PipelineError(f"Extraction child exited {child.returncode}; inspect {out / 'parallel-worker.log'}")
            if build.get("status") != "knowledge_extracted_supervisor_review_required":
                raise pipeline.ValidationError("Child did not finish extraction accounting")
            result = {"status": build["status"], "uncached_model_calls": build["uncached_model_calls"]}
        points = runner.read(out / "points.json")
        if points.get("status") != "extraction_complete":
            raise pipeline.ValidationError("Point extraction is incomplete")
        if set(points.get("unit_coverage", {})) != {unit["unit_id"] for unit in units}:
            raise pipeline.ValidationError("Completed extraction does not account for every source unit")
        pipeline.atomic_json(out / "supervisor-pack.json", {
            "job_id": job["job_id"], "source_id": job["source_id"], "title": job["title"],
            "evidence_kind": meta["kind"], "run_result": result,
            "model_audit": {"narrative_audit": "not run during knowledge pass"},
            "draft_sha256": None, "supervisor_review": "pending", "merged": False,
            "nonextractable_pages": job.get("nonextractable_pages", []),
            "required_checks": ["source detail omissions and unsupported upgrades", "first-use definitions",
                                "worked examples", "math/code correctness", "visuals and existing-book overlap"]})
        update.update(status="knowledge_extracted", draft_sha256=None)
        attempt.update(status="knowledge_extracted", model_calls=result["uncached_model_calls"])
    except pipeline.CallBudgetExceeded as error:
        update.update(status="paused_call_budget", last_error=str(error))
        attempt.update(status="paused_call_budget", error=str(error))
    except (pipeline.PipelineError, OSError, ValueError) as error:
        update.update(status="failed", last_error=str(error))
        attempt.update(status="failed", error=str(error))
    attempt.update(elapsed_seconds=round(time.monotonic() - started, 2), finished_at=pipeline.iso_utc())
    return {**update, "attempt": attempt}


def work(state: Path, max_jobs: int = 50, max_calls: int = 200, model: str = "qwen3:4b",
         output_tokens: int = 8192, chunk_chars: int = 4000, retry_failed: bool = False,
         only: list[str] | None = None, extract_only: bool = True, workers: int = 2) -> dict:
    if not extract_only or not 1 <= workers <= 4 or max_jobs < 0:
        raise pipeline.ValidationError("Parallel runner supports extraction only, 1–4 workers, and nonnegative max_jobs")
    state = Path(state)
    stop = threading.Event()
    with runner.worker_lock(state):
        queue = runner.read(state / "queue.json")
        ids = [job["job_id"] for job in queue["jobs"]]
        if len(ids) != len(set(ids)):
            raise pipeline.ValidationError("Duplicate queue job IDs cannot run concurrently")
        eligible = {"queued", "running", "paused_call_budget"} | ({"failed"} if retry_failed else set())
        selected = [job for job in queue["jobs"] if job["status"] in eligible
                    and (only is None or job["job_id"] in only)]
        if max_jobs:
            selected = selected[:max_jobs]
        iterator = iter(selected)
        active = {}
        interrupted = False
        pool = futures.ThreadPoolExecutor(max_workers=workers, thread_name_prefix="engine-source")

        def submit_one():
            job = next(iterator, None)
            if job is None:
                return False
            job.update(status="running", started_at=pipeline.iso_utc())
            pipeline.atomic_json(state / "queue.json", queue)
            runner.status(state)
            task = pool.submit(extract_job, state, dict(job), model, max_calls, output_tokens, chunk_chars, stop)
            active[task] = job
            return True

        def commit(task):
            job = active.pop(task)
            try:
                result = task.result()
            except Exception as error:
                result = {"status": "failed", "last_error": str(error),
                          "attempt": {"status": "failed", "error": str(error), "finished_at": pipeline.iso_utc()}}
            attempt = result.pop("attempt")
            job.update(result)
            job.setdefault("attempts", []).append(attempt)
            pipeline.atomic_json(state / "queue.json", queue)
            runner.status(state)
            print(json.dumps({"job_id": job["job_id"], **attempt}), flush=True)

        try:
            for _ in range(workers):
                submit_one()
            while active:
                done, _ = futures.wait(active, timeout=0.5, return_when=futures.FIRST_COMPLETED)
                for task in done:
                    commit(task)
                    submit_one()
        except KeyboardInterrupt:
            interrupted = True
            stop.set()
        except BaseException:
            stop.set()
            raise
        finally:
            pool.shutdown(wait=True)
            for task in list(active):
                commit(task)
        if interrupted:
            raise KeyboardInterrupt
        return runner.status(state)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--state-dir", type=Path, default=runner.DEFAULT_STATE)
    parser.add_argument("--workers", type=int, default=2)
    parser.add_argument("--max-jobs", type=int, default=50)
    parser.add_argument("--only", action="append")
    args = parser.parse_args()
    print(json.dumps(work(args.state_dir, workers=args.workers, max_jobs=args.max_jobs, only=args.only)))


if __name__ == "__main__":
    main()
