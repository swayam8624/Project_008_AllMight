"""Independent stdlib regression tests for the Phase 30 evidence pipeline.

No DCC installation or external downloads are needed. All filesystem
mutations remain confined to temporary per-test workspaces.
"""
import hashlib
import json
import math
import os
import pathlib
import sys
import tempfile
import unittest

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parents[1]))
from production_evidence import (
    benchmark_summary, exact_pixel_mismatch, fingerprint, image_mse,
    image_psnr, make_plan, manifest, monotonic_span, parse_frame,
    publish, stable_json, verify_published, workspace_path,
)


class ProductionContractTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = pathlib.Path(self.tmp.name)
        for frame in (1, 2, 3):
            (self.root / f"panel.{frame:04d}.png").write_bytes(
                bytes([frame, 100, frame * 7]))
    def tearDown(self):
        self.tmp.cleanup()

    def plan(self, names=None):
        if names is None:
            names = [f"panel.{i:04d}.png" for i in (1, 2, 3)]
        return make_plan(self.root, names, "take-v001", first=1, last=3, run_id="fixed")

    def test_valid_dry_run_does_not_mutate(self):
        before = sorted(p.name for p in self.root.iterdir())
        p = self.plan()
        self.assertTrue(p.safe_to_publish)
        self.assertEqual([f.frame for f in p.frames], [1, 2, 3])
        self.assertEqual(p.padding, 4)
        self.assertEqual(sorted(x.name for x in self.root.iterdir()), before)

    def test_manifest_determinism_and_content_hash(self):
        a = self.plan(["panel.0003.png", "panel.0001.png", "panel.0002.png"])
        b = self.plan(["panel.0002.png", "panel.0003.png", "panel.0001.png"])
        self.assertEqual(stable_json(manifest(a)), stable_json(manifest(b)))
        expected = hashlib.sha256((self.root / "panel.0001.png").read_bytes()).hexdigest()
        self.assertEqual(a.frames[0].digest, expected)
        self.assertEqual(fingerprint(self.root / "panel.0001.png"), expected)

    def test_gap_duplicate_padding_and_range(self):
        p = self.plan(["panel.0001.png", "panel.0003.png"])
        self.assertIn("FRAME_GAP", {d.code for d in p.diagnostics})
        self.assertFalse(p.safe_to_publish)
        with self.assertRaises(ValueError):
            publish(p)
        # Two filename separators can express the same frame number.
        (self.root / "panel-0001.png").write_bytes(b"duplicate")
        q = self.plan(["panel.0001.png", "panel-0001.png",
                       "panel.0002.png", "panel.0003.png"])
        self.assertIn("FRAME_DUPLICATE", {d.code for d in q.diagnostics})
        (self.root / "panel.03.png").write_bytes(b"different padding")
        z = self.plan(["panel.0001.png", "panel.0002.png", "panel.03.png"])
        self.assertIn("FRAME_MIXED_PADDING", {d.code for d in z.diagnostics})

    def test_traversal_rejected(self):
        with self.assertRaises(ValueError):
            workspace_path(self.root, pathlib.Path("../outside"))
        p = self.plan(["../panel.0001.png"])
        self.assertIn("FRAME_PATH", {d.code for d in p.diagnostics})
        with self.assertRaises(ValueError):
            make_plan(self.root, [], "../outside", first=1, last=3)

    @unittest.skipUnless(hasattr(os, "symlink"), "no symlink API")
    def test_symlink_escape(self):
        with tempfile.TemporaryDirectory() as foreign:
            outside = pathlib.Path(foreign) / "panel.0002.png"
            outside.write_bytes(b"secret")
            (self.root / "panel.0002.png").unlink()
            try:
                os.symlink(outside, self.root / "panel.0002.png")
            except (OSError, NotImplementedError):
                self.skipTest("symlinks unavailable in test environment")
            plan = self.plan()
            self.assertIn("FRAME_UNREADABLE", {d.code for d in plan.diagnostics})
            self.assertFalse(plan.safe_to_publish)

    def test_successful_atomic_new_version_publication_and_integrity(self):
        plan = self.plan()
        output = publish(plan)
        self.assertTrue((output / "manifest.json").is_file())
        self.assertEqual(verify_published(output), [])
        self.assertEqual(json.loads((output / "manifest.json").read_text())["schema_version"], 1)
        with self.assertRaises(FileExistsError):
            publish(plan)
        self.assertEqual(list(self.root.glob(".allmight-stage-*")), [])

    def test_content_change_aborts_and_cleans_staging(self):
        plan = self.plan()
        (self.root / "panel.0002.png").write_bytes(b"changed after planning")
        with self.assertRaises(ValueError):
            publish(plan)
        self.assertFalse((self.root / "take-v001").exists())
        self.assertEqual(list(self.root.glob(".allmight-stage-*")), [])

    def test_published_corruption_detected(self):
        target = publish(self.plan())
        (target / "panel.0001.png").write_bytes(b"corrupt")
        self.assertEqual([d.code for d in verify_published(target)], ["FINGERPRINT"])

    def test_exact_metrics_and_psnr_conventions(self):
        self.assertEqual(image_mse([0., 1.], [0., 0.]), 0.5)
        self.assertAlmostEqual(image_psnr([0., 1.], [0., 0.]), 10*math.log10(2))
        self.assertTrue(math.isinf(image_psnr([0., 1.], [0., 1.])))
        self.assertEqual(exact_pixel_mismatch([1, 2, 3], [1, 9, 3]), 1)
        with self.assertRaises(ValueError):
            image_mse([1, 2], [1])
        with self.assertRaises(ValueError):
            image_mse([math.nan], [0])

    def test_untrusted_size_and_device_name_bounds(self):
        with self.assertRaises(ValueError):
            make_plan(self.root, [], "take-v001", first=1, last=100001)
        with self.assertRaises(ValueError):
            make_plan(self.root, [], "CON", first=1, last=3)
        self.assertIsNone(parse_frame("panel." + "0" * 100000 + ".png"))

    @unittest.skipUnless(hasattr(os, "symlink"), "no symlink API")
    def test_published_symlink_cannot_escape_integrity_check(self):
        target = publish(self.plan())
        with tempfile.TemporaryDirectory() as foreign:
            secret = pathlib.Path(foreign) / "secret.png"
            secret.write_bytes(b"external")
            (target / "panel.0001.png").unlink()
            try:
                os.symlink(secret, target / "panel.0001.png")
            except (OSError, NotImplementedError):
                self.skipTest("symlink permission unavailable")
            errors = verify_published(target)
            self.assertIn("PATH_ESCAPE", {x.code for x in errors})

    def test_benchmark_statistics_and_monotonic_span(self):
        result = benchmark_summary([40, 10, 20, 30, 50])
        self.assertEqual(result["median_ns"], 30)
        self.assertEqual(result["mean_ns"], 30)
        self.assertEqual(result["p90_ns"], 50)
        self.assertEqual(result["p99_ns"], 50)
        value, nanoseconds = monotonic_span(lambda: 42)
        self.assertEqual(value, 42)
        self.assertGreaterEqual(nanoseconds, 0)
        with self.assertRaises(ValueError):
            benchmark_summary([])


if __name__ == "__main__":
    unittest.main(verbosity=2)
