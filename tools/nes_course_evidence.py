#!/usr/bin/env python3
"""Fail-closed provenance and CTest evidence for Project AllMight cross-project labs.

This script NEVER compiles LaTeX, changes manuscript content, or reports that
a test passed merely because its source file or CTest registration exists.

Examples (only at a user-chosen checkpoint / within CI):
  python3 tools/nes_course_evidence.py audit
  python3 tools/nes_course_evidence.py collect --build build/nes-ci-gcc \
      --junit build/nes-ci-gcc/ctest-junit.xml \
      --output build/nes-ci-gcc/nes-course-evidence.json
  python3 tools/nes_course_evidence.py verify --record ...json

Audit checks physical source contracts and CMake target registration, NOT a
compiler execution. Collect refuses absent/empty, skipped or failed tests and
captures actual CTest XML, source SHA-256, build toolchain and source Git SHA.
"""
from __future__ import annotations

import argparse
from dataclasses import dataclass
import hashlib
import json
import os
from pathlib import Path
import platform
import re
import subprocess
import sys
from typing import Any
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[1]
COURSE = ROOT / "Book Production/Seven-Book Curriculum/Book01"
CMAKE = COURSE / "CMakeLists.txt"
MASTER = ROOT / "Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md"
INSERTS = ROOT / "tools/apply_cpp23_project_inserts.py"
EXPECTED = (
    ("NES-01", "ines_header_cpp23_lab", "tools/labs/ines_header_cpp23_lab.cpp"),
    ("NES-02", "nes_bus_nrom_cpp23_lab", "tools/labs/nes_bus_nrom_cpp23_lab.cpp"),
    ("NES-02T", "nes_cartridge_transaction_cpp23_lab", "tools/labs/nes_cartridge_transaction_cpp23_lab.cpp"),
    ("NES-02P", "nes_ownership_proofs_cpp23_lab", "tools/labs/nes_ownership_proofs_cpp23_lab.cpp"),
    ("NES-05A", "typed_asset_pipeline_cpp23_lab", "tools/labs/typed_asset_pipeline_cpp23_lab.cpp"),
    ("NES-06G", "generational_asset_registry_cpp23_lab", "tools/labs/generational_asset_registry_cpp23_lab.cpp"),
    ("NES-07C", "nes_cpu_debugger_cpp23_lab", "tools/labs/nes_cpu_debugger_cpp23_lab.cpp"),
    ("NES-08G", "nes_work_dependency_graph_cpp23_lab", "tools/labs/nes_work_dependency_graph_cpp23_lab.cpp"),
    ("NES-09M", "nes_gpu_memory_contract_cpp23_lab", "tools/labs/nes_gpu_memory_contract_cpp23_lab.cpp"),
    ("NES-10R", "nes_reproducible_benchmark_cpp23_lab", "tools/labs/nes_reproducible_benchmark_cpp23_lab.cpp"),
    ("NES-11B", "nes_scoped_frame_upload_cpp23_lab", "tools/labs/nes_scoped_frame_upload_cpp23_lab.cpp"),
    ("NES-12Q", "nes_command_lifecycle_cpp23_lab", "tools/labs/nes_command_lifecycle_cpp23_lab.cpp"),
    ("GPU-13D", "gpu_device_resource_cpp23_lab", "tools/labs/gpu_device_resource_cpp23_lab.cpp"),
    ("GPU-14F", "gpu_submit_retire_cpp23_lab", "tools/labs/gpu_submit_retire_cpp23_lab.cpp"),
    ("GPU-15M", "gpu_metal_frame_cpp23_lab", "tools/labs/gpu_metal_frame_cpp23_lab.cpp"),
    ("GPU-16P", "gpu_profile_ownership_cpp23_lab", "tools/labs/gpu_profile_ownership_cpp23_lab.cpp"),
)
EXPECTED_MARKERS = tuple(name.replace("-", "") for name, _, _ in EXPECTED)
EXTRA_SOURCES = (
    "tools/labs/nes_course/ines_layout.hpp",
    "tools/labs/nes_course/nes_bus_nrom.hpp",
    "tools/labs/nes_course/cartridge_runtime.hpp",
    "tools/labs/nes_course/typed_asset_pipeline.hpp",
    "tools/labs/nes_course/generational_asset_registry.hpp",
    "tools/labs/nes_course/cpu6502_subset.hpp",
    "tools/labs/nes_course/cpu_debug_instruments.hpp",
    "tools/labs/nes_course/cpu_debug_session.hpp",
    "tools/labs/nes_course/cpu_debug_session.cpp",
    "tools/labs/nes_course/work_dependency_graph.hpp",
    "tools/labs/nes_course/gpu_memory_contract_oracle.hpp",
    "tools/labs/nes_course/measurement_protocol.hpp",
    "tools/labs/nes_course/scoped_frame_upload_ring.hpp",
    "tools/labs/nes_course/command_submission_lifecycle.hpp",
    "tools/labs/gpu_course/device_resource_contracts.hpp",
    "tools/labs/gpu_course/native_vulkan_probe.cpp",
    "tools/labs/gpu_course/submission_retirement.hpp",
    "tools/labs/gpu_course/native_vulkan_submit_probe.cpp",
    "tools/labs/gpu_course/metal_frame_contracts.hpp",
    "tools/labs/gpu_course/native_metal_compute_probe.mm",
    "tools/labs/gpu_course/native_metal_unavailable.cpp",
    "tools/labs/gpu_course/gpu_profile_ownership.hpp",
    "tools/labs/gpu_course/native_metal_callback_probe.mm",
    "tools/labs/gpu_course/native_metal_callback_unavailable.cpp",
    "tools/apply_cpp23_project_inserts.py",
    "tools/tests/test_cpp23_project_inserts.py",
    "tools/tests/test_nes_course_evidence.py",
    ".github/workflows/allmight-cpp23-nes-course.yml",
    "Book Production/Seven-Book Curriculum/Book01/CMakeLists.txt",
    "Book Production/Seven-Book Curriculum/Book01/CMakePresets.json",
    "Book Production/Seven-Book Curriculum/README.md",
    "Book Production/Seven-Book Curriculum/source-routing-foundations.md",
    "Book Production/Seven-Book Curriculum/project-lab-matrix.md",
)

