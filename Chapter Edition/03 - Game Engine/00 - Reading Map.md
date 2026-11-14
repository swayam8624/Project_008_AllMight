# Game Engine — From First Frame to Living Worlds — Reading Map

Follow one visible player action through memory, engine boundaries, rendering, tools, physics and game systems. Each major chapter preserves the complete lessons and working examples from the continuous master.

[[Continuous Notes/Game Engine - From First Frame to Living Worlds|Continuous master]] · [[Chapter Edition/README|All three books]]

| Chapter | Estimated pages | What you will learn |
|---|---:|---|

| [[Chapter Edition/03 - Game Engine/01 - A game promise becomes bytes and running state|01 — A game promise becomes bytes and running state]] | 48.8 | Explain how a player-visible feature depends on data representation, object lifetime, and a sequence of engine services. |

| [[Chapter Edition/03 - Game Engine/02 - How bytes become programs, objects, and resource references|02 — How bytes become programs, objects, and resource references]] | 50.0 | Reconstruct the path from serialized bytes to typed state and identify the lifetime and identity rules at each boundary. |

| [[Chapter Edition/03 - Game Engine/03 - Object representation, lifetime, and enforceable promises|03 — Object representation, lifetime, and enforceable promises]] | 50.2 | Distinguish representation from validation and explain how constructors, ownership, and lifetime rules protect invariants. |

| [[Chapter Edition/03 - Game Engine/04 - Resource ownership, failure paths, and review practice|04 — Resource ownership, failure paths, and review practice]] | 55.8 | Follow a resource through acquisition, use, replacement, failure, and shutdown while keeping claims tied to demonstrated evidence. |

| [[Chapter Edition/03 - Game Engine/05 - Generic asset pipelines with stable identity|05 — Generic asset pipelines with stable identity]] | 49.0 | Choose which asset operations can be generic and which depend on semantic type, then implement identity that survives storage movement. |

| [[Chapter Edition/03 - Game Engine/06 - Queries, lazy ranges, allocators, and module boundaries|06 — Queries, lazy ranges, allocators, and module boundaries]] | 43.4 | Design a query that returns only the needed view and state the ownership, allocator, and compilation boundaries it depends on. |

| [[Chapter Edition/03 - Game Engine/07 - Typed APIs and measured CPU data structures|07 — Typed APIs and measured CPU data structures]] | 43.5 | Connect strong types and PIMPL to measured CPU costs, then select containers and scheduling structures by their operation patterns. |

| [[Chapter Edition/03 - Game Engine/08 - Algorithms, application boundaries, and a first playable|08 — Algorithms, application boundaries, and a first playable]] | 52.8 | Select algorithms for world queries and navigation, then connect them to a minimal application and a playable validation loop. |

| [[Chapter Edition/03 - Game Engine/09 - Player choices, rules, and readable feedback|09 — Player choices, rules, and readable feedback]] | 56.3 | Analyze a feature as a loop of visible options, player action, authoritative state change, and legible consequence. |

| [[Chapter Edition/03 - Game Engine/10 - Engine changes across subsystem contracts|10 — Engine changes across subsystem contracts]] | 57.9 | Explain how an engine change crosses dependencies and preserve stable contracts among game code, tools, runtime services, and backends. |

| [[Chapter Edition/03 - Game Engine/11 - Streaming resources through explicit states|11 — Streaming resources through explicit states]] | 58.2 | Model streaming as request, decode, validation, upload, publication, use, and retirement, including stale completion and memory-budget cases. |

| [[Chapter Edition/03 - Game Engine/12 - Transactions, recovery, and safe project boundaries|12 — Transactions, recovery, and safe project boundaries]] | 55.9 | Design undo/redo, save recovery, path validation, and package publication so partial failure cannot silently corrupt authored work. |

| [[Chapter Edition/03 - Game Engine/13 - Portable startup, windows, events, and input|13 — Portable startup, windows, events, and input]] | 49.6 | Describe startup/shutdown dependencies and separate native window events, application input state, and game actions. |

| [[Chapter Edition/03 - Game Engine/14 - Application clocks and stable input identity|14 — Application clocks and stable input identity]] | 49.3 | Distinguish wall time, frame time, and simulation ticks, and ensure input identifiers remain meaningful across updates. |

