# Book 1 publication proof — 19 November 2026

## Scope and source identity

This is a checkpoint for the source-preserving release branch `curriculum/cpp23-nes-project-pass01`. The canonical 16 chapter source bodies and 18 reversible project additions remain intact; the publisher additionally prepares 16 chapter-specific academic enrichments, 16 learning gates, one digital master, two 7 × 10-inch print masters and four diagnostic masters.

The GitHub Actions workflow `allmight-book01-expanded-reader.yml` successfully prepared the 16 chapters, verified the exact source/addition inventory, compiled the four C++23 companions, and rendered **41 of 41 Mermaid figures** at commit `e395ec31` (run 44551925593). All those figures were present in the generated source archive. The later print-header correction has a separate CI run.

## LuaLaTeX compilation proofs

The generated source artifact was built in the analysis environment using LuaLaTeX and local substitute text fonts (Noto Serif / STIX Math where the environment lacks TeX Gyre). This **does not assert pixel-for-pixel parity with MacTeX**. Actual reader-style code remains configured to use the preferred fonts when available; the font fallback was confined to the local proof copy.

| Master | Local first-pass pages | Embedded figure instances | First-pass TeX compilation | Geometry scan |
|---|---:|---:|---|---|
| Diagnostic Part 1 | 253 | 15 | Passed | No clipped text |
| Diagnostic Part 2 | 273 | 6 | Passed | No clipped text |
| Diagnostic Part 3 | 290 | 13 | Passed | No clipped text |
| Diagnostic Part 4 | 311 | 7 | Passed | No clipped text |
| Complete digital | 1,114 | 41 | Passed | No clipped text, misplaced header, overlapping header or duplicate folio |
| Paperback Volume I | 528 | 21 | Passed | 7 × 10 in; no detected geometry or code-size blocker |
| Paperback Volume II | 604 | 20 | Passed | 7 × 10 in; no detected geometry or code-size blocker |

**The listed counts are first-pass local proof estimates.** The actual release tool runs multiple TeX passes, builds the back-of-book word index, writes publication-proof JSON and enforces the 828-page paperback cap. That may change pagination. The user should validate final KDP interiors with the real MacTeX font installation and the KDP previewer.

## Defects identified and permanently guarded

1. Chapter 9 contained `\\texttt{simd_sum}` and `\\texttt{simd_prefix_exclusive_sum}` in source-protected display math. Raw TeX text macros require escaped underscores. The math renderer now escapes text-mode specials while keeping real mathematical subscripts unchanged. Regression tests cover both Chapter 9 expressions.
2. Generated `\\texttt{}` identifiers containing nested `\\textless{}` and `\\textgreater{}` macros bypassed the old one-level regex. A brace-balanced transform now offers invisible line breaks inside nested template names and API paths, preserving verbatim code and the original Markdown.
3. The long Chapter 6 filesystem acceptance formula had to be presented over several aligned lines. Display reflow is strictly downstream of source-identity checks, and regression tests verify logical conjuncts are not lost.
4. The shared print profile initialized fancyhdr with a header width of about 449.5 pt. The final 7 × 10-inch Book 1 geometry has a text measure of approximately 388.1 pt. The reader style now resets `\\headwidth` to `\\textwidth` **after** setting page geometry and before running headers, fixing both clipped outer headers and out-of-page folios in paperbacks.
5. The PDF proof checker still rejects truly undersized listings, cropped text, missing metadata or figures, incorrect trim, and exceeded paperback page count. Ordinary numeric sub/superscripts are not misclassified as undersized code if they accompany readable baseline glyphs.
6. Source preparation explicitly rejects disappeared, reordered or modified fenced code and diagrams, missing or duplicate math markers, and changed index tokens. These checks protect source integrity before compilation.

## Final user-side acceptance

Run all source tests, source preparation and verification, then digital and both paperback builds in the Mac checkout. Do not distribute any PDF merely because LuaLaTeX exits successfully: the publisher may refuse it on layout checks, and physical interiors still require proofing in KDP Print Previewer. Review actual equations, bitmap figure captions, contents/index pages, and selected listings before public release.

The local verification is evidence of compilation and geometry, not formal certification of every technical claim, code example, equations' semantics, PDF accessibility tagging, or copy-editing across more than a thousand pages.
