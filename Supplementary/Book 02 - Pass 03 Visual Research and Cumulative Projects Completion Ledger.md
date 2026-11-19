# Book Two — Pass 03 Visual Research and Cumulative Projects Completion Ledger

**Series:** From Bits to Research  
**Volume:** Book Two, *From Equations to Geometry*  
**Pass:** 3 of 4 — completed for the defined original scientific-figures and cumulative-project scope  
**Date:** 2026-11-19  
**Branch:** `curriculum/book02-mathematics-refinement-pass01`  
**Acceptance:** Successful exact source-code visual generator/test run, full original-chapter preservation regression, research lab parity tests, independently inspected image proof and browser-interactive companion. This is *not* the final Book Two publication acceptance.

## 1. Scientific asset deliverable

The new `tools/labs/book02_visual_atlas.py` generator performs **real mathematical and numerical calculations**, saves the original observations, renders seven print-vector and digital figures, and writes a manifest with model/source/units/method/seed/limitations.

| Figure slug | Source experiment | Data rows |
|---|---|---:|
| `orient2d-certification` | Exact-sign binary64 orientation and conservative filtered fallback | 52 |
| `ransac-consensus` | 80 synthetic inliers + 30 synthetic structured outliers; RANSAC/TLS comparison | 110 |
| `geodesic-fields` | Computed finite-element heat, distance, signed error | 625 |
| `geodesic-surface-3d` | 3D scalar height view of computed distance and mesh error | 625 |
| `geodesic-resolution` | Six planar mesh refinements with recorded errors | 6 |
| `robust-fitting` | Synthetic nonlinear curve with a gross outlier, Huber/SciPy comparison | 35 |
| `mis-variance` | 90 seeded replicate draws for three samplers and five equal budgets | 1,350 |
| **Total** | **Seven independent figures** | **2,803 numerical data rows** |

Every figure exports `PDF`, `SVG`, and `PNG`, a `CSV` with the exact plotted numerical rows, and a manifest entry with data SHA256, assumptions and meaning. The 7 figures are reproducible via `python tools/labs/book02_visual_atlas.py --output <destination>`.

The new `tools/labs/book02_atlas_viewer_template.html` + `book02_build_viewer.py` produces an **offline standalone HTML application** containing the same CSV observations, rather than a separately simulated display dataset. Its six working interactive views cover a rotating 3D numerical scalar surface, field/heat/error toggles, threshold-based membership against a fixed RANSAC model, ordinary/robust fit comparison, Monte Carlo equal-budget distribution selection, and near-collinear binary64 predicate inspection. No external JS, CDN, account, analytics, or network dependency is built into it.

The interactive viewer does not secretly recompute an RANSAC fit when a threshold slider is moved; the scope is clearly disclosed. The 3D surface height represents computed geodesic distance, never real-world terrain.

## 2. Exact book integration and preservation

Seven new figure-lesson source fragments are placed beside the existing relevant chapters, preserving the original text:

| New supplementary TeX | Attached original source chapter |
|---|---|
| `28-atlas-orientation.tex` | `01-foundations-i-ch018` |
| `29-atlas-geodesic.tex` | `02-foundations-ii-ch025` |
| `30-atlas-geodesic-3d.tex` | `02-foundations-ii-ch025` |
| `31-atlas-resolution.tex` | `02-foundations-ii-ch025` |
| `32-atlas-fitting.tex` | `02-foundations-ii-ch027` |
| `33-atlas-sampling.tex` | `02-foundations-ii-ch030` |
| `34-atlas-ransac.tex` | `02-foundations-ii-ch031` |

Additionally, `35-cumulative-numerical-geometry-atlas.tex` is a substantive reader capstone following the final original source chapter. It teaches source and result provenance, geometric predicates, robust fitting, finite-element stiffness/mass, geodesic fields, error/refinement studies, objective comparison, sampling and uncertainty, scientific figure ethics and an explicit acceptance contract.

The `LaTeX/Tools/book02_visuals.py` working-copy-only insertion supports `include_rich_atlas=True` for print, while the earlier lightweight contract tests remain compatible via the default `False`. `LaTeX/Tools/build.py` now opts in to rich source-generated figures for the Book Two stage and both physical parts. The hook invokes the same Python version running the build script, and refuses an incomplete generated figure file rather than silently replacing it with a placeholder. Other books are unchanged by the Book Two stage marker.

The generated PDFs are not committed as opaque binary blobs; they are authored on demand from the versioned mathematical source, and a complete validated ZIP is additionally attached to the GitHub Actions visual workflow as a reproducible deliverable.

## 3. Independent verification evidence

The three relevant GitHub Actions workflows all succeeded on the full source branch after the cumulative project was attached:

- [Book Two Mathematical Pass One Contracts, run 44591579330](https://github.com/swayam8624/Project_008_AllMight/actions/runs/44591579330) — original 16 chapter-source inputs and additive placement contract.
- [Book Two Research Reproduction and Numerical Parity, run 44591579395](https://github.com/swayam8624/Project_008_AllMight/actions/runs/44591579395) — research methods, SciPy numerical comparison and C++23 signed/matrix contracts.
- [Book Two Visual Research Atlas, run 44591579360](https://github.com/swayam8624/Project_008_AllMight/actions/runs/44591579360) — the complete seven-figure source generator, numerical source CSV checks, image format validity, offline HTML embed, no-network declaration, JavaScript syntax and artifact export.

The first completed full viewer export was independently inspected using a local headless Chromium browser with the viewer HTML supplied directly as content (because direct `file://` navigation was restricted by the automated browser sandbox). Six study mode buttons were present, metric widgets updated correctly in every examined tab, and no JavaScript page errors were raised. This does **not** replace testing on every EPUB reader or mobile browser.

A seven-page consolidated figure PDF was generated from the resulting vector figure PDFs and rendered for visual inspection in the assistant's working environment. One observed print-readability issue (overcrowded log-axis tick labels in MIS budget plot) was corrected in source before the final successful export.

**No original paper benchmark time or GPU improvement was invented.**

## 4. Explicit limitation and Pass 4 handoff

Pass 3 is **complete for the defined scientific-figure and cumulative project delivery scope**, but it does not mean the full book is publication-complete:

- A complete 16-chapter LuaLaTeX page-by-page proof on the author's pinned macOS environment is not yet accepted. Scientific figure PDFs are independently valid, and their insertion path is tested; full trim-size captions, placement and page breaks remain in Pass 4.
- The native EPUB generator currently uses a separate frozen Pandoc AST, not this additive TeX working-copy injection. These new figures and capstone must still be semantically included in the ebook's content spine, with figure alt-text, reading order and full EPUBCheck validation. The SVG and PNG sources are ready for that work.
- Complete original-text mathematical proof/derivation correctness, external source-paper claims, citation licensing and entire-book editorial narrative must receive final independent Pass 4 checking. Previous selective review is not full coverage.
- The KairoMath and KairoPhysicsMath repositories remain pedagogical source-case studies until exact-head native full compiler/test acceptance; their original status declarations should not be overridden by this atlas's separate successful numerical tests.

**Next and final pass:** Book Two Pass 4 — full publication-level evidence, mathematical source review, cross-chapter continuity, PDF/EPUB semantic parity and user-side compile readiness. Preserve all original substantive material unless a demonstrated error requires correction.

No Mac command execution is required now; code, tests, figures and reports are on the Book Two development branch.
