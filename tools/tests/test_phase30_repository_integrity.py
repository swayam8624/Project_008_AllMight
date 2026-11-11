"""Repository-wide portability and original-title coverage checks.

The Windows-safe path audit is deliberately cross-repository because Windows
checkout fails before a Python unit test can begin if any tracked path uses
forbidden platform characters. See Phase 30 source/verification ledger.
"""
from __future__ import annotations

import pathlib
import re
import subprocess
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
OLD_TITLES = (
    "Function contracts: domains, codomains, and images",
    "Nonlinear least squares: Gauss-Newton and Levenberg-Marquardt",
    "Sampling quality: stratification, quasi-Monte Carlo, and blue noise",
)


class RepositoryPortability(unittest.TestCase):
    def test_no_invalid_windows_checkout_paths(self):
        output = subprocess.check_output(
            ["git", "-C", str(ROOT), "ls-files", "-z"])
        paths = [p.decode("utf-8") for p in output.split(b"\0") if p]
        invalid = []
        reserved = re.compile(
            r"^(CON|PRN|AUX|NUL|COM[1-9]|LPT[1-9])(?:\.|$)",
            flags=re.IGNORECASE)
        for path in paths:
            for component in path.split("/"):
                if (re.search(r'[<>:"\\|?*]', component)
                        or component.endswith((" ", "."))
                        or reserved.match(component)):
                    invalid.append(path)
                    break
        self.assertEqual(invalid, [], "tracked Windows-incompatible path names")

    def test_no_stale_links_to_renamed_concept_notes(self):
        hits = []
        for path in ROOT.rglob("*.md"):
            text = path.read_text(encoding="utf-8")
            wiki_links = re.findall(r"\[\[([^\]]+)\]\]", text)
            for link in wiki_links:
                if any(old in link for old in OLD_TITLES):
                    hits.append(f"{path.relative_to(ROOT)}: {link}")
        self.assertEqual(hits, [], "stale note links after Windows-safe rename")

    def test_all_one_hundred_canonical_phase30_titles_mapped_once(self):
        path = ROOT / "Supplementary" / "Foundations.md"
        book = path.read_text(encoding="utf-8")
        marker = "## Phase 30 production evidence and mastery — Parts 2901–3000"
        self.assertEqual(book.count(marker), 1)
        section = book.split(marker, 1)[1].split("\n## ", 1)[0]
        found = [int(n) for n in re.findall(r"^\| (29\d\d|3000)\. ", section, re.M)]
        self.assertEqual(found, list(range(2901, 3001)))

    def test_phase30_reference_targets_exist(self):
        targets = (
            "tools/production_evidence.py",
            "tools/tests/test_production_evidence.py",
            ".github/workflows/allmight-phase30-evidence.yml",
            "Continuous Notes/Production - From Source Files to Evidence and Defensible Systems.md",
            "Visual Recall/Phase 30 - From Publication to Research Defense.md",
        )
        for relative in targets:
            with self.subTest(target=relative):
                self.assertTrue((ROOT / relative).is_file())


if __name__ == "__main__":
    unittest.main(verbosity=2)
