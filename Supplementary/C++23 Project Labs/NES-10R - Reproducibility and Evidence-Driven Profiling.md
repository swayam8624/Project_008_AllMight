### NES-10R — A build that existed on one machine is not yet a reproducible result

A sequence of C++23 laboratories is now connected by real code rather than by optimistic diagrams. We parse an iNES cartridge, own the program bytes, map the CPU bus, replace cartridge state transactionally, publish typed assets, reject stale generational handles, execute a small part of the 6502 instruction set, observe instructions through a separately compiled debugger, schedule dependent host work and model the memory obligations of hypothetical GPU operations. The original chapter already contains several thousand lines about CMake, compilers, modules, reproducibility, CI, sanitizers, profiling and repository management. We will **retain all of that**. What it needs from this continuing project is an actual source-to-build-to-evidence contract connecting the nine previous milestones.

A functioning program and a documented program are not identical. A source file may contain an assertion but never have been compiled. A CMake target may be registered yet fail during linking. A successful build may have compiled different source revisions from the ones now being inspected. A passing test may exercise only one narrow path. A profiler can print a number without measuring meaningful work. A green benchmark may optimize away the operation it supposedly times. These are not theoretical concerns: they are the reasons competent graphics engineers and researchers make provenance, correctness and measurement part of the implementation rather than ceremonial documentation after development.

For **NES-10R** we extend the same project with four complementary artifacts: a public CMake usage-requirement interface and isolated presets; a deterministic C++23 correctness-and-measurement executable; a Python source/CTest evidence collector with fail-closed negative tests; and a four-lane GitHub Actions pipeline that is designed to preserve evidence. No PDF is compiled, no continuous manuscript is rewritten, no fake benchmark speedup is announced, and no Vulkan/Metal GPU work is claimed.

#### One build graph contains many different kinds of edges

The previous chapter's task graph encoded dependencies between CPU operations and conceptual graphics stages. CMake also constructs a graph, but its edges have their own meaning. A source file belongs to an executable or library target. A library must exist before a dependent executable can link successfully. A public header may require a language standard, compile definition, include search path or another target's transitive usage requirements. The generator lowers those declarations into instructions for Ninja, Make or an IDE's build system. An include appearing in a source file is not a substitute for declaring its required link target.

Our course's independent CMake root is `Book Production/Seven-Book Curriculum/Book01/CMakeLists.txt`. It now registers **ten** cumulative C++23 executables, starting with the iNES parser and ending with this evidence and profiling laboratory. The seventh and eighth executables must compile `cpu_debug_session.cpp` as a separate translation unit, because the public debugger uses a real PIMPL implementation with out-of-line operations. Compiling only the interface header could hide missing definitions until link time.

A new `INTERFACE` target named `nes_course_public_api` records the public include root and the C++23 language requirement. Such a target generates no separate object file. Instead, another target links it to receive *usage requirements*. The real C++ translation units and their source ownership remain explicit.

```cmake
# The INTERFACE target carries a contract, not a compiled library body.
add_library(nes_course_public_api INTERFACE)

# Public consumers can find nes_course/*.hpp.
target_include_directories(nes_course_public_api INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../tools/labs"
)
target_compile_features(nes_course_public_api INTERFACE cxx_std_23)

# One executable consumes the documented interface.
target_link_libraries(nes_reproducible_benchmark_cpp23_lab
    PRIVATE nes_course_public_api
)
```

The include-root expression above is the actual CMake source-directory-relative path used in `CMakeLists.txt`; it resolves to the repository’s `tools/labs` folder when this Book01 project is configured. The executable does not expose its own public API through that link, so `PRIVATE` is appropriate. If a library publicly exposes the types in another target's headers, the required include/feature dependencies may instead need `PUBLIC` or `INTERFACE` propagation. These CMake keywords are build-system visibility rules, not C++ access specifiers and not security barriers.

A public header must also provide its own standard-library dependencies. The new measurement header uses `std::convertible_to` in a template constraint and therefore includes `<concepts>` directly. It must not depend on another header accidentally having included that declaration. A clean consumer that compiles the header alone is a better test of this contract than a monolithic build in which every translation unit happens to include twenty unrelated headers first.

