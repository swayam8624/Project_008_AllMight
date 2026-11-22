# Glossary of Key Terms

This glossary defines terms at the level required to use the chapters correctly. It is deliberately not a substitute for the language or API specifications. Chapter references below point to the numbered reading chapters; they remain stable across both paperbacks.

**Abstract machine (Chapters 2–4).** The model of program behavior defined by a programming language. Its objects, lifetimes, sequenced operations and observable behavior need not correspond one-for-one to processor instructions.

**Address (Chapters 1–2).** A representation used to locate storage in an address space. An address alone says neither that a particular C++ object is alive nor that an access is permitted.

**Alignment (Chapter 2).** A constraint on which addresses may validly contain an object or be used for a specified operation. Alignment and object size are distinct constraints.

**Atomic operation (Chapters 9 and 12).** An operation with indivisible effects relative to a defined competing-access model. Atomicity does not automatically provide a global synchronization barrier or high throughput.

**Barrier (Chapters 9, 12, 14–15).** A synchronization mechanism that constrains some combination of execution order and memory visibility within a defined scope. Barrier names are not semantically interchangeable between APIs.

**Binade (Chapter 1).** An interval of floating-point values with a common spacing scale determined by the exponent; within a normal binade, representable numbers have approximately uniform absolute spacing.

**Borrowed view (Chapters 2, 3, 11).** A non-owning reference to existing storage or objects. Its validity depends on the referenced object's lifetime and any associated synchronization constraints.

**Buffer lifetime (Chapters 11–15).** The interval during which a resource remains valid for all its intended consumers. In asynchronous systems it may extend beyond the host function that created or submitted it.

**Cache locality (Chapters 7, 9, 16).** The tendency of accesses to reuse nearby locations in time or address space, often reducing memory-system cost. The effect depends on actual hardware and workload.

**Command buffer (Chapters 12–15).** A representation of operations recorded or encoded for subsequent device execution. Recording, submission, execution and retirement are different events.

**Compaction (Chapters 8–9).** Transforming selected elements into a densely indexed output. Parallel implementations commonly count, scan, and scatter to assign nonoverlapping destinations.

**Contract (throughout).** A statement of permitted inputs, required outputs, invariants and failure behavior at an interface or operation boundary.

**Data race (Chapters 4, 9, 12).** Conflicting accesses to shared memory that lack the ordering required by the governing memory model. The precise consequences differ between languages and device execution models.

**Descriptor (Chapter 13).** An API-defined description through which GPU work accesses a resource. Its bindings and lifetime are governed by the descriptor and execution rules of the relevant API.

**Divergence (Chapter 9).** Differences in control flow among concurrently executed lanes that can reduce utilization when execution resources are shared.

**Endianness (Chapters 1–2).** The rule determining how bytes of a multibyte representation are ordered in storage or a stream. It does not determine the abstract numeric value by itself.

**Fence (Chapters 12, 14–15).** A synchronization object or mechanism used to establish or observe completion/ordering obligations, with meaning defined by the specific API.

**Generational handle (Chapter 6).** An index or slot identity combined with a generation discriminator, allowing a container to reject some stale references after reuse. Generation wrap and concurrent access still require policy.

**Invariant (throughout).** A condition that must remain true at defined points across valid operations. A useful implementation states when an invariant may be temporarily broken and when it must be restored.

**ISA — instruction set architecture (Chapters 1, 7).** The programmer-visible specification of machine instructions, registers and execution behavior. An ISA is not the same as a processor's internal microarchitecture.

**Lifetime (Chapters 2–4, 11–15).** The period during which an object or resource is valid under the applicable rules. Storage availability and object lifetime are not interchangeable.

**Memory coalescing (Chapter 9).** Servicing nearby lane addresses through efficiently grouped device-memory transactions. The transaction details are architecture dependent.

**Memory visibility (Chapters 9, 12, 14–15).** Conditions under which one operation's memory effects may be observed by another. Execution completion and visibility must be distinguished.

**Move operation (Chapter 3).** A language-level operation that may transfer resources or state subject to its type's guarantees. It is not automatically a bytewise transfer.

**Object identity (Chapters 2–3, 6).** The identity of a particular object during its lifetime. Equality of bytes, values or addresses does not by itself establish identical objects.

**Occupancy (Chapter 16).** The relationship between active execution resources and hardware capacity on a processor. Higher occupancy is not always higher performance.

