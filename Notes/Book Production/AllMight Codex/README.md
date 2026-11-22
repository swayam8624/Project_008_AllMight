# The AllMight Codex — book production

This folder contains a source-preserving production of the three current manuscripts as six volumes. The original continuous notes and the original interior template remain the editorial sources. A production export does not certify exhaustive coverage of every book, video, or field of research previously discussed.

**Production is in progress.** QA previews and individual chapter documents are intermediate artifacts. The final validation report will identify the six finished EPUBs, six InDesign books, and exported PDFs when their checks are complete.

| Book | Volume I | Volume II |
|---|---|---|
| The Foundations Bible | Chapters 1–20 | Chapters 21–39 |
| The Visual Computing Bible | Chapters 1–21 | Chapters 22–43 |
| The Engine Builder’s Bible | Chapters 1–24 | Chapters 25–47 |

The print edition uses the original AllMight Vintage Technical interior at a 7 × 10 inch trim, with Palatino text, Menlo code, reusable native styles, and restrained vector ornaments. The earlier novel-derived template is retained only as a historical prototype. Each volume is an InDesign Book containing threaded chapter documents, a linked contents document, and a reference-companion document. The original template remains unchanged.

The reflowable edition uses selectable text and code, semantic tables, MathML equations, SVG diagrams, and linked topics. Six separate EPUBs retain the volume structure. A combined collection EPUB is also planned so that links between volumes can resolve inside one package; support for links opening a separate EPUB varies by reader.

XML is the structured import source. The XML folder retains original SVG references and formula metadata. InDesign/Linked XML contains derived import files that use verified vector PDFs for print placement and retain the original SVG location. The sharedAssets folder contains reusable equation, diagram, and print assets. AST files preserve the source structure and enable independent checks against the original Markdown.

Layout-only formula and Mermaid changes are recorded in normalization ledgers. They adjust rendering and page fit; the original Markdown is preserved. A successful renderer check proves that an expression rendered and agrees with its recorded source, not that every mathematical or technical claim has received an independent subject-matter review.

Do not move individual chapter documents away from their volume or discard sharedAssets before packaging; native graphics and XML are linked resources. Consult the final delivery report for verified results rather than relying on the existence of an output file.
