"""A reproducible, host-neutral frame-asset publication and evidence oracle.

The file intentionally avoids importing Blender, Maya, Houdini or Nuke.
Each DCC host is an adapter to this common validation/publication contract.
No operation mutates a project until publish(plan) is called explicitly.

Limits: single writer per target, one local filesystem for atomic rename,
no network filesystem durability claim, no pixel decoder or DCC invocation.
"""

from __future__ import annotations

import hashlib
import itertools
import json
import math
import os
import re
import shutil
import statistics
import tempfile
import time
import uuid
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Iterable, Sequence

SCHEMA_VERSION = 1
FRAME_PATTERN = re.compile(
    r"^(?P<stem>.+?)[._-](?P<frame>\d+)\.(?P<suffix>png|exr)$",
    flags=re.IGNORECASE,
)


@dataclass(frozen=True, order=True)
class Diagnostic:
    code: str
    severity: str
    message: str
    path: str = ""


@dataclass(frozen=True)
class Frame:
    frame: int
    source: str
    digest: str
    bytes: int


@dataclass(frozen=True)
class Plan:
    schema_version: int
    run_id: str
    workspace: str
    target: str
    stem: str
    first_frame: int
    last_frame: int
    padding: int
    frames: tuple[Frame, ...]
    diagnostics: tuple[Diagnostic, ...]

    @property
    def safe_to_publish(self) -> bool:
        return not any(d.severity == "error" for d in self.diagnostics)


def stable_json(value: object) -> bytes:
    """Stable encoding; the ordering and separators are part of the contract."""
    return (json.dumps(value, sort_keys=True, separators=(",", ":"), ensure_ascii=False)
            + "\n").encode("utf-8")


def fingerprint(path: Path) -> str:
    """Hash actual file content, not the path or modification timestamp."""
    digest = hashlib.sha256()
    with path.open("rb") as source:
        while chunk := source.read(1 << 20):
            digest.update(chunk)
    return digest.hexdigest()


def workspace_path(root: Path, candidate: Path) -> Path:
    """Reject .. and symlink escapes after canonicalizing the real directory.

    Treat input as untrusted, even if it originated in a DCC export script.
    A changed symlink between validation and open is a TOCTOU limitation:
    this is a single-writer educational oracle, not an OS sandbox.
    """
    real_root = root.resolve(strict=True)
    if not real_root.is_dir():
        raise ValueError("workspace root must be a directory")
    path = candidate if candidate.is_absolute() else real_root / candidate
    path = path.resolve(strict=False)
    if not path.is_relative_to(real_root):
        raise ValueError(f"path escapes workspace: {candidate}")
    return path


def parse_frame(name: str) -> tuple[str, int, int, str] | None:
    match = FRAME_PATTERN.fullmatch(name)
    if match is None:
        return None
    digits = match.group("frame")
    # A malformed untrusted name must not trigger huge-integer conversion work.
    if len(digits) > 12:
        return None
    return match.group("stem"), int(digits), len(digits), match.group("suffix").lower()