| [[Chapter Edition/03 - Game Engine/15 - Sparse-set components and generational entity handles|15 — Sparse-set components and generational entity handles]] | 53.2 | Implement component membership and entity identity while accounting for swap removal, slot reuse, and stale handles. |

| [[Chapter Edition/03 - Game Engine/16 - Archetypes, chunks, relationships, and transforms|16 — Archetypes, chunks, relationships, and transforms]] | 54.0 | Compare sparse-set and archetype operations, then specify how entity migration and transform dependencies preserve identity and update order. |

| [[Chapter Edition/03 - Game Engine/17 - ECS performance through cache and scheduling|17 — ECS performance through cache and scheduling]] | 53.7 | Explain how cache-line ownership and scheduling influence an ECS workload, and propose measurements that isolate the bottleneck. |

| [[Chapter Edition/03 - Game Engine/18 - ECS contracts, diagnostics, and GPU handoff|18 — ECS contracts, diagnostics, and GPU handoff]] | 43.7 | Audit ECS invariants with diagnostics and tests, then explain how entity data becomes render work with explicit ownership. |

| [[Chapter Edition/03 - Game Engine/19 - GPU work and reusable Metal frames|19 — GPU work and reusable Metal frames]] | 51.3 | Trace one frame’s command, buffer, and completion state, and determine when each frame-scoped allocation can be reused. |

| [[Chapter Edition/03 - Game Engine/20 - GPU performance models and principled redesign|20 — GPU performance models and principled redesign]] | 53.6 | Use a performance model to distinguish instruction, bandwidth, occupancy, and scheduling limits, then design a comparable measurement. |

| [[Chapter Edition/03 - Game Engine/21 - Buffers, images, memory, and uploads|21 — Buffers, images, memory, and uploads]] | 63.6 | Select resource layouts and memory types from API requirements, and trace a CPU upload until the GPU may safely consume it. |

| [[Chapter Edition/03 - Game Engine/22 - Vulkan synchronization and multi-frame lifetimes|22 — Vulkan synchronization and multi-frame lifetimes]] | 58.7 | Derive the dependency and lifetime requirements for a resource crossing Vulkan commands, queues, or frames in flight. |

| [[Chapter Edition/03 - Game Engine/23 - Execute and measure a Metal frame|23 — Execute and measure a Metal frame]] | 54.5 | Reconstruct command encoding, submission, presentation, and timing for a Metal frame while preserving each object’s ownership. |

| [[Chapter Edition/03 - Game Engine/24 - Profiling and languageplatform ownership|24 — Profiling and language/platform ownership]] | 39.7 | Interpret profiling output with build and workload context, and cross language/platform boundaries without leaking ownership or global state. |

| [[Chapter Edition/03 - Game Engine/25 - Renderer evolution through resource ownership|25 — Renderer evolution through resource ownership]] | 61.1 | Evolve a renderer while keeping the owner, lifetime, and completion condition visible for every resource and frame packet. |

| [[Chapter Edition/03 - Game Engine/26 - Descriptor arrays and backend limits|26 — Descriptor arrays and backend limits]] | 61.0 | Design descriptor indexing and binding around device capabilities, reserved slots, and a validated shader-visible index range. |

| [[Chapter Edition/03 - Game Engine/27 - Preparing and publishing runtime content|27 — Preparing and publishing runtime content]] | 63.1 | Specify the inputs and version key for derived content, and publish it only after validation while rejecting stale asynchronous results. |

| [[Chapter Edition/03 - Game Engine/28 - BVH traversal, refitting, and SIMD|28 — BVH traversal, refitting, and SIMD]] | 45.6 | Trace a BVH query, update bounds after motion, and describe the invariants required for wide nodes and SIMD traversal. |

| [[Chapter Edition/03 - Game Engine/29 - Dynamic spatial indices for moving worlds|29 — Dynamic spatial indices for moving worlds]] | 60.6 | Choose a dynamic broad-phase structure for a stated motion and density workload, including duplicate suppression and update rules. |

| [[Chapter Edition/03 - Game Engine/30 - Depth, projection, and shadow-map precision|30 — Depth, projection, and shadow-map precision]] | 60.0 | Derive how projection maps distance to depth and explain precision, bias, and coverage limits in a shadow pass. |