class EvidenceError(RuntimeError):
    """Evidence is missing, inconsistent, stale, or explicitly not passing."""

def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()

def git_sha(root: Path) -> str:
    result = subprocess.run(
        ["git", "-C", str(root), "rev-parse", "HEAD"],
        capture_output=True, text=True, check=False,
    )
    if result.returncode:
        raise EvidenceError("cannot record an exact Git commit SHA")
    value = result.stdout.strip()
    if not re.fullmatch(r"[0-9a-f]{40}", value):
        raise EvidenceError("git HEAD did not return a full 40-character SHA")
    return value

def source_paths() -> tuple[str, ...]:
    lesson_prefix = "Supplementary/C++23 Project Labs/"
    chapters = {
        "NES-01": "NES-01 - Cartridge Bytes and Trusted Layout.md",
        "NES-02": "NES-02 - Owned NROM and Memory Bus.md",
        "NES-02T": "NES-02T - Transactional Cartridge Replacement.md",
        "NES-02P": "NES-02P - Ownership Proofs for an Emulator.md",
        "NES-05A": "NES-05A - Type-Safe Asset Import and Publication.md",
        "NES-06G": "NES-06G - Generational Identity and Safe Reuse.md",
        "NES-07C": "NES-07C - CPU Instructions and Typed Debugger Structures.md",
        "NES-08G": "NES-08G - Dependency Graphs and Explicit GPU Work Contracts.md",
        "NES-09M": "NES-09M - GPU Memory and Ordering Contracts.md",
        "NES-10R": "NES-10R - Reproducibility and Evidence-Driven Profiling.md",
        "NES-11B": "NES-11B - Scoped Borrows and CPU-GPU Frame Buffering.md",
        "NES-12Q": "NES-12Q - Command Lifecycles and Uncertain Submission.md",
        "GPU-13D": "GPU-13D - Cross-Project Vulkan Devices and Resource Contracts.md",
        "GPU-14F": "GPU-14F - Frame Submission Synchronization and Retirement.md",
        "GPU-15M": "GPU-15M - Execute Verify and Measure a Metal Frame.md",
        "GPU-16P": "GPU-16P - GPU Profiling and Apple Callback Ownership.md",
    }
    return tuple(dict.fromkeys(
        [source for _, _, source in EXPECTED]
        + list(EXTRA_SOURCES)
        + [lesson_prefix + chapters[name] for name, _, _ in EXPECTED]
        + ["Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md"]
    ))

