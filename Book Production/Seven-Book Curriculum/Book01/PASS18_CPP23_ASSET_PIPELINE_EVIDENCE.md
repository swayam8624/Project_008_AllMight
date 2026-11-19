# Pass 18 — Real C++23 asset ingestion, executable local tests, integration boundaries

**Authoring date:** 19 November 2026. **Branch:** `curriculum/cpp23-nes-project-pass01`.
**Goal:** Close the major worked-example gaps identified in Pass17 without
regenerating the full source manuscript or starting unrelated projects.

## Concrete additions

| Addition | Location | Engineering purpose |
| --- | --- | --- |
| Long-form integrated lesson | `Supplementary/C++23 Project Labs/CPP-18A - Filesystem Ranges Formatting and Coroutine Asset Import.md` | 42K+ characters teaching filesystem, bounded input, ranges/view borrowing, `std::format`/`std::print` feature checks, `co_yield`/`co_await`, frame suspension, cooperative cancellation, `std::jthread`, typed asset publication and cross-project reconstruction |
| Portable library | `tools/labs/foundations_course/modern_asset_ingestion.hpp` | Real filesystem reader, path-validation policy, binary size limits, owned CRC-like **noncryptographic** FNV-1a fingerprint, sorted ranges/views, move-only coroutine generator, manual suspend/resume task, host-threaded owned result aggregation and type-safe diagnostics |
| Adversarial executable | `tools/labs/modern_asset_ingestion_cpp23_lab.cpp` | Creates real temporary directory/files and valid TXR4/MSH1 inputs, refuses traversal/symlink/size/NUL errors, proves byte ownership across coroutine suspensions, stop-token cancellation, duplicate inputs and aggregate budget, then consumes original Pass05/06 codec/registry when include tree is available |
| Build/test registration | `Book Production/Seven-Book Curriculum/Book01/CMakeLists.txt` | A **17th** executable and CTest, with shared course public include interface and Threads::Threads; the normal full-course build makes the actual earlier generational registry header visible |
| Strict additive insertion | `tools/apply_cpp23_project_inserts.py` and `tools/tests/test_cpp23_project_inserts.py` | Stage marker-delimited CPP-18A immediately after the previous Pass06 registry supplement at the unchanged Chapter06 `From a living asset registry to reliable queries and transformations` anchor; verify source restoration, repeat application and conflict refusal |
| Source-pinned evidence + source audit | `tools/nes_course_evidence.py`, its tests, `tools/book01_coverage_audit.py` and its tests | Enforce 17 named CMake targets/CTest entries, previous 16 chapter-source segments, the newly staged Chapter06 supplement, required C++/lesson files, unchanged master and real JUnit requirement before declaring all-pass compilation |

**Important:** The master is not changed during staged authoring. Passing an
audit inventory means only that source entries exist and agree; it never
certifies compilation or an actual GPU result.

## Actual executed standalone test evidence

The authoring environment exposed these compilers:

- GCC **14.2.0** (Debian libstdc++) and Clang **17.0.0** with the available
  C++23 standard library. The `std::format`, `std::print` and ranges feature
  probes compiled and produced numeric feature-macro identifiers; the
  alternate fallback behavior remains for vendor libraries lacking support.
- A standalone source-equivalent Pass18 library/lab pair was built and
  executed successfully under **GCC 14 C++23** with full warning options
  (`-Wall -Wextra -Wpedantic -Wconversion -Werror`), and under
  **Clang 17 C++23** with the same warnings.
- The same standalone implementation passed an optimized **GCC Release**
  executable and a **Clang AddressSanitizer/UndefinedBehaviorSanitizer**
  executable with frame pointers, C++23 and the same warning checks.
- The final retest, after adding aggregate byte-budget enforcement, worker
  exception containment, NUL-path refusal and pre-worker budget validation,
  passed GCC optimized and Clang ASan+UBSan, with real temporary filesystem
  fixtures and owned coroutine/cancellation/worker tests.

The observed process success message was:

```text
Standalone source-only smoke: existing registry integration requires the course include directory; not tested here.
AllMight C++23 file/range/coroutine batch: 2 accepted assets
Pass18: filesystem/ranges/format/coroutines and host jthread checks passed; native sandbox TOCTOU defenses NOT proven.
```

