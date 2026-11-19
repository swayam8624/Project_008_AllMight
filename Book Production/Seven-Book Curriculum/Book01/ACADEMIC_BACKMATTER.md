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

# Primary References and Further Reading

The following are entry points to the normative documents or maintained documentation relevant to the text. Their versions and feature availability should be verified for the reader's actual toolchain. A reading-list entry does not turn an illustrative code sample into an independently verified implementation.

**C++ language and library.** The ISO/IEC 14882 standard defines the C++ language and standard library (consult an official licensed copy where applicable). The [C++ working draft](https://eel.is/c++draft/) provides publicly readable draft wording and may differ from the published standard. The [C++23 feature inventory](https://en.cppreference.com/w/cpp/23) is a navigational secondary reference for feature availability; implementation support must still be checked.

**Computer architecture.** The [RISC-V ISA specifications](https://riscv.org/technical/specifications/) provide a freely available example of an instruction-set contract. An ISA specification defines permitted behavior, not the internal pipeline design or cache arrangement of every compatible processor.

**Vulkan.** The [Vulkan specification](https://docs.vulkan.org/spec/latest/) and [memory model appendix](https://docs.vulkan.org/spec/latest/appendices/memorymodel.html) are primary references for device, resource, ordering and visibility rules. Validate the feature and extension set of the specific device rather than assuming that every published mechanism is available on every installation.

**Metal.** Apple's [Metal documentation](https://developer.apple.com/documentation/metal), [resource synchronization guidance](https://developer.apple.com/documentation/metal/resource-synchronization) and [Metal 4 core API guide](https://developer.apple.com/documentation/metal/understanding-the-metal-4-core-api) describe versioned Apple-specific execution and resource contracts. Metal 4 examples are not silently interchangeable with Metal 3 code.

**Build and verification.** The [CMake documentation](https://cmake.org/cmake/help/latest/) explains the configure, build and dependency model. The [Clang documentation](https://clang.llvm.org/docs/) covers compiler diagnostics and sanitizers. The [Khronos Vulkan validation layers repository](https://github.com/KhronosGroup/Vulkan-ValidationLayers) explains what validation can check, but validation success is not a proof of all application behavior.

**Experimental methods.** Consult a primary paper or standard for a specialized algorithm, specify an independent correctness witness, and describe uncertainty and limitations. Source code can demonstrate an implementation, but the underlying mathematical or API claim must be supported separately.

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

# Technical Provenance and Publication Limits

The reader edition is derived from the preserved first sixteen Computing Foundations chapter bodies, eighteen reversible project laboratories, and an integrative mastery laboratory. Production changes to typography, chapter headings, figure rendering, code-fence metadata and mathematical escaping do not silently replace the canonical teaching source. The production manifest records source identity and generated artifacts; use it to trace a disputed passage.

Named Kairo, Maveb, Vulkax and other project implementations may be discussed as source-reported snapshots. Such claims are not statements about the current behavior of a particular repository without an immutable revision and a reproducible verification record. They are also not statements about what every compliant C++ compiler, Vulkan driver or Metal device must do.

A complete academic release needs an independent technical review of commands, proof obligations and API examples, a true page-level index, and a successful print preview. This working edition provides the apparatus and acceptance tests needed to carry out that release review without misrepresenting its status.
