# Pass 19 — Integrated Book01 CPU trace and evidence boundary

**Date:** 19 November 2026. **Branch:** `curriculum/cpp23-nes-project-pass01`.

## Purpose and preservation

Pass 19 answers the main Pass 17 P0.3 open requirement: connect real previously implemented subsystems into one meaningful, source-auditable portable acceptance trace. This is **an authored integrated executable plus additive textbook lesson, not yet a completed compiler or CTest certification**. The original canonical continuous manuscript remains unchanged. No LaTeX regeneration or compilation has been requested or performed. The earlier 5,000+ pages are not replaced.

The authored executable is `tools/labs/book01_end_to_end_cpp23_lab.cpp`; the authored lesson is `Supplementary/C++23 Project Labs/CPP-19E - A Complete Asset to Evidence Trace.md`. Both are registered under the existing Book01 curriculum workflow. The 18th CMake executable and 18th CTest registration are `book01_end_to_end_cpp23_lab` and `cpp_19e_end_to_end_asset_evidence_cpu_trace`.

## Existing code reused rather than cloned

| Source implementation | Exact role in the new capstone |
| --- | --- |
| `foundations_course/modern_asset_ingestion.hpp` (Pass 18) | `Root` and `WorkerBatch`: read actual bounded temporary TXR4/MSH1 files, own result bytes, apply worker budgets |
| `nes_course/typed_asset_pipeline.hpp` (Pass 05) | Original `Spec<TextureTag>::decode` and `Spec<MeshTag>::decode`; malformed data fails before scene staging |
| `nes_course/generational_asset_registry.hpp` (Pass 06) | Owner/slot/generation handles, typed lookup, staged import, erase and stale-handle rejection |
| `nes_course/work_dependency_graph.hpp` (Pass 08) | Six real host tasks with read/write resource keys, planned topological order and independent edge verification |
| `nes_course/gpu_memory_contract_oracle.hpp` (Pass 09) | Actual CPU model tests missing visibility, explicit memory dependency, completion-pending retirement and completed retirement |

The TXR4 and MSH1 files are teaching fixtures, not production graphics formats. The workload's weighted RGBA8 RGB sum should equal **297**; the triangle's signed twice-area in squared 1/256 quantization units should equal **65536**. Repeated run receipts must compare byte-for-byte even though the ephemeral filesystem root and process-local registry owner IDs vary.

The fixture's independently derived 64-bit FNV-1a values are **8952040735453606912** for the 20-byte TXR4 file and **9136700281888616565** for the 36-byte MSH1 file. The new metamorphic checks change alpha without changing the RGB sum, and reverse triangle winding to reverse the signed area while changing source identity. A deliberately malformed mesh, truncated texture and missing file must each fail at the correct layer. The scene publication fixture rejects stale scene revision 6 when current revision is 7, accepts revision 7 once, increments to 8, then refuses a second revision-7 result. This is single-threaded optimistic publication; it is **not** proof of thread-safe scene commit. CPU-modeled memory transitions do **not** prove Vulkan/Metal driver ordering.

## Source identity (Git blob values, not execution receipts)

The current Pass19 capstone source is Git blob `44bf32b451041d7924a0f107502ff254eec602f4` (434 lines), following further independent metamorphic tests. The accompanying lesson is Git blob `d635dca71c0db6064c3196325a9d4f544bba570a`. These hashes identify committed text bytes; they do not establish successful compilation.

At the time of this source inspection, the CMake file declares **18** `add_executable` targets and **18** named `add_test` calls; the insertion script declares **18** marker-delimited source additions; and the provenance manifest declares **18** source/test pairs. These are source inventory facts only. Test wiring preserves the original first 16 chapter assignments, adds Pass 18 to original Chapter 06, and maps Pass 19 to original Chapter 10 at the unique `REPRODUCIBILITY, CI...` heading. The same heading also anchors earlier Pass 10 content; the insertion script is explicitly ordered and refuses altered prior markers.

The source preservation fixture now includes an eighteenth insertion and checks that stripping only marked additions recovers the **exact original source bytes**. The coverage and evidence unit tests expect 18 declarations, not 17. Until those tests execute, this remains an authored source-level contract.

## Status — what was and was not executed

| Check | Status |
| --- | --- |
| Inspect Pass 05, 06, 08, 09 and 18 public interfaces | Completed via connected repository file inspection |
| Write new integrated C++ source | Completed and committed |
| Write continuous chapter lesson, derivations and exercises | Completed and committed |
| Register CMake/CTest, source-pinned provenance, additive insertion | Authored; source counts cross-checked (18 each) |
| Run new executable against the complete repo headers | **Not executed in this environment** |
| Run complete 18-target GCC/Clang/AppleClang build | **Not executed** |
| Run AddressSanitizer/UBSan and source-pinned JUnit tests for all 18 | **Not executed** |
| Execute `tools/apply_cpp23_project_inserts.py --apply` on the canonical manuscript | **Not executed — deferred by user instruction** |
| Run LaTeX build/export | **Not executed — deferred by user instruction** |
| Actual Metal/Vulkan device work, GPU timing, concurrent editor publication | **Not implemented or verified by this portable CPU pass** |

Source commits and documentation should not be described as an observed successful full build. The critical immediate next action at the user-requested checkpoint is to run the source-pinned 18-target full compiler matrix, inspect its genuine CTest/JUnit reports, repair integration errors without deleting prior material, and independently verify the exact byte-preserving insertion and the final narrative layout. The original notes and derivations remain authoritative.

## Defects and research limitations still outstanding

