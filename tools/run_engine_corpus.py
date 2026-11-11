#!/usr/bin/env python3
"""Plan and run the imported engine corpus; live notes require supervisor approval."""
from __future__ import annotations

import argparse
import contextlib
import datetime as dt
import json
import os
from pathlib import Path
import shutil
import sys
import time

try:
    from . import notes_pipeline as pipeline
    from .engine_corpus import derive_spans
except ImportError:
    import notes_pipeline as pipeline
    from engine_corpus import derive_spans

DEFAULT_STATE = Path.home() / ".codex/source-cache/engine-corpus-2026-11-07"


def read(path: Path):
    return json.loads(path.read_text(encoding="utf-8"))


def normalized_index(state: Path):
    path = state / "source-index.json"
    if path.exists():
        index = read(path)
    else:
        index = read(state / "corpus-manifest.json")
    videos = index.get("videos", [])
    if isinstance(videos, dict):
        videos = [{"video_id": key, **value} for key, value in videos.items()]
    books = index.get("books", [])
    if isinstance(books, dict):
        books = list(books.values())
    return videos, books


def share_raw(source: Path, target: Path):
    target.parent.mkdir(parents=True, exist_ok=True)
    if not target.exists():
        try:
            os.link(source, target)
        except OSError:
            shutil.copy2(source, target)


def subtitle_source(state: Path, sid: str, words: int = 150):
    """Use traceable passages as actual model input, keeping the raw cue source."""
    meta, units = pipeline.load_source(state, sid)
    if meta.get("format") not in {"vtt", "srt"}:
        return meta
    origin = f"{meta['origin']}:ordered-subtitle-spans-v1:{words}"
    derived_id = pipeline.source_identifier(origin, meta["source_sha256"], meta["kind"])
    folder = state / "sources" / derived_id
    metadata_path = folder / "source.json"
    if metadata_path.exists():
        derived, _ = pipeline.load_source(state, derived_id)
        return derived
    spans = derive_spans(units, words, sid)
    by_id = {unit["unit_id"]: unit for unit in units}
    for span in spans:
        parents = [by_id[uid] for uid in span["locator"]["constituent_unit_ids"]]
        start = min(unit["locator"]["start_seconds"] for unit in parents)
        end = max(unit["locator"]["end_seconds"] for unit in parents)
        span["locator"].update(start_seconds=start, end_seconds=end,
                               start_timestamp=pipeline._format_vtt_time(start),
                               end_timestamp=pipeline._format_vtt_time(end))
        span["unit_id"] = pipeline.unit_identifier(derived_id, span["locator"], span["text"])
        span["text_sha256"] = pipeline.sha256_bytes(span["text"].encode())
    traced = [uid for span in spans for uid in span["locator"]["constituent_unit_ids"]]
    if traced != [unit["unit_id"] for unit in units]:
        raise pipeline.ValidationError("Subtitle passage derivation lost or reordered cue identities")
    share_raw(state / "sources" / sid / meta["raw_file"], folder / meta["raw_file"])
    share_raw(state / "sources" / sid / "units.json", folder / "cue-units.json")
    pipeline.atomic_json(folder / "units.json", spans)
    derived = {**meta, "source_id": derived_id, "origin": origin, "unit_count": len(spans),
               "units_sha256": pipeline.sha256_file(folder / "units.json"),
               "format_metadata": {**meta.get("format_metadata", {}), "raw_cue_count": len(units),
                                   "span_count": len(spans), "target_span_words": words},
               "extra_provenance": {**meta.get("extra_provenance", {}), "parent_source_id": sid,
                                    "original_cue_units": "cue-units.json", "all_cue_ids_accounted": True}}
    pipeline.atomic_json(metadata_path, derived)
    pipeline.load_source(state, derived_id)
    return derived


