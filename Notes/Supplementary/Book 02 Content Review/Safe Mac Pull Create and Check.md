# Book Two — Safe Mac checkout, source checks, print and EPUB candidates

This workflow preserves the existing Obsidian `Master` checkout and its divergent local branch. It makes a **separate Git worktree** for the Book Two remote branch. Never run `git pull origin curriculum/book02-mathematics-refinement-pass01` while sitting on the old Master branch.

## 1. Fetch and create the isolated checkout

Run this on the Mac **once**:

```bash
MASTER="$HOME/Library/Mobile Documents/iCloud~md~obsidian/Documents/Master"
BOOK02="$HOME/Desktop/Project_008_AllMight_Book02"
REMOTE_BRANCH="curriculum/book02-mathematics-refinement-pass01"

git -C "$MASTER" fetch origin \
  "refs/heads/$REMOTE_BRANCH:refs/remotes/origin/$REMOTE_BRANCH"

git -C "$MASTER" worktree add --track \
  -b "$REMOTE_BRANCH" "$BOOK02" "origin/$REMOTE_BRANCH"

git -C "$BOOK02" status -sb
```

For subsequent updates: `git -C "$BOOK02" pull --ff-only`. Do not create the same worktree a second time.

## 2. Python and external build tools

```bash
cd "$HOME/Desktop/Project_008_AllMight_Book02"
export PATH="/Library/TeX/texbin:/opt/homebrew/bin:/usr/local/bin:$PATH"

if command -v python3.12 >/dev/null 2>&1; then
  python3.12 -m venv .venv-book02
else
  python3 -m venv .venv-book02
fi

PY="$PWD/.venv-book02/bin/python"
"$PY" -m pip install -r tools/labs/requirements-book02-atlas.txt pypdf

COVER="$HOME/Downloads/From Equations to Geometry (1).jpg"
test -f "$COVER" && sips -g pixelWidth -g pixelHeight "$COVER"

"$PY" "Book Production/AllMight Codex/LaTeX/Tools/book02_source_preflight.py"

"$PY" "Book Production/AllMight Codex/LaTeX/Tools/book02_release_candidates.py" \
  --check-only --cover-image "$COVER"
```

The prerequisite gate checks source hashes, 36 supplementary lessons, two original physical main files, TeX, Pandoc, Java, the original author-local source-cache helper, EPUBCheck and the actual cover dimensions. A successful preflight is permission to **attempt** the full build, not an assertion that KDP accepts the output.

## 3. Test, build and verify

```bash
"$PY" -m unittest discover -s tools/labs -p 'test_book02_*.py' -v
"$PY" -m unittest discover \
  -s 'Book Production/AllMight Codex/LaTeX/Tools' \
  -p 'test_book02_*.py' -v

"$PY" "Book Production/AllMight Codex/LaTeX/Tools/book02_release_candidates.py" \
  --print-only

"$PY" "Book Production/AllMight Codex/LaTeX/Tools/book02_release_candidates.py" \
  --epub-only --cover-image "$COVER"
```

The candidate builder intentionally merges the two separate successful build reports, but refuses to combine results if the source revision fingerprint has changed in between. Source changes require rebuilding both candidate formats.

Review the combined report:

```bash
"$PY" - <<'PY'
import json
from pathlib import Path

latex = Path("Book Production/AllMight Codex/LaTeX")
current = json.loads((latex/"CURRENT.json").read_text())
generation = Path(current["generation_path"]).name
report_path = latex/"Build"/generation/"book02-publication-candidates"/"book02-candidate-verification.json"
report = json.loads(report_path.read_text())
assert report["source_lock"]["original_chapters"] == 16
assert report["source_lock"]["additive_sources"] == 36
assert len(report["print_parts"]) == 2
assert report["epub"] and report["epub"]["supplement_source_count"] == 36
assert report["epub"]["svg_asset_count"] >= 13
assert report["epub"]["epubcheck"]["exit_code"] == 0
assert report["epub"]["cover_sha256"]
for item in report["print_parts"]:
    assert Path(item["candidate_path"]).is_file()
    print("PDF", item["id"], item["pages"], "pages:", item["candidate_path"])
print("EPUB:", report["epub"]["candidate_path"])
print("REPORT:", report_path)
PY
```

**Caveat:** Successful automation validates source identity, execution of supported mathematical oracles, packaging, declared EPUB structure and PDF page counts. Actual page-by-page mathematical review, layout/alt-text reader checks, KDP preview, and the paper/book source-claim audit must still be done before a public publication claim. The JPEG is an EPUB front cover, not a full paperback wraparound cover with trim, bleed and spine.


## 4. Deliver verified candidates into the canonical Obsidian Master folder

The Desktop worktree exists **only** as a safe branch-isolated build environment.
It is not the official destination of the books. The Obsidian `Master` vault
continues to be the canonical project workspace.

The Noto Emoji font is already committed at
`Book Production/AllMight Codex/LaTeX/Assets/Fonts/NotoEmoji-Static.ttf`.
The compiler now stages it automatically in the nested working folder.
Neither a second font download nor manually moving fonts into Master is necessary.

Only **after both complete physical PDFs and the full stage EPUB have succeeded**
and their combined verification report exists, run:

```bash
cd "$HOME/Desktop/Project_008_AllMight_Book02"
PY="$PWD/.venv-book02/bin/python"
MASTER="$HOME/Library/Mobile Documents/iCloud~md~obsidian/Documents/Master"
TARGET="$MASTER/Book Production/AllMight Codex/LaTeX/Build"

"$PY" "Book Production/AllMight Codex/LaTeX/Tools/book02_release_candidates.py" \
  --export-only --export-dir "$TARGET"
```

The result is a versioned subfolder of Master containing the two verified PDFs,
the EPUB and their SHA-checked verification report. The exporter will refuse
missing/invalid candidates and will never overwrite an existing different
output. It does not modify the original Markdown source, move the original
font, alter Git branches, or merge potentially divergent `Master` history.

To merge **source changes** into the canonical Master branch in a subsequent
step, first reconcile and back up its current Git state. Do not force-pull
the Book Two branch onto Master or run a destructive checkout.