def parse_registered_targets(source: str) -> tuple[str, ...]:
    # An audit intentionally only claims that add_executable(...) is present.
    # It does not claim the CMake project successfully configures or builds.
    return tuple(re.findall(r"(?m)^\s*add_executable\(\s*([A-Za-z0-9_]+)", source))

def parse_test_names(source: str) -> tuple[str, ...]:
    return tuple(re.findall(r"(?m)^\s*add_test\(\s*NAME\s+([A-Za-z0-9_]+)", source))

def inspect_sources(root: Path = ROOT) -> dict[str, Any]:
    missing = [p for p in source_paths() if not (root / p).is_file()]
    if missing:
        raise EvidenceError("required source files are missing: " + ", ".join(missing))
    cmake = (root / "Book Production/Seven-Book Curriculum/Book01/CMakeLists.txt").read_text()
    targets = parse_registered_targets(cmake)
    declared = tuple(target for _, target, _ in EXPECTED)
    if len(targets) != len(set(targets)) or set(targets) != set(declared):
        raise EvidenceError(f"CMake target inventory mismatch: {targets!r}")
    tests = parse_test_names(cmake)
    if len(tests) != len(set(tests)) or len(tests) != len(EXPECTED):
        raise EvidenceError(f"CTest registration inventory mismatch: {tests!r}")
    patcher = (root / "tools/apply_cpp23_project_inserts.py").read_text()
    for name, _, _ in EXPECTED:
        if f'"{name}",' not in patcher:
            raise EvidenceError(f"lesson {name} is not registered for insertion")
        lesson = next(
            p for p in source_paths()
            if p.startswith("Supplementary/C++23 Project Labs/" + name + " - ")
        )
        data = (root / lesson).read_text()
        if not data.startswith("### " + name + " "):
            raise EvidenceError(f"lesson heading does not match staged section {name}")
    paths = source_paths()
    return {
        "kind": "source_inventory_only_not_a_build_result",
        "expected_targets": list(declared),
        "ctest_names": list(tests),
        "source_sha256": {p: digest((root / p).read_bytes()) for p in sorted(paths)},
    }

def parse_cmake_cache(build: Path) -> dict[str, str]:
    cache = build / "CMakeCache.txt"
    if not cache.is_file():
        raise EvidenceError("CMakeCache.txt missing; no evidence of a configured build")
    wanted = {
        "CMAKE_CXX_COMPILER", "CMAKE_CXX_COMPILER_ID",
        "CMAKE_CXX_COMPILER_VERSION", "CMAKE_CXX_FLAGS",
        "CMAKE_BUILD_TYPE", "CMAKE_GENERATOR",
        "CMAKE_HOME_DIRECTORY", "CMAKE_CXX_STANDARD",
    }
    values: dict[str, str] = {}
    for line in cache.read_text(errors="replace").splitlines():
        if not line or line[0] in "#/" or ":" not in line or "=" not in line:
            continue
        key, value = line.split("=", 1)
        name = key.split(":", 1)[0]
        if name in wanted:
            values[name] = value
    if "CMAKE_CXX_COMPILER" not in values:
        raise EvidenceError("CMakeCache has no recorded C++ compiler")
    return values

