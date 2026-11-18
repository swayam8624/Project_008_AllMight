#!/usr/bin/env python3
"""Tests for fail-closed evidence collection: negative cases are important.

Test suite uses the Python standard library only. It never compiles C++,
edits a master manuscript or generates a fake green receipt as a real result.
"""
from __future__ import annotations

import json
from pathlib import Path
import sys
import tempfile
import unittest

REPO = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPO / "tools"))
import nes_course_evidence as evidence  # noqa: E402


class EvidenceContractTests(unittest.TestCase):
    def test_source_inventory_audit_is_not_a_build_certificate(self) -> None:
        item = evidence.inspect_sources(REPO)
        self.assertEqual(item["kind"], "source_inventory_only_not_a_build_result")
        self.assertEqual(len(item["expected_targets"]), 11)
        self.assertEqual(len(item["ctest_names"]), 11)
        self.assertEqual(len(item["source_sha256"]), len(evidence.source_paths()))
        self.assertTrue(item["source_sha256"].get(
            "Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md"
        ))
        self.assertEqual(len(item["source_sha256"][
            "tools/labs/nes_course/measurement_protocol.hpp"
        ]), 64)

    def test_declaration_parsers_check_semantic_names_not_raw_counts(self) -> None:
        cmake = (
            "# add_executable(commented_lab foo.cpp)\n"
            "add_executable(alpha main.cpp)\n"
            "add_executable(beta tests.cpp)\n"
            "add_test(NAME test_alpha COMMAND alpha)\n"
            "add_test(NAME test_beta COMMAND beta)\n"
        )
        self.assertEqual(
            evidence.parse_registered_targets(cmake), ("alpha", "beta"))
        self.assertEqual(
            evidence.parse_test_names(cmake), ("test_alpha", "test_beta"))

    @staticmethod
    def write_xml(path: Path, *, fail: bool = False,
                  skip: bool = False, duplicate: bool = False) -> None:
        extra = "<failure message='regression failed'/>" if fail else (
            "<skipped/>" if skip else "")
        second = "a" if duplicate else "b"
        path.write_text(
            "<testsuites><testsuite tests='2' failures='" +
            ("1" if fail else "0") + "' skipped='" +
            ("1" if skip else "0") + "'>"
            "<testcase name='a'>" + extra + "</testcase>"
            "<testcase name='" + second + "'/></testsuite></testsuites>"
        )

    def test_only_actual_complete_junit_can_pass(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "results.xml"
            with self.assertRaises(evidence.EvidenceError):
                evidence.parse_ctest_junit(path, 2)
            self.write_xml(path)
            actual = evidence.parse_ctest_junit(path, 2)
            self.assertEqual(actual["status"], "pass")
            self.assertEqual(actual["names"], ["a", "b"])
            with self.assertRaises(evidence.EvidenceError):
                evidence.parse_ctest_junit(path, 10)
            with self.assertRaises(evidence.EvidenceError):
                evidence.validate_junit_names(["a", "b"], ["a", "c"])
            self.write_xml(path, duplicate=True)
            with self.assertRaises(evidence.EvidenceError):
                evidence.parse_ctest_junit(path, 2)

    def test_failure_or_skipped_test_never_gets_success_receipt(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "results.xml"
            for flag in ({"fail": True}, {"skip": True}):
                self.write_xml(path, **flag)
                with self.assertRaises(evidence.EvidenceError):
                    evidence.parse_ctest_junit(path, 2)
            path.write_text("<malformed")
            with self.assertRaises(Exception):
                evidence.parse_ctest_junit(path, 2)

    def test_real_cmake_cache_mandatory_for_provenance(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            base = Path(tmp)
            with self.assertRaises(evidence.EvidenceError):
                evidence.parse_cmake_cache(base)
            (base / "CMakeCache.txt").write_text(
                "// generated test fixture\n"
                "CMAKE_CXX_COMPILER:FILEPATH=/usr/bin/clang++\n"
                "CMAKE_BUILD_TYPE:STRING=Debug\n"
                "CMAKE_GENERATOR:INTERNAL=Ninja\n"
            )
            details = evidence.parse_cmake_cache(base)
            self.assertEqual(details["CMAKE_CXX_COMPILER"], "/usr/bin/clang++")
            self.assertEqual(details["CMAKE_BUILD_TYPE"], "Debug")

    def test_freshness_guard_rejects_missing_or_stale_binaries(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            build = Path(tmp)
            (build / "CMakeCache.txt").write_text(
                "CMAKE_CXX_COMPILER:FILEPATH=/usr/bin/clang++\n"
            )
            junit = build / "ctest-junit.xml"
            self.write_xml(junit)
            with self.assertRaises(evidence.EvidenceError):
                evidence.check_fresh_compiled_results(REPO, build, junit)
            for _, name, _ in evidence.EXPECTED:
                (build / name).write_bytes(b"fixture-not-a-real-binary")
            # Force the XML to be older than these binaries. The guard
            # refuses a stale file even if its testcase elements look green.
            import os
            os.utime(junit, (1, 1))
            with self.assertRaises(evidence.EvidenceError):
                evidence.check_fresh_compiled_results(REPO, build, junit)

    def test_source_change_invalidates_a_passing_record(self) -> None:
        inventory = evidence.inspect_sources(REPO)
        self.assertTrue(evidence.git_sha(REPO))
        record = {
            "schema": 1,
            "kind": "observed_compiler_and_test_evidence",
            "source_git_sha": evidence.git_sha(REPO),
            "ctest": {
                "status": "pass",
                "executed": 11,
                "names": list(inventory["ctest_names"]),
            },
            "inventory": inventory,
        }
        # This synthetic fixture is used only INSIDE the unit test. We do
        # not emit it as a provenance receipt or claim CTest actually ran.
        evidence.verify_record(REPO, record)
        record["inventory"]["source_sha256"][
            "tools/labs/nes_course/measurement_protocol.hpp"
        ] = "0" * 64
        with self.assertRaises(evidence.EvidenceError):
            evidence.verify_record(REPO, record)
        record["source_git_sha"] = "0" * 40
        with self.assertRaises(evidence.EvidenceError):
            evidence.verify_record(REPO, record)

    def test_no_partial_or_wrong_kind_receipts(self) -> None:
        bad = {"schema": 1, "kind": "source_inventory_only_not_a_build_result"}
        with self.assertRaises(evidence.EvidenceError):
            evidence.verify_record(REPO, bad)


if __name__ == "__main__":
    unittest.main()
