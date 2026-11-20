"""Book Two font resolution and non-destructive canonical Master export tests.

These tests do not require LuaLaTeX, do not alter the original source
manuscript, and never attach or publish any font file to the conversation.
"""
from __future__ import annotations

import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest.mock import patch

import build
import book02_release_candidates as candidates


class BookTwoAssetAndMasterContracts(unittest.TestCase):

    def test_nested_book02_worktree_font_is_staged_from_real_repo(self) -> None:
        original = build.ROOT / "Assets" / "Fonts" / "NotoEmoji-Static.ttf"
        self.assertTrue(original.is_file(), original)
        original_hash = hashlib.sha256(original.read_bytes()).hexdigest()
        with tempfile.TemporaryDirectory() as temp:
            nested = (Path(temp) / "Book Production" / "AllMight Codex"
                      / "LaTeX" / "Build" / ".generation-sample"
                      / "book02-publication-candidates" / "print"
                      / "02-Mathematics-for-Visual-Computing-P01")
            nested.mkdir(parents=True)
            staged = build.prepare_book_font_assets(nested)
            self.assertEqual(staged, nested / "Assets" / "Fonts" / original.name)
            self.assertEqual(hashlib.sha256(staged.read_bytes()).hexdigest(), original_hash)
            # A repeat build must preserve the correct staged asset.
            self.assertEqual(build.prepare_book_font_assets(nested), staged)
            self.assertEqual(hashlib.sha256(original.read_bytes()).hexdigest(), original_hash)
            # Even a stale temporary file gets repaired without touching the
            # canonical file in ROOT/Assets/Fonts.
            staged.write_bytes(b"bad")
            build.prepare_book_font_assets(nested)
            self.assertEqual(hashlib.sha256(staged.read_bytes()).hexdigest(), original_hash)

    def test_compile_stages_font_before_latexmk_is_invoked(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            folder = Path(temp) / "Build" / "book02 worktree" / "print" / "P01"
            folder.mkdir(parents=True)
            (folder / "main.tex").write_text("\\documentclass{book}\n", encoding="utf-8")

            def witness(command, *, cwd, env, stdout, stderr):
                cwd = Path(cwd)
                self.assertEqual(cwd, folder)
                self.assertTrue((cwd / "allmight-book.sty").is_file())
                self.assertTrue((cwd / "Assets" / "Fonts" / "NotoEmoji-Static.ttf").is_file())
                self.assertTrue((cwd / "Assets" / "Fonts" / "NotoEmoji-Static.ttf").stat().st_size > 100_000)
                self.assertIn("-lualatex", command)
                return subprocess.CompletedProcess(command, 1)

            with patch.object(build.subprocess, "run", side_effect=witness) as invoked:
                response = build.compile_tex(folder, "/usr/bin/latexmk", dict(os.environ))
            self.assertEqual(response["returncode"], 1)
            self.assertTrue(invoked.called)

    def test_verified_artifacts_can_copy_into_master_without_mutating_sources(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            temp = Path(temp)
            root = temp / "Book02-checkout" / "LaTeX"
            generation = root / "Generations" / ".generation-test"
            generation.mkdir(parents=True)
            status_dir = root / "Build" / generation.name / "book02-publication-candidates"
            status_dir.mkdir(parents=True)
            master = temp / "Master" / "Book Production" / "AllMight Codex" / "LaTeX" / "Build" / "Book02"
            master.mkdir(parents=True)
            source_files=[]
            for name,payload in (
                ("Book02.pdf",b"test-only-complete-book-pdf"),
                ("Book02.epub",b"test-only-epub"),
            ):
                path = status_dir / name
                path.write_bytes(payload)
                source_files.append(path)
            digest = candidates.digest
            record = {
                "generation":generation.name,
                "source_fingerprint_sha256":"f"*64,
                "single_pdf":{"candidate_path":str(source_files[0]),
                              "sha256":digest(source_files[0]),"pages":1017},
                "epub":{
                    "candidate_path":str(source_files[-1]),
                    "sha256":digest(source_files[-1]),
                    "epubcheck":{"exit_code":0},
                },
            }
            report = status_dir / "book02-candidate-verification.json"
            report.write_text(json.dumps(record), encoding="utf-8")
            with patch.object(candidates, "ROOT", root), patch.object(
                candidates, "source_fingerprint", return_value="f"*64
            ):
                exported = candidates.export_candidates_to_master(generation, master)
                self.assertTrue(exported.name.startswith("Book02-From-Equations-to-Geometry-generation-test-"))
                self.assertTrue((exported / "From Equations to Geometry.pdf").is_file())
                self.assertTrue((exported / "From Equations to Geometry.epub").is_file())
                self.assertTrue((exported / "book02-candidate-verification.json").is_file())
                portable = exported / "book02-master-export-manifest.json"
                self.assertTrue(portable.is_file())
                portable_data = json.loads(portable.read_text(encoding="utf-8"))
                self.assertEqual(portable_data["source_fingerprint_sha256"], "f"*64)
                self.assertEqual(portable_data["schema"],"allmight-book02-master-export-v1")
                for entry in portable_data["copied_candidates"].values():
                    copied_file = exported / entry["file"]
                    self.assertTrue(copied_file.is_file())
                    self.assertEqual(candidates.digest(copied_file),entry["sha256"])
                # Idempotent, with no overwrite or Git operations.
                self.assertEqual(exported,candidates.export_candidates_to_master(generation,master))
                self.assertEqual(portable_data["copied_candidates"]["complete_pdf"]["pages"],1017)
                # An EPUBCheck failure must not prevent exporting the verified
                # complete PDF with the explicit print-only option.
                with patch.object(candidates,"source_fingerprint",return_value="f"*64):
                    pdf_only=candidates.export_candidates_to_master(generation,master,print_only=True)
                self.assertTrue((pdf_only/"From Equations to Geometry.pdf").is_file())
                tampered = exported / "From Equations to Geometry.epub"
                tampered.write_bytes(b"modified-book")
                with self.assertRaisesRegex(RuntimeError, "Refusing to overwrite"):
                    candidates.export_candidates_to_master(generation,master)


if __name__ == "__main__":
    unittest.main()