def parse_ctest_junit(xml_path: Path, expected_count: int) -> dict[str, Any]:
    if not xml_path.is_file():
        raise EvidenceError("CTest JUnit result missing; no tests can be certified")
    document = ET.parse(xml_path)
    root = document.getroot()
    cases = root.findall(".//testcase")
    if len(cases) != expected_count:
        raise EvidenceError(
            f"expected {expected_count} executed tests, observed {len(cases)}"
        )
    identifiers = [case.attrib.get("name", "") for case in cases]
    if not all(identifiers) or len(identifiers) != len(set(identifiers)):
        raise EvidenceError("CTest names missing or repeated")
    for case in cases:
        if any(case.find(name) is not None for name in ("failure", "error", "skipped")):
            raise EvidenceError(f"CTest has nonpassing result: {case.attrib.get('name')}")
        # JUnit can report suite-level failures (including incomplete cases);
        # reject those too below.
    for suite in [root, *root.findall(".//testsuite")]:
        for field in ("failures", "errors", "skipped"):
            try:
                count = int(suite.attrib.get(field, "0"))
            except ValueError as exc:
                raise EvidenceError("invalid JUnit suite outcome count") from exc
            if count:
                raise EvidenceError(f"CTest JUnit suite reports {field}={count}")
    return {"executed": len(cases), "names": sorted(identifiers), "status": "pass"}

def validate_junit_names(test_names: list[str], expected_names: list[str]) -> None:
    if sorted(test_names) != sorted(expected_names):
        raise EvidenceError("executed CTest names differ from source-declared tests")

def check_fresh_compiled_results(root: Path, build: Path, junit: Path) -> None:
    """Never certify an old XML file against newly edited or missing binaries.

    This is a freshness sanity check, not cryptographic proof of the trusted
    runner. Test runners must still execute ctest for the relevant build.
    """
    if not junit.is_file():
        raise EvidenceError("CTest JUnit file absent; no current test evidence")
    cache = build / "CMakeCache.txt"
    if not cache.is_file():
        raise EvidenceError("CMake cache absent; no configured build")

    # Every target must have an actual executable in this CMake binary dir,
    # not merely a source declaration. The current course targets output at
    # the top level; Windows suffix supports future Windows CI extensions.
    executables = []
    suffix = ".exe" if os.name == "nt" else ""
    for _, target, _ in EXPECTED:
        binary = build / (target + suffix)
        if not binary.is_file():
            raise EvidenceError(f"compiled executable not found: {binary}")
        executables.append(binary)

    newest_input = max(
        [cache.stat().st_mtime_ns]
        + [(root / path).stat().st_mtime_ns for path in source_paths()]
        + [binary.stat().st_mtime_ns for binary in executables]
    )
    if junit.stat().st_mtime_ns < newest_input:
        raise EvidenceError(
            "CTest JUnit is older than source, build cache or compiled executable"
        )

    changed = subprocess.run(
        ["git", "-C", str(root), "status", "--porcelain=v1",
         "--untracked-files=no", "--", *source_paths()],
        capture_output=True, text=True, check=False,
    )
    if changed.returncode != 0:
        raise EvidenceError("cannot verify source working-tree cleanliness")
    if changed.stdout.strip():
        raise EvidenceError(
            "tracked source modifications are not committed; "
            "cannot tie test receipt to HEAD"
        )

