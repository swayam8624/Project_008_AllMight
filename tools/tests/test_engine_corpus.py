"""Local regression tests for audited engine-corpus import boundaries."""
from __future__ import annotations

import json
from pathlib import Path
import tempfile
import unittest
from unittest import mock

from tools import engine_corpus
from tools import notes_pipeline


class SubtitleImportTests(unittest.TestCase):
    def test_import_keeps_cue_identity_times_and_technical_tokens(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            source = root / "fixture.vtt"
            source.write_text(
                """WEBVTT

cue-A
00:00:01.000 --> 00:00:02.000
Use <c.green>std::vector&lt;T&gt;</c> with C++.

cue-B
00:00:01.800 --> 00:00:03.000
Use <c.green>std::vector&lt;T&gt;</c> with C++.

cue-C
00:00:04.000 --> 00:00:05.000
Compare a < b and C++ with C.
""",
                encoding="utf-8",
            )
            meta = notes_pipeline.import_source(
                source, root / "state", "youtube:fixture:youtube_caption",
                kind="youtube_caption", extra={"video_id": "fixture"},
            )
            loaded_meta, units = notes_pipeline.load_source(root / "state", meta["source_id"])

        self.assertEqual(loaded_meta["kind"], "youtube_caption")
        self.assertEqual([u["locator"]["cue_id"] for u in units], ["cue-A", "cue-B", "cue-C"])
        self.assertEqual(len({u["unit_id"] for u in units}), 3)
        self.assertEqual([u["locator"]["start_timestamp"] for u in units], ["00:00:01.000", "00:00:01.800", "00:00:04.000"])
        self.assertEqual(units[1]["text"], "")
        self.assertTrue(units[1]["duplicate_only"])
        self.assertEqual(units[1]["raw_text"], "Use std::vector<T> with C++.")
        self.assertEqual(units[2]["text"], "Compare a < b and C++ with C.")
        self.assertEqual(units[2]["locator"]["start_seconds"], 4.0)

    def test_rolling_overlap_does_not_erase_cue_ids_or_raw_text(self) -> None:
        cues = notes_pipeline.parse_vtt(
            """WEBVTT

first
00:00:00.000 --> 00:00:01.500
The render queue

second
00:00:01.000 --> 00:00:02.000
queue submits Vulkan work
"""
        )
        self.assertEqual([cue["cue_id"] for cue in cues], ["first", "second"])
        self.assertEqual([cue["text"] for cue in cues], ["The render queue", "submits Vulkan work"])
        self.assertEqual(cues[1]["raw_text"], "queue submits Vulkan work")
        self.assertEqual(cues[1]["overlap_removed_words"], 1)

    def test_derived_spans_coalesce_cues_without_losing_provenance(self) -> None:
        cues = notes_pipeline.parse_vtt(
            """WEBVTT

cue-1
00:00:00.000 --> 00:00:01.000
std::vector&lt;T&gt; and C++

cue-2
00:00:01.000 --> 00:00:02.000
and C++ submit Vulkan commands

cue-3
00:00:03.000 --> 00:00:04.000
The GPU waits on a fence
"""
        )
        units = []
        for i, cue in enumerate(cues, 1):
            units.append({
                "unit_id": f"cue-unit-{i}", "text": cue["text"], "raw_text": cue["raw_text"],
                "locator": {"kind": "subtitle_cue", "format": "vtt", "cue_index": cue["cue_index"],
                            "cue_id": cue["cue_id"], "start_timestamp": cue["start_timestamp"],
                            "end_timestamp": cue["end_timestamp"], "start_seconds": cue["start_seconds"],
                            "end_seconds": cue["end_seconds"]},
            })
        spans = engine_corpus.derive_spans(units, target_words=4, source_id="fixture-source")
        self.assertEqual([s["locator"]["constituent_unit_ids"] for s in spans],
                         [["cue-unit-1", "cue-unit-2"], ["cue-unit-3"]])
        self.assertEqual([s["locator"]["cue_start"] for s in spans], [1, 3])
        self.assertEqual([s["locator"]["cue_end"] for s in spans], [2, 3])
        self.assertEqual(spans[0]["locator"]["start_timestamp"], "00:00:00.000")
        self.assertEqual(spans[0]["locator"]["end_timestamp"], "00:00:02.000")
        self.assertEqual(spans[0]["raw_text"], "std::vector<T> and C++ and C++ submit Vulkan commands")
        self.assertIn("std::vector<T>", spans[0]["text"])
        self.assertIn("C++", spans[0]["text"])
        self.assertEqual([uid for s in spans for uid in s["locator"]["constituent_unit_ids"]],
                         [u["unit_id"] for u in units])


class CorpusInventoryTests(unittest.TestCase):
    def test_import_snapshot_counts_stay_separate_from_later_acquisitions(self) -> None:
        state = Path("/Users/swayamsingal/.codex/source-cache/engine-corpus-2026-11-07")
        if not (state / "source-index.json").exists():
            self.skipTest("Local corpus integration cache is unavailable")
        index = json.loads((state / "source-index.json").read_text(encoding="utf-8"))
        manifest = json.loads((state / "corpus-manifest.json").read_text(encoding="utf-8"))
        videos = index["videos"]
        snapshot = list(manifest["videos"].values())
        self.assertEqual(len(videos), 746)
        self.assertEqual(sum(bool(v["source_ids"]) for v in snapshot), 456)
        self.assertEqual(sum(not v["source_ids"] for v in snapshot), 290)
        self.assertEqual({v["video_id"] for v in videos}, {v["video_id"] for v in snapshot})
        self.assertLessEqual({v["video_id"] for v in snapshot if v["source_ids"]},
                             {v["video_id"] for v in videos if v["source_ids"]})
        statuses = {}
        for video in videos:
            status = video["legacy_reading_status"]
            statuses[status] = statuses.get(status, 0) + 1
        self.assertEqual(statuses, {"full caption read": 448, "full generated transcript read": 7, "not reviewed": 291})
        asr = manifest["sources"][next(k for k in manifest["sources"] if k.startswith("cCEmwjfHIkY:local_asr:"))]
        self.assertEqual(asr["kind"], "local_asr")
        self.assertTrue(asr["path"].endswith(".vtt"))
        video = next(v for v in videos if v["video_id"] == "cCEmwjfHIkY")
        self.assertEqual(video["legacy_reading_status"], "not reviewed")
        self.assertTrue(video["source_ids"])

    def test_assignment_rows_use_file_and_pdf_sha256_and_all_four_books_import(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp); state = root / "state"; audit = root / "audit.json"
            assignment = root / "assignment.json"; files = []
            for number in range(4):
                pdf = root / f"book-{number}.pdf"; pdf.write_bytes(f"pdf-{number}".encode())
                files.append({"book": f"Book {number}", "file": str(pdf),
                              "pdf_sha256": notes_pipeline.sha256_file(pdf), "total_pdf_pages": 3})
            audit.write_text(json.dumps({"items": []}), encoding="utf-8")
            assignment.write_text(json.dumps(files), encoding="utf-8")

            def fake_import(path, _state, _origin, title, kind, extra):
                return {"source_id": f"id-{Path(path).stem}", "unit_count": 2,
                        "format_metadata": {"total_pdf_pages": 3}}

            with mock.patch.object(engine_corpus.np, "import_source", side_effect=fake_import), mock.patch(
                "sys.argv", ["engine_corpus.py", "--audit", str(audit), "--state-dir", str(state),
                             "--pdf-assignment", str(assignment), "--kohi-pdf", str(root / "absent.pdf")]
            ):
                engine_corpus.main()
            manifest = json.loads((state / "corpus-manifest.json").read_text(encoding="utf-8"))
            index = json.loads((state / "source-index.json").read_text(encoding="utf-8"))
        self.assertEqual(len(manifest["books"]), 4)
        self.assertEqual({b["title"] for b in index["books"]}, {row["book"] for row in files})
        self.assertEqual({b["source_id"] for b in index["books"]}, {f"id-book-{n}" for n in range(4)})
        self.assertEqual({b["pdf_path"] for b in index["books"]}, {row["file"] for row in files})


if __name__ == "__main__":
    unittest.main()
