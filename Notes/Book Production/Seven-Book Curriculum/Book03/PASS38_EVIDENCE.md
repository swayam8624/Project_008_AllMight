# Pass 38 — complete reader staging and publication proof

The full-reader source preparation at revision 08c34c100042b35247f3a35db0f7e351ab65eb78 passed on Pandoc 3.7.0.2: [run 45571742458](https://github.com/swayam8624/Project_008_AllMight/actions/runs/45571742458). It processed all 32 chapters, 142 nonblank SVG-derived PNGs, 2,533 parsed math nodes and 64 code blocks, staged shared print/EPUB ASTs with stable chapter anchors, and wrote an HTML/MathML preview, source hashes and twelve illustration contact sheets. Artifact 15004635092 was uploaded (16,633,761 bytes); it is a temporary CI proof bundle, not a released ebook. The later caption-order and artifact QA amendments are not included in that passed revision. Rendering success does not certify every diagram's visual or scientific quality.

The source scan found captions in Chapters 07 and 10 out of reading order. The captions and any matching prose references are now renumbered; file names and asset identities remain unchanged. The new builder checks actual parsed equation labels, source image/caption pairing, caption order and internal anchors, and rejects MathML conversion warnings. FRONT_MATTER.md and BACK_MATTER.md provide the preface, chapter route, laboratory guide and notation reference.

## Local print smoke evidence

A small independent fixture (not the full manuscript) compiled using Pandoc 3.1.11.1 and LuaLaTeX in the working container. The first attempt failed because the preferred TeX Gyre Pagella OTF and math fonts were absent. With installed DejaVu Serif/DejaVu Math TeX Gyre substituted, the same print header produced three 576 x 720 pt pages; the rendered chapter page was inspected and showed the numbered aligned equation, 10pt body and colored syntax-highlighted code box. The full GitHub build installs the preferred font packages. A minimal EPUB3 smoke fixture independently confirmed Pandoc's actual MathML, preformatted code text and whole-document chapter navigation targets; the artifact verifier handles both file and fragment navigation. This is not a completed full-book PDF/EPUB test.

## Build and review (from repository root)

Use the existing Code Parts/build layout. Install Pandoc 3+, Python with CairoSVG and Pillow, LuaLaTeX/TeX Live with the declared fonts and packages, Poppler utilities, and EPUBCheck for formal validation. The proof output directory must not exist.

~~~sh
python3 "Code Parts/tools/book03_prepare_reader.py" --build
python3 "Code Parts/tools/book03_proof_qa.py" "Code Parts/build/book03-reader-proof"
~~~

The driver writes exactly one proof PDF and one proof EPUB in the new proof directory, with shared source receipts. No promotion or overwrite. Optional --cover is EPUB-only and must be an explicitly approved image; the print proof remains a title-page proof. The new CI is configured to compile both formats and run actual output QA (embedded figures, code, MathML, navigation, extracted chapter titles, 8x10 size, page samples); that full path is not yet certified. Review the actual logs and contact sheets before treating it as a release. A final approved cover, reader typography/figure review, complete cross-format acceptance and historical manifest reconciliation remain outstanding. Do not infer historical source-span consistency from the successful 25-blob custody gate.