#### Why one mutable build directory is not a repeatable experiment

CMake remembers compiler paths, generator choices, build modes, feature flags and other configuration inputs in its build tree. Reusing an existing directory while switching a GCC Debug configuration to Clang Release or ASan may leave stale assumptions or cause configuration refusal. Separate build directories are not decorative; they isolate different *build identities*.

The new `Book01/CMakePresets.json` declares a Ninja Debug configuration, a Ninja Release configuration and a separate Clang AddressSanitizer/UndefinedBehaviorSanitizer configuration. Their output trees reside under `build/nes-course/`, away from the source manuscript. A Debug run is optimized for diagnostics; a Release run is the sensible starting point for actual performance measurement; sanitizer runs alter emitted instructions and memory behavior to catch selected error classes. None of these should be casually compared as though they represented the same code-generation environment.

A preset saying `clang++` does not pin an exact compiler binary or SDK. Linux Clang may use libstdc++; AppleClang on macOS generally uses a different system toolchain and standard library. Different versions can support different C++23 features and have different optimization decisions. The evidence collector therefore reads the actual CMakeCache compiler path, build type, generator and flags, while CI prints version information. A stricter bit-reproducible build also needs exact external SDK/compiler packages, dependency checksums and relevant environment inputs. **Reproducible source configuration**, **reproducible observable behavior** and **bit-identical final binaries** are three different goals.

The same distinction applies to the user's larger engine repositories. A project that builds in an existing developer worktree may rely on an untracked local header, a sibling folder, a cached shader binary, an installed driver, or a hand-edited configuration. A clean runner exposes those assumptions. The correct repair is to declare a requirement, pin a dependency or make an input explicit—not to modify the user's original long-form book until a build failure disappears.

#### The tool must not pronounce success from a source-code inventory

The new `tools/nes_course_evidence.py` begins with an explicit manifest of the ten executables and ten staged chapters. Its `audit` operation checks physical source presence, executable target declarations, named CTest registrations, lesson headings and the existence of required build tools, then computes SHA-256 for every declared source file and the original continuous manuscript. This is deliberately labeled `source_inventory_only_not_a_build_result`.

Why such an awkward label? Because source auditing is useful but cannot establish compilation. A correct repository manifest might point to code containing a missing include, a failed static assertion or undefined behavior. We refuse to convert an inventory count into “ten tests passed.” One of the strongest lessons from research methodology is to make every evidence label mean exactly the property actually observed.

A separate `collect` operation requires a real configured CMakeCache and a CTest JUnit XML file. It parses each executed test case, requires precisely the expected count, rejects repeated or missing names, rejects skipped, failed or errored tests, and checks that the names correspond to the currently declared CTest registration. Only after those checks can it write a source-pinned receipt containing the Git commit SHA, toolchain configuration, host information, test outcomes and source digests.

```text
Physical source inventory exists
           |
           v
Manifest/lesson/target audit               NOT a build result
           |
           v
CMake configuration writes a cache         configured, not compiled
           |
           v
Compiler and linker finish                 built, not necessarily correct
           |
           v
CTest executes every registered lab         executions may fail or skip
           |
           v
JUnit is complete: zero failures/errors/skips and exact names
           |
           v
Git SHA + source SHA-256 + CMake compiler configuration
           |
           v
Passing receipt for the tested source and this build environment
```

This flow is intentionally **fail-closed**. Missing XML is not evidence of zero failures. A skipped test is not equivalent to a pass. An old JUnit file with the wrong test names is not proof about the present target inventory. A malformed cache that does not identify the compiler cannot support a reproducibility claim. The collector writes the receipt via a temporary file and replace operation, not as a partially written success report if a later step fails.

