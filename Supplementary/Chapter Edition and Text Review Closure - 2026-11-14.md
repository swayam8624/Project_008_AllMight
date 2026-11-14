# Chapter Edition and Text Review Closure — 14 November 2026

The [[Chapter Edition/README|new chapter edition]] copies the three continuous masters into substantial reading chapters. The original masters remain intact. Each chapter retains its complete contiguous source span, including equations, code and figures, and adds an opening bridge, goal, recall prompt and navigation.

## Chapter inventory

| Book | Major chapters | Estimated page range |
|---|---:|---:|
| [[Chapter Edition/01 - Computing Foundations/00 - Reading Map|Computing Foundations - From Bits to C++ and Systems APIs]] | 39 | 40.7–65.3 |
| [[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|University - Worlds, Images and Inference - Graphics, Vision and Photography]] | 43 | 32.1–69.2 |
| [[Chapter Edition/03 - Game Engine/00 - Reading Map|Game Engine - From First Frame to Living Worlds]] | 47 | 37.7–63.6 |

These are reading estimates, not rendered PDF pages. The model allows 400 prose words, 48 wrapped code lines or 36 equation/table lines per page, with figure and spacing allowances. Source ledgers are retained as explicitly named atlas/reference chapters; the repeated XR walkthrough is retained and labeled. The edition shares Concepts, Supplementary and image assets with the surrounding vault.

## Review closure and evidence boundary

The final outstanding review pass closed **1,230 claim records across 66 sources**. The receipts contain **1,036 cited source/unit pairs** and **114 destination paragraph mappings**. Exact original claim dictionaries, cited-unit ownership, full retained-text hashes, exact quotation anchors and destination paragraph hashes were checked. The retained extracted-claim ledger now records **29,715 reviewed and zero pending**, across 590 extracted records.

This closes the recorded extracted-claim text backlog. It does not certify every original audio/video frame, every uncited page, every research result, or a working implementation of each idea. The separate acquisition queue still contains 103 open records: 95 queued book ranges, four failed book ranges, two paused book ranges, one book range requiring visual review and one queued video. Captions, raw sources and acquisition scripts remain retained. No acquisition was restarted in this pass.

## Teaching additions and supervision

The pass added focused explanations where the source comparison found a genuine gap. Existing deep treatments were preserved rather than padded with repeated research references.