| [[Chapter Edition/03 - Game Engine/31 - Spatial-index serialization and query evidence|31 — Spatial-index serialization and query evidence]] | 44.3 | Define a reproducible spatial-query workload and compare candidate structures for correctness, update cost, and query quality. |

| [[Chapter Edition/03 - Game Engine/32 - Terrain materials and image composition|32 — Terrain materials and image composition]] | 56.1 | Implement and validate a bounded material blend, including texture inputs, weights, and visible edge cases. |

| [[Chapter Edition/03 - Game Engine/33 - Versioned scenes and transactional loading|33 — Versioned scenes and transactional loading]] | 56.6 | Design schema validation, stable IDs, migration, and transactional replacement for a saved scene. |

| [[Chapter Edition/03 - Game Engine/34 - Rigid-body motion from forces to contact|34 — Rigid-body motion from forces to contact]] | 50.6 | Relate forces and impulses to linear/angular motion, compute simple inertia, and explain the assumptions behind a rigid-body contact step. |

| [[Chapter Edition/03 - Game Engine/35 - A complete rigid-body state before the formula audit|35 — A complete rigid-body state before the formula audit]] | 42.1 | Describe the rigid-body state and its update responsibilities, including body-local force points, world transforms, and the distinction between stored state and derived values. |

| [[Chapter Edition/03 - Game Engine/36 - Auditing capsule mass properties and collision formulas|36 — Auditing capsule mass properties and collision formulas]] | 56.5 | Check capsule mass and inertia formulas dimensionally and geometrically before treating them as an engine implementation. |

| [[Chapter Edition/03 - Game Engine/37 - Physical examples and state validation|37 — Physical examples and state validation]] | 57.0 | Use physical examples to validate rigid-body state, inertia, and transformation synchronization against explicit invariants. |

| [[Chapter Edition/03 - Game Engine/38 - Integrator choices and energy drift|38 — Integrator choices and energy drift]] | 56.9 | Derive one-step updates and predict stability and energy changes for a stated step size and force model. |

| [[Chapter Edition/03 - Game Engine/39 - Composable timesteps and stable simulation policy|39 — Composable timesteps and stable simulation policy]] | 56.9 | Explain when timestep subdivision preserves a model and when parameterization or state updates change the result. |

| [[Chapter Edition/03 - Game Engine/40 - Coupled contacts and iterative response|40 — Coupled contacts and iterative response]] | 49.0 | Reconstruct the contact constraints and solve policy for a multi-contact system, stating where the reference witness stops. |

| [[Chapter Edition/03 - Game Engine/41 - Evidence, tools, and revisable design|41 — Evidence, tools, and revisable design]] | 51.0 | Use the right artifact and evidence level to test a design question, distinguishing observed behavior, inference, and future intent. |

| [[Chapter Edition/03 - Game Engine/42 - From cartridge bytes to a reproducible machine|42 — From cartridge bytes to a reproducible machine]] | 52.9 | Trace a stateful system from bytes to reproducible execution and identify what must be serialized or synchronized for replay. |

| [[Chapter Edition/03 - Game Engine/43 - Compiling renderer intent into backend work|43 — Compiling renderer intent into backend work]] | 45.4 | Map a render request to backend work while preserving resource dependencies, barriers, and lifetime ownership. |

| [[Chapter Edition/03 - Game Engine/44 - Cross-backend conversion and agreement|44 — Cross-backend conversion and agreement]] | 37.7 | Define parity criteria for a cross-backend path, including formats, coordinate conventions, synchronization, and output comparison. |

| [[Chapter Edition/03 - Game Engine/45 - Rasterizing kernels and connecting world identity|45 — Rasterizing kernels and connecting world identity]] | 59.5 | Trace kernel projection and compositing while stating the conditions needed to associate visible samples with editable world entities. |

| [[Chapter Edition/03 - Game Engine/46 - Game design from mechanics to narrative interfaces|46 — Game design from mechanics to narrative interfaces]] | 61.2 | Connect formal rules, player experience, cultural context, and camera policy to a specific playable situation. |

| [[Chapter Edition/03 - Game Engine/47 - Narrative, simulation, and AI in playable worlds|47 — Narrative, simulation, and AI in playable worlds]] | 60.4 | Explain how story constraints and AI decisions enter simulation while preserving player agency, navigation correctness, and observable outcomes. |