Its `verify` mode recalculates the source hashes and Git commit and refuses the receipt if the current source differs. This is a versioning problem akin to our earlier asset generation handles: a handle from an older occupant must not silently designate a newly published asset, and a test receipt from an older source state must not silently certify newly edited code. The code does not establish tamper-proof cryptographic attestation of the CI service; its trusted inputs remain the build runner and its produced test file. The manifest also does not pin every external SDK or third-party library. Those limits must remain explicit.

#### Test the evidence collector's refusal paths, not just the happy path

A checker that accepts one properly formed file can still be dangerously wrong. If it ignores skipped tests, counts duplicate results twice or fails to compare source digests, it can manufacture a false sense of reliability. The new `tools/tests/test_nes_course_evidence.py` therefore uses isolated temporary JUnit fixtures to exercise negative cases.

One valid synthetic two-case file can be parsed **only as a parser fixture**. Asking that same file to certify ten expected tests must fail. A duplicated case name must fail. So must a single failed case, a skipped case, missing XML or malformed XML. The fixture also verifies that the source inventory's type is **not a build result**, that the CMakeCache parser requires an actual compiler, and that changing a stored SHA-256 invalidates a hypothetical record. A synthetic in-memory passing record used in this test is never exported as a real CI certificate.

The important rule is that a failed prerequisite blocks downstream claims. If CMake configuration fails, the pipeline cannot proceed as if compilation succeeded. If the compiler fails, a preexisting test XML cannot certify the new binary. If a sanitizer reports an error in the exercised path, this should be investigated rather than hidden behind a summary count that omits the sanitizer lane. If the documentation insertion finds two identical anchors, it must refuse to edit, not choose one arbitrarily.

This follows the same engineering principle as our iNES parser's bounded offsets, the asset registry's stale-generation rejection, and the GPU-memory reference model's uninitialized byte checks: **do not publish a value with a stronger meaning than the checks have established**.

#### Correctness and profiling are deliberately different modes

The tenth C++ executable, `nes_reproducible_benchmark_cpp23_lab.cpp`, has two modes. Its ordinary invocation is a deterministic correctness laboratory registered as the tenth CTest target. It checks stable results from four real preceding modules: owned NROM loading; a three-instruction CPU program; the CPU work dependency planner; and checked exclusive prefix scan. It also tests the measurement utility's quantile convention, refusal of zero trials/iterations, bound checks and rejection of inconsistent work results.

For the CPU fixture, the reset vector points to `0x8000` and the program begins with `LDA #$7F`, `STA $02` and `NOP`. Using the same previously built `Cpu6502Subset` and `CpuBus`, a correct run leaves accumulator `0x7F`, PC `0x8005`, the mirrored RAM byte `0x1802` equal to `0x7F` and a modeled instruction cost total of **seven cycles**. That cost is the partial emulator's modeled 6502 instruction count, not seven host nanoseconds or seven CPU clock cycles on the modern machine executing the test.

For the work-planner fixture, the lab constructs 32 tasks with deterministic read/write resource accesses and validates each output edge against the resulting topological order and earliest-finish/earliest-start constraints. The prefix scan handles a fixed 256-count sequence and compares each generated offset with an independent accumulation. Each run returns a numeric witness, and sixteen repeated correctness rounds require stable results. None of those checks proves all possible workloads correct. They do prevent the most obvious form of benchmark cheating: reporting an impressive time after accidentally removing the work whose result was supposed to be computed.

A separate `--benchmark` mode performs **actual host timing** only when explicitly invoked. It measures owned NROM decode, the CPU's three-instruction reset-and-execute operation, the 32-task graph plan and the 256-element scan. These are different operations with different allocation and verification costs. The report must retain their names rather than combining them into a nonsensical universal “NES speed” number.

#### A performance sample needs a mathematical definition

The `measurement_protocol.hpp` utility takes a declared number of warmups, timed trials and iterations per trial. It executes a workload that returns a deterministic 64-bit witness, mixes those witnesses into a per-trial checksum, and rejects trials that return inconsistent checksums under identical inputs. A compiler signal fence discourages trivially eliding all work but does not guarantee the optimizer cannot change the experiment. Anyone presenting a serious microbenchmark should still inspect generated machine code and compare independent correctness oracles.

