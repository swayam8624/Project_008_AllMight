# Book One — KDP eBook and Digital-PDF Release

**Book:** From Bits to Parallel Systems — C++23, CPU Execution, GPU Memory, and Reliable Software  
**Series:** From Bits to Research, Book One of Seven  
**Author:** Swayam Singal

## Original cover artwork

Save the actual user-provided artwork as a real RGB JPEG or PNG file on the Mac. Do not synthesize a similar-looking image or silently substitute a title-page screenshot. The marketing cover for KDP is uploaded separately from the EPUB, which also embeds an internal cover.

For best quality prepare a JPEG approximately **1600 × 2560 pixels** (portrait). Keep the text legible at thumbnail size and preserve aspect ratio when scaling. Official sources: [KDP eBook cover](https://kdp.amazon.com/en_US/help/topic/G6GTK3T3NUHKLEFX), [EPUB upload](https://kdp.amazon.com/en_US/help/topic/G200634390), [Kindle Previewer](https://kdp.amazon.com/en_US/help/topic/G202131170).

## Build on the author's Mac

From the Project AllMight repository root, on curriculum/cpp23-nes-project-pass01:

~~~sh
git switch curriculum/cpp23-nes-project-pass01
git pull --ff-only origin curriculum/cpp23-nes-project-pass01

.venv-book01/bin/python tools/test_book01_render_support.py
.venv-book01/bin/python tools/test_book01_publication_edit.py
.venv-book01/bin/python tools/test_book01_pdf_proof.py

.venv-book01/bin/python tools/build_book01_expanded_reader.py --check
.venv-book01/bin/python tools/build_book01_expanded_reader.py --prepare
.venv-book01/bin/python tools/build_book01_expanded_reader.py --verify-prepared

# Point to the exact cover JPEG file.
COVER="$HOME/Downloads/book01-original-cover.jpg"

# Exact art on the first page of the continuous digital PDF.
.venv-book01/bin/python tools/stage_book01_cover.py --cover "$COVER"
.venv-book01/bin/python tools/build_book01_expanded_reader.py --compile

# Source-based EPUB 3 — not a PDF-to-image conversion.
.venv-book01/bin/python tools/build_book01_kdp_epub.py --cover "$COVER"
~~~

The EPUB output should be written to build/book01-expanded-reader/Book01-From-Bits-to-Parallel-Systems-KDP.epub. A structural QA report is in build/book01-expanded-reader/epub-preflight/epub-preflight.json.

**Important:** a syntactically valid EPUB does not establish Kindle compatibility. Do not publish without the final Kindle Previewer checks.

## Final independent publication gates

1. **Preservation:** all 16 original chapters, 18 chapter integrations, additional scientific explanations, final capstone, academic back matter and references are present. No formula/diagram placeholder survives.
2. **Identity:** series, book, subtitle, author, copyright and edition data agree with the KDP Bookshelf. No temporary academic proof labels, fabricated ISBNs or Git hashes appear as front matter.
3. **Cover:** the exact supplied RGB artwork appears inside the EPUB and on the complete digital PDF. Upload a high-resolution marketing cover separately on KDP. A paperback requires a separate front/spine/back wrap PDF, not this title-page image.
4. **EPUB:** run EPUBCheck (zero errors); verify the ZIP mimetype, resource manifest, linked figures, navigable table of contents, chapter boundaries, copyright page and backlinks.
5. **Math:** confirm Unicode glyphs, fractions, alignments, subscripts, Greek symbols, boxed equations and MathML render correctly on Kindle. No raw TeX commands should be printed as prose.
6. **Code:** C++ listings remain character-for-character meaningful, including braces, semicolons, quotes, directives, templates and indentation. Inspect narrow screens and font-size changes.
7. **Tables and diagrams:** check wrapping, caption association, no duplicate source labels, no unreadable small diagrams. Use responsive layouts where possible.
8. **Citations:** verify publisher books, editions, ISBNs, DOI resolver links, GitHub repositories and applicable third-party figure licences. A mutable default branch is not an immutable research artefact.
9. **Independent content review:** technical proof and reading review must cover Chapters 5–16 and final capstone, not only the first four chapters.
10. **Devices:** open the EPUB in [Kindle Previewer](https://kdp.amazon.com/en_US/help/topic/G202131170), examine phone/tablet/e-ink views and adjust font sizes. Open it in KDP Online Previewer after upload. Any visible formula or code corruption is release-blocking.

**Do not upload a PDF-derived EPUB proof as the final book.** PDF text extraction corrupts mathematical expressions, reading order, table semantics, code and some hyperlinks. Use the source-based EPUB build above for publication.