def book_subset(state: Path, sid: str, first: int, last: int):
    """Derive a page job without re-extracting or duplicating the whole PDF."""
    meta, units = pipeline.load_source(state, sid)
    selected = [unit for unit in units if first <= unit["locator"].get("page_number", 0) <= last]
    if not selected:
        return None
    origin = f"{meta['origin']}:physical-pages:{first}-{last}"
    derived_id = pipeline.source_identifier(origin, meta["source_sha256"], meta["kind"])
    folder = state / "sources" / derived_id
    folder.mkdir(parents=True, exist_ok=True)
    raw = folder / meta["raw_file"]
    if not raw.exists():
        try:
            os.link(state / "sources" / sid / meta["raw_file"], raw)
        except OSError:
            shutil.copy2(state / "sources" / sid / meta["raw_file"], raw)
    parent_ids = [unit["unit_id"] for unit in selected]
    selected = [{**unit, "parent_unit_id": unit["unit_id"],
                 "unit_id": pipeline.unit_identifier(derived_id, unit["locator"], unit["text"])}
                for unit in selected]
    pipeline.atomic_json(folder / "units.json", selected)
    derived = {**meta, "source_id": derived_id, "origin": origin,
               "title": f"{meta['title']} — PDF pages {first}–{last}",
               "unit_count": len(selected), "units_sha256": pipeline.sha256_file(folder / "units.json"),
               "format_metadata": {**meta.get("format_metadata", {}),
                                   "selected_original_page_numbers": list(range(first, last + 1))},
               "extra_provenance": {**meta.get("extra_provenance", {}), "parent_source_id": sid,
                                    "parent_unit_ids": parent_ids, "derivation": "exact page selection; no model rewriting"}}
    pipeline.atomic_json(folder / "source.json", derived)
    pipeline.load_source(state, derived_id)
    return derived


def plan(state: Path, pages_per_job: int = 12):
    if pages_per_job < 1:
        raise pipeline.ValidationError("pages per job must be positive")
    videos, books = normalized_index(state)
    queue_path = state / "queue.json"
    old = read(queue_path) if queue_path.exists() else {"jobs": []}
    jobs = {job["job_id"]: job for job in old["jobs"]}
    for video in videos:
        ids = video.get("source_ids", [])
        # Prefer acquired captions; importer orders these before ASR alternatives.
        if not ids:
            continue
        sid = ids[0]
        meta = subtitle_source(state, sid)
        sid = meta["source_id"]
        key = "video-" + video.get("video_id", video.get("id", sid)) + "-" + meta["units_sha256"][:12]
        if key not in jobs:
            legacy = video.get("legacy_reading_status", video.get("reading_status", "not reviewed"))
            jobs[key] = {"job_id": key, "kind": "video", "video_id": video.get("video_id", video.get("id")),
                         "source_id": sid, "title": meta["title"], "status": "queued",
                         "source_sha256": meta["source_sha256"], "units_sha256": meta["units_sha256"],
                         "legacy_reading_status": legacy, "priority": 0 if legacy == "not reviewed" else 20,
                         "supervisor_review": "pending", "merged": False, "attempts": []}
    for book in books:
        sid = book["source_id"]
        meta, _ = pipeline.load_source(state, sid)
        total = book.get("total_pdf_pages", book.get("page_count", meta["format_metadata"]["total_pdf_pages"]))
        for first in range(1, total + 1, pages_per_job):
            last = min(total, first + pages_per_job - 1)
            key = f"book-{sid}-{first:04d}-{last:04d}"
            if key in jobs:
                continue
            derived = book_subset(state, sid, first, last)
            text_pages = []
            if derived:
                _, selected = pipeline.load_source(state, derived["source_id"])
                text_pages = sorted({unit["locator"]["page_number"] for unit in selected})
            missing = sorted(set(range(first, last + 1)) - set(text_pages))
            jobs[key] = {"job_id": key, "kind": "book", "source_id": derived["source_id"] if derived else sid,
                         "parent_source_id": sid, "title": derived["title"] if derived else f"{meta['title']} — PDF pages {first}–{last}",
                         "pdf_pages": [first, last], "text_pages": text_pages, "nonextractable_pages": missing,
                         "source_sha256": meta["source_sha256"], "units_sha256": derived["units_sha256"] if derived else meta["units_sha256"],
                         "status": "queued" if derived else "requires_visual_review", "priority": 10, "supervisor_review": "pending", "merged": False,
                         "attempts": []}
    result = {"schema_version": 1, "updated_at": pipeline.iso_utc(), "videos_in_index": len(videos),
              "books_in_index": len(books), "jobs": sorted(jobs.values(), key=lambda job: (job["priority"], job["job_id"]))}
    pipeline.atomic_json(queue_path, result)
    return status(state)