**Oracle (Chapters 4, 9, 16).** An independent reference procedure or property used to assess correctness, especially when optimizing or benchmarking a more complex implementation.

**Prefix scan (Chapters 8–9).** A transformation producing prefix aggregates under an associative operation. An exclusive scan of counts constructs nonoverlapping output offsets.

**RAII (Chapters 3–4, 11).** Resource Acquisition Is Initialization: a C++ technique coupling resource management to object lifetime and destruction so cleanup follows scope and ownership.

**Serialization (Chapters 1–2).** Conversion between in-memory state and a defined external representation. The format must specify width, byte order, versioning and validation behavior rather than relying on native object layout.

**SIMD / SIMT (Chapters 7, 9).** Related execution models in which instructions or instruction streams operate across multiple data lanes or invocations. The distinction matters for control flow and subgroup behavior.

**Source snapshot (throughout).** A statement about a particular project revision or observed implementation, not a normative C++ or graphics API guarantee.

**Subgroup (Chapter 9).** A collection of GPU invocations on which a device exposes defined collective operations. Available widths and operations must be queried or guaranteed by the relevant API.

**Synchronization scope (Chapters 9, 12, 14–15).** The set of operations, resources or execution agents to which a particular ordering mechanism applies.

**Threadgroup / workgroup (Chapters 9, 15).** A group of invocations with some API-defined shared resources and synchronization capabilities. Group size is an algorithmic and hardware constraint.

**Undefined behavior (Chapters 2–4, 11).** Behavior for which the programming language makes no required behavioral guarantee in the circumstances specified by its rules. A repeatable observed outcome does not make it defined.

**Value category (Chapter 3).** A classification of expressions affecting overload resolution, binding and the permitted form of initialization or movement.

**Vulkan queue (Chapters 13–14).** A device execution interface with specific supported operations and synchronization obligations. Queue-family capability and ownership transfers are part of correct use.

# Laboratory and Companion-Code Guide

The recurring projects are intended as concrete demonstrations of general mechanisms, not as prerequisites for accepting their author's opinions. A student should be able to reconstruct the contract before inspecting the implementation.

| Stage | Core chapters | Reproducible result | Failure case to test |
|---|---|---|---|
| Binary-machine representation | 1–2 | Decoder with explicit bit widths, bounds and byte order | Truncated or malformed input |
| C++ ownership and value transitions | 2–4 | Resource wrapper with traceable construction, move and destruction | Exception or partial construction |
| Generic data pipelines | 5 | A typed transformation preserving validation and ownership | Incorrect type or capacity |
| Registries and stable identity | 6–7 | Generational-handle lookup with stale-handle rejection | Reuse of a freed slot |
| Graphs and dependencies | 7–8 | Search or dependency graph with a test oracle | Cycle or invalid edge assumption |
| Parallel primitives | 8–9 | Predicate/count/scan/scatter or stable ordering | Empty inputs and contention |
| Build and measurement discipline | 10 | Reproducible configurations and a correctness gate | Changed toolchain or failed baseline |
| CPU–GPU resource lifecycle | 11–12 | Host/device timeline showing last use and legal retirement | Buffer reuse before completion |
| Vulkan resource path | 13–14 | Resource-creation and submission trace with validation enabled | Missing dependency or incorrect state |
| Metal execution | 15–16 | Frame measurement with separate CPU and GPU timing | Misidentified submission/completion |