def make_plan(
    root: Path,
    source_names: Sequence[str],
    target_name: str,
    *,
    first: int,
    last: int,
    run_id: str = "deterministic-lab",
) -> Plan:
    """Collect deterministic diagnostics without mutating the workspace."""
    if first < 0 or last < first:
        raise ValueError("frame range must be a nonempty nonnegative interval")
    if last - first + 1 > 100_000:
        raise ValueError("frame range exceeds the explicit 100000-frame budget")
    if "/" in target_name or "\\" in target_name or target_name in ("", ".", ".."):
        raise ValueError("target_name must be one immediate directory name")
    if not re.fullmatch(r"[A-Za-z0-9_.-]+", target_name):
        raise ValueError("unsafe target name")
    reserved = {"CON", "PRN", "AUX", "NUL"} | {f"COM{i}" for i in range(1, 10)} | {f"LPT{i}" for i in range(1, 10)}
    if target_name.split(".", 1)[0].upper() in reserved:
        raise ValueError("target name is reserved on Windows")
    real_root = root.resolve(strict=True)
    target = workspace_path(real_root, Path(target_name))
    if target == real_root:
        raise ValueError("target may not equal workspace")
    if target.exists():
        raise FileExistsError("target already published: immutable version required")

    problems: list[Diagnostic] = []
    entries: list[Frame] = []
    seen: set[int] = set()
    stems: set[str] = set()
    paddings: set[int] = set()
    suffixes: set[str] = set()

    for name in sorted(source_names):
        # Names must be single filenames, not directories or traversal paths.
        if name != Path(name).name or name in ("", ".", "..") or "\\" in name:
            problems.append(Diagnostic("FRAME_PATH", "error", "invalid source filename", name))
            continue
        parsed = parse_frame(name)
        if parsed is None:
            problems.append(Diagnostic("FRAME_NAME", "error", "not a padded image frame", name))
            continue
        stem, number, padding, suffix = parsed
        stems.add(stem)
        paddings.add(padding)
        suffixes.add(suffix)

        try:
            path = workspace_path(real_root, Path(name))
            if not path.is_file():
                problems.append(Diagnostic("FRAME_MISSING_FILE", "error", "source is not a file", name))
                continue
            # Directories and symlinks escaping root were rejected above.
            digest = fingerprint(path)
            size = path.stat().st_size
        except (ValueError, OSError) as exc:
            problems.append(Diagnostic("FRAME_UNREADABLE", "error", str(exc), name))
            continue

        if number in seen:
            problems.append(Diagnostic("FRAME_DUPLICATE", "error", "duplicate frame number", name))
        seen.add(number)
        if not first <= number <= last:
            problems.append(Diagnostic("FRAME_OUTSIDE_RANGE", "error", "outside requested range", name))
        entries.append(Frame(number, name, digest, size))

    for frame in range(first, last + 1):
        if frame not in seen:
            problems.append(Diagnostic("FRAME_GAP", "error", "missing frame", str(frame)))
    if len(stems) > 1:
        problems.append(Diagnostic("FRAME_MIXED_STEMS", "error", "multiple sequence stems"))
    if len(paddings) > 1:
        problems.append(Diagnostic("FRAME_MIXED_PADDING", "error", "padding widths differ"))
    if len(suffixes) > 1:
        problems.append(Diagnostic("FRAME_MIXED_FORMAT", "error", "multiple file formats"))

    # Diagnostics are stable, even if input filename order changes.
    problems.sort()
    entries.sort(key=lambda e: (e.frame, e.source))
    return Plan(
        schema_version=SCHEMA_VERSION,
        run_id=run_id,
        workspace=str(real_root),
        target=str(target),
        stem=next(iter(stems)) if len(stems) == 1 else "",
        first_frame=first,
        last_frame=last,
        padding=next(iter(paddings)) if len(paddings) == 1 else 0,
        frames=tuple(entries),
        diagnostics=tuple(problems),
    )


def manifest(plan: Plan) -> dict[str, object]:
    """A publishable cache/render manifest with exact source fingerprints."""
    return {
        "schema_version": plan.schema_version,
        "run_id": plan.run_id,
        "stem": plan.stem,
        "frame_range": [plan.first_frame, plan.last_frame],
        "padding": plan.padding,
        "frames": [asdict(frame) for frame in plan.frames],
    }


def _write_fsynced(path: Path, data: bytes) -> None:
    with path.open("xb") as output:
        output.write(data)
        output.flush()
        os.fsync(output.fileno())


