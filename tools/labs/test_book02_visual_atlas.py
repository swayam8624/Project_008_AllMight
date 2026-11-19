"""Book Two Pass 3: provenance, rendering and non-destructive book integration tests.

Run in a pinned Python environment:
    python -m unittest -v test_book02_visual_atlas

This tests numerical/figure generation with *real* NumPy, SciPy and Matplotlib.
The integration test uses the already-generated original files to avoid
duplicating the full renderer unnecessarily; the normal CLI and CI render the
full atlas end to end.
"""
from __future__ import annotations

import csv
import json
import math
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch
import xml.etree.ElementTree as ET

import numpy as np

from book02_visual_atlas import FIGURES, generate, field_data
from book02_papers_geometry import synthetic_line_data, ransac_line


REPO = Path(__file__).resolve().parents[2]
LATEX_TOOLS = REPO / "Book Production" / "AllMight Codex" / "LaTeX" / "Tools"
sys.path.insert(0, str(LATEX_TOOLS))
from book02_visuals import RICH_FIGURES, install_book02_visuals  # noqa: E402


class Book02AtlasVerification(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.tmp = tempfile.TemporaryDirectory()
        cls.root = Path(cls.tmp.name)
        cls.atlas = cls.root / "atlas"
        cls.manifest = generate(cls.atlas)

    @classmethod
    def tearDownClass(cls) -> None:
        cls.tmp.cleanup()

    def test_offline_viewer_contains_real_data_and_no_external_dependencies(self) -> None:
        viewer = self.atlas / "Numerical-Geometry-Atlas.html"
        self.assertTrue(viewer.is_file())
        html = viewer.read_text(encoding="utf-8")
        self.assertIn("Numerical Geometry Atlas", html)
        self.assertNotIn("__ATLAS_DATA__", html)
        self.assertNotIn("https://", html)
        self.assertNotIn("http://", html)
        self.assertNotIn("<script src=", html)
        self.assertIn("application/json", html)
        import re
        block = re.search(r'<script id="atlas-json" type="application/json">(.*?)</script>',
                          html, re.S)
        self.assertIsNotNone(block)
        embedded = json.loads(block.group(1))
        self.assertEqual(set(embedded["tables"]), set(FIGURES))
        self.assertEqual(len(embedded["tables"]["geodesic-fields"]),625)
        self.assertEqual(len(embedded["tables"]["ransac-consensus"]),110)
        self.assertEqual(len(embedded["tables"]["mis-variance"]),1350)
        self.assertEqual(embedded["metadata"]["ransac-consensus"]["seed"],73013)
        self.assertTrue(all(math.isfinite(v) for v in
                            embedded["metadata"]["ransac-consensus"]["fit_line"].values()))

    def test_seven_source_backed_figures_and_three_formats(self) -> None:
        manifest = self.manifest
        self.assertEqual(manifest["schema"], "allmight-book02-atlas-1")
        self.assertEqual([f["name"] for f in manifest["figures"]], list(FIGURES))
        self.assertEqual(len(manifest["figures"]), 7)
        for figure in manifest["figures"]:
            self.assertTrue(figure["type"])
            self.assertTrue(figure["units"])
            self.assertTrue(figure["caveat"])
            self.assertGreater(figure["rows"], 5)
            for fmt in ("pdf", "svg", "png"):
                path = self.atlas / figure["files"][fmt]
                self.assertTrue(path.is_file(), path)
                self.assertGreater(path.stat().st_size, 1000, path)
                if fmt == "pdf":
                    self.assertTrue(path.read_bytes().startswith(b"%PDF-"))
                elif fmt == "png":
                    self.assertTrue(path.read_bytes().startswith(b"\x89PNG\r\n\x1a\n"))
                else:
                    self.assertEqual(ET.parse(path).getroot().tag.split("}")[-1], "svg")
            datapath = self.atlas / figure["data"]
            with datapath.open(newline="") as f:
                data = list(csv.DictReader(f))
            self.assertEqual(len(data), figure["rows"])
        self.assertTrue((self.atlas/"manifest.json").is_file())

    def test_all_exported_numeric_data_finite(self) -> None:
        for figure in self.manifest["figures"]:
            with (self.atlas / figure["data"]).open(newline="") as f:
                rows = list(csv.DictReader(f))
            for row in rows:
                for key, value in row.items():
                    if key == "decision_path" or key == "method":
                        continue
                    self.assertTrue(math.isfinite(float(value)),
                                    f"{figure['name']} {key}={value}")

    def test_source_derived_distance_csv_and_metadata(self) -> None:
        item = next(f for f in self.manifest["figures"] if f["name"] == "geodesic-fields")
        V, F, result, truth, source = field_data()
        self.assertEqual(item["rows"], len(V))
        self.assertEqual(item["source"], source)
        self.assertAlmostEqual(item["mae_m"],
                               float(np.mean(np.abs(result.distances-truth))))
        with (self.atlas / item["data"]).open(newline="") as f:
            rows = list(csv.DictReader(f))
        for index in (0, source, len(V)//2, len(V)-1):
            row = rows[index]
            self.assertAlmostEqual(float(row["signed_error_m"]),
                                   float(result.distances[index]-truth[index]))
        self.assertEqual(len(F), 2*(25-1)**2)

    def test_ransac_figure_matches_original_consensus(self) -> None:
        points, truth = synthetic_line_data()
        result = ransac_line(points, threshold=.08, seed=73013)
        item = next(f for f in self.manifest["figures"] if f["name"] == "ransac-consensus")
        self.assertEqual(item["rows"], len(points))
        self.assertEqual(item["selected_inliers"], len(result.inlier_indices))
        self.assertEqual(item["iterations"], result.iterations)
        with (self.atlas/item["data"]).open(newline="") as f:
            rows = list(csv.DictReader(f))
        self.assertEqual({i for i,r in enumerate(rows) if r["consensus_inlier"] == "1"},
                         set(result.inlier_indices))
        self.assertEqual({i for i,r in enumerate(rows) if r["generator_inlier"] == "1"},
                         set(truth))

    def test_equal_budget_mis_and_independent_seeds(self) -> None:
        item = next(f for f in self.manifest["figures"] if f["name"] == "mis-variance")
        with (self.atlas / item["data"]).open(newline="") as f:
            rows = list(csv.DictReader(f))
        for budget in (40,80,160,320,640):
            subset=[r for r in rows if int(r["budget_evaluations"])==budget]
            self.assertEqual(len(subset),3*90)
            for method in ("uniform", "tilted", "balanced"):
                by_method = [r for r in subset if r["method"] == method]
                self.assertEqual(len(by_method),90)
                self.assertEqual(len({r["seed"] for r in by_method}),90)
                # The fact that each estimate differs does not prove RNG
                # independence, but confirms no seed is repeated per method.
                self.assertLess(abs(np.mean([float(r["estimate"]) for r in by_method])
                                    - (math.e-1)),.05)

    def test_deterministic_output_data(self) -> None:
        second = self.root / "rerender"
        result = generate(second, ("ransac-consensus", "mis-variance"))
        old = {x["name"]: x["data_sha256"] for x in self.manifest["figures"]}
        for figure in result["figures"]:
            self.assertEqual(figure["data_sha256"], old[figure["name"]])

    def test_latex_install_with_real_rendered_assets(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            main = Path(tmp)/"main.tex"
            # Minimal generated-book witness; original chapter source lines
            # must remain exactly once and in the same order.
            originals = [
                r"\input{../current/02-Mathematics-for-Visual-Computing-P01/content/01-foundations-i-ch018.tex}",
                r"\input{../current/02-Mathematics-for-Visual-Computing-P02/content/02-foundations-ii-ch025.tex}",
            ]
            main.write_text("\n".join([
                r"\documentclass{book}",
                r"\usepackage[print]{allmight-book}",
                r"\SetBookStage{2}{Mathematics for Visual Computing}",
                r"\begin{document}",
                *originals,
                r"\end{document}",
            ]), encoding="utf-8")

            def copy_prebuilt(args, *, check, text, stdout, stderr):
                self.assertIn("--output", args)
                destination = Path(args[args.index("--output")+1])
                shutil.copytree(self.atlas, destination, dirs_exist_ok=True)
                return subprocess.CompletedProcess(args, 0, stdout="BOOK02_ATLAS_OK")

            with patch("book02_visuals.subprocess.run", side_effect=copy_prebuilt):
                installed = install_book02_visuals(main, include_rich_atlas=True)
            rendered = main.read_text()
            for original in originals:
                self.assertEqual(rendered.count(original), 1)
            self.assertIn("28-atlas-orientation.tex", installed)
            for name in ("29-atlas-geodesic.tex", "30-atlas-geodesic-3d.tex",
                         "31-atlas-resolution.tex"):
                self.assertIn(name, installed)
            self.assertTrue((Path(tmp)/"book02-figures"/"rendered"/"figures"/
                             "geodesic-fields.pdf").is_file())
            saved=main.read_bytes()
            self.assertEqual(install_book02_visuals(main,include_rich_atlas=True),[])
            self.assertEqual(saved,main.read_bytes())

    def test_rich_chapter_mapping_covers_all_seven_figures_and_bridge(self) -> None:
        snippets={n for v in RICH_FIGURES.values() for n in v}
        self.assertEqual(len(snippets),8)
        source = REPO/"Book Production"/"AllMight Codex"/"LaTeX"/"Assets"/"Book02"
        self.assertIn("36-from-probability-to-pixels.tex",snippets)
        self.assertTrue((self.atlas/"coverage"/"triangle-pixel-coverage.pdf").is_file())
        self.assertEqual(self.manifest["coverage_bridge"]["exact_fraction"],"119/192")
        for slug in FIGURES:
            matches=[n for n in snippets if slug in (source/n).read_text()]
            self.assertEqual(len(matches),1,slug)
            self.assertIn(r"\includegraphics", (source/matches[0]).read_text())


if __name__ == "__main__":
    unittest.main()
