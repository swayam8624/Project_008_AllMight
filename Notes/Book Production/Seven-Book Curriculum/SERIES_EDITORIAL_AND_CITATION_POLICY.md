# From Bits to Research — series identity and citation policy

## Reader-facing identity

**Series:** *From Bits to Research*. **Scope:** Computing, Mathematics, Graphics, Simulation, and Scientific Discovery. **Author:** Swayam Singal. **Architecture:** seven integrated teaching books with projects, proofs, experiments and visually communicated results. This document governs future authoring; it does not assert that seven finished, verified books already exist.

The first book's full title is *From Bits to Parallel Systems: C++23, CPU Execution, GPU Memory, and Reliable Software*. Its two print halves, *Volume I — From Bits to Programs* and *Volume II — From Programs to GPUs*, are **physical print volumes inside Book One**, not additional books in the curriculum.

Every book's title pages should present the series first, then its book number and specific title/subtitle, then the author. A restrained copyright page should contain copyright and rights, a direct repository/companion-code URL and a reference/corrections pointer. Do **not** print build hashes, source revisions, CI identifiers, working-proof labels, "no ISBN" notices or speculative publisher/imprint data there. Keep build identities in manifests or reproducibility supplements. Add an ISBN or imprint only if actually obtained or assigned.

## The seven-book argument

1. **Bits, programs and reliable systems:** representations, C++23, lifetimes, DSA, CPUs, GPUs and testable software.
2. **Mathematics for visual computing:** analysis, geometry, numerical methods, probability, optimisation and uncertainty.
3. **Geometry, cameras and image formation:** robust shape, camera models, calibration, reconstruction and uncertainty.
4. **Light transport and image synthesis:** material/light models, intersections, acceleration structures, sampling, ray tracing and verification.
5. **Spatial worlds, engine state and simulation:** engine architecture, physics, contact, determinism and stable simulation.
6. **GPU architecture and real-time rendering:** Vulkan, Metal, render graphs, parallel GPU work, PBR, profiling and practical rendering.
7. **Vision, XR, research and visual evidence:** perception, interaction, neural/hybrid representations, experiments, research writing, replication, technical demos and visual communication.

The individual book titles may be editorially refined without flattening this teaching sequence. Concepts must be developed when the reader needs them, from the mechanism and evidence onward; avoid merely naming a topic and deferring its explanation. Projects accumulate through the books and should always have failure cases, executable oracles and a clear distinction between a teaching example and a validated research implementation.

## Citations and persistent identifiers

Use **DOIs for works that have one** (usually papers and some electronic books), linking the resolver as `https://doi.org/<DOI>`. For printed books, record the actual edition, year, publisher and verified ISBN; a book does not automatically have a DOI. For a standard, cite the issuing body, identifier and edition/revision, plus its official URL. Versioned web documentation needs a product/API version and an access/check date when version drift could change the meaning.

A bibliographic entry is not enough for a specific borrowed claim. Place the author/year or a stable citation key beside the relevant derived result in the chapter, link it to the complete entry, and distinguish **normative specifications**, **peer-reviewed articles**, **secondary tutorials**, **vendor guidance**, **software source**, **measurements**, **original explanations**, and **adapted/reproduced figures**. Prefer original publications over commentary when establishing novelty or correctness. Never fabricate a DOI, ISBN, revision, page number, claim of reproduction or repository address.

Source code gets a working clickable repository link and, when asserting an exact result, an immutable release/tag/commit link, inputs and reproduction commands. A mutable `main` branch is a navigation link, not a source-identity citation. The Book 1 example library lives in `Book01/ACADEMIC_BACKMATTER.md`. Update later books' reference catalogues only after checking each source.

## Visual research is part of the curriculum, not decoration

Teach readers to produce and audit:
- explanatory figures and algorithm/data-flow diagrams (clear labels, visually distinct data paths, scale and uncertainty);
- scientific plots from reproducible scripts and recorded raw data (units, aggregation, intervals and negative results);
- 3D experiments, renders and camera/light choices (geometry, orientation, normalisation and perceptual intent);
- comparative animations and visual grids (controlled seeds, matched camera, consistent colour/lighting/geometry, measured and simulated data clearly separated);
- paper figures, slides, posters and interactive demos (legibility at print size, captions, accessibility and verifiable provenance).

**Figure evidence gate:** state the scientific question, input/source, method and parameter set, what is observed versus simulated, the independent reference/oracle, visual comparison controls, uncertainty, and a reproducible path to regenerate the figure. High-quality visual design is welcome; visual artefacts must not conceal intersections, temporal artefacts, unfair baselines or missing experiments. Never call an attractive render an independent correctness proof.

For external figures, check the applicable licence/permission and provide source attribution in the caption. If redrawing an external result, label it *adapted from* with a proper citation. Generated or computer-synthesised graphics must be described accurately when that matters scientifically.

## Editorial and preservation constraints

Keep canonical teaching text intact. Correct a proven factual, mathematical or technical defect explicitly; otherwise add deeper worked explanations, derived equations, independently checkable examples, experiments, and appropriately sourced references *in context*. Distinguish proposed future chapters from already released books. Do not replace a thousand-page original with a short export, a teaser or a reconstructed summary.

Book 1 generation currently lives in `tools/build_book01_expanded_reader.py` and its reader design in `Book01/book01-reader.sty`. This series document is a policy for later books, not a trigger to regenerate their manuscripts. Native LaTeX/PDF builds are performed by the author on the requested platform.