- [[Continuous Notes/Game Engine - From First Frame to Living Worlds#A thumbnail cache is derived render output with explicit invalidation|A thumbnail cache is derived render output with explicit invalidation]]
- [[Continuous Notes/Game Engine - From First Frame to Living Worlds#A staged explosion is a timed game force field, not a blast simulation|A staged explosion is a timed game force field, not a blast simulation]]
- [[Continuous Notes/Game Engine - From First Frame to Living Worlds#Mesh-local geometry and entity transforms must agree|Mesh-local geometry and entity transforms must agree]]
- [[Continuous Notes/Game Engine - From First Frame to Living Worlds#A particle aggregate is a useful model for connected, flexible geometry|A particle aggregate is a useful model for connected, flexible geometry]]
- [[Continuous Notes/Game Engine - From First Frame to Living Worlds#Rules are shared agreements that make play possible|Rules are shared agreements that make play possible]]
- [[Continuous Notes/Game Engine - From First Frame to Living Worlds#Design by connecting play, technology, and story|Design by connecting play, technology, and story]]
- [[Continuous Notes/Game Engine - From First Frame to Living Worlds#Particle cables and rods are distance constraints represented as contacts|Particle cables and rods are distance constraints represented as contacts]]
- [[Continuous Notes/Game Engine - From First Frame to Living Worlds#What changes when the solver works directly on predicted positions|What changes when the solver works directly on predicted positions]]

Supervision corrected missing LaTeX escapes/control characters before integration, distinguished homogeneous mesh points from ordinary vectors, and checked the numeric examples. In the cable/rod material, the book’s prose recommends two opposing contacts while its displayed rod function emits at most one. The notes explicitly identify that discrepancy. A cable at exact target length can need a velocity response despite zero positional error. The explosion example distinguishes a stationary-radius front-width bound from moving-body relative motion and labels its numeric impulse as a held-force integration example.

## Targeted research

The new position-method comparison uses the original [PBD publication](https://doi.org/10.2312/PE/vriphys/vriphys06/071-080) and the authors’ [XPBD paper](https://mmacklin.com/xpbd.pdf). The thumbnail discussion uses the author’s [Dirty Flag chapter](https://github.com/munificent/game-programming-patterns/blob/master/book/dirty-flag.markdown) alongside the retained Hazel walkthrough. The [pbrt GPU mapping treatment](https://www.pbr-book.org/4ed/Wavefront_Rendering_on_GPUs/Mapping_Path_Tracing_to_the_GPU) was consulted as a comparison; wavefront staging is already developed in the graphics master, so it did not require another duplicate lesson. An official Cornell course page was found during discovery; no new lecture-caption review is claimed.

## Reproducibility

`tools/chapter_plans/` stores the reviewed chapter titles, semantic anchors and teaching bridges. `python3 tools/build_chapter_edition.py` regenerates the edition; `--check` checks it without writing. `Chapter Edition/MANIFEST.json` records full-master, source-span, rendered-body and generated-file hashes. The builder verifies exact source-span reconstruction and every fenced code/diagram body, retargets inter-book note links, rebases ordinary Markdown links, and validates local Markdown files. Opaque pasted citation markers are preserved as HTML comments on separate lines so they do not attach to display-math delimiters; bare `operator[](i)` is formatted as inline code.

Executed successfully in the active vault:

- `python3 -m unittest discover -s tools/tests -p 'test_*integrity.py'` — 10 tests passed.
- `python3 -m unittest discover -s tools/tests -p test_chapter_edition.py` — seven tests passed.
- `python3 tools/build_chapter_edition.py --check` — all three books, 129 chapters and 134 generated files agree; 11,583,066 source bytes reconstructed.
- `python3 tools/check_vault_links.py --json` — 605 notes, 12,860 wiki targets and 318 asset targets; zero errors.
- `python3 tools/check_continuous_code_sync.py` — 26 declared complete programs agree with their saved source and comments. No new compilation claim is made for this copy-only pass.
- `python3 tools/check_three_volume_consolidation.py` — prior teaching and the documented XR relocation are preserved.
- `python3 tools/check_patch_idempotence.py` — all four retained authoring patches leave the books unchanged on both isolated passes.
- `git diff --check` — passed for the tracked diff. New chapter bodies retain original source whitespace by design.

The builder separately checks ordinary local Markdown targets and preserves all **5,950 fenced code/diagram bodies**. New numerical traces were checked: the explosion example gives weight 0.5, force 50 N, sampled impulse 0.5 N·s and velocity increment 0.1 m/s; the inverse-mass cable/rod examples restore the stated target distance. The XPBD zero-compliance formula reduces algebraically to the earlier hard distance correction. These are source/content/navigation checks; native Obsidian appearance, actual PDF pagination, every remote URL, video visuals, all historical implementations and live GPU behavior are not certified. No commit, push or pull request was made.

## Per-source review receipts

Each row accounts for this pass’s pending claim set, not an entire book or channel. Short quotation anchors are private review evidence, not new lecture text. Full receipts and backups remain under `.codex/source-cache/chapter-edition-2026-11-14/` outside the vault.

| Source | Reviewed claims | Cited source units |
|---|---:|---:|
| Where Hazel is at and what we've been up to // October 2025 Hazel Dev Log (`src-ecbe7152986514151345`) | 29 | 15 |
| Game Design Fundamentals Rules of Play — PDF pages 121–132 (`src-5d22cbaf981a8a95e080`) | 27 | 9 |
| Game Design Theory and Practice — PDF pages 61–72 (`src-145657dc89ebcae22142`) | 26 | 11 |
| Drawing Rotated Quads // Game Engine series (`src-0601c188833fefcd6e14`) | 25 | 23 |
| Kohi #010: Input Part 2 - Windows (Vulkan Game Engine Series) (`src-b3f9a75c265ea9f9dcb3`) | 23 | 22 |
| Game Design Fundamentals Rules of Play — PDF pages 1–12 (`src-8b1f5621b9c0f9fa4c48`) | 23 | 17 |
| Game Physics Engine Development — PDF pages 145–156 (`src-2ebd3580c0aca3b6c3b8`) | 22 | 11 |
| 3D Game Programming - Episode 17.5 - Creating an Applet (`src-34a5d655dfe4f02f5db1`) | 22 | 20 |
| 3D Game Programming - Episode 4 - Drawing Pixels! (`src-3154b768404aeff67156`) | 20 | 20 |
| The IMPORTANCE of DEBUG CODE \| Hazel Dev Log (`src-8cbea0b58ec5d5af5038`) | 20 | 20 |
| GAME ENGINE DEV REACTS to Cyberpunk 2077 — Official Trailer — The Gig (`src-fff5a938b021222cb6a1`) | 19 | 12 |
| Kohi # 028: A Triangle! A Quad! (Vulkan Game Engine Series) (`src-617956abe3f5a395a557`) | 18 | 17 |
| Game Physics Engine Development — PDF pages 229–240 (`src-208a7cf194874782f559`) | 17 | 12 |
| Game Physics Engine Development — PDF pages 457–468 (`src-e95761de798420ed3ee8`) | 17 | 12 |
| My Game Engine can now PLAY GAMES (`src-a4898f8442ba63ced1d9`) | 16 | 16 |
| Why I'm MAKING a Game Engine (`src-27420b1ec22543c6c70e`) | 15 | 15 |
| How We Made a Game in 72 HOURS using OUR OWN GAME ENGINE (`src-a4a444b59a9a500e028d`) | 15 | 15 |
| Game Physics Engine Development — PDF pages 265–276 (`src-9a899d22435e03b127df`) | 14 | 12 |
| My Game Engine can now SHOOT THINGS! (`src-13a6e79a4407f88fb954`) | 14 | 14 |
| 3D Game Programming - Episode 6 - Performance Boosting (`src-4565a0e2cbbe9be81105`) | 12 | 12 |
| Do you want to make GAMES or game ENGINES? (`src-0da6d52af46b8ede0ad0`) | 11 | 11 |
| Game Design Fundamentals Rules of Play — PDF pages 673–684 (`src-9ebb43dfed54002dd578`) | 7 | 12 |
| Game Design Theory and Practice — PDF pages 25–36 (`src-e05b82ca14bef31191c2`) | 29 | 12 |
| Game Physics Engine Development — PDF pages 61–72 (`src-570e5b4bc6aecc228423`) | 27 | 12 |
| Kohi #054: Transforms (Vulkan Game Engine Series) (`src-5eea4647098df7573f9c`) | 26 | 22 |
| Sparky Engine Dev Log #11 - Profiling and Planning (`src-e7bad6778eb70b791e88`) | 24 | 23 |
| Sparky Engine Dev Log #10 - Preparing for Release (`src-ebaf940c56470388d43c`) | 24 | 24 |
| 3D Physics! // Hazel Engine Dev Log (`src-e978102975089ee0d579`) | 23 | 23 |
| Game Physics Engine Development — PDF pages 121–132 (`src-73f31bfc40908cbbefbf`) | 22 | 11 |
| Fixing Skeletal Animation Image-Based Lighting in Hazel (`src-b04c18046ad50129e151`) | 21 | 15 |
| Making Games with Hazel + More Mesh Workflow // Game Engine Dev Log (`src-96d21d912ea1d5d27a7c`) | 21 | 21 |
| Is this TOO REALISTIC? Unreal Engine 5 UNRECORD Gameplay Trailer // Game Engine Dev Reacts (`src-bc321b12127c8f7ddb08`) | 19 | 19 |
| GAME ENGINE DEVELOPER Reacts to Ratchet & Clank: Rift Apart (`src-27495243d7b595fcd3d6`) | 19 | 19 |
| Why you NEED to use a GAME ENGINE! (`src-b4d920b7e0d5d82ec70d`) | 18 | 18 |
| LOOK AFTER YOURSELF \| Hazel Engine Dev Log (`src-abf646156c91b218d1a7`) | 18 | 18 |
| Game Physics Engine Development — PDF pages 97–108 (`src-7184228b619ff415a093`) | 16 | 11 |
| Game Physics Engine Development — PDF pages 157–168 (`src-ba88608cc67c48af8212`) | 16 | 10 |
| He MADE A GAME using my GAME ENGINE (`src-1640d3dd7f7d1bbf9e26`) | 16 | 16 |
| First-Person Camera Controller and Physics // Hazel Dev Log (`src-2b13f50934eb2aef6a0d`) | 15 | 15 |
| Precompiled Headers \| Game Engine series (`src-36a9f01c5c108c5e758f`) | 14 | 13 |
| GAME ENGINE DEV REACTS to Microsoft Flight Simulator - Pre-Order Launch Trailer (`src-73aba01471ea619f4f6d`) | 14 | 14 |
| Game Physics Engine Development — PDF pages 349–360 (`src-9a4dd3de146a3a879114`) | 12 | 12 |
| 3D Game Programming - Episode 2 - Game Loop (`src-7148546b075b19e4f904`) | 11 | 11 |
| Game Physics Engine Development — PDF pages 481–481 (`src-4b8786edded484a71c22`) | 1 | 1 |
| Hazel 2024.1 - Our New Biggest Release Ever (`src-e9521cb09a94cea9deb3`) | 28 | 28 |
| Game Engine Dev Plays Horror Game - Deppart Prototype (`src-3b5bd3f6bf8d1088fd12`) | 28 | 26 |
| Game Physics Engine Development — PDF pages 181–192 (`src-cb7c39e5d4ce1aa78d4b`) | 26 | 12 |
| Modern OpenGL (Glad) \| Game Engine series (`src-afab7cd024e9ba4020bd`) | 24 | 24 |
| The FUTURE of Hazel (+ Material Assets!) // Game Engine Dev Log (`src-2bfa5f4f1f97226669cb`) | 23 | 23 |
| Ep. 16: Batch Renderer Textures - Sparky Engine (How To Make a Game Engine) (`src-7d0770f5faf88bdddf77`) | 23 | 23 |
| Game Physics Engine Development — PDF pages 337–348 (`src-fe1515cce28474060a35`) | 22 | 12 |
| 2D Renderer Transforms \| Game Engine series (`src-3fd22c0647ea61455659`) | 22 | 22 |
| We Made a Game About Delivering Coffee in a War Zone in Hazel in 72 Hours! (`src-1383171c5b89f364c507`) | 20 | 20 |
| Ep. 5.5: Unions - Sparky Engine (How To Make a Game Engine) (`src-fb27333a75b1f4afdbda`) | 20 | 20 |
| Game Physics Engine Development — PDF pages 433–444 (`src-d5038be5a6ffc822931b`) | 19 | 11 |
| LIGHTS! // Hazel Engine Dev Log (`src-f458dfc8b00fafbcc196`) | 18 | 18 |
| PREFABS in Game Engines (`src-7ce0ccfa518281cc5218`) | 17 | 17 |
| 3D Game Programming - Episode 1 - Window (`src-2781a8753853a62ca50b`) | 17 | 17 |
| Sparky Engine Dev Log #9 - Skybox and FPS Camera (`src-2eead7416832437fc2e3`) | 16 | 16 |
| Episode 18.5 - Creating an EXE File in Java (`src-4b9a37c225c1db08d78e`) | 15 | 15 |
| Game Physics Engine Development — PDF pages 193–204 (`src-86da578e3720f256f8de`) | 15 | 12 |
| Game Physics Engine Development — PDF pages 1–12 (`src-11633e0a67c1b0b2f1ae`) | 15 | 11 |
| NEW SERIES! - Graphics/OpenGL/Game Engine (Update Vlog) (`src-5ba3ebe3d4d21c8bbcd8`) | 13 | 13 |
| Skeletal Animation! // Hazel Engine Dev Log (`src-9208df5056f84e46d087`) | 12 | 12 |
| 3D Game Programming - Episode 7 - FPS Counter (`src-82edb6e78b7330098168`) | 11 | 11 |
| Game Design Fundamentals Rules of Play — PDF pages 685–694 (`src-fab09b188b35d013cfa1`) | 8 | 5 |