def status(state: Path):
    queue = read(state / "queue.json") if (state / "queue.json").exists() else {"jobs": []}
    decisions = {}
    for path in (state / "quality/decisions").glob("*.json"):
        decision = read(path)
        decisions[decision["job_id"]] = decision
    accepted = []
    for job in queue["jobs"]:
        decision = decisions.get(job["job_id"], {})
        if (decision.get("decision") == "accepted" and
                decision.get("source_sha256") == job["source_sha256"] and
                decision.get("units_sha256") == job["units_sha256"]):
            accepted.append(decision)
    counts = {}
    for job in queue["jobs"]:
        counts[job["status"]] = counts.get(job["status"], 0) + 1
    result = {"updated_at": pipeline.iso_utc(), "jobs": len(queue["jobs"]), "statuses": counts,
              "supervisor_accepted": len(accepted),
              "merged": sum(bool(decision.get("merged")) for decision in accepted),
              "locally_extracted_videos": sum(job["kind"] == "video" and job["status"] in {"knowledge_extracted", "draft_ready"} for job in queue["jobs"]),
              "locally_extracted_pdf_text_pages": sum(len(job.get("text_pages", [])) for job in queue["jobs"]
                                                       if job["kind"] == "book" and job["status"] in {"knowledge_extracted", "draft_ready"}),
              "locally_processed_pdf_pages": sum(len(job.get("text_pages", []))
                                                  for job in queue["jobs"] if job["kind"] == "book" and job["status"] == "draft_ready"),
              "scope": "Local drafting and source accounting are separate from supervisor acceptance and a verified live merge."}
    pipeline.atomic_json(state / "worker-status.json", result)
    lines = ["# Engine corpus processing", "", result["scope"], "",
             f"Queued source jobs: {result['jobs']}; supervisor accepted: {result['supervisor_accepted']}; merged: {result['merged']}.", "",
             f"Local knowledge extraction: {result['locally_extracted_videos']} video sources and {result['locally_extracted_pdf_text_pages']} PDF text pages. These counts do not claim video-frame or whole-book visual inspection.", "",
             "| State | Jobs |", "|---|---:|"]
    lines.extend(f"| {name} | {count} |" for name, count in sorted(counts.items()))
    lines.extend(["", "Read each completed job's review.md and supervisor-pack.json before merging. The runner cannot write the live notes.", ""])
    pipeline.atomic_write(state / "PROGRESS.md", "\n".join(lines).encode())
    return result


@contextlib.contextmanager
def worker_lock(state: Path):
    path = state / "worker.lock"
    if path.exists():
        pid = read(path).get("pid")
        try:
            os.kill(int(pid), 0)
        except (ProcessLookupError, TypeError, ValueError):
            path.unlink()
        else:
            raise pipeline.PipelineError(f"A corpus worker is already running with PID {pid}")
    fd = os.open(path, os.O_CREAT | os.O_EXCL | os.O_WRONLY, 0o600)
    with os.fdopen(fd, "w") as handle:
        json.dump({"pid": os.getpid(), "started_at": pipeline.iso_utc()}, handle)
    try:
        yield
    finally:
        path.unlink(missing_ok=True)


def reuse_extraction(state: Path, job: dict, out: Path):
    """An earlier failed writing pass need not invalidate complete source mapping."""
    if not (out / "build-state.json").exists() or not (out / "points.json").exists():
        return None
    prior = read(out / "build-state.json")
    if prior.get("passes", {}).get("extract_account") != "complete":
        return None
    meta, units = pipeline.load_source(state, job["source_id"])
    digest = pipeline.sha256_bytes(pipeline.canonical_json([{"source_id": meta["source_id"],
                  "source_sha256": meta["source_sha256"], "units_sha256": meta["units_sha256"]}]))
    points = read(out / "points.json")
    if prior.get("source_digest") != digest or set(points["unit_coverage"]) != {u["unit_id"] for u in units}:
        return None
    if not (out / "before-extraction-handoff.json").exists():
        pipeline.atomic_json(out / "before-extraction-handoff.json", prior)
    points["status"] = "extraction_complete"
    pipeline.atomic_json(out / "points.json", points)
    ledger = {"sources": [meta], "units": [{"unit_id": u["unit_id"], "locator": u["locator"],
               "text_sha256": u["text_sha256"], **points["unit_coverage"][u["unit_id"]]} for u in units],
               "note": "Existing complete extraction checkpoint reused; not a supervisor approval."}
    pipeline.atomic_json(out / "source-ledger.json", ledger)
    prior.update(status="knowledge_extracted_supervisor_review_required", uncached_model_calls=0,
                 extraction_checkpoint_reused=True)
    pipeline.atomic_json(out / "build-state.json", prior)
    return {"run_id": prior["run_id"], "run_dir": str(out), "status": prior["status"],
            "unit_count": len(units), "point_count": len(points["points"]), "uncached_model_calls": 0,
            "extraction_checkpoint_reused": True}


