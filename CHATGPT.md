# CHATGPT.md — FROM BITS TO RESEARCH

**Repository:** \`swayam8624/Project_008_AllMight\`  
**Series:** **FROM BITS TO RESEARCH**  
**Series promise:** *An Integrated Journey Through Computing, Mathematics, Graphics, Simulation, and Scientific Discovery.*  
**Author:** Swayam Singal  
**Architecture:** Exactly **seven integrated technical books**, each with one reflowable EPUB and one complete, single-book reading PDF as the canonical reader exports. Internal print parts, partial PDFs, source generation stages and experimental previews do **not** count as separate books.

> **Read this first for any AI/Codex/ChatGPT session working in the Master repository.** Read \`AGENTS.md\` too. The user's currently named book takes precedence; do not mistake reference volumes for active targets. Do not run a full LaTeX/PDF build unless explicitly requested; the author's Mac owns platform-specific compilation.

## The seven canonical books

The series titles, subtitles and central promises below are intentional editorial identity. Legacy stage IDs and old filenames may differ; **never substitute those internal names on covers or in canonical exports**.

| # | Book title | Subtitle | Teaching purpose |
|---|---|---|---|
| **01** | **From Bits to Parallel Systems** | **C++23, CPU Execution, GPU Memory, and Reliable Software** | Foundations of modern computing and systems engineering. |
| **02** | **From Equations to Geometry** | **Mathematics, Numerical Methods, Optimization, and Visual Computing** | The mathematics needed to understand, construct, and simulate computational worlds. |
| **03** | **From Geometry to Images** | **3D Geometry, Transformations, Cameras, Projection, and Image Formation** | How three-dimensional information becomes an image. |
| **04** | **From Light to Realism** | **Rendering, Materials, Ray Tracing, Global Illumination, and Image Synthesis** | The physics, mathematics, and algorithms behind convincing images. |
| **05** | **From Code to Living Worlds** | **Game Engine Architecture, Physics, Collision Detection, and Simulation** | Building complete interactive worlds with physically meaningful behavior. |
| **06** | **From Frames to Real Time** | **GPU Architecture, Vulkan, Metal, Advanced Rendering, and Performance Engineering** | Transforming rendering algorithms into production-quality, high-performance graphics systems. |
| **07** | **From Perception to Discovery** | **Computer Vision, Neural Rendering, XR, Scientific Experimentation, and Research** | Reconstructing reality, developing new methods, validating experiments, creating scientific visualizations, and communicating original research. |

Never compress this curriculum into generic graphics textbooks. Maintain the coherent progression **bits → mathematics → geometry → light → living worlds → real-time systems → perception/research**.

### Book Seven: scientific communication is a first-class skill

Book Seven must teach, *in executable and verifiable depth*, how to build beautiful **and scientifically accurate** 3D visualizations, geometry/physics animations, uncertainty and error heatmaps, benchmark dashboards, experimental figures, synchronized comparative visual grids, interactive demonstrations, paper figures, posters, and publication-quality research talks/presentations. Explain camera, scale, units, legends, color maps, typography, uncertainty intervals, aggregation choices, equal-comparison controls, negative results, reproducibility and provenance. A pleasing visualization alone is **not** proof that the underlying algorithm is correct.

## Editorial philosophy — depth without bloat

1. Teach from first principles in a **continuous explanatory narrative**, not disconnected bullet summaries. Develop every idea from the concrete problem, intuition and mechanisms through equations, derivations, units, algorithm, worked examples, implementation, tests, failure cases, and its links to what follows.
2. Preserve the original manuscript and source documents. **Add in context**. Do not silently delete, truncate, paraphrase a long original into a short summary, reorder, or collapse thousands of pages because export is inconvenient. Correct a *proven* mathematical/technical error transparently; log the change.
3. Build cumulative, meaningful **C++23** projects throughout *all seven books*, not just one NES, graphics engine, or demo. Compilable, commented, well-structured code; CMake/CTest; independent oracles; adversarial inputs; deterministic seeds; expected outputs; warnings; performance measurements with baselines and numerical correctness gates.
4. Python is secondary: optional reference computations, numerical analysis, data processing, experiment orchestration, plotting and independent validation. It must not replace the intended C++23 engineering labs.
5. Number equations, figures, examples, theorems and exercises as appropriate; provide working internal references, meaningful figure captions/alternative text, answers or checking criteria, notation tables, chapter prerequisites and trustworthy bibliographies.
6. Cover computational geometry, linear algebra, numerical conditioning, differentiation, optimization, probability, uncertainty, physics, graphics, GPU and modern research with **genuine derivational and experimental depth**. If a requested subject is thin or absent, say so and expand rather than claiming complete coverage.
7. Avoid repeating the same caveat in every section. Make limitations concrete using a failing input, rendering artifact, counterexample, benchmark or bug story. Cite primary papers, official standards and actual specifications; **never fabricate** a DOI, ISBN, experiment, result, source inspection or publication approval.
8. Treat figures, tables, diagrams, code, test logs, captions, references, page design and navigation as part of teaching, not decoration.

## Publication rules — EPUB first, one PDF per book

Current release priority: **EPUB**. Each book ultimately has exactly two canonical digital files, one per format:

\`\`\`text
Master/
└── From Bits to Research - Publications/
    ├── EPUB/
    │   ├── 01 - From Bits to Parallel Systems.epub
    │   ├── 02 - From Equations to Geometry.epub
    │   └── ... 03–07 ONLY when separately approved ...
    ├── PDF/
    │   ├── 01 - From Bits to Parallel Systems.pdf
    │   ├── 02 - From Equations to Geometry.pdf
    │   └── ... 03–07 ONLY when a SINGLE complete book PDF exists ...
    ├── Archive/
    └── Manifests/
\`\`\`

This is the **distribution/output library**, not the source tree; it is intentionally local and Git-ignored. \`tools/publication_library.py\` inventories and promotes explicit, verified candidates into it, preserves historical editions in Archive, and emits manifest/provenance files. **Never auto-promote all seven historic EPUB exports**: a file existing on disk is not evidence of a publication-ready reader edition. **Never publish physical parts** such as P01/P02 as the sole "PDF" of a book. Print-on-demand could require multiple physical interiors; the digital reading PDF must still be one complete file per book.

**Release gates for every EPUB**
- Correct final author-supplied cover **visibly rendered** in the opening spine document, with EPUB 3 cover-image metadata and a tested thumbnail; title, subtitle, author, publisher/ISBN only if legitimately assigned, copyright/rights, language, description and series metadata.
- Complete original content, navigable two/three-level table of contents and NCX fallback; accurate reading order and chapter numbers; linked glossary/index/figures/cross-references; no missing attachments implied by prose.
- Moderate XHTML fragment sizes; MathML semantics and *exact* OPF \`mathml\` properties; accessible images; no duplicate XML/SVG IDs, broken links, blank figures or harmful dark-mode CSS.
- Official **EPUBCheck 5.4.0** on the *exact final bytes* with **0 errors and preferably 0 warnings**. Test in Apple Books plus at least one independent reader; test small-screen layout, figures, tables, mathematical notation, external links and font controls. Older Kindle math fallback is a separate compatibility decision.
- Verify original chapter/math/code/source preservation, independent figure accuracy, linked references and code compilation. Attach QA output, SHA-256, cover provenance and build commands. Never claim a generated PDF/EPUB is certified just because a pipeline ran.

**Release gates for every PDF**
- **One unified complete reading PDF per book** (even when the LaTeX source comprises multiple physical parts), correct cover/front matter, bookmarks/TOC, internal links, equation/figure numbers, correctly styled code and pedagogical boxes, readable scientific figures, typography, no clipping/overflow, usable page layout and author-led page-by-page visual QA. A few successful compile tests do not certify the whole book.
- Preserve physical print-part source trees internally. They are not redundant. Special print interiors, trim/spine sizes and POD constraints are a **separate** publication concern.

## Current source of truth, migration cautions and boundaries

- Current generated seven-stage inventory: \`Book Production/AllMight Codex/LaTeX/CURRENT.json\`. Generation identifiers such as \`.generation-n3azv4es\` and legacy source stage names are build internals, **not** canonical book titles.
- Canonical long notes live in \`Continuous Notes/\` and teach across several books; \`Book Production/Seven-Book Curriculum/\` contains series policies, academic references, Book One sources and lab planning. \`Book Production/AllMight Codex/LaTeX/\` contains the publisher's production tools, genuine intermediate sources, AST/asset manifests, code generation and layout evidence.
- Read \`Book Production/Seven-Book Curriculum/SERIES_EDITORIAL_AND_CITATION_POLICY.md\` before updating bibliography, research figures, academic claims, copyright or publication identity.
- **Book One is read-only context when working on Book Two** unless the author explicitly asks to change Book One. Likewise, when editing any book, do not silently touch another volume, its manuscript or cover.
- Historical \`Book Production/AllMight Codex/LaTeX/Books/Ebooks/\` includes all seven **older** generated stage EPUBs and even a duplicate Book Six export; do not mistake these for approved releases or delete tracked evidence without verifying consumers.
- For Book Two (as of **2026-11-20**), a refined full EPUB reader candidate was produced and structurally audited; the independently checked actual cover, 16 chapters, 10,432 MathML nodes, 128 exercises, index and source companion are present. Independent on-device reader QA and the one-volume PDF remain release gates. Local Book Two build entrypoint: \`Book Production/AllMight Codex/LaTeX/Tools/book02_release_candidates.py --epub-only --cover-image "<author JPEG>"\`. This entrypoint depends on the author's original Mac source cache. Do not claim CI's smaller additive proof EPUB equals the entire 16-chapter book.
- For Book One, prior ebook/PDF artifacts and proof records exist, but confirm the chosen **specific complete artifact** independently before promoting it. Books 03–07 have historic exports, not automatically final reader editions.

## Safety rules for repository cleanup

**Never mass-delete or move**: \`Continuous Notes/\`, \`CURRENT.json\`, \`physical-parts.json\`, \`Generations/\`, \`.generation-*/\`, \`Assets/\`, \`AST/\`, conversion manifests, \`Book Production/Seven-Book Curriculum/\`, Book01 source/lab directories, chapter source, original images/fonts, project/lab tests, C++23 sources, scientific evidence or an active build's working output. Some apparently old folders are necessary reproducibility/provenance inputs.

The local Master working tree may contain unrelated or uncommitted changes (previously, \`.DS_Store\` and a Book One continuous note). **Do not reset, clean, stash, overwrite, or discard user changes** to make a pull or checkout work. Read \`git status\` first. A file being named "old" or "duplicate" does not establish that it is safe to delete.

1. Inventory first with \`tools/publication_library.py scan\`.
2. Promote only a **complete named EPUB/PDF chosen explicitly** (never guess among candidates) using \`promote --book N --epub PATH\` and/or \`--pdf PATH\`.
3. Run \`verify\`, inspect SHA-256 and release manifest, and check the book in its reader.
4. Use \`tidy\` only to **quarantine exact-byte duplicate, untracked, non-build review exports** after a dry run. Never sweep the source tree or delete files based on names alone.
5. Historical release files remain in \`Archive/\` until explicitly accepted for later deletion; default is **no permanent deletion**.

## Session protocol for future AI agents

Start by identifying **which of the seven books** is active, the requested artifact (EPUB, PDF, chapter, code, research), the current branch and source revision, existing QA, the actual author cover asset and whether it is a candidate or an accepted release. Read the relevant original source rather than relying only on a generated file tree, summary or index. Give an explicit change plan for substantial work; execute tool-backed changes; test and report **what actually passed**. Link to exact generated files or commit SHAs. For Mac-only builds, supply short, self-contained **zsh-safe commands** with quoted paths, \`set -e\`, no interactive \`#\` comments, and no assumptions that uploads appear magically in Downloads or Master. Never instruct the author to run a command until its input/output paths and dependencies are grounded.

**North star:** seven definitive, narratively coherent, scientifically defensible, exceptionally readable and beautifully typeset technical books that take a motivated reader from the bit-level machine to original, reproducible research.