The fixture checks deterministic content and source fingerprints, but its FNV-1a checksum is not cryptographic. Production release should use strong artifact digests and source provenance. The original Pass 18 path checks are not a race-free hostile-filesystem sandbox. Production async import additionally needs a bounded persistent job queue, cancellation policy under blocked I/O, actor/mutex-protected scene publication and safe lifetime transfer into the actual renderer. The memory oracle checks a conceptual model rather than a native GPU queue. Research data products must add exact numerical or geometric independent oracles, dataset validation splits, and real measured performance/effect sizes.

**Book 01 is not declared complete by the number 19.** The pass provides a connected, testable capstone and an honest missing-verification ledger for the later controlled checkpoint.

## Pass 20 — Actual multi-toolchain repair and evidence (19 November 2026)

The initial C++23 end-to-end program is no longer merely an authored source:
[isolated full-header Ubuntu CMake/CTest run 44473542774](https://github.com/swayam8624/Project_008_AllMight/actions/runs/44473542774)
compiled the real source and executed its one targeted test successfully. The
later [run 44474079815](https://github.com/swayam8624/Project_008_AllMight/actions/runs/44474079815)
compiled and passed again after adding the eight-worker mutex-protected scene
publication and its revision-overflow/exception-path checks. These are **one
capstone test per run**, not 18 complete labs or an observed ThreadSanitizer
proof.

### The source of the original zero-job CI failures

For the original four-job workflow, GitHub recorded immediate failed runs
with zero jobs and no compiler logs. An independent executable smoke workflow
proved runner availability. We ran `actionlint` in a genuine GitHub job and
observed a specific invalid path-filter glob at line 16:
`Supplementary/C++23 Project Labs/**`. Bare `+` is a special glob operator;
the filter was not valid GitHub Actions pattern syntax. It was replaced with
the legal `Supplementary/**`, after which GitHub actually scheduled the
full GCC, Clang, macOS and sanitizer jobs. The failed zero-job runs preceding
this fix were **not compiler failures**.

### Real compiler and test failures exposed once CI could start

- The legacy dependency-graph NROM lab attempted
  `std::move(*parsed)` from a `const std::expected<NromCartridge,...>`.
  The contained cartridge is move-only; const made the attempted move
  resolve to a deleted copy. Changed the local result to non-const
  `auto parsed` with explanatory source comments, preserving ownership.
- A separate GPU submission/retirement test and the Metal-frame reference
  used `check(bool,...)` directly with `std::expected<void,Error>`.
  `std::expected::operator bool` is explicit. They now use typed
  `std::expected<T,E>` overloads asking `has_value()`.
- The optional Objective-C++ Metal test had braced `uint32_t` initializers
  combining `uint32_t` coordinates with `std::size_t` modulo offsets.
  AppleClang rejected the narrowing. The tiny modulo operands are now
  explicitly converted after reasoning from the bounded 32-box test size.
- Ubuntu Clang 18 combined with the runner's libstdc++ exposed no usable
  `std::expected`, because its Concepts feature level failed the library's
  availability gate. Linux Clang and Clang-ASan jobs now select Clang 19
  and compile a preflight `std::expected` fixture. They do not redefine
  standard feature-test macros to manufacture support.
- The macOS 15/Xcode 16.4 environment lacked `std::stop_token` and
  `std::jthread`; neither was skipped or replaced by a fake stub.
  The Apple job now targets the macOS 26 image and has an explicit
  C++23 `std::jthread`/`std::stop_token` compile-time preflight.

### Observed suite receipts, source-revision qualification

In [full-matrix run 44474946935](https://github.com/swayam8624/Project_008_AllMight/actions/runs/44474946935),
from commit `0697e7e181f7d7a0e1c71d53f1f598bf52930c03`,
the **GCC Debug job passed all 18 CTests**, the three Python unit-test
groups (6, 8 and 5 tests), the additive original-manuscript integrity audit,
and the source-pinned evidence collector/verification. The **Clang 19
ASan+UBSan job also completed successfully** under that commit.
The AppleClang job for that revision still used Xcode 16.4 and failed for
missing standard library APIs, so this was **not an all-platform green run**.
A later amendment switched the Apple toolchain to macOS 26 and added a
compiler preflight, which requires an actual newer run before declaring
macOS verification.

When source or documentation changes after a test, its Git SHA/source hashes
change. Prior green outcomes remain trustworthy historical evidence for the
exact commit stated but are **not certificates for any later HEAD**.
A release declaration requires all four jobs and their fail-closed evidence
collectors to complete successfully at the same final commit. GitHub
workflow runs are linked here rather than inventing locally executed results.

### What the new concurrency lesson actually proves

The existing `CPP-19E` staged lesson has a new long-form Pass20 section,
which derives the mutex-protected revision/receipt swap as one linearizable
publication, including no-throw move/swap constraints, a saturated revision
refusal, owned snapshots, a start-gate guard for jthread-construction failure,
and an eight-contender exactly-one-winner test. The numeric fixtures also
have independent full-file FNV-1a expectations plus alpha-only/winding-flip
metamorphic changes. An observed passing CTest establishes **tested behavior
on the executed run**. It is not a proof against untested interleavings,
not a ThreadSanitizer finding, and not synchronization evidence for actual
Metal or Vulkan GPU resource retirement.

**Source-preservation invariant:** The original continuous master was never
edited, shortened, reassembled, or compiled into LaTeX. The full curriculum
still has 16 originally mapped reading chapters in Book01, 18 staged exact
additive lessons/test targets, and 18 registered default CTests. Pass20
deepened the already staged CPP-19E lesson and repaired test sources; it
did not invent a nineteenth original chapter or replace the existing book.