def publish(plan: Plan) -> Path:
    """Stage a new immutable version, verify contents, then rename it.

    Requires an external single-writer policy. Atomic directory rename
    protects readers of a NEW target on one local filesystem; it does not
    imply crash-proof cross-filesystem transactions or atomically replace
    existing nonempty output directories.
    """
    if not plan.safe_to_publish:
        raise ValueError("refusing publish with error diagnostics")
    root = Path(plan.workspace).resolve(strict=True)
    target = workspace_path(root, Path(plan.target))
    if target.exists():
        raise FileExistsError("immutable publish target already exists")
    if target.parent != root:
        raise ValueError("published target must be directly in workspace")

    stage = Path(tempfile.mkdtemp(prefix=".allmight-stage-", dir=root))
    promoted = False
    try:
        for frame in plan.frames:
            source = workspace_path(root, Path(frame.source))
            if fingerprint(source) != frame.digest:
                raise ValueError(f"source changed after planning: {frame.source}")
            destination = stage / frame.source
            with source.open("rb") as input_file, destination.open("xb") as output:
                shutil.copyfileobj(input_file, output, length=1 << 20)
                output.flush()
                os.fsync(output.fileno())
            if fingerprint(destination) != frame.digest:
                raise ValueError(f"staged content differs: {frame.source}")

        _write_fsynced(stage / "manifest.json", stable_json(manifest(plan)))
        # Publication point: same parent / same filesystem.
        os.rename(stage, target)
        promoted = True
        # Filesystem directory fsync and platform durability are not portable
        # here; do not claim a power-loss atomic guarantee.
        return target
    finally:
        if not promoted and stage.exists():
            shutil.rmtree(stage)


def verify_published(target: Path) -> list[Diagnostic]:
    """Independent read-after-publish integrity check."""
    errors: list[Diagnostic] = []
    meta = json.loads((target / "manifest.json").read_text(encoding="utf-8"))
    if meta.get("schema_version") != SCHEMA_VERSION:
        errors.append(Diagnostic("SCHEMA", "error", "unrecognized manifest schema"))
    for row in meta["frames"]:
        name = row["source"]
        if Path(name).name != name:
            errors.append(Diagnostic("PATH", "error", "manifest path is not a filename", name))
            continue
        path = target / name
        resolved = path.resolve(strict=False)
        if not resolved.is_relative_to(target.resolve(strict=True)):
            errors.append(Diagnostic("PATH_ESCAPE", "error", "published link escapes target", name))
            continue
        if not path.is_file() or fingerprint(path) != row["digest"]:
            errors.append(Diagnostic("FINGERPRINT", "error", "published content mismatch", name))
    return sorted(errors)


def image_mse(reference: Sequence[float], test: Sequence[float]) -> float:
    if not reference or len(reference) != len(test):
        raise ValueError("image arrays must be nonempty and equally sized")
    if any(not math.isfinite(x) for x in itertools.chain(reference, test)):
        raise ValueError("nonfinite pixel channel")
    return math.fsum((a - b) ** 2 for a, b in zip(reference, test)) / len(reference)


def image_psnr(reference: Sequence[float], test: Sequence[float], peak: float = 1) -> float:
    if peak <= 0:
        raise ValueError("nonpositive signal peak")
    mse = image_mse(reference, test)
    return math.inf if mse == 0 else 10 * math.log10(peak * peak / mse)


def exact_pixel_mismatch(a: Sequence[int], b: Sequence[int]) -> int:
    if len(a) != len(b):
        raise ValueError("pixel sequences have different lengths")
    return sum(x != y for x, y in zip(a, b))


def benchmark_summary(samples_ns: Sequence[int]) -> dict[str, float]:
    """Nearest-rank percentiles, not a promise about population latency."""
    if not samples_ns or any(x < 0 for x in samples_ns):
        raise ValueError("timing samples must be present and nonnegative")
    ordered = sorted(samples_ns)
    def rank(p: float) -> int:
        return max(0, math.ceil(p * len(ordered)) - 1)
    return {
        "count": float(len(ordered)),
        "median_ns": float(statistics.median(ordered)),
        "mean_ns": float(statistics.fmean(ordered)),
        "p90_ns": float(ordered[rank(0.90)]),
        "p99_ns": float(ordered[rank(0.99)]),
        "min_ns": float(ordered[0]),
        "max_ns": float(ordered[-1]),
    }


def monotonic_span(action) -> tuple[object, int]:
    start = time.perf_counter_ns()
    result = action()
    return result, time.perf_counter_ns() - start


def run_id() -> str:
    return uuid.uuid4().hex