This is **local standalone test evidence**, not a source-pinned
GitHub CTest/JUnit receipt. The local compile deliberately lacked the
`nes_course/generational_asset_registry.hpp` include path, so the optional
integration portion in the lab was **not executed in the local runs**.
The normal repository CMake includes `tools/labs` and therefore selects
that integration; actual successful compilation and runtime behavior of
the integrated code remains to be observed. The local authoring sources
were subsequently reproduced and committed through the connected GitHub
connector, so exact Git blob-equivalent compilation is also a separate
check. No result here is presented as an observed full 17-target build.

**Additional actual isolated CMake/CTest evidence:** A dedicated, local
throwaway CMake project was configured and built with Ninja and `g++`
(C++23, strict warnings), and with `clang++` (C++23, strict warnings,
ASan+UBSan). Both executed their single standalone CTest and produced
JUnit XML. Observed results:

```text
GCC Debug:           1/1 tests passed (0 failed)
Clang ASan+UBSan:    1/1 tests passed (0 failed)
```

This is **two standalone 1/1 CTest outcomes**, not the repository's
seventeen-target CTest or a source-pinned course JUnit acceptance receipt.
The local compiler also intentionally tried
`-DALLMIGHT_COURSE_REQUIRE_REGISTRY=1` without the earlier headers,
and **observed a compile-time #error**. This fail-closed condition
is now enabled by the real Book01 CMake target, meaning the full
course cannot advertise a passing test while silently skipping the
previous generational registry integration.

A GitHub Actions workflow already registers GCC, Clang, AppleClang and
sanitizer jobs plus strict source-pinned JUnit receipts; their successful
execution was not observed at this authoring checkpoint. No native
Vulkan/Metal/GPU workload was executed as part of Pass18.

## Contract, negative cases and deliberately unproved properties

| Contract | Test or source evidence | Limitation |
| --- | --- | --- |
| Relative filesystem path only | Reject absolute, `.`, `..`, empty, NUL-containing and observed symlink paths | `std::filesystem` status/canonical checks before `ifstream` are not race-free against a concurrent malicious path replacement |
| Bounded file reader | Fixed-size 4096-char stream chunks, 1 MiB default per-file cap, explicit size refusal and no prefix success | Does not authenticate content or validate actual PNG/glTF/KTX; TXR4/MSH1 domain decoder separately validates |
| Bounded result retention | Worker checks 8 MiB default aggregate accepted-byte budget, emits typed budget error, stops consuming | A large upstream manifest or huge quantity of error messages still needs an entry-count cap and bounded consumer queue |
| Owned coroutine frame | `Root` and `std::vector<Request>` passed by value; generator handle move-only, destructors destroy frame | Never resume a coroutine concurrently or after destruction; no full custom async event-loop implementation here |
| Correct view lifetime | `std::views::filter` used only while coroutine-owned request vector exists; `std::ranges::sort` uses member projection | Returning filter-view iterators after owner destruction would dangle; borrowed-range checking is not a complete lifetime verifier |
| Explicit co_await | Two-step `std::suspend_always` task with shared atomic state; manual resume and post-suspend cancellation | The awaiter does not schedule real I/O; cooperative stop cannot forcibly interrupt an active filesystem read |
| Actual host concurrency | `std::jthread` worker, stop token, mutex-protected result transfer, owned byte vectors | One generator resumer, not a lock-free executor or actual GPU compute queue |
| Exception containment | Worker catches unexpected exceptions, stores `std::exception_ptr` and rethrows from `finish()` | A caller must handle partial successes; intentionally no cross-asset transaction commit semantics |
| Format/print | Actual feature-gated `std::format` and `std::println` paths compiled locally with available standard library | Library feature differences on AppleClang/libc++ need actual Apple build; formatted strings do not automatically sanitize untrusted filenames |
| Typed asset publication | Lab explicitly calls original `GenerationalAssetRegistry::import<TextureTag/MeshTag>` and original `lookup/erase` operations | Full course include/build needed; local standalone test did not resolve previous headers and prints that limitation |
| Native renderer/physics/XR integration | Real project architecture informs names, constraints and reconstruction exercises | This pass does not patch or run actual Maveb/Kairo, import their large assets, implement Kyoto solver or measure XR display latency |

## Remaining full-book acceptance work

The whole **seventeen-target** GCC/Clang/macOS compilation, sanitizer matrix
and JUnit evidence must be run and failures repaired. Next, verify actual
typed registry integration and full binary fixture decode under the same
committed source revision. The original Chapter06 and its full manuscript
must remain intact while placing the 17th additive lesson.

For production, the loader must gain native race-resistant sandbox path
resolution, a bounded input/worker queue and a persistent asset-revision
commit protocol. An editor must not publish stale worker results into a
newly opened scene. The reader also needs final bridges/figures, cross-book
source ownership and a validated end-to-end capstone beyond these 17 labs.