The timer is `std::chrono::steady_clock`, and durations are represented as nanoseconds of elapsed host time. It reports minimum, median, nearest-rank 90th percentile, maximum and the work checksum. For sorted samples $t_0,\ldots,t_{n-1}$, the nearest-rank percentile uses the zero-based index

$$
i_p=\left\lceil\frac{pn}{100}\right\rceil-1
$$

for a nonempty sample sequence and the stated endpoint convention. With nine samples containing values 1 through 9, the median is 5 and the 90th percentile is 9. The self-test uses that known distribution rather than trusting an unexplained statistics helper.

A percentile is not a confidence interval, a minimum is not a CPU architectural lower bound, and a benchmark result is not universal across machines. Very small workloads may be dominated by timer overhead or the surrounding result validation. Memory allocation, data cache warmth, branch prediction, power scaling, background processes and thermal state can alter the observed samples. The protocol records them as observations, not as laws of nature.

Comparing a GCC Debug run with a Clang Release run is usually not an algorithm comparison. Comparing sanitizer-instrumented measurements with ordinary Release measurements is even less justified. For a meaningful optimization result, keep source identity, compiler, flags, input sizes, seeds, warmed state and measurement technique matched. Repeat across independent process invocations, report variance and inspect any cases where the output checksum changes. A proposed speedup is not valid if one version stopped checking asset generations, ignored missing graph dependencies or skipped memory-range validation to save time.

The reported time must also be attached to a precise semantic operation. Owned NROM import involves allocations and byte copying; cached lookup of an already loaded cartridge is a different API. The partial CPU executor includes reset and instruction handling; a lower-level opcode dispatch benchmark omitting reset answers another question. The graph benchmark includes topological planning and independent validation; a faster graph path that silently drops hazard edges is semantically different. It is entirely legitimate to measure these separate scenarios, but they must not masquerade as identical work.

#### Use the CI matrix as a failure map, not a decorative badge

The existing GitHub Actions workflow has been extended from two Linux compiler configurations to four evidence-producing lanes: GCC on Ubuntu 24.04; Clang on Ubuntu 24.04; AppleClang on macOS 15; and Clang under AddressSanitizer and UndefinedBehaviorSanitizer. The GCC lane also checks manuscript insertion idempotence and source integrity, runs the Python negative tests, and refuses to generate an evidence receipt unless all the CTest outcomes genuinely match the expected inventory.

The other lanes build the same cumulative C++23 executables and collect their own JUnit and source-pinned receipts. The macOS lane is specifically valuable because Linux Clang is not a substitute for AppleClang and its platform standard library. The sanitizer lane is designed to reveal selected address and undefined-behavior defects in the paths exercised by the ten labs; passing it would still not prove that there are no data races or GPU synchronization mistakes.

The workflow's path filters include the C++ sources, manuscript addition files, CMake configuration, Python evidence tooling and the workflow itself. Rapid chapter commits cancel superseded in-progress runs to reduce unnecessary CI work. The jobs have read-only repository permissions and can upload structured evidence artifacts after the required checks have succeeded.

**A workflow file is not evidence of a successful run.** GitHub Actions can be disabled, a runner can be unavailable, a job can fail during dependency setup, or an actual compiler error can prevent CTest from executing. Until an observed current run reports its status and artifacts, the honest statement is “CI configured, result not verified.” This distinction is required even if an earlier pass once compiled successfully.

The CI jobs also do not generate or compile LaTeX. They dry-run the strict insertion utility against the original continuous notes and check for unintended source changes. The user has reserved the final pull, manuscript application and comprehensive native verification for a consolidated checkpoint rather than between every revision pass. The workflow can run remotely on pushes without requiring any commands from the user now.

#### Original book integrity is a source-level contract

The canonical manuscript is not an expendable output of the new lab generator. The user expects the earlier thousands of pages of carefully developed material to remain present. Each new long-form lesson is therefore staged in `Supplementary/C++23 Project Labs/` and registered by an exact source anchor and unique marker. The insertion script refuses duplicate anchors, malformed markers, partial edits and conflicts. It verifies that removing only newly inserted blocks restores the earlier bytes of the manuscript.

