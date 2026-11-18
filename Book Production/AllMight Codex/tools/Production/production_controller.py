#!/usr/bin/env python3
"""Checkpointed, fingerprinted controller for the AllMight book production pipeline.

The default command is a read-only plan. Stage commands are argv arrays and are
never invoked unless the explicit `run` subcommand is selected.
"""
from __future__ import annotations

import argparse
import contextlib
import datetime as dt
import fcntl
import hashlib
import json
import os
import re
import subprocess
import sys
import time
import uuid
from pathlib import Path
from typing import Any, Iterator

SCHEMA = "allmight-production-controller-v1"
STATE_SCHEMA = "allmight-production-controller-state-v1"


class ControllerError(RuntimeError):
    pass


def utc_now() -> str:
    return dt.datetime.now(dt.timezone.utc).isoformat(timespec="seconds").replace("+00:00", "Z")


def sha_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def sha_file(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def canonical_hash(value: Any) -> str:
    return sha_bytes(json.dumps(value, sort_keys=True, ensure_ascii=False, separators=(",", ":")).encode("utf-8"))


def atomic_json(path: Path, value: Any) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    tmp = path.with_name(path.name + f".{os.getpid()}.tmp")
    tmp.write_text(json.dumps(value, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    os.replace(tmp, path)


def is_within(path: Path, parent: Path) -> bool:
    try:
        path.resolve().relative_to(parent.resolve())
        return True
    except ValueError:
        return False


def read_json(path: Path) -> Any:
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise ControllerError(f"Cannot read JSON {path}: {exc}") from exc


def source_rows(manifest_path: Path) -> tuple[list[dict[str, Any]], list[dict[str, Any]]]:
    """Read chapter source fingerprints declared by the production manifest."""
    data = read_json(manifest_path)
    rows: list[dict[str, Any]] = []
    errors: list[dict[str, Any]] = []
    for volume in data.get("volumes", []):
        for index, chapter in enumerate(volume.get("chapters", []), 1):
            source_value = chapter.get("source")
            expected = chapter.get("source_sha256") or chapter.get("file_sha256")
            if not source_value:
                errors.append({"kind": "missing-source-path", "volume": volume.get("id"), "chapter": chapter.get("id"), "index": index})
                continue
            path = Path(source_value)
            row = {"volume": volume.get("id"), "chapter_id": chapter.get("id"), "source": str(path), "expected_sha256": expected}
            if not path.is_file():
                row["actual_sha256"] = None
                row["status"] = "missing"
                errors.append({"kind": "missing-source", **row})
            else:
                actual = sha_file(path)
                row["actual_sha256"] = actual
                row["status"] = "verified" if expected and actual == expected else "fingerprint-missing" if not expected else "mismatch"
                if row["status"] != "verified":
                    errors.append({"kind": "source-hash-" + row["status"], **row})
            rows.append(row)
    return rows, errors


def validate_config(config: dict[str, Any], config_path: Path) -> dict[str, Any]:
    errors: list[str] = []
    if config.get("schema") != SCHEMA:
        errors.append(f"schema must be {SCHEMA}")
    workspace = config.get("workspace", {})
    for key in ("source_root", "production_root", "private_root", "manifest"):
        if not workspace.get(key):
            errors.append(f"workspace.{key} is required")
    private_root = Path(workspace.get("private_root", ".")).resolve()
    source_root = Path(workspace.get("source_root", ".")).resolve()
    production_root = Path(workspace.get("production_root", ".")).resolve()
    if is_within(private_root, source_root) or is_within(private_root, production_root):
        errors.append("workspace.private_root must be outside source_root and production_root")
    state_root_value = config.get("state_root")
    if not state_root_value:
        errors.append("state_root is required and must be private")
    else:
        state_root = Path(state_root_value).resolve()
        if not is_within(state_root, private_root):
            errors.append("state_root must be inside workspace.private_root")
        if is_within(state_root, source_root) or is_within(state_root, production_root):
            errors.append("state_root must not be inside source_root or production_root")
    manifest_path = Path(workspace.get("manifest", ".")).resolve()
    if not manifest_path.is_file():
        errors.append(f"production manifest is missing: {manifest_path}")

    fingerprints = config.get("fingerprints", [])
    fingerprint_names: set[str] = set()
    for item in fingerprints:
        name = str(item.get("name", ""))
        path_value = item.get("path")
        if not name or name in fingerprint_names:
            errors.append(f"fingerprint name is missing or duplicated: {name!r}")
        fingerprint_names.add(name)
        if not path_value:
            errors.append(f"fingerprint {name!r} has no path")
        elif not Path(path_value).is_file():
            errors.append(f"fingerprint {name!r} is missing: {path_value}")
    manual_ids = set()
    for gate in config.get("manual_gates", []):
        gid = str(gate.get("id", ""))
        if not gid or gid in manual_ids:
            errors.append(f"manual gate id is missing or duplicated: {gid!r}")
        manual_ids.add(gid)
        if gate.get("domain") not in ("print", "epub", "native", "collection", "custom"):
            errors.append(f"manual gate {gid!r} has an unknown domain")

    stages = config.get("stages", [])
    stage_ids: set[str] = set()
    for stage in stages:
        sid = str(stage.get("id", ""))
        if not sid or sid in stage_ids:
            errors.append(f"stage id is missing or duplicated: {sid!r}")
        stage_ids.add(sid)
        if stage.get("domain") not in ("links", "source", "assets", "print", "epub", "native", "package", "custom"):
            errors.append(f"stage {sid!r} has an unknown domain")
        if stage.get("domain") == "native" and not stage.get("exclusive_resource"):
            errors.append(f"native stage {sid!r} must declare exclusive_resource")
        if not isinstance(stage.get("argv"), list) or not stage.get("argv"):
            if not stage.get("units"):
                errors.append(f"stage {sid!r} needs argv or per-chapter units")
        if stage.get("argv") and any(not isinstance(arg, str) for arg in stage["argv"]):
            errors.append(f"stage {sid!r} argv must be a string array")
        if stage.get("verify_argv") and (not isinstance(stage["verify_argv"], list) or not stage["verify_argv"]):
            errors.append(f"stage {sid!r} verify_argv must be a nonempty string array")
        for dep in stage.get("depends_on", []):
            if dep == sid:
                errors.append(f"stage {sid!r} cannot depend on itself")
        for unit in stage.get("units", []):
            if not unit.get("id") or not unit.get("argv"):
                errors.append(f"stage {sid!r} has a unit missing id or argv")
            if any(not isinstance(arg, str) for arg in unit.get("argv", [])):
                errors.append(f"stage {sid!r} unit argv must be a string array")
            if unit.get("verify_argv") and (not isinstance(unit["verify_argv"], list) or not unit["verify_argv"]):
                errors.append(f"stage {sid!r} unit verify_argv must be a nonempty string array")
    gate_domains = {gate.get("domain") for gate in config.get("manual_gates", [])}
    for domain in ("print", "epub"):
        if any(stage.get("domain") == domain and stage.get("requires_visual_review") for stage in stages) and domain not in gate_domains:
            errors.append(f"{domain} stages requiring visual review need a separate manual gate in that domain")

    if len(stage_ids) == len(stages):
        for stage in stages:
            unknown = set(stage.get("depends_on", [])) - stage_ids
            if unknown:
                errors.append(f"stage {stage['id']!r} has unknown dependencies: {sorted(unknown)}")
        try:
            topological_stages(stages)
        except ControllerError as exc:
            errors.append(str(exc))

    if errors:
        raise ControllerError("Invalid controller configuration:\n- " + "\n- ".join(errors))
    return {
        "source_root": source_root,
        "production_root": production_root,
        "private_root": private_root,
        "state_root": Path(state_root_value).resolve(),
        "manifest_path": manifest_path,
        "fingerprints": fingerprints,
        "stages": stages,
        "manual_gates": config.get("manual_gates", []),
        "config_path": config_path.resolve(),
    }


def topological_stages(stages: list[dict[str, Any]]) -> list[dict[str, Any]]:
    by_id = {stage["id"]: stage for stage in stages}
    ordered: list[dict[str, Any]] = []
    temporary: set[str] = set()
    permanent: set[str] = set()

    def visit(sid: str) -> None:
        if sid in permanent:
            return
        if sid in temporary:
            raise ControllerError(f"stage dependency cycle includes {sid!r}")
        temporary.add(sid)
        for dep in by_id[sid].get("depends_on", []):
            if dep not in by_id:
                raise ControllerError(f"stage {sid!r} depends on unknown stage {dep!r}")
            visit(dep)
        temporary.remove(sid)
        permanent.add(sid)
        ordered.append(by_id[sid])

    for stage in stages:
        visit(stage["id"])
    return ordered


def input_snapshot(validated: dict[str, Any]) -> dict[str, Any]:
    entries: list[dict[str, Any]] = []
    fingerprint_errors: list[dict[str, Any]] = []
    for item in validated["fingerprints"]:
        path = Path(item["path"]).resolve()
        actual = sha_file(path)
        expected = item.get("expected_sha256") or item.get("sha256")
        row = {"name": item["name"], "role": item.get("role", "input"), "path": str(path), "sha256": actual, "expected_sha256": expected}
        row["status"] = "verified" if not expected or expected == actual else "mismatch"
        if row["status"] == "mismatch":
            fingerprint_errors.append({"kind": "fingerprint-mismatch", **row})
        entries.append(row)
    manifest = validated["manifest_path"]
    chapter_rows, source_errors = source_rows(manifest)
    return {
        "config_path": str(validated["config_path"]),
        "config_sha256": sha_file(validated["config_path"]),
        "manifest_path": str(manifest),
        "manifest_sha256": sha_file(manifest),
        "fingerprints": entries,
        "fingerprint_errors": fingerprint_errors,
        "chapters": chapter_rows,
        "chapter_source_errors": source_errors,
    }


def stage_fingerprints(stages: list[dict[str, Any]], snapshot: dict[str, Any]) -> dict[str, str]:
    fp_by_name = {item["name"]: item["sha256"] for item in snapshot["fingerprints"]}
    result: dict[str, str] = {}
    for stage in topological_stages(stages):
        names = stage.get("inputs", [])
        missing = set(names) - set(fp_by_name)
        if missing:
            raise ControllerError(f"stage {stage['id']!r} references unknown fingerprints: {sorted(missing)}")
        all_names = set(fp_by_name) if stage.get("fingerprint_all_inputs", True) else set(names)
        payload = {
            "stage": stage,
            # A stage may opt into a narrow checkpoint dependency set when its
            # command validates a stable global input package independently of
            # downstream per-document runner/config artifacts.
            "all_declared_inputs": {name: fp_by_name[name] for name in sorted(all_names)},
            "inputs": {name: fp_by_name[name] for name in sorted(names)},
            "dependencies": {dep: result[dep] for dep in sorted(stage.get("depends_on", []))},
            "config_sha256": snapshot["config_sha256"] if stage.get("fingerprint_all_inputs", True) else None,
            "manifest_sha256": snapshot["manifest_sha256"],
        }
        result[stage["id"]] = canonical_hash(payload)
    return result


def template_context(validated: dict[str, Any], snapshot: dict[str, Any], state_dir: Path | None = None, unit: dict[str, Any] | None = None) -> dict[str, str]:
    ctx = {
        "source_root": str(validated["source_root"]),
        "production_root": str(validated["production_root"]),
        "private_root": str(validated["private_root"]),
        "manifest": str(validated["manifest_path"]),
        "controller": str(Path(__file__).resolve()),
        "config": str(validated["config_path"]),
        "state_dir": str(state_dir or validated["state_root"]),
        "python": sys.executable,
    }
    if unit:
        ctx.update({k: str(v) for k, v in unit.items() if isinstance(v, (str, int, float))})
        if "source" in unit:
            ctx["chapter_source"] = str(unit["source"])
    return ctx


def render_argv(template: list[str], context: dict[str, str]) -> list[str]:
    rendered: list[str] = []
    for arg in template:
        try:
            value = arg.format_map(context)
        except KeyError as exc:
            raise ControllerError(f"unbound command template variable {exc.args[0]!r}") from exc
        rendered.append(value)
    return rendered


def manifest_chapters(snapshot: dict[str, Any]) -> dict[str, dict[str, Any]]:
    return {str(row["chapter_id"]): row for row in snapshot.get("chapters", []) if row.get("chapter_id")}


def plan_data(config: dict[str, Any], config_path: Path, chapter_filter: str | None = None) -> dict[str, Any]:
    validated = validate_config(config, config_path)
    snapshot = input_snapshot(validated)
    stage_hashes = stage_fingerprints(validated["stages"], snapshot)
    chapters = manifest_chapters(snapshot)
    if chapter_filter and chapter_filter not in chapters:
        raise ControllerError(f"unknown chapter id for retry: {chapter_filter}")
    chapter_error_ids = {row.get("chapter_id") for row in snapshot.get("chapter_source_errors", [])}
    stages_out = []
    for stage in topological_stages(validated["stages"]):
        units = stage.get("units", [])
        if chapter_filter:
            selected_units = [unit for unit in units if unit.get("id") == chapter_filter]
            applies = bool(selected_units)
        else:
            selected_units = units
            applies = True
        commands = []
        for unit in selected_units:
            unit_context = dict(chapters.get(unit.get("id"), {}));unit_context.update(unit)
            ctx = template_context(validated, snapshot, unit=unit_context)
            commands.append({
                "unit_id": unit["id"],
                "argv": render_argv(unit["argv"], ctx),
                "verify_argv": render_argv(unit.get("verify_argv", stage.get("verify_argv", [])), ctx) or None,
                "status": "blocked_source_fingerprint" if unit.get("id") in chapter_error_ids else "planned",
            })
        if not units and applies:
            ctx = template_context(validated, snapshot)
            commands.append({
                "unit_id": None,
                "argv": render_argv(stage.get("argv", []), ctx),
                "verify_argv": render_argv(stage.get("verify_argv", []), ctx) or None,
                "status": "planned",
            })
        stages_out.append({
            "id": stage["id"],
            "domain": stage["domain"],
            "depends_on": stage.get("depends_on", []),
            "exclusive_resource": stage.get("exclusive_resource"),
            "requires_visual_review": bool(stage.get("requires_visual_review", False)),
            "fingerprint": stage_hashes[stage["id"]],
            "applies_to_selected_chapter": applies,
            "commands": commands,
        })
    delivery_domains = {}
    for domain in ("print", "epub", "native", "collection"):
        domain_stages = [stage for stage in stages_out if stage["domain"] == domain]
        domain_gates = [gate for gate in validated["manual_gates"] if gate.get("domain") == domain]
        if domain_stages or domain_gates:
            delivery_domains[domain] = {
                "stages": [stage["id"] for stage in domain_stages],
                "manual_review_gates": [gate["id"] for gate in domain_gates],
                "requires_human_review": any(stage["requires_visual_review"] for stage in domain_stages) or bool(domain_gates),
            }
    return {
        "schema": "allmight-production-controller-plan-v1",
        "mode": "read-only-plan",
        "generated_at": utc_now(),
        "config_path": str(config_path.resolve()),
        "config_sha256": snapshot["config_sha256"],
        "manifest_sha256": snapshot["manifest_sha256"],
        "chapter_count": len(chapters),
        "chapter_fingerprint_errors": snapshot["chapter_source_errors"],
        "fingerprint_errors": snapshot["fingerprint_errors"],
        "stage_count": len(stages_out),
        "stages": stages_out,
        "delivery_domains": delivery_domains,
        "manual_gates": validated["manual_gates"],
        "visual_review_required": any(stage.get("requires_visual_review") for stage in validated["stages"]),
        "plan_is_not_evidence_of_execution": True,
    }


@contextlib.contextmanager
def exclusive_lock(path: Path, blocking: bool = False) -> Iterator[None]:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("a+") as stream:
        flags = fcntl.LOCK_EX | (0 if blocking else fcntl.LOCK_NB)
        try:
            fcntl.flock(stream.fileno(), flags)
        except BlockingIOError as exc:
            raise ControllerError(f"exclusive lock is already held: {path}") from exc
        try:
            yield
        finally:
            fcntl.flock(stream.fileno(), fcntl.LOCK_UN)


def resource_name(value: str) -> str:
    return re.sub(r"[^A-Za-z0-9_.-]+", "_", value)


def quarantine_path(lock_root: Path, resource: str) -> Path:
    return lock_root / (resource_name(resource) + ".quarantine.json")


def active_native_quarantine(lock_root: Path, resource: str) -> dict[str, Any] | None:
    path = quarantine_path(lock_root, resource)
    if not path.is_file():
        return None
    data = read_json(path)
    return data if data.get("active") else None


def record_native_timeout(lock_root: Path, resource: str, stage_id: str, unit_id: str, command: dict[str, Any]) -> dict[str, Any]:
    path = quarantine_path(lock_root, resource)
    previous = read_json(path) if path.is_file() else {"schema": "allmight-native-quarantine-v1", "history": []}
    event = {
        "stage_id": stage_id,
        "unit_id": unit_id,
        "timed_out_at": command.get("finished_at") or utc_now(),
        "command_argv": command.get("argv"),
        "command_error": command.get("error"),
        "stdout_sha256": command.get("stdout_sha256"),
        "stderr_sha256": command.get("stderr_sha256"),
    }
    previous.setdefault("history", []).append(event)
    previous.update({"schema": "allmight-native-quarantine-v1", "resource": resource, "active": True, "active_timeout": event, "quarantined_at": utc_now()})
    atomic_json(path, previous)
    return previous


def command_timed_out(record: dict[str, Any] | None) -> bool:
    return bool(record and str(record.get("error") or "").startswith("timeout after "))


def revalidate_inputs(validated: dict[str, Any], baseline: dict[str, Any]) -> tuple[bool, dict[str, Any]]:
    current = input_snapshot(validated)
    return canonical_hash(current) == canonical_hash(baseline), current


def save_state(path: Path, state: dict[str, Any]) -> None:
    state["updated_at"] = utc_now()
    atomic_json(path, state)


def load_state(path: Path, run_id: str, config_sha: str) -> dict[str, Any]:
    if not path.is_file():
        return {"schema": STATE_SCHEMA, "run_id": run_id, "config_sha256": config_sha, "stages": {}, "manual_gates": {}, "created_at": utc_now()}
    state = read_json(path)
    if state.get("schema") != STATE_SCHEMA or state.get("run_id") != run_id:
        raise ControllerError(f"state file schema/run id mismatch: {path}")
    return state


def resolve_output_paths(values: list[str], context: dict[str, str]) -> list[Path]:
    return [Path(render_argv([item], context)[0]).resolve() for item in values]


def output_fingerprints(paths: list[Path]) -> tuple[list[dict[str, Any]], list[str]]:
    rows = []
    missing = []
    for path in paths:
        if not path.is_file():
            missing.append(str(path))
        else:
            rows.append({"path": str(path), "sha256": sha_file(path), "bytes": path.stat().st_size})
    return rows, missing


def command_record(argv: list[str], cwd: Path, log_stem: Path, timeout: float | None) -> dict[str, Any]:
    started = utc_now()
    start_clock = time.monotonic()
    stdout_path = log_stem.with_suffix(".stdout.log")
    stderr_path = log_stem.with_suffix(".stderr.log")
    stdout_path.parent.mkdir(parents=True, exist_ok=True)
    try:
        with stdout_path.open("wb") as out, stderr_path.open("wb") as err:
            result = subprocess.run(argv, cwd=cwd, stdout=out, stderr=err, shell=False, timeout=timeout, check=False)
        exit_code = result.returncode
        error = None
    except subprocess.TimeoutExpired as exc:
        exit_code = None
        error = f"timeout after {timeout}s"
        stdout_path.write_bytes(exc.stdout if isinstance(exc.stdout, bytes) else (exc.stdout or "").encode())
        stderr_path.write_bytes(exc.stderr if isinstance(exc.stderr, bytes) else (exc.stderr or "").encode())
    except OSError as exc:
        exit_code = None
        error = repr(exc)
        stdout_path.write_text("", encoding="utf-8")
        stderr_path.write_text(error + "\n", encoding="utf-8")
    return {
        "argv": argv,
        "cwd": str(cwd),
        "started_at": started,
        "finished_at": utc_now(),
        "duration_seconds": round(time.monotonic() - start_clock, 3),
        "exit_code": exit_code,
        "error": error,
        "stdout_log": str(stdout_path.resolve()),
        "stderr_log": str(stderr_path.resolve()),
        "stdout_sha256": sha_file(stdout_path),
        "stderr_sha256": sha_file(stderr_path),
    }


def verified_prior(record: dict[str, Any] | None, fingerprint: str, outputs: list[Path], verify_argv: list[str], cwd: Path, log_stem: Path, timeout: float | None) -> tuple[bool, dict[str, Any] | None]:
    if not record or record.get("status") != "verified" or record.get("fingerprint") != fingerprint:
        return False, None
    current_outputs, missing = output_fingerprints(outputs)
    if missing or current_outputs != record.get("outputs", []):
        return False, None
    if not verify_argv:
        return False, None
    verification = command_record(verify_argv, cwd, log_stem.with_name(log_stem.name + "-resume-verify"), timeout)
    return verification.get("exit_code") == 0 and not verification.get("error"), verification


def overall_status(state: dict[str, Any], required_stages: list[str], manual_gates: list[dict[str, Any]]) -> str:
    if state.get("input_integrity", {}).get("status") != "verified":
        return "incomplete"
    for sid in required_stages:
        if state.get("stages", {}).get(sid, {}).get("status") != "verified":
            return "incomplete"
    if any(state.get("manual_gates", {}).get(gate["id"], {}).get("result") == "fail" for gate in manual_gates):
        return "review_failed"
    for gate in manual_gates:
        if state.get("manual_gates", {}).get(gate["id"], {}).get("result") != "pass":
            return "awaiting_visual_review"
    return "complete_verified"


def run_controller(config: dict[str, Any], config_path: Path, state_dir: Path, chapter_id: str | None = None, selected_stage: str | None = None) -> tuple[dict[str, Any], int]:
    validated = validate_config(config, config_path)
    if not is_within(state_dir.resolve(), validated["state_root"]) or is_within(state_dir.resolve(), validated["source_root"]) or is_within(state_dir.resolve(), validated["production_root"]):
        raise ControllerError("run state directory must be inside configured private state_root and outside source/production roots")
    snapshot = input_snapshot(validated)
    if snapshot["chapter_source_errors"] or snapshot["fingerprint_errors"]:
        raise ControllerError("source/design/generator/asset fingerprint preflight failed; no stage commands were launched")
    hashes = stage_fingerprints(validated["stages"], snapshot)
    chapters = manifest_chapters(snapshot)
    if chapter_id and chapter_id not in chapters:
        raise ControllerError(f"unknown chapter id for retry: {chapter_id}")
    stage_by_id = {stage["id"]: stage for stage in validated["stages"]}
    if selected_stage and selected_stage not in stage_by_id:
        raise ControllerError(f"unknown stage id: {selected_stage}")
    state_dir.mkdir(parents=True, exist_ok=True)
    state_path = state_dir / "state.json"
    state = load_state(state_path, state_dir.name, snapshot["config_sha256"])
    baseline_input_sha = canonical_hash(snapshot)
    state.update({
        "input_snapshot": snapshot,
        "plan_fingerprints": hashes,
        "input_integrity": {"status": "verified", "baseline_sha256": baseline_input_sha, "checked_at": utc_now()},
    })
    lock_root = validated["private_root"] / ".production-controller-locks"
    controller_lock = lock_root / "controller.lock"
    current_errors = 0
    selected_order = topological_stages(validated["stages"])
    if selected_stage:
        # Include required predecessors; later unrelated stages are not selected.
        needed = {selected_stage}
        todo = [selected_stage]
        while todo:
            sid = todo.pop()
            for dep in stage_by_id[sid].get("depends_on", []):
                if dep not in needed:
                    needed.add(dep);todo.append(dep)
        selected_order = [stage for stage in selected_order if stage["id"] in needed]

    with exclusive_lock(controller_lock):
        for stage in selected_order:
            sid = stage["id"]
            rec = state.setdefault("stages", {}).setdefault(sid, {"units": {}})
            inputs_match, current_snapshot = revalidate_inputs(validated, snapshot)
            if not inputs_match:
                state["input_integrity"] = {
                    "status": "changed_during_run",
                    "baseline_sha256": baseline_input_sha,
                    "current_sha256": canonical_hash(current_snapshot),
                    "checked_at": utc_now(),
                    "changed_snapshot": current_snapshot,
                }
                rec.update({"status": "blocked_input_fingerprint_changed", "fingerprint": hashes[sid]})
                current_errors += 1;save_state(state_path, state);break
            if stage.get("domain") == "native":
                lock_name = stage["exclusive_resource"]
                quarantine = active_native_quarantine(lock_root, lock_name)
                if quarantine:
                    rec.update({
                        "status": "blocked_native_quarantine",
                        "exclusive_resource": lock_name,
                        "quarantine_path": str(quarantine_path(lock_root, lock_name)),
                        "active_timeout": quarantine.get("active_timeout"),
                        "fingerprint": hashes[sid],
                    })
                    current_errors += 1;save_state(state_path, state);continue
            dep_states = {dep: state["stages"].get(dep, {}).get("status") for dep in stage.get("depends_on", [])}
            bad_deps = [dep for dep, status in dep_states.items() if status != "verified"]
            if bad_deps:
                rec.update({"status": "blocked_dependency", "blocked_by": bad_deps, "fingerprint": hashes[sid]})
                current_errors += 1;save_state(state_path, state);continue
            if chapter_id and not stage.get("units"):
                unit_rec = rec.get("units", {}).get("__stage__", {})
                ctx = template_context(validated, snapshot, state_dir)
                verify_argv = render_argv(stage.get("verify_argv", []), ctx) if stage.get("verify_argv") else []
                outputs = resolve_output_paths(stage.get("outputs", []), ctx)
                unit_fp = canonical_hash({"stage_fp": hashes[sid], "unit": None, "argv": render_argv(stage.get("argv", []), ctx), "verify_argv": verify_argv, "source": None})
                if rec.get("status") == "verified" and rec.get("fingerprint") == hashes[sid]:
                    verify_log = state_dir / "logs" / sid / "chapter-retry-global-verify"
                    try:
                        lock_name = stage.get("exclusive_resource")
                        stage_lock = lock_root / (re.sub(r"[^A-Za-z0-9_.-]+", "_", lock_name) + ".lock") if lock_name else None
                        lock_context = exclusive_lock(stage_lock) if stage_lock else contextlib.nullcontext()
                        with lock_context:
                            can_reuse, verification = verified_prior(unit_rec, unit_fp, outputs, verify_argv, Path(stage.get("cwd", str(validated["production_root"]))), verify_log, float(stage["timeout_seconds"]) if stage.get("timeout_seconds") else None)
                        if stage.get("domain") == "native" and verification and command_timed_out(verification):
                            record_native_timeout(lock_root, stage["exclusive_resource"], sid, "__stage__", verification)
                            rec.update({
                                "status": "blocked_native_quarantine",
                                "exclusive_resource": stage["exclusive_resource"],
                                "quarantine_path": str(quarantine_path(lock_root, stage["exclusive_resource"])),
                                "fingerprint": hashes[sid],
                            })
                            current_errors += 1
                            save_state(state_path, state)
                            continue
                        if can_reuse:
                            unit_rec.update({"status": "verified", "fingerprint": unit_fp, "verification": verification, "resumed": True})
                            rec["units"]["__stage__"] = unit_rec
                            rec.update({"status": "verified", "fingerprint": hashes[sid], "chapter_retry_reused": True})
                        else:
                            rec.update({"status": "blocked_stale_dependency", "fingerprint": hashes[sid], "reason": "global stage result could not be reverified for chapter retry"})
                            current_errors += 1
                    except ControllerError as exc:
                        rec.update({"status": "blocked_exclusive_lock", "error": str(exc), "fingerprint": hashes[sid]})
                        current_errors += 1
                else:
                    rec.update({"status": "not_applicable_to_chapter_retry", "fingerprint": hashes[sid]})
                save_state(state_path, state);continue
            units = stage.get("units", [])
            selected_units = [unit for unit in units if not chapter_id or unit.get("id") == chapter_id]
            if units and not selected_units:
                rec.update({"status": "no_selected_unit", "fingerprint": hashes[sid]})
                save_state(state_path, state);continue
            commands = selected_units or [None]
            lock_name = stage.get("exclusive_resource")
            stage_lock = lock_root / (resource_name(lock_name) + ".lock") if lock_name else None
            lock_context = exclusive_lock(stage_lock) if stage_lock else contextlib.nullcontext()
            try:
                with lock_context:
                    stage_failures = []
                    for unit in commands:
                        unit_id = unit["id"] if unit else "__stage__"
                        unit_rec = rec["units"].setdefault(unit_id, {})
                        unit_context = dict(chapters.get(unit_id, {}));unit_context.update(unit or {})
                        ctx = template_context(validated, snapshot, state_dir, unit_context if unit else None)
                        argv = render_argv(unit["argv"], ctx) if unit else render_argv(stage["argv"], ctx)
                        verify_template = (unit or {}).get("verify_argv", stage.get("verify_argv", []))
                        verify_argv = render_argv(verify_template, ctx) if verify_template else []
                        cwd = Path((unit or {}).get("cwd", stage.get("cwd", str(validated["production_root"])))).resolve()
                        outputs = resolve_output_paths((unit or {}).get("outputs", stage.get("outputs", [])), ctx)
                        unit_fp = canonical_hash({"stage_fp": hashes[sid], "unit": unit, "argv": argv, "verify_argv": verify_argv, "source": chapters.get(unit_id)})
                        timeout = float(stage["timeout_seconds"]) if stage.get("timeout_seconds") else None
                        can_reuse, previous_verify = verified_prior(unit_rec, unit_fp, outputs, verify_argv, cwd, state_dir / "logs" / sid / f"{unit_id}-reuse", timeout)
                        if can_reuse:
                            unit_rec.update({"status": "verified", "fingerprint": unit_fp, "resumed": True, "verification": previous_verify})
                            save_state(state_path, state);continue
                        unit_rec.update({"status": "running", "fingerprint": unit_fp, "resumed": False})
                        save_state(state_path, state)
                        command_log = state_dir / "logs" / sid / unit_id
                        command = command_record(argv, cwd, command_log, timeout)
                        unit_rec["command"] = command
                        if stage.get("domain") == "native" and command_timed_out(command):
                            quarantine = record_native_timeout(lock_root, stage["exclusive_resource"], sid, unit_id, command)
                            unit_rec["native_quarantine_path"] = str(quarantine_path(lock_root, stage["exclusive_resource"]))
                            unit_rec["native_quarantine_active"] = bool(quarantine.get("active"))
                        if command["exit_code"] != 0 or command["error"]:
                            unit_rec.update({"status": "command_failed", "outputs": [], "verification": None})
                            stage_failures.append(unit_id);current_errors += 1;save_state(state_path, state);continue
                        output_rows, missing = output_fingerprints(outputs)
                        unit_rec["outputs"] = output_rows
                        if missing:
                            unit_rec.update({"status": "output_gate_failed", "missing_outputs": missing, "verification": None})
                            stage_failures.append(unit_id);current_errors += 1;save_state(state_path, state);continue
                        if not verify_argv:
                            unit_rec.update({"status": "succeeded_unverified", "verification": None})
                            stage_failures.append(unit_id);current_errors += 1;save_state(state_path, state);continue
                        verification = command_record(verify_argv, cwd, command_log.with_name(command_log.name + "-verify"), timeout)
                        unit_rec["verification"] = verification
                        if stage.get("domain") == "native" and command_timed_out(verification):
                            quarantine = record_native_timeout(lock_root, stage["exclusive_resource"], sid, unit_id, verification)
                            unit_rec["native_quarantine_path"] = str(quarantine_path(lock_root, stage["exclusive_resource"]))
                            unit_rec["native_quarantine_active"] = bool(quarantine.get("active"))
                        if verification["exit_code"] != 0 or verification["error"]:
                            unit_rec.update({"status": "verification_failed"})
                            stage_failures.append(unit_id);current_errors += 1;save_state(state_path, state);continue
                        inputs_match, current_snapshot = revalidate_inputs(validated, snapshot)
                        if not inputs_match:
                            state["input_integrity"] = {
                                "status": "changed_during_run",
                                "baseline_sha256": baseline_input_sha,
                                "current_sha256": canonical_hash(current_snapshot),
                                "checked_at": utc_now(),
                                "changed_snapshot": current_snapshot,
                            }
                            unit_rec.update({"status": "stale_input_fingerprint", "stale_input_check_at": utc_now()})
                            stage_failures.append(unit_id);current_errors += 1;save_state(state_path, state);break
                        unit_rec.update({"status": "verified", "verified_at": utc_now()})
                        save_state(state_path, state)
                    if stage_failures:
                        rec.update({"status": "failed", "fingerprint": hashes[sid], "failed_units": stage_failures})
                    elif units:
                        all_units = {u["id"] for u in units}
                        states = {key: value.get("status") for key, value in rec["units"].items()}
                        rec.update({"status": "verified" if all(states.get(uid) == "verified" for uid in all_units) else "partial", "fingerprint": hashes[sid]})
                    else:
                        rec.update({"status": "verified", "fingerprint": hashes[sid]})
            except ControllerError as exc:
                rec.update({"status": "blocked_exclusive_lock", "error": str(exc), "fingerprint": hashes[sid]})
                current_errors += 1
            save_state(state_path, state)
            if state.get("input_integrity", {}).get("status") != "verified":
                break
        required_stages = [stage["id"] for stage in validated["stages"] if stage.get("required", True)]
        inputs_match, final_snapshot = revalidate_inputs(validated, snapshot)
        if not inputs_match:
            state["input_integrity"] = {
                "status": "changed_during_run",
                "baseline_sha256": baseline_input_sha,
                "current_sha256": canonical_hash(final_snapshot),
                "checked_at": utc_now(),
                "changed_snapshot": final_snapshot,
            }
            current_errors += 1
        state["run_status"] = overall_status(state, required_stages, validated["manual_gates"])
        state["run_finished_at"] = utc_now()
        state["chapter_retry"] = chapter_id
        save_state(state_path, state)
    return state, (0 if state["run_status"] == "complete_verified" else 3 if state["run_status"] == "awaiting_visual_review" else 2 if current_errors else 1)


def record_review(config: dict[str, Any], config_path: Path, state_dir: Path, gate_id: str, reviewer: str, evidence_path: Path, result: str, note: str = "") -> dict[str, Any]:
    validated = validate_config(config, config_path)
    if not is_within(state_dir.resolve(), validated["state_root"]) or is_within(state_dir.resolve(), validated["source_root"]) or is_within(state_dir.resolve(), validated["production_root"]):
        raise ControllerError("review state directory must be private")
    gate = next((item for item in validated["manual_gates"] if item.get("id") == gate_id), None)
    if gate is None:
        raise ControllerError(f"unknown manual review gate: {gate_id}")
    if not reviewer.strip() or result not in ("pass", "fail"):
        raise ControllerError("review requires a nonempty reviewer and result pass|fail")
    evidence_path = evidence_path.resolve()
    if not evidence_path.is_file():
        raise ControllerError(f"review evidence file is missing: {evidence_path}")
    state_path = state_dir / "state.json"
    lock_path = validated["private_root"] / ".production-controller-locks" / "controller.lock"
    with exclusive_lock(lock_path):
        state = load_state(state_path, state_dir.name, sha_file(config_path))
        state.setdefault("manual_gates", {})[gate_id] = {
            "result": result,
            "reviewer": reviewer.strip(),
            "evidence_path": str(evidence_path),
            "evidence_sha256": sha_file(evidence_path),
            "note": note,
            "recorded_at": utc_now(),
        }
        required_stages = [stage["id"] for stage in validated["stages"] if stage.get("required", True)]
        state["run_status"] = overall_status(state, required_stages, validated["manual_gates"])
        save_state(state_path, state)
    return state


def parse_utc_timestamp(value: Any, label: str) -> dt.datetime:
    if not isinstance(value, str) or not value.strip():
        raise ControllerError(f"heartbeat evidence requires {label}")
    try:
        parsed = dt.datetime.fromisoformat(value.replace("Z", "+00:00"))
    except ValueError as exc:
        raise ControllerError(f"heartbeat evidence has invalid {label}: {value!r}") from exc
    if parsed.tzinfo is None:
        raise ControllerError(f"heartbeat evidence {label} must include a timezone")
    return parsed.astimezone(dt.timezone.utc)


def clear_native_quarantine(config: dict[str, Any], config_path: Path, resource: str, supervisor: str, heartbeat_evidence_path: Path, note: str = "") -> dict[str, Any]:
    validated = validate_config(config, config_path)
    known_resources = {stage.get("exclusive_resource") for stage in validated["stages"] if stage.get("domain") == "native"}
    if resource not in known_resources:
        raise ControllerError(f"unknown native exclusive_resource: {resource}")
    if not supervisor.strip():
        raise ControllerError("quarantine clear requires a named supervisor")
    evidence_path = heartbeat_evidence_path.resolve()
    if not evidence_path.is_file():
        raise ControllerError(f"heartbeat evidence file is missing: {evidence_path}")
    evidence = read_json(evidence_path)
    if not isinstance(evidence, dict):
        raise ControllerError("heartbeat evidence must be a JSON object")
    if evidence.get("resource") != resource or evidence.get("status") != "idle" or evidence.get("idle_confirmed") is not True:
        raise ControllerError("heartbeat evidence must confirm this resource is idle")
    heartbeat_path = Path(evidence.get("heartbeat_path", "")).resolve()
    if not heartbeat_path.is_file():
        raise ControllerError("heartbeat evidence must identify an existing heartbeat file")
    heartbeat_sha = sha_file(heartbeat_path)
    if evidence.get("heartbeat_sha256") != heartbeat_sha:
        raise ControllerError("heartbeat file hash does not match the supplied evidence")

    lock_root = validated["private_root"] / ".production-controller-locks"
    path = quarantine_path(lock_root, resource)
    controller_lock = lock_root / "controller.lock"
    resource_lock = lock_root / (resource_name(resource) + ".lock")
    with exclusive_lock(controller_lock):
        with exclusive_lock(resource_lock):
            if not path.is_file():
                raise ControllerError(f"no quarantine record exists for {resource}")
            quarantine = read_json(path)
            if not quarantine.get("active"):
                raise ControllerError(f"native resource {resource} is not quarantined")
            timed_out_at = parse_utc_timestamp(quarantine.get("active_timeout", {}).get("timed_out_at"), "timeout timestamp")
            observed_at = parse_utc_timestamp(evidence.get("observed_at"), "observed_at")
            if observed_at <= timed_out_at:
                raise ControllerError("idle heartbeat observation must be later than the recorded native timeout")
            heartbeat = read_json(heartbeat_path)
            if not isinstance(heartbeat, dict) or heartbeat.get("resource") != resource or heartbeat.get("status") != "idle" or heartbeat.get("idle_confirmed") is not True:
                raise ControllerError("heartbeat file must confirm this resource is idle")
            heartbeat_observed_at = parse_utc_timestamp(heartbeat.get("observed_at"), "heartbeat observed_at")
            if heartbeat_observed_at <= timed_out_at or heartbeat_observed_at != observed_at:
                raise ControllerError("heartbeat file must contain the matching post-timeout idle observation")
            if sha_file(heartbeat_path) != heartbeat_sha:
                raise ControllerError("heartbeat file changed while quarantine evidence was being checked")
            clear_record = {
                "supervisor": supervisor.strip(),
                "note": note,
                "cleared_at": utc_now(),
                "heartbeat_evidence_path": str(evidence_path),
                "heartbeat_evidence_sha256": sha_file(evidence_path),
                "heartbeat_path": str(heartbeat_path),
                "heartbeat_sha256": heartbeat_sha,
                "observed_at": evidence["observed_at"],
            }
            quarantine.setdefault("clear_history", []).append(clear_record)
            quarantine.update({"active": False, "cleared_by": clear_record, "clear_reason": "supervisor-confirmed idle heartbeat"})
            atomic_json(path, quarantine)
    return quarantine


def load_config(path: Path) -> dict[str, Any]:
    data = read_json(path)
    if not isinstance(data, dict):
        raise ControllerError("controller config must be a JSON object")
    return data


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--config", type=Path, required=True, help="JSON controller plan; commands are argv arrays, never shell strings")
    sub = parser.add_subparsers(dest="command")
    p_plan = sub.add_parser("plan", help="read-only preflight and stage plan; this is the default")
    p_plan.add_argument("--chapter", help="show only the per-chapter retry units for this chapter id")
    p_run = sub.add_parser("run", help="explicitly execute configured stage commands")
    p_run.add_argument("--state-dir", type=Path, required=True, help="private checkpoint/log directory")
    p_run.add_argument("--chapter", help="retry only configured units for this chapter id")
    p_run.add_argument("--stage", help="run this stage and any missing dependencies")
    p_review = sub.add_parser("review", help="record a human visual-review result against evidence")
    p_review.add_argument("--state-dir", type=Path, required=True)
    p_review.add_argument("--gate", required=True)
    p_review.add_argument("--reviewer", required=True)
    p_review.add_argument("--evidence", type=Path, required=True)
    p_review.add_argument("--result", choices=("pass", "fail"), required=True)
    p_review.add_argument("--note", default="")
    p_status = sub.add_parser("status", help="print saved checkpoint state without inferring completion from files")
    p_status.add_argument("--state-dir", type=Path, required=True)
    p_clear = sub.add_parser("clear-native-quarantine", help="supervisor clear after matching post-timeout idle-heartbeat evidence")
    p_clear.add_argument("--resource", required=True)
    p_clear.add_argument("--supervisor", required=True)
    p_clear.add_argument("--heartbeat-evidence", type=Path, required=True, help="JSON idle evidence with a post-timeout heartbeat file path and SHA-256")
    p_clear.add_argument("--note", default="")
    args = parser.parse_args(argv)
    command = args.command or "plan"
    try:
        config = load_config(args.config.resolve())
        if command == "plan":
            chapter = getattr(args, "chapter", None)
            data = plan_data(config, args.config.resolve(), chapter)
            print(json.dumps(data, ensure_ascii=False, indent=2))
            return 2 if data["chapter_fingerprint_errors"] or data["fingerprint_errors"] else 0
        if command == "run":
            state, code = run_controller(config, args.config.resolve(), args.state_dir.resolve(), args.chapter, args.stage)
            print(json.dumps({"run_status": state.get("run_status"), "state_file": str((args.state_dir / 'state.json').resolve()), "chapter_retry": args.chapter, "stages": state.get("stages", {}), "manual_gates": state.get("manual_gates", {})}, ensure_ascii=False, indent=2))
            return code
        if command == "review":
            state = record_review(config, args.config.resolve(), args.state_dir.resolve(), args.gate, args.reviewer, args.evidence, args.result, args.note)
            print(json.dumps({"gate": args.gate, "result": args.result, "run_status": state.get("run_status"), "state_file": str((args.state_dir / 'state.json').resolve())}, indent=2))
            return 0
        if command == "status":
            state_path = args.state_dir.resolve() / "state.json"
            if not state_path.is_file():
                print(json.dumps({"status": "no_execution_checkpoint", "state_file": str(state_path)}, indent=2))
                return 1
            state = read_json(state_path)
            print(json.dumps(state, ensure_ascii=False, indent=2))
            return 0
        if command == "clear-native-quarantine":
            quarantine = clear_native_quarantine(config, args.config.resolve(), args.resource, args.supervisor, args.heartbeat_evidence, args.note)
            print(json.dumps({"resource": args.resource, "active": quarantine.get("active"), "cleared_by": quarantine.get("cleared_by")}, ensure_ascii=False, indent=2))
            return 0
        raise ControllerError(f"unsupported command: {command}")
    except ControllerError as exc:
        print(json.dumps({"status": "blocked_preflight", "error": str(exc)}, ensure_ascii=False, indent=2), file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