**Do not reinterpret successful standalone tests as full book certification.**
This pass closes a genuine C++23 teaching hole and provides real, locally
exercised engineering code; it does not finish the entire seven-book project
or authorize a user-side pull/LaTeX regeneration.


## Pass18 resumed verification and bounded-manifest hardening

**Status:** Follow-up to the earlier Pass18 authoring work, on the same curriculum branch, with no canonical manuscript editing or LaTeX compilation.

The existing worker's aggregate byte budget had one meaningful limitation: a malicious or accidentally huge manifest containing many *failing* asset requests could still produce arbitrarily many lightweight error records, because errors consumed no successful payload bytes. The change adds an independent **request-count bound** (default 1024, configurable 1–4096) and a 4096-byte per-path limit. These are checked **before launching the `std::jthread` worker**, so a rejected manifest cannot begin asynchronously mutating receipt state. The existing per-file byte cap and total successful-payload budget remain separate.

The `Root::load` interface now accepts an optional `std::stop_token` and checks cancellation before path resolution, at each 4096-byte file-read iteration and before publication of the owned result. The coroutine passes its caller's token into the real reader. This can stop future chunk operations and refuses to publish a partially read success result. It cannot forcibly interrupt an OS filesystem syscall currently blocked inside `std::ifstream`, so it is not a hard real-time cancellation guarantee.

New deterministic tests cover an over-limit manifest, zero manifest capacity, overlong path rejection directly and through `WorkerBatch`, capped error records, pre-requested cancellation of a real file read, and preservation of path information in formatted diagnostics. Earlier typed asset fixtures, coroutine ownership and bounded byte budget tests still execute.

### Exact committed source identity of the resumed standalone test

| Source | Git blob SHA-1, verified identical to local build inputs |
| --- | --- |
| `tools/labs/foundations_course/modern_asset_ingestion.hpp` | `23a581956e35b8ffd6116e5483ff96a49e790be6` |
| `tools/labs/modern_asset_ingestion_cpp23_lab.cpp` | `28369c2bb8ac68d0c08b529b02152c6d5038c46c` |

These are **Git blob IDs** (SHA-1 over `blob LENGTH\0` and file bytes), not generic hashes of the raw files. The source hashes were compared against the exact local files immediately before a fresh rebuild.

### Executed local compiler and test evidence — 19 November 2026

| Source-verified run | Build flags / runner | Observed result |
| --- | --- | --- |
| Standalone GCC 14.2.0 | CMake + Ninja; C++23; `-Wall -Wextra -Wpedantic -Wconversion -Werror` | **1/1 CTest passed** after rebuilding exact committed source |
| Standalone Clang 17.0.0 | CMake + Ninja; C++23; same strict warnings; `-fsanitize=address,undefined` | **1/1 CTest passed** after rebuilding exact committed source |

Actual machine-readable standalone JUnit output was produced locally at
`/mnt/data/allmight_pass18/build-gcc/pass18-final-junit.xml` and
`/mnt/data/allmight_pass18/build-clang-asan/pass18-final-junit.xml`. These are **working-container paths**, not checked-in repository files or macOS hardware evidence. Both one-test suites reported zero failed and zero skipped tests.

**What was not exercised:** The standalone build did not have the existing `nes_course/generational_asset_registry.hpp` header in its include tree, and the executable **explicitly printed that the Pass06 registry import path was not run**. A negative compile check for `ALLMIGHT_COURSE_REQUIRE_REGISTRY=1` previously confirmed it refuses the missing shared header; this is correct fail-closed behavior, not proof the actual integration compiled. The complete **17-target Book01** CTest suite, actual shared codec/registry integration, AppleClang SDK compilation, native Metal/Vulkan execution, production race-resistant sandboxing, scene-revision transaction testing and real graphical output remain unverified. Do not present the standalone results as any of those.

### Release consequence

Pass18 closes real worked-example gaps in `std::filesystem`, file streams, standard ranges/views, `std::format`/`std::println` (feature-gated), `co_yield`, `co_await` and cooperative host work. The resumed hardening also closes an obvious unbounded-request/error-queue path. It does **not** yet finish Book 1. The next hard gate is to compile and run the **actual seventeen registered CMake targets with the real registry headers**, repair any compiler/CTest issues, and produce source-pinned JUnit receipts across supported toolchains. Separately, improve prose bridges and the seven-book ownership/routing map before release.