def collect(root: Path, build: Path, junit: Path) -> dict[str, Any]:
    check_fresh_compiled_results(root, build, junit)
    inventory = inspect_sources(root)
    actual = parse_ctest_junit(junit, len(EXPECTED))
    validate_junit_names(actual["names"], inventory["ctest_names"])
    cache = parse_cmake_cache(build)
    compiler_path = cache["CMAKE_CXX_COMPILER"]
    version = subprocess.run(
        [compiler_path, "--version"], capture_output=True, text=True,
        check=False, timeout=10,
    )
    if version.returncode != 0 or not version.stdout.strip():
        raise EvidenceError("could not confirm actual C++ compiler version")
    cache["COMPILER_VERSION_BANNER"] = version.stdout.splitlines()[0]
    home = cache.get("CMAKE_HOME_DIRECTORY")
    if home and Path(home).resolve() != (root / "Book Production/Seven-Book Curriculum/Book01").resolve():
        raise EvidenceError("CMakeCache belongs to a different source tree")
    # No fabricated result claims: a passing JUnit file must be explicitly
    # provided and must match all target registrations for this source tree.
    return {
        "schema": 1,
        "kind": "observed_compiler_and_test_evidence",
        "source_git_sha": git_sha(root),
        "host": {"platform": platform.platform(), "python": platform.python_version()},
        "compiler": cache,
        "ctest": actual,
        "inventory": inventory,
        "note": "This is host C++23 evidence, not GPU/Metal/Vulkan validation.",
    }

def verify_record(root: Path, record: dict[str, Any]) -> None:
    if record.get("schema") != 1 or record.get("kind") != "observed_compiler_and_test_evidence":
        raise EvidenceError("record is not a completed build-and-test receipt")
    if record.get("source_git_sha") != git_sha(root):
        raise EvidenceError("record refers to a different Git commit")
    latest = inspect_sources(root)
    if record.get("inventory", {}).get("source_sha256") != latest["source_sha256"]:
        raise EvidenceError("source files changed since the recorded test")
    tests = record.get("ctest", {})
    if tests.get("status") != "pass" or tests.get("executed") != len(EXPECTED):
        raise EvidenceError("record lacks passing evidence for all targets")
    validate_junit_names(tests.get("names", []), latest["ctest_names"])

def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    choices = parser.add_subparsers(dest="command", required=True)
    audit = choices.add_parser("audit", help="inspect source only; never certify a build")
    audit.add_argument("--root", type=Path, default=ROOT)
    capture = choices.add_parser("collect", help="collect only actual all-pass JUnit evidence")
    capture.add_argument("--root", type=Path, default=ROOT)
    capture.add_argument("--build", type=Path, required=True)
    capture.add_argument("--junit", type=Path, required=True)
    capture.add_argument("--output", type=Path, required=True)
    verify = choices.add_parser("verify", help="reject records invalidated by source changes")
    verify.add_argument("--root", type=Path, default=ROOT)
    verify.add_argument("--record", type=Path, required=True)
    args = parser.parse_args(argv)
    try:
        if args.command == "audit":
            data = inspect_sources(args.root.resolve())
            print(json.dumps(data, indent=2, sort_keys=True))
        elif args.command == "collect":
            record = collect(args.root.resolve(), args.build.resolve(), args.junit.resolve())
            args.output.parent.mkdir(parents=True, exist_ok=True)
            temporary = args.output.with_suffix(args.output.suffix + ".tmp")
            if temporary.exists():
                raise EvidenceError("refusing to overwrite an existing temporary receipt")
            try:
                with temporary.open("x", encoding="utf-8") as handle:
                    json.dump(record, handle, indent=2, sort_keys=True)
                    handle.write("\n")
                    handle.flush()
                    os.fsync(handle.fileno())
                os.replace(temporary, args.output)
            finally:
                if temporary.exists():
                    temporary.unlink()
            print(json.dumps({
                "result": "pass",
                "source_git_sha": record["source_git_sha"],
                "executed_tests": record["ctest"]["executed"],
                "receipt": str(args.output),
            }, sort_keys=True))
        else:
            verify_record(args.root.resolve(), json.loads(args.record.read_text()))
            print(json.dumps({"result": "verified-current-source", "record": str(args.record)}))
        return 0
    except (EvidenceError, ET.ParseError, OSError, ValueError, subprocess.TimeoutExpired) as exc:
        print(f"NES evidence refused: {exc}", file=sys.stderr)
        return 1

if __name__ == "__main__":
    raise SystemExit(main())
