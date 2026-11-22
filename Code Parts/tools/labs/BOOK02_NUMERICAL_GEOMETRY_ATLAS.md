# Book Two — Numerical Geometry Atlas

**Volume:** *From Equations to Geometry*, book two of *From Bits to Research*  
**Stage:** Pass 3 of 4 — original figures, source-driven numerical evidence, and cumulative project  
**Role:** executable companion to existing book chapters, not replacement for them.

The atlas is a small, independently authored scientific-computing studio. It begins with methods derived or reconstructed in the earlier Book Two source laboratories: exact orientation on represented binary64 values, consensus fitting, finite-element surface operators and a geodesic heat method, damped Huber nonlinear optimization, and importance sampling. It calculates genuine data, saves reproducible CSV observations, and produces publication-oriented PDF, scalable SVG and preview PNG figures.

It also creates an **offline standalone interactive HTML atlas** from those exact CSV tables. Unlike a screenshot-only presentation, the viewer lets a reader rotate a computed three-dimensional scalar surface, switch between heat, estimated distance, analytical distance and signed-error fields, alter the RANSAC inlier threshold *against a fixed recorded fit*, compare robust fitting variants, inspect sampling error over equal integrand-evaluation budgets, and inspect the orientation filter's transition to exact fallback.

The viewer is a **data explorer**; its controls must not be misconstrued as re-running all research algorithms. Recompute and regenerate the export for a new numerical experiment. The HTML contains no remote JavaScript, external CDN, analytics, or API dependency.

## Execution — when preparing the book

On an environment running Python 3.12 or compatible:

```bash
python3 -m pip install -r tools/labs/requirements-book02-atlas.txt

python3 tools/labs/book02_visual_atlas.py \
  --output output/book02-numerical-atlas

python3 -m unittest -v \
  tools/labs/test_book02_visual_atlas.py
```

The working book pipeline calls `install_book02_visuals(..., include_rich_atlas=True)` in both physical-book and logical-stage builds. It therefore generates the same plots before LuaLaTeX, using the **same Python interpreter** as the LaTeX build script. The book PDF pipeline also requires the production environment's existing `pypdf`, LuaLaTeX and `allmight-book.sty`. The other six books are not modified by the integration hook.

If source content or numerical dependencies are missing, an explicit error is raised rather than placing an empty graphic in the book.

### Outputs

The generated atlas folder contains:

- `figures/*.pdf` — high-resolution vector figure sources for the actual Book Two LaTeX print.
- `figures/*.svg` — scalable digital figures; usable as semantic EPUB image assets after the final stage-specific conversion/figure-caption integration.
- `figures/*.png` — preview/reader-friendly figure variants.
- `data/*.csv` — the exact numerical observations used in each plot.
- `manifest.json` — numerical and plotting versions, data SHA256 fingerprints, scene parameters, figure meanings, units, sample seeds and inference limitations.
- `Numerical-Geometry-Atlas.html` — standalone, no-network interactive numerical companion, containing embedded data from the same CSV exports.

The GitHub Action `.github/workflows/book02-visual-atlas-pass03.yml` runs all figure tests and creates a downloadable artifact containing the above.

## Seven original scientific plates

| Figure name | Mathematical construction | Validation and interpretation |
|---|---|---|
| `orient2d-certification` | Exact-sign orientation over represented binary64 near-collinear inputs | Filter vs rational fallback vs input rounded away, not runtime |
| `ransac-consensus` | 80 synthetic near-line observations and 30 declared structured outliers | Seeded consensus/refit vs all-observation TLS; entire residual population shown |
| `geodesic-fields` | Triangular FEM stiffness, lumped mass, implicit heat step and Poisson reconstruction | Numerical heat and distance plus signed error against Euclidean flat-domain oracle |
| `geodesic-surface-3d` | Three-dimensional lifting of the computed planar scalar field | Height = estimated distance, not physical elevation; signed error on same mesh |
| `geodesic-resolution` | Mesh-resolution sweep, constant physical domain and source | Observed mean/median/max absolute errors, not promised convergence order or speedup |
| `robust-fitting` | Generated exponential curve with one gross outlier; ordinary and robust loss fits | Custom Huber/LM-like vs SciPy Huber and original generating curve; all residuals retained |
| `mis-variance` | Monte Carlo estimation of the exact integral e−1 with three samplers | 90 independently seeded replicates per equal evaluation budget, signed error and observed variance |

All numerical examples are educational and narrowly scoped; **none** is described as a complete reproduction of an original paper's published benchmark figures or performance claims.

## How to develop this capstone as a reader

Implement the maths independently before copying any API call. For orientation, begin with determinants and an exact-sign oracle; then make the floating-point filter refuse uncertain results. For RANSAC, differentiate the success probability idealization from the actual observed consensus and show how changing threshold affects false acceptance. For discrete differential geometry, derive the P1 basis gradients and energy first, then measure matrix invariants, boundary effects and flat-domain geodesic error. For nonlinear fitting, derive the Jacobian and robust objective; verify derivative correctness before running the optimizer. For sampling, derive the balance estimator's expectation under the correct measure, and only then compare replicate variance at equal cost.

A checkpoint is not accepted until tests, data exports, visualization labels, source assumptions and a deliberate failure case are present. Visual appeal is not a substitute for correctness, but readable figures are an integral part of good scientific communication.

### Production and publication boundaries

The seven rich figures are integrated into the **print-oriented** Book Two composition by a build-copy-only injection. The underlying original 16 chapter TeX sources remain unchanged. The new final chapter-end project text is also additive and source-referenced.

The **semantic EPUB pipeline remains an explicit Pass 4 item.** It is currently driven by a separate frozen Pandoc AST rather than the working LaTeX source. Copying PDFs into that EPUB process or assuming LaTeX modifications automatically propagate would be incorrect. Pass 4 must map each figure and supplementary lesson to the corresponding canonical chapter content, use accessible image descriptions, verify reflowable figure sizing and reading order, and run EPUBCheck.

The final Pass 4 also owns complete page-fit checks at actual trim size, font/line breaking, references/citations, external paper-claim verification, production project instructions and universal cross-figure labeling. Where colour is essential, include a figure legend and a label or an alternative accessible signal.

## Scientific source limitations

The foundational paper and code references are documented in the [Pass 2 source ledger](../../Supplementary/Book%2002%20-%20Pass%2002%20Primary%20Source%20and%20Implementation%20Evidence%20Ledger.md). These implementations are inspired by Shewchuk's predicate work, Fischler and Bolles' RANSAC, Crane/Weischedel/Wardetzky's heat method, and Veach's multiple-importance-sampling framework. They are **not** ports of complete author implementations. Synthetic data are generated explicitly; the viewer does not make measurements of CPU, GPU, real surfaces or physical instruments.

The quality bar is a scientifically useful distinction between something that looks correct, something that has passed a numerical oracle, and something actually reproduced under the conditions of a research publication.