The tenth addition is placed before the original heading `### REPRODUCIBILITY, CI, TESTING, DIAGNOSTICS, PROFILING AND THE COMPLETE ENGINE FLOW`. The book's existing introductory build-system and module narrative remains before it; its substantial preexisting CI/profiling discussion remains after it. That choice lets the learner transition from abstract build graphs to the specific ten-lab source-to-evidence system, then return to the original mature engine tooling without losing a paragraph.

The manifest and insert utility have different responsibilities. The manifest says which source, target and lesson files are expected and records their hashes. The insert utility ensures that additions do not silently replace source text. A later editor must still review the *meaning* of the new material: proof correctness, pedagogical flow, cited source versions, runnable code and whether any statements accidentally outrun the evidence. Passing a hash check does not establish scientific or mathematical truth.

The original full manuscript is still the source authority. A short LaTeX preview or a derived chapter edition is not authorized to overwrite it merely because those outputs are easier to edit. This is as important as the C++ strong failure guarantee: if a new improvement cannot be published safely, preserve the previous valid state and report the blocking condition.

#### What success would justify and what it would not

If a real CTest run eventually reports ten passing labs on GCC, it would establish that those test cases compiled, linked and ran as expected on that recorded GCC environment. A separate Clang or AppleClang result would add cross-toolchain evidence. A sanitizer run would add evidence for the instrumented error classes and paths. A source-pinned receipt would connect each result to the exact committed source and build configuration. That is substantial engineering progress and a sound foundation for further performance research.

It would **not** establish a fully accurate NES 2A03 CPU, PPU, APU, controller, DMA or renderer. It would not prove the conceptual GPU memory oracle implements Vulkan's memory model or Metal 4 queue synchronization. It would not certify that the Kairo, Maveb or Kyoto worktrees have been audited or rebuilt, nor that the seven-book mastery curriculum has reconciled every supplied book, paper or playlist. Each of those is a separate source-and-evidence task.

A student should be able to reconstruct this pass by creating the public CMake target, using a clean compiler configuration, compiling both sides of the PIMPL boundary, proving CTest names and results correspond to the expected targets, tampering with a synthetic JUnit file and watching the collector refuse a passing receipt, deriving an actual NROM/CPU/graph/scan workload checksum, and explaining why the benchmark's median under one environment cannot become a universal performance number.

The next important engineering discipline is **narrow claims**. If an artifact only contains configured build rules, describe configuration. If it contains successful compilation, describe a build. If it has complete JUnit results, describe those executed tests. If it has measured durations, report the exact samples and environment. A sophisticated research presentation is credible not because it uses more impressive adjectives, but because another engineer can trace every conclusion to a reproducible experiment.

**References for version-specific verification.** CMake's [target usage requirements](https://cmake.org/cmake/help/latest/manual/cmake-buildsystem.7.html), [presets](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html), [CTest reference](https://cmake.org/cmake/help/latest/manual/ctest.1.html), the [C++ steady-clock contract](https://en.cppreference.com/w/cpp/chrono/steady_clock), [LLVM AddressSanitizer](https://clang.llvm.org/docs/AddressSanitizer.html), [LLVM UndefinedBehaviorSanitizer](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html), and [Google Benchmark methodology](https://github.com/google/benchmark/blob/main/docs/user_guide.md) provide primary or established documentation for the tools discussed. Those sources specify semantics; they do not substitute for test output from this repository's current commit.

**Continue the original chapter.** We return immediately to “REPRODUCIBILITY, CI, TESTING, DIAGNOSTICS, PROFILING AND THE COMPLETE ENGINE FLOW,” retaining its extensive existing target, module, repository and diagnostic narrative. In Chapter 11, “C++ Const, Borrowing, and CPU–GPU Buffering,” the same CPU debugger, generational resource identity and conceptual device buffers will become an explicit const-correctness and lifetime-bound borrowing laboratory, with a route toward real multi-buffered graphics work.
