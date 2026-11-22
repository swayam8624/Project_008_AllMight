# AGENTS.md — instructions for AI coding assistants

**Before changing files in this repository, read [CHATGPT.md](CHATGPT.md) in full.** It is the current seven-book editorial and publication charter, including the canonical titles, one-EPUB/one-PDF release rule, C++23-first pedagogy, Book Seven research-visualization requirements, source preservation rules, and cleanup safety gates.

Also read [Series Editorial and Citation Policy](Notes/Book%20Production/Seven-Book%20Curriculum/SERIES_EDITORIAL_AND_CITATION_POLICY.md) for claims, licensing, citations, production and visual evidence.

**Do not infer that historic EPUB/PDF files are finished books.** The active book is the one explicitly named by the author. Book One and all other volumes are read-only references during Book Two work.

**Never delete, compact or overwrite** `Continuous Notes/`, original chapters, active generations, converter ASTs/assets, original covers, source manifests, C++23 labs, experimental evidence, or unrelated local edits. Never perform `git clean -fdx`, broad `rm -rf`, forced checkouts or destructive reset.

**Release locations:** `From Bits to Research/EPUB/` and `.../PDF/`, one complete file per book per format. Use `python Code Parts/tools/publication_library.py scan` to inspect candidate files; use explicit `promote` with `--apply` only after checking input and quality gates. The publication folder is intentionally Git-ignored.

When supplying shell commands to the author (macOS zsh), use quoted paths and avoid bare interactive comment lines (`#`). Validate work with actual tests; distinguish successful synthetic CI proofs from the entire seven-book production and separate EPUBCheck from manual reader QA.