The public curriculum repository is [Project AllMight](https://github.com/swayam8624/Project_008_AllMight). The relevant teaching sources live under `Book Production/Seven-Book Curriculum/Book01/`, `Supplementary/C++23 Project Labs/`, and `Chapter Edition/01 - Computing Foundations/`. Before presenting any code as a reproducible published companion, freeze an immutable commit, state the compiler and platform requirements, run the relevant tests, and record the resulting exact source hash. The working branch alone is not a permanent version identifier.

A code listing in this textbook may represent a complete program, a function fragment, deliberately invalid code, algorithmic pseudocode, or an API-specific operation. Only a complete program accompanied by its dependencies, compile command and oracle is asserted to be independently runnable. Pseudocode must not be described as valid C++23 merely because it resembles a C++ loop.

## Four independently testable companion programs

The public repository also contains a self-contained **Companion** folder at `Book Production/Seven-Book Curriculum/Book01/Companion/`. It uses C++23, CMake and CTest without any private engine dependency. These are deliberately limited teaching machines; none is presented as a commercial emulator or actual Vulkan/Metal implementation.

| Companion file | Exercise | Independent negative or boundary case |
|---|---|---|
| `src/binary_cpu.cpp` | Execute a minimal eight-bit load/add/store/halt instruction set | Truncated opcode, unknown opcode, execution budget |
| `src/generational_handles.cpp` | Allocate, erase, recycle and resolve generation-checked registry handles | Reject stale handle and duplicate erase |
| `src/parallel_scan.cpp` | Compute exclusive scan and stable scatter with a serial reference | Reject invalid predicate or mismatched extent; 200 seeded cases |
| `src/retirement_contract.cpp` | Simulate pending device uses and completion-controlled retirement | Reject premature retirement, duplicate completion, re-submission after retirement |

Build them from the checkout root:

```sh
cmake -S "Book Production/Seven-Book Curriculum/Book01/Companion" -B build/book01-companion -DCMAKE_BUILD_TYPE=Debug
cmake --build build/book01-companion --parallel
ctest --test-dir build/book01-companion --output-on-failure
```

A passing CTest run is evidence for these models on that compiler and host. It does not establish compatibility with an actual NES cartridge, correctness of Vulkan or Metal synchronization, or performance on a GPU. Readers should inspect and extend each oracle before treating it as a research benchmark. For reproducible software citations, use a specific repository release, archived tag or commit permalink from the release materials. Source hashes remain in the machine-readable provenance manifest, not in the reader-facing copyright page.


# Experimental Record and Reproducibility

A credible experiment records a question, a predicted result, a reference answer, and the environment in which the observation was made. For each laboratory, maintain an experiment record with the following fields.

| Field | What it must establish |
|---|---|
| Claim | A proposition precise enough to be disproved |
| System under test | Source revision, compiler, flags, API version, device and OS |
| Input identity | Dataset, seeds, format version and material assumptions |
| Independent oracle | How correctness is established without trusting the optimized code |
| Baseline | Which alternative is compared, and why the comparison is fair |
| Measurements | Units, warm-up, repetitions, aggregation and uncertainty |
| Boundary cases | Empty, maximal, malformed, concurrent or adversarial inputs |
| Failures | Unexpected output, validation errors and genuine unsupported cases |
| Interpretation | What the data support, and what remains unknown |

For performance comparisons, distinguish a speedup from a correctness improvement. State the direction of ratios explicitly, report measurement variation rather than only a single favorable run, and separate CPU encoding/submission costs from GPU execution costs when the API permits reliable measurement. Keep both a success case and a counterexample; the latter is often more valuable for understanding a design boundary.

# References, Standards, and Further Reading

References are listed where they materially support a language rule, mathematical mechanism, algorithm, measurement method, or practical implementation choice. **Books are normally identified by edition and ISBN; journal papers and some electronic books have DOIs; living specifications need versioned URLs.** No identifier should be invented merely to fill a field. A link to source code is evidence that an implementation exists, not proof that its claims are correct. For a research publication, cite the exact paper, page/section, standard revision, or archived software version supporting the particular claim.

## Standards and authoritative documentation

- **[S1] C++23 (Chapters 2–5, 10–12):** ISO/IEC 14882:2024, *Programming languages — C++*, [ISO catalogue](https://www.iso.org/standard/83626.html). The openly readable [C++ working draft](https://eel.is/c++draft/) is a *draft*, not the purchased ISO publication. [cppreference](https://en.cppreference.com/w/cpp/23) is a navigational secondary reference; actual library availability depends on the compiler and standard-library implementation.
- **[S2] Processor architecture (Chapters 1, 7):** [RISC-V specifications](https://riscv.org/technical/specifications/) provide concrete ISA contracts. Distinguish the abstract instruction set from a particular implementation's cache, pipeline, or memory timings.
- **[S3] Vulkan (Chapters 9, 12–14):** Khronos, [Vulkan specification](https://docs.vulkan.org/spec/latest/), [Vulkan memory model appendix](https://docs.vulkan.org/spec/latest/appendices/memorymodel.html), and [Vulkan Validation Layers](https://github.com/KhronosGroup/Vulkan-ValidationLayers). Record the specification and device feature set used.
- **[S4] Metal (Chapters 9, 12, 15–16):** Apple, [Metal API documentation](https://developer.apple.com/documentation/metal), [resource synchronization](https://developer.apple.com/documentation/metal/resource-synchronization), and [Metal 4 core API](https://developer.apple.com/documentation/metal/understanding-the-metal-4-core-api). Do not conflate Metal 3 and Metal 4 guarantees.
- **[S5] Builds and diagnostics (Chapters 4, 10, 16):** [CMake reference](https://cmake.org/cmake/help/latest/) and [Clang documentation](https://clang.llvm.org/docs/). A particular build must additionally record its compiler, flags, target environment, and test results.

## Books: editions and persistent identifiers

- **[B1]** Bjarne Stroustrup, *A Tour of C++*, 3rd ed., Addison-Wesley Professional, 2022. **ISBN 978-0-13-681648-5.** [Publisher catalogue](https://www.informit.com/store/tour-of-c-plus-plus-9780136816485). Useful for language-level orientation, but its principal presentation is C++20; consult [S1] for specifically C++23 requirements.
- **[B2]** Scott Meyers, *Effective Modern C++*, O'Reilly Media, 2014. **ISBN 978-1-4919-0399-5.** [Publisher edition](https://www.oreilly.com/library/view/effective-modern-c/9781491908419/). Particularly useful for C++11/14 ownership, move and deduction idioms; do not treat it as a C++23 specification.
- **[B3]** Marc Gregoire, *Professional C++*, 6th ed., Wiley, 2024. **ISBN 978-1-394-19317-2** (print); **book DOI [10.1002/9781394193202](https://doi.org/10.1002/9781394193202)** (electronic edition). [Publisher edition](https://www.wiley.com/en-in/professional-c-6th-edition-p-9781394193172). Further study of object models, templates, library design, tooling, and engineering practice.
- **[B4]** Thomas H. Cormen, Charles E. Leiserson, Ronald L. Rivest, and Clifford Stein, *Introduction to Algorithms*, 4th ed., MIT Press, 2022. **ISBN 978-0-262-04630-5.** [MIT Press edition](https://mitpress.mit.edu/9780262046305/introduction-to-algorithms/). Follow for asymptotic analysis, sorting, graph algorithms and correctness arguments.
- **[B5]** Tamara Munzner, *Visualization Analysis and Design*, A K Peters/CRC Press, 2014. **DOI [10.1201/B17511](https://doi.org/10.1201/B17511)**. [Author's book resources](https://www.cs.ubc.ca/~tmm/vadbook/). A bridge from numerical results to perceptually and analytically defensible figures; developed substantially in later research-oriented books.

## Research articles: DOI-resolved primary literature

- **[P1]** David Goldberg, “What Every Computer Scientist Should Know About Floating-Point Arithmetic,” *ACM Computing Surveys*, 23(1), 1991. **DOI [10.1145/103162.103163](https://doi.org/10.1145/103162.103163)**. Relevant to Chapter 1 numerical representation, rounding and error.
- **[P2]** Samuel Williams, Andrew Waterman, and David Patterson, “Roofline: An Insightful Visual Performance Model for Multicore Architectures,” *Communications of the ACM*, 52(4), 2009. **DOI [10.1145/1498765.1498785](https://doi.org/10.1145/1498765.1498785)**. Relevant to Chapters 7, 9, 12 and 16, where arithmetic intensity and memory bandwidth limit speed.
- **[P3]** Hans-J. Boehm and Sarita V. Adve, “Foundations of the C++ Concurrency Memory Model,” *PLDI*, 2008. **DOI [10.1145/1375581.1375591](https://doi.org/10.1145/1375581.1375591)**. Explains motivating concurrency semantics; the current normative wording remains [S1].
- **[P4]** Victoria Interrante, “Art and Science in Visualization,” in *Visualization Handbook*, 2005, pp. 781–805. **DOI [10.1016/B978-012387582-2/50042-3](https://doi.org/10.1016/B978-012387582-2/50042-3)**. A companion for later chapters on designing convincing *and* faithful scientific views.

## Companion repositories and implementation case studies

[Project AllMight](https://github.com/swayam8624/Project_008_AllMight) is the seven-book curriculum repository. The [Book 1 companion programs](https://github.com/swayam8624/Project_008_AllMight/tree/curriculum/cpp23-nes-project-pass01/Book%20Production/Seven-Book%20Curriculum/Book01/Companion) are distinct from the larger engineering projects below. Their public repositories have been checked for existence, but neither publication nor this index constitutes an audit or endorsement of their current correctness.

| Project repository | What to investigate | Main teaching connection |
|---|---|---|
| [NESEmulator](https://github.com/swayam8624/NESEmulator) | Cartridge bytes, instruction execution, state and replay | Book 1: Chapters 1–4, 7, 10 |
| [MLLibrary](https://github.com/swayam8624/MLLibrary), [KairoMath](https://github.com/swayam8624/KairoMath) | Numerical primitives, data representations, measurement | Books 1–2 |
| [KairoGeometry](https://github.com/swayam8624/KairoGeometry), [KairoSpatial](https://github.com/swayam8624/KairoSpatial) | Geometric predicates, spatial structures, reproducible queries | Books 2–5 |
| [KairoRayTracer](https://github.com/swayam8624/KairoRayTracer) | Ray tracing and image synthesis | Book 4 |
| [KairoECS](https://github.com/swayam8624/KairoECS), [KairoAssets](https://github.com/swayam8624/KairoAssets) | Entity/resource identities and asset life cycles | Books 1 and 5 |
| [KairoPhysicsEngine](https://github.com/swayam8624/KairoPhysicsEngine), [kyoto-common-contact](https://github.com/swayam8624/kyoto-common-contact) | Physics, contact, collision methods and experimental comparisons | Books 5 and 7 |
| [KairoRenderer](https://github.com/swayam8624/KairoRenderer), [KairoGameEngine](https://github.com/swayam8624/KairoGameEngine) | Rendering pipelines, Vulkan/Metal integration and engine construction | Books 5–6 |
| [Maveb](https://github.com/swayam8624/Maveb) | Hybrid scene representation, real-time visual computing and evidence | Books 3–4, 6–7 |
| [Vulkax](https://github.com/swayam8624/Vulkax), [FoveaBench](https://github.com/swayam8624/FoveaBench) | Research experiments, vision/XR evaluation and reproducibility | Book 7 |

**Attribution rule for every book:** when borrowing a formal result, claim, figure, benchmark, idea or adapted example, identify the original author and source at the point of use, then supply its full reference here or in the relevant book's bibliography. A GitHub URL is not a substitute for the original paper's DOI. Original illustrations should be labelled as original; adapted or reproduced figures need source attribution and appropriate permission or licence, not merely a bibliography entry. For an experimental figure, also preserve the scripts, input data identity, plotting parameters and raw measurements.

# Concept Locator

This is a chapter-based locator, not a full word-level index. It is designed to remain accurate across the two physical volumes while a detailed page-level scholarly index is being prepared.

| Concept | Principal chapters |
|---|---|
| Binary, arithmetic overflow, floating point, quantization | 1 |
| Address spaces, alignment, object representation | 1–2 |
| Lifetime, identity, RAII, move operations | 2–4 |
| Exceptions, error guarantees, sanitizers | 3–4 |
| Generic programming, constraints, type-safe data pipelines | 5 |
| Handles, registries, caching, stable IDs | 6–7 |
| Data structures, graphs, search, dependency ordering | 7–8 |
| Parallel scan, scatter, radix sorting | 8–9 |
| Workgroup geometry, divergence, coalescing, atomics | 9 |
| Reproducible build, instrumentation, profiling | 10, 16 |
| Const correctness, borrowing, buffer life cycles | 11–12 |
| Device queues, dispatch, completion and barriers | 12–15 |
| Vulkan devices, memory, descriptors, pipelines | 13–14 |
| Metal command encoding and resource modes | 15–16 |
| Independent oracles, performance evidence | 4, 10, 16–17 |

# Source Integrity, Attribution, and Corrections

The reader edition is assembled from preserved foundation chapters, reversible C++23 laboratory additions, and a final mastery laboratory. Publisher-facing source tracking, generated-file hashes and build traces are recorded in repository manifests rather than printed on the copyright page. These records assist reproducibility and corrections; they are not a substitute for source-specific citations within a scientific claim.

Named Kairo, Maveb, Vulkax and other implementations are treated as inspectable case studies, not established truths. The links above point to actively changing repositories. Reproduce experiments against an archived release, tag or immutable commit, and independently inspect the relevant behaviour before citing a result.

Report errors or propose corrections through [Project AllMight's issue tracker](https://github.com/swayam8624/Project_008_AllMight/issues). Include the book and chapter, disputed statement, authoritative source or counterexample, and—when executable code is involved—a minimal reproducer. Research figures and code examples should remain legible, traceable and reproducible even when their visual design is ambitious.
