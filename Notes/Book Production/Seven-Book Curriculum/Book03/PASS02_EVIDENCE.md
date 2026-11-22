# Book 03 — Pass 02 evidence: Every Place Needs an Address

**Series:** From Bits to Research  
**Book:** 03 — From Geometry to Images  
**Branch:** curriculum/book03-geometry-images-pass01  
**Date:** 23 November 2026  
**Release state:** manuscripts/code authored; NO Book 03 PDF/EPUB build, release or publication promotion.

## 1. Authored work

Chapter 02, \`Chapters/02 - Every Place Needs an Address.md\`, is a long-form, reader-first chapter of roughly 5,500 words. It uses the same tiny house introduced in Chapter 01 and advances the scene's mathematical identity from raw image addresses to local/world points. Each concept is introduced with a human-readable problem, then a precise convention, a derivation, a reproducible numeric example, a test or a deliberate counterexample, exercises and a story bridge to the next chapter.

The technical route includes:

- Points versus displacement vectors; why point+point is not an automatically meaningful operation; an explicit affine midpoint example.
- A chosen world origin, axes, units and a changed coordinate-frame description.
- A 90-degree rotated local frame: local (3,1) → world (1,4), with explicit basis columns and origin (2,1).
- Affine transformation \(p_W=o_W+A p_L\) versus displacement transformation \(d_W=A d_L\).
- An inverse derived from the determinant; a nonorthogonal sheared basis whose inverse is not its transpose; singular and near-singular frames.
- World X right/Y up versus continuous image u right/v down, with explicit positive scale in pixels/metre.
- An exact worked world (1,2) metre → continuous (380.5,120.5) pixel → discrete pixel index (380,120) example.
- Parent-child frame composition, transformation order, the first homogeneous-coordinate bridge and distinct operations for points/vectors.
- C++23 sample code, test criteria, deliberate broken cases, detailed worked problems, glossary and source-custody notes.

## 2. New original diagrams

**Assets/03-two-frames-one-place.svg:** mathematically grounded local/world transform with the local origin and axes and the exact (3,1)→(1,4) worked example. The schematic is intended for print/ebook export only after layout proof and accessibility review.

**Assets/04-world-to-image-frame.svg:** contrast of upward Y in world coordinates with downward v in image coordinates; the labeled 60 pixels-per-metre example shows that increasing world Y decreases image v.

Both source SVGs have a meaningful title, description and vector labels. Neither is represented as measured empirical data; they are explanatory geometric constructions.

## 3. New C++23 project material

**P02 sources** under \`Code Parts/tools/labs/book03_cpp23/\`:

- \`include/book03/frame_math.hpp\` — Point2 versus Vector2, a validated Frame2 with affine forward/inverse and vector transforms, Frame2 composition, and ImageMap2 with continuous coordinate / discrete pixel separation.
- \`src/frame_playground.cpp\` — a portable terminal REPL with origin, angle, local point, inspection and SVG output; numerical calculations use frame_math.hpp rather than a separate diagram formula.
- \`tests/frame_math_tests.cpp\` — compile-time point-plus-point rejection, forward and inverse quarter-turn, nonorthogonal basis, singular/near-singular cases, composition order, translation-invariant vector direction, world-up/image-down sample mapping, image edge conditions, nonfinite and overflow paths.
- \`CMakeLists.txt\` — additional book03_frames and book03_frame_tests targets, sharing the existing C++23-only laboratory.
- \`editorial_gate.py\` — checks two long original chapters, all four authored SVGs, code locations, link placements, original source references and 32 planned chapter identifiers.
- \`.github/workflows/book03-cpp23-pixel-canvas.yml\` — expands the existing CI definitions with frame tests, source-produced SVG inspection and a reproducible REPL fixture.

**Type-design correction:** The initial P02 implementation briefly called its primitive WorldPoint2 even when it held local coordinates. It was renamed to Point2 (and WorldVector2 to Vector2). The types enforce point-versus-vector semantics, while coordinate-frame identity is still specified at the API boundary. This is an explicit limitation, not a dishonest claim of compile-time tagged frames.

## 4. Grounded original-source integration

\`PASS02_LEGACY_AUDIT.md\` identifies nine pre-existing source chapters at Git blob SHAs, their original source-span identifiers and the planned Book 03 reader destination. It confirms that the original source distinguishes right-handed 3D view-space -Z, world-to-camera extrinsics, homogeneous clipping, optical exposure and mesh topology. **The new Chapter 02 is a planar affine on-ramp, not a replacement for any of those full 3D derivations.**

Original long continuous manuscripts and original LaTeX Stage 03 generation were not edited. A later source-unit-level audit, source-preserving composition and author review are required.

## 5. Verification state

**Verified by direct GitHub comparison** at the Pass 02 checkpoint: the branch is ahead of the unchanged main branch and every changed path is inside the dedicated Book03 notes, book03_cpp23 code or Book 03 workflow; no pre-existing source, other book, publication file or repository-wide directory structure has been modified.

**Authored, not yet reported as passing:** C++23 compile/CTest, CLI and SVG generation, editor source check and GitHub Actions results. The workflow exists, but a passed run cannot be inferred from its source. The final compiler/test outputs should be inspected separately before claiming a green native test result.

**Not run or claimed:** Mac final build, LuaLaTeX complete reader, EPUB composition/EPUBCheck, KDP preview, PDF page-by-page QA, or public book release. The author will perform the Mac full-book compile after the entire book has been developed.

## 6. Next logical round — Chapter 03

**Shapes That Know Their Borders:** Build out triangle orientation, signed area and barycentric geometry with a robust planar inspector. Connect the first visual hit tests to the previously published Book 02 math without removing or copying its source wholesale. Preserve the original long-source predicates and mesh chapters and make the new C++23 triangle lab provide an independently checkable exact oracle, deliberate collinearity failures, and generated figures. Keep the current Notes/, Code Parts/ and From Bits to Research/ directory contract unchanged.
