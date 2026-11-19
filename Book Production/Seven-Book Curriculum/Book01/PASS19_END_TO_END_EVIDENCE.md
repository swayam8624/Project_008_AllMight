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

A deliberately malformed mesh, truncated texture and missing file must each fail at the correct layer. The scene publication fixture rejects stale scene revision 6 when current revision is 7, accepts revision 7 once, increments to 8, then refuses a second revision-7 result. This is single-threaded optimistic publication; it is **not** proof of thread-safe scene commit. CPU-modeled memory transitions do **not** prove Vulkan/Metal driver ordering.

## Source identity (Git blob values, not execution receipts)

The new capstone file was written to GitHub as Git blob `82327555df3c1dc08f92a14d36425fde9f1401fa`. The accompanying lesson is Git blob `d635dca71c0db6064c3196325a9d4f544bba570a`. These hashes identify committed text bytes; they do not establish successful compilation.

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
