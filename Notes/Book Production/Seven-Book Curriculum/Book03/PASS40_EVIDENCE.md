# Pass 40 — production proof and Mac handoff

At revision 7b094d21845b0499d21d85deffc64776406f9075, preferred-font production run [45581346191](https://github.com/swayam8624/Project_008_AllMight/actions/runs/45581346191) completed preparation and three LuaLaTeX passes, but the strict print gate found two horizontal overflows. The actual retained artifact 15008055189 was retrieved. Its PDF is 418 pages, 576 by 720 pt, has 490 outline entries including all 32 numbered chapter openings, and contains 142 distinct image XObjects. Independent PyMuPDF scanning found no blank pages or text/image blocks outside the page boundary. Six pages were rendered and visually inspected. The EPUB contains 2,533 MathML elements; the source proof reports zero HTML warnings. This failed proof is not release-ready.

The exact TeX/log lines identified a long companion source path in Chapter 09 (2.25735 pt) and the Chapter 12 title (12.85484 pt). The reader now uses a split list of relative companion paths and a slightly smaller ragged-right chapter heading while preserving 10pt prose and 8.3pt code. The next production run must confirm zero overfull boxes, missing characters and undefined references. EPUBCheck 5.4.0 is mandatory for complete proof builds; CI downloads the official release asset and verifies SHA-256 33350c61038e71dfb3d45a76aed04bf5481e6d5500cb780f6e98db8bbd15a28c. Validation runs before the longer print build. Version 4.2.6 previously installed by apt was not sufficient for the intended acceptance gate.

A cover path is optional for proof generation, and must be explicitly approved. When supplied, a portrait PNG/JPEG is staged byte-for-byte, used by both formats, and recorded by SHA-256. A synthetic 800x1000 fixture tested the LuaLaTeX first-page cover hook without overfull warnings; no real Book03 cover has been approved or tested yet. Actual-output QA checks the EPUB cover image bytes and presence of a first-page PDF image. It does not certify cover aesthetics. Without a cover, the files are title-page proofs only.

## Mac execution

Follow the existing repository, never recreate the directory structure. Run in the actual Master checkout with the Book03 branch checked out and a clean worktree. Check the latest publication CI status first. A complete proof requires Pandoc 3+, LuaLaTeX and the TeX Gyre Pagella / TeX Gyre Pagella Math / DejaVu Sans Mono font families; Python with CairoSVG and Pillow, Poppler tools, CMake and a C++23 compiler, Java and EPUBCheck 5.4.0 available on PATH. Use a fresh proof output directory under Code Parts/build, and retain logs.

From the repository root (use a new unique OUT directory on reruns):

~~~sh
git status --short
git fetch origin curriculum/book03-geometry-images-pass01
git switch curriculum/book03-geometry-images-pass01
git pull --ff-only origin curriculum/book03-geometry-images-pass01
OUT="Code Parts/build/book03-reader-proof-$(date +%Y%m%d-%H%M%S)"
python3 "Code Parts/tools/book03_prepare_reader.py" --build --output "$OUT"
python3 "Code Parts/tools/book03_proof_qa.py" "$OUT"
~~~

Supply --cover with a verified approved Book03 cover when available. Run the native CMake/CTest suite from the same Git commit. Formal EPUBCheck success, final print log, all output QA, reader visual review and confirmed cover are required before promotion. Use publication_library.py --help and promote --book 3 --pdf PATH --epub PATH as a dry run first; only use --apply after explicit release approval. Never use --replace for Books01/02, git reset/clean, or overwrite an existing canonical book. Historical manifest drift remains disclosed, distinct from original source custody. The manuscript and canonical output library remain unchanged by this proof driver.