def work(state: Path, max_jobs: int, max_calls: int, model: str, output_tokens: int,
         chunk_chars: int, retry_failed: bool = False, only: list[str] | None = None,
         extract_only: bool = False):
    """One bounded, resumable pass; bad jobs cannot promote their own reviews."""
    with worker_lock(state):
        queue = read(state / "queue.json")
        eligible = {"queued", "paused_call_budget", "running"}
        if not extract_only:
            eligible.add("knowledge_extracted")
        if retry_failed:
            eligible.add("failed")
        attempted = 0
        for job in queue["jobs"]:
            if job["status"] not in eligible or (only and job["job_id"] not in only):
                continue
            if max_jobs and attempted >= max_jobs:
                break
            attempted += 1
            job["status"] = "running"
            job["started_at"] = pipeline.iso_utc()
            pipeline.atomic_json(state / "queue.json", queue)
            status(state)
            out = state / "drafts" / job["job_id"]
            started = time.monotonic()
            attempt = {"started_at": job["started_at"], "model": model}
            try:
                meta, source_units = pipeline.load_source(state, job["source_id"])
                if meta["units_sha256"] != job["units_sha256"]:
                    raise pipeline.ValidationError("Source units changed since queue planning; replan")
                effective_chars = max(chunk_chars, max(len(unit["text"]) for unit in source_units))
                result = reuse_extraction(state, job, out) if extract_only else None
                if result is None:
                    result = pipeline.build_pipeline(state, [job["source_id"]], model=model, max_calls=max_calls,
                                                     output_tokens=output_tokens, chunk_chars=effective_chars,
                                                     out=out, extract_only=extract_only)
                audit = read(out / "audit.json") if not extract_only else {"summary": {"narrative_audit": "not run during knowledge pass"}}
                points = read(out / "points.json")
                pack = {"job_id": job["job_id"], "source_id": job["source_id"], "title": job["title"],
                        "evidence_kind": meta["kind"], "run_result": result, "model_audit": audit["summary"],
                        "uncovered_points": points.get("uncovered_narrative_points", []),
                        "draft_sha256": pipeline.sha256_file(out / "draft.md") if not extract_only else None,
                        "supervisor_review": "pending", "merged": False,
                        "nonextractable_pages": job.get("nonextractable_pages", []),
                        "required_checks": ["source support and prototype limits", "first-use definitions",
                                            "worked examples and reconstruction depth", "overlap with existing book",
                                            "math/code correctness and figure readability"]}
                pipeline.atomic_json(out / "supervisor-pack.json", pack)
                job.update(status="knowledge_extracted" if extract_only else "draft_ready",
                           draft_dir=str(out), draft_sha256=pack["draft_sha256"])
                attempt.update(status=job["status"], model_calls=result["uncached_model_calls"])
            except pipeline.CallBudgetExceeded as error:
                job.update(status="paused_call_budget", draft_dir=str(out), last_error=str(error))
                attempt.update(status=job["status"], error=str(error))
            except (pipeline.PipelineError, OSError, ValueError) as error:
                job.update(status="failed", draft_dir=str(out), last_error=str(error))
                attempt.update(status="failed", error=str(error))
            attempt["elapsed_seconds"] = round(time.monotonic() - started, 2)
            attempt["finished_at"] = pipeline.iso_utc()
            job["attempts"].append(attempt)
            pipeline.atomic_json(state / "queue.json", queue)
            status(state)
            print(json.dumps({"job_id": job["job_id"], **attempt}), flush=True)
        return status(state)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--state-dir", type=Path, default=DEFAULT_STATE)
    sub = parser.add_subparsers(dest="command", required=True)
    command = sub.add_parser("plan")
    command.add_argument("--pages-per-job", type=int, default=12)
    sub.add_parser("status")
    command = sub.add_parser("work")
    command.add_argument("--max-jobs", type=int, default=50, help="0 processes all queued jobs once")
    command.add_argument("--max-calls", type=int, default=200)
    command.add_argument("--model", default="qwen3:4b")
    command.add_argument("--output-tokens", type=int, default=8192)
    command.add_argument("--chunk-chars", type=int, default=4000)
    command.add_argument("--retry-failed", action="store_true")
    command.add_argument("--only", action="append", help="A particular planned job ID")
    command.add_argument("--phase", choices=["extract", "all"], default="extract",
                         help="Gather the whole source corpus first; all additionally writes and audits drafts")
    args = parser.parse_args()
    state = args.state_dir.expanduser().resolve()
    pipeline.ensure_state_dirs(state)
    try:
        if args.command == "plan":
            result = plan(state, args.pages_per_job)
        elif args.command == "work":
            result = work(state, args.max_jobs, args.max_calls, args.model, args.output_tokens,
                          args.chunk_chars, args.retry_failed, args.only, args.phase == "extract")
        else:
            result = status(state)
        print(json.dumps(result, indent=2))
    except (pipeline.PipelineError, OSError, ValueError) as error:
        print(json.dumps({"status": "error", "error": str(error)}), file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
