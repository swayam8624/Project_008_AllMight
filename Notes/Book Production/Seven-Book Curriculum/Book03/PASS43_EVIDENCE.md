# Pass 43 — final reader-facing editing and publication handoff

The 444-page uploaded review was structurally sound, but code rendered smaller than intended, several end pages were sparse, the title page lacked series identity, and production history intruded on the reading experience. Pass 43 adds 30 new explanatory SVGs to Chapters 18–32 (two per chapter), raises the figure inventory from 142 to 172, improves the print monospace and title layout, provides a curated subject index with live EPUB anchors and LaTeX page references, and adds concise chapter orientation panels. The source editor uses explicit rules, preserves original Markdown and records displaced prose in reader-editorial-provenance.json. No algorithms or source masters are replaced. The newly added mathematical notation pass is targeted, not an exhaustive proofread.

Actual earlier CI 45596834550: EPUBCheck 5.4.0 completed with zero errors/warnings; LuaLaTeX failed because the Linux runner could not resolve STIX. This pass probes installed STIX-family names before compiling. The new build and full visual review must still be run. No final page count is asserted, and no Book01/02 artifact is changed. A few sparsely occupied pages can still be appropriate at chapter boundaries; the QA report now identifies text-sparse pages for human review, including figure-only pages, rather than claiming all were removed.

## Canonical outputs on the Mac

Compile in a temporary proof directory using the established builder, with the approved cover and EPUBCheck 5.4.0, then run the proof verifier and native tests. Use the publication_library.py promote command for Book 3 only. Dry-run first; apply after the build and visual QA pass, using --replace only when the existing canonical Book03 files are being superseded. This archives the prior Book03 pair rather than altering Books01/02. The completed deliverables belong in the existing local distribution library:

- From Bits to Research/PDF/03 - From Geometry to Images.pdf
- From Bits to Research/EPUB/03 - From Geometry to Images.epub

Run publication_library.py verify after promotion; compare hashes with the validated proof files. Do not create a second canonical folder or leave the only final copies in Code Parts/build. Keep logs in the proof directory and respect existing Git ignore policy for the publication library. The approved cover is supplied locally by its exact recorded hash.
