# Book 03 — The reader, the original sources, and the release promise

## The reader to whom we are speaking

Imagine someone looking at a game screenshot, asking “But how did *that* appear on my screen?” They may not know what a view matrix is. They might be twelve, fifty, or a six-year-old who wants to know why a far-away tree looks smaller. The prose must always give them a door into the idea. It must not pretend difficult mathematics is easy by omitting it. Start with a concrete question and build patiently toward real mathematics and real code.

**Accessible does not mean childish.** We avoid jargon at first use; later we use precise terms. Introduce new symbols next to their real meaning and units. Explain why a programmer would care. Build a single evolving example across several pages so the reader can watch their mental model change.

## Every chapter's acceptance gates

1. **A question worth answering.** A real-world image, model or failure starts the story.
2. **An honest mental model.** Define what is continuous, what is discrete, what is measured, what is stored and what is only an idealization.
3. **A derivation that can be checked.** Dimensions, coordinate conventions, domains, assumptions and counterexamples are explicit.
4. **A worked example with exact numbers.** The reader can reproduce the answer without “trust the library.”
5. **C++23 that teaches.** Named structures, comments explaining ownership/invariants and readable APIs. Standard C++ first. Build/debug controls must not masquerade as a real renderer.
6. **Evidence.** CMake/CTest, deterministic fixtures, malformed input tests and independent references when appropriate. A screenshot is not a correctness proof.
7. **An illustration plan.** State exactly what the diagram reveals, what its labels mean, how generated data can be reproduced and how the figure should read in grayscale and on a phone.
8. **Exercises with response criteria.** Derive, implement, break, explain, extend. Provide useful answers or testable rubrics; do not hide all understanding in solution code.
9. **A natural handoff.** The last idea should create a question that the next chapter genuinely resolves.
10. **Source custody.** Existing explanations and diagrams remain traceable. Never shorten the original manuscript silently.

## Book-specific conventions

- Math vectors are columns unless a chapter explicitly studies another convention.
- Distinguish a point (a place) from a vector (a displacement) and a pixel index (an integer address) from a sample position (a continuous location).
- Define every spatial frame, handedness, unit, matrix multiplication order and storage layout separately. None can be guessed from an API name.
- Chapter 01 uses a top-left image origin, x rightward, y downward, and integer pixel indices with centers at (x+1/2, y+1/2). The chapter's RGB values are *encoded channel values*, not radiometrically linear light.
- Debug views have provenance; vector diagrams must use accurate geometry. Attractive images cannot substitute for reference tests.
- Reserve full BSDFs, path-tracing convergence, and transport for Book 4; game physics for Book 5; production GPU implementation for Book 6; neural reconstruction and XR for Book 7. Book 3 still develops interfaces and first principles needed by these subjects.

## Exact layout and file custody

The November 22 repository refactor is authoritative: **Notes/** for source manuscripts, **Code Parts/** for programs, **From Bits to Research/** for final exports. Do not reinstate the previous top-level “Book Production” or “From Bits to Research - Publications” directory structure.

The original three continuous masters stay in Notes/Continuous Notes/. Their generated copies in Notes/Chapter Edition/ stay source-derived. The existing LaTeX generation, including Notes/Book Production/AllMight Codex/LaTeX/CURRENT.json, all .generation-* snapshots, ASTs, reference notes, source figures and manifests, is read-only until a separately verified composition step. Where material needs refinement, first draft a new chapter in the Book03 working source and maintain a source-routing record; integrate by exact anchors or controlled composition, never by destructive overwrites.

No change to releases for Books 1 or 2. No new EPUB/PDF in pass 01. The author will run the final macOS publication compilation only after the whole book is completed.

## Storytelling constraints

A friendly voice has agency. Use analogies with limits. A pixel can be compared to a tiny addressable tile, but the tile is not literally a camera's patch of solid color. An “eye” analogy is useful for camera pose, but eyes and pinhole cameras are not identical. The playful paragraph earns its place when it makes the math or the bug more understandable.

Do not start with 32 summaries and call that a book. Do not copy a playlist transcript, assume a third-party license, present an invented citation or claim that code was tested when it was merely written. Long paragraphs are welcome when they actually develop a thought; cut redundancy rather than depth.

## Release evidence, later

The final reader builds will need source-preservation receipts, math/code checks, working diagrams and captions, consistent notation, legitimate references, EPUBCheck on exact EPUB bytes, page-by-page PDF review, and direct EPUB reader QA. A proof page, old seven-stage EPUB or standalone chapter cannot count as the complete Book 03 edition.
