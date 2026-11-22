#!/usr/bin/env python3
"""Regression tests for Pass17's NON-MUTATING chapter/lesson/source auditor.

The positives require actual source inventory present in the checkout.
The negatives mutate temporary *copies* or replace only the auditor's
read paths; the real manuscript, chapters and assets are never written.
"""
from __future__ import annotations

import importlib.util
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT=Path(__file__).resolve().parents[3]
SCRIPT=ROOT/"Code Parts/tools/book01_coverage_audit.py"
SPEC=importlib.util.spec_from_file_location("book01_audit_for_unittest",SCRIPT)
assert SPEC and SPEC.loader
AUDIT=importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name]=AUDIT
SPEC.loader.exec_module(AUDIT)


class CoverageAuditTests(unittest.TestCase):
    def test_git_blob_identity_has_canonical_encoding(self):
        # Git blob SHA-1 covers the byte length prefix as well as content;
        # it is not a raw sha1 of the content bytes alone.
        self.assertEqual(
            AUDIT.git_blob(b"test content\n"),
            "d670460b4b4aece5915caf5c68d12f560a9fe3e4",
        )

    def test_source_safety_and_all_sixteen_lessons(self):
        result=AUDIT.audit()
        self.assertTrue(result["ok"],"\n".join(result.get("faults",[])))
        self.assertEqual(result["canonical_git_blob"],AUDIT.EXPECTED_BLOB)
        self.assertEqual(result["reading_map_chapters"],39)
        self.assertEqual(result["staged_additions"],18)
        self.assertEqual(result["registered_executables"],18)
        self.assertEqual(result["registered_ctests"],18)
        self.assertEqual(result["audited_chapters"],16)
        self.assertEqual(len(result["additional_gap_lessons"]),2)
        self.assertEqual(result["additional_gap_lessons"][0]["lesson_id"],"CPP-18A")
        self.assertEqual(result["additional_gap_lessons"][0]["source_chapters"],[6])
        self.assertEqual(result["additional_gap_lessons"][1]["lesson_id"],"CPP-19E")
        self.assertEqual(result["additional_gap_lessons"][1]["source_chapters"],[10])
        self.assertFalse(result["compiler_results_verified"])
        self.assertFalse(result["native_gpu_results_verified"])
        self.assertFalse(result["editorial_or_research_completeness_verified"])
        for ch in result["chapters"]:
            self.assertEqual(ch["exact_source_anchor_count"],1)
            self.assertGreater(ch["original_chars"],10000)
            self.assertGreater(ch["lesson_chars"],8000)

    def test_refuses_missing_canonical_bytes_without_mutating_repository(self):
        with tempfile.TemporaryDirectory() as directory:
            wrong=Path(directory)/"fake_master.md"
            wrong.write_bytes(b"# replacement is not an authorized book\n")
            with patch.object(AUDIT,"MASTER",wrong):
                result=AUDIT.audit()
            self.assertFalse(result["ok"])
            self.assertIn("changed", " ".join(result["faults"]))
            self.assertEqual(wrong.read_bytes(),b"# replacement is not an authorized book\n")

    def test_rejects_missing_or_extra_ctest_registration(self):
        with tempfile.TemporaryDirectory() as directory:
            file=Path(directory)/"broken_cmake.txt"
            original=AUDIT.CMAKE.read_text("utf-8")
            # Removing one register breaks exact inventory.
            file.write_text(
                original.replace(
                    "add_test(NAME gpu_16p_cross_project_callback_ownership_and_profile_evidence_cpu_only",
                    "#add_test(NAME gpu_16p_cross_project_callback_ownership_and_profile_evidence_cpu_only",
                    1,
                ),"utf-8"
            )
            with patch.object(AUDIT,"CMAKE",file):
                result=AUDIT.audit()
            self.assertFalse(result["ok"])
            self.assertTrue(any("CTest" in fault for fault in result["faults"]))
            self.assertEqual(AUDIT.CMAKE.read_text("utf-8"),original)

    def test_preserves_zero_code_fence_as_warning_not_completeness_failure(self):
        # Good structural packaging does NOT imply adequate narrative/code
        # proximity. Existing NES-08G/NES-09M prose-only lessons are flagged
        # for a meaningful source-to-teaching audit, not automatically erased.
        result=AUDIT.audit()
        self.assertTrue(any("NES-08G" in x and "fenced" in x
                            for x in result["warnings"]))
        self.assertTrue(any("NES-09M" in x and "fenced" in x
                            for x in result["warnings"]))


if __name__=="__main__":
    unittest.main()
