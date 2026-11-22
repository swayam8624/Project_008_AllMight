# Project AllMight — the seven-book multi-project teaching spine

**Binding authoring policy, 19 November 2026:** An NES emulator is one valuable
long-running exercise in binary representation and state machines, **not the
sole project or master metaphor of the seven-book course**. Choose the
project whose real engineering requirements best teach the concept. Never
force Vulkan, GPU compute, numerical optimization, Gaussian splatting,
physics, machine learning, vision, XR or game-engine architecture into an
artificial NES example merely to preserve superficial continuity.

This is an **additive routing document**. Do not shorten, reconstruct or
rewrite the complete existing thousands-page manuscript. Preserve original
prose, explanations, derivations and referenced material. Stage source
additions until the user's consolidated application checkpoint.

## The project-selection and acceptance contract

Each chapter must identify its primary project and, when helpful, one
comparison project. Inspect the actual relevant source path and commit
revision; do not claim "source-audited" solely from a repo description or
from a prior assistant summary. Explain the underlying concept from first
principles with long-form prose, derivations, commented C++23 code,
representation, invariants and meaningful failure cases. Add an actual
CMake/CTest target and an independent correctness oracle. Report native
hardware tests, compiler executions and benchmark numbers **only** when
their evidence has been observed. Where a real project is unavailable,
label the proposed integration as a design and do not invent completed work.

Separate **source identity**, **correctness**, **lifetime**, **performance**,
**user experience**, **numerical accuracy** and **scientific reproducibility**.
These are different experiments even if one project needs all of them.

## Principal teaching projects

| Project / actual source evidence | Strongest recurring purpose | Progression |
| --- | --- | --- |
| `NESEmulator` and prior AllMight NROM/partial 6502 | Bits, CPU bus, memory, ownership, emulated time, debuggers, test oracles | Later PPU/APU/video/audio where explicitly relevant, not every chapter |
| `KairoRenderer` / `KairoGameEngine` | Native Vulkan objects, render queues, C++ modules, memory, images, descriptors, engine lifecycle, render graphs, editor | First actual Vulkan device/resource probe in Pass13; real draw and synchronization checks in later B6 |
| `Maveb` | Metal frame resources, camera/geometry, PBR, Gaussian rendering, mesh/TSDF reconstruction, uncertainty and numerical parity | GPU shader/resource audits, Gaussian pipeline compute, real device captures and reconstruction benchmarks |
| `KairoPhysicsEngine` / Kyoto collision broad phase | AABB, SAP/DBVT/Guard, temporal coherence, multi-material collision, narrow phase, exact pair oracle, simulation validity | Fair independent algorithm comparisons, pair buffers/scan and correct collision dynamics alongside performance |
| `KairoMath` / `KairoGeometry` / `KairoSIMD` / `KairoRayTracer` | C++23 numerical types, matrix/geometry, SIMD, sampling, ray tracing and independent high-accuracy reference | Integrate concrete source revisions when math, camera, path tracing and performance topics arise |
| `KairoGPU` | Backend-independent GPU computation with actual Metal and CPU fallback paths | Validate buffer owner, numerical CPU/GPU parity and capability-gated dispatch |
| `Vulkax` and XR / ILLIXR / foveation work | Scientific controls, graphics/physics uncertainty, gaze/vision, human-independent XR measurements | Read exact sources at corresponding milestones; do not infer runtime or benchmark results from project titles |

**Evidence boundary:** I inspected actual main-branch code in
KairoRenderer, Maveb, KairoPhysicsEngine and KairoGPU during Pass13. The
Kyoto common-contact *worktree* and ILLIXR VM were **not** fetched in this
pass; they are planned integration sources, not claimed inspected code.

## Book-to-project routing, B1–B7

**Book 1 — Bits, Programs and Reliable Systems.** Keep the real NES
parser/bus/interpreter for representation, ownership, state machines and
debugging, but use Kairo C++ module architecture, typed mesh/asset
pipelines, CMake, profiling, portable failure semantics and developer
tooling whenever they are the better setting. From Chapter13 the active
GPU device/resource lab is **cross-project**, not NES-only.

**Book 2 — Mathematics for Visual Computing.** Build an evolving C++23
numerical library rooted in KairoMath/KairoGeometry/KairoPhysicsMath:
vector and matrix algebra, stability, condition numbers, factorizations,
numerical differentiation, root finding, least squares, probability,
Monte Carlo and uncertainty; test against independent high-precision
references and use Maveb and Vulkax models as meaningful clients.

**Book 3 — Geometry, Cameras and Image Formation.** Grow a CPU
reference camera and rasterizer around Maveb geometry/reconstruction,
KairoGeometry predicates and KairoRayTracer. Prove coordinate-frame
transforms, projection, clipping, raster samples, robust geometry,
camera calibration, thin lens/exposure and inverse-problem residuals.

**Book 4 — Light Transport and Image Synthesis.** Use KairoRayTracer
and Maveb's render/pipeline work to implement BSDFs, sampling, Monte
Carlo integration, path tracing, material/lighting models and energy/
convergence tests. The NES emulator is not a path tracer.

**Book 5 — Spatial Worlds, Engine State and Simulation.** Use KairoPhysicsEngine
and Kyoto work to derive AABB/spatial indexes, SAP/DBVT/Guard, broad/narrow
phase, rigid/soft collisions, temporal coherence, scene lifecycle and
CPU/GPU numerical simulation. Preserve collision correctness, visual
penetration/folding metrics and exact pair oracles **alongside** throughput.

**Book 6 — GPU Architecture and Real-Time Rendering.** KairoRenderer is
the primary native Vulkan system and Maveb the contrasting native Metal
system. Cover device discovery, memory requirements, buffers/images,
resource views, descriptors, shader layout, SPIR-V/MSL, render graphs,
frame synchronization, Gaussian/TSDF and real GPU profiler evidence.
Capability-policy success must not be misrepresented as GPU execution.

**Book 7 — Vision, XR and Playable Worlds.** Use actual camera, reconstruction,
Vulkax and ILLIXR/gaze/foveation pipelines for visual tracking, XR
latency, neural rendering, dynamic scenes, human-independent tests and
rendered perceptual quality. Bring results into KairoEditor and game
runtime where concrete APIs exist.

This matrix does not bar other relevant user repos. KairoAssets,
KairoEditor, KairoECS, KairoReflection, KairoPipelineCore and future
source-provided projects can become the best project spine for particular
chapters after source inspection.

## Real code actually inspected in Pass13

| Inspected source, main branch | Exact observed Git blob | Grounded conclusion / open follow-up |
| --- | --- | --- |
| `KairoRenderer/VulkanInstance.cppm` | `d4ab482103d16b4b32a6ff213eae82a62c0af919` | Actual vkCreateInstance and instance-extension/layer enumeration |
| `KairoRenderer/VulkanDevice.cppm` | `6d3d46361496f021fda9f8253ac5c5684d1a251a` | Actual graphics/present selection, swapchain and independentBlend requirements; headless compute needs a different profile |
| `KairoRenderer/VulkanBuffer.cppm` | `5c277db2f0b5dfdae86d4ca8d2ba83fd7ef78afd` | Real HOST_VISIBLE|HOST_COHERENT buffer. Visible destructor unmaps, then frees memory, then destroys bound buffer: **targeted review required** |
| `KairoRenderer/VulkanTexture.cppm` | `a183d99eeeb520c746a1402455b4deea1cb3b164` | VkImage, device-local memory, staging/transition and correct image-before-memory destructor ordering |
| `KairoRenderer/VulkanRenderer.cppm` | `306bba0719948ecc4475a0de0112f15a00eee9a5` | Real surface acquire/submit/present flow; not compiled during this curriculum pass |
| `KairoRenderer/MetalRenderer.cppm` | `78f8dd55116b860b43af17838c4eea161528702c` | Separate Metal runtime, resource handles, presentation and validation |
| `Maveb/engine/metal/src/FrameContext.cpp` | `e610d5b2e4b0e467f4e93d3fed6a7b694a47a5e6` | 8 MiB shared upload, 256 KiB shared readback and 64 MiB private transient heap; inspect checked alignUp preconditions before editing |
| `Maveb/engine/metal/include/aether/metal/FrameContext.hpp` | `c0b9efbad52088ad89f5033510e6e1b8afd020c4` | BufferSlice owns no Metal buffer; carries buffer pointer, offset, bytes and CPU address |
| `Maveb/engine/gaussian/include/aether/gaussian/GaussianAsset.hpp` | `fa4a4f76f3479ec22c19a1c93432ab48321863db` | Actual Gaussian attribute representation and spherical-harmonic data, not an invented splat layout |
| `KairoPhysicsEngine/Broadphase.cppm` | `4c6a013f3f90e11916845d561501d3853472ad8b` | CPU broad-phase candidate-pair oracle and spatial proxy representation |
| `KairoGPU/README.md` | `a8c10ea572e20f0e3fd26d55e79ad40de729b71f` | Describes bounded Metal compute and CPU fallback; implementation not freshly compiled here |

No source modifications were made to those separate repositories.

## Immediate Pass13 milestone and evidence boundaries

`GPU-13D` is the first intentionally non-NES target in this pass sequence:
`tools/labs/gpu_course/device_resource_contracts.hpp` evaluates real-world
workload *requirements* against synthetic device snapshots with separate
graphics/compute/transfer/present and memory-type/heap constraints. The
`gpu_device_resource_cpp23_lab.cpp` C++23 correctness test covers Kairo
scenes, Maveb Gaussian compute, collision-pair buffers, XR gaze/render and
one NES overlay.

An optional `gpu_course/native_vulkan_probe.cpp` is compiled with the real
Vulkan SDK only if CMake finds it. The separate `--probe-native` path uses
real Vulkan calls to enumerate devices and queues, create a headless
VkDevice and VkBuffer, query memory requirements, allocate/bind VkDeviceMemory
and release resources in valid dependency order. Missing SDK/adapter returns
**unavailable**, not a falsely passing GPU result. Normal CTest remains
portable CPU tests and never executes this native path.

No PDF/LaTeX build, canonical manuscript replacement, existing external
repo edit, MoltenVK performance run, GPU shader execution or live device
capability claim is authorized or implied by this milestone.

**Future checkpoint:** Confirm exact source revisions, compile and run all
targets, materialize JUnit evidence, apply strict additive inserts without
replacing original text, and conduct independent device-specific validation.


## Pass14 Kairo/Maveb submission source ledger (read-only)

The Pass14 cross-project target follows this primary evidence trail. These are **actually fetched connected GitHub files**, with the observed main-branch Git blob IDs, not assumed versions on the user's Mac.

| Original repository source | Observed Git blob SHA | What the actual code shows / review action |
| --- | --- | --- |
| `KairoRenderer/VulkanCommand.cppm` | `ac131fdd82a6558f231de669622c41395bd8e249` | Creates one primary VkCommandBuffer from a resettable VkCommandPool; Begin resets before vkBeginCommandBuffer. Pool/buffer cannot be reset until prior submitted use completes |
| `KairoRenderer/VulkanSync.cppm` | `8945c4d5d9a343966e4ac853395672c52e542617` | Creates a single initially-signaled in-flight fence, an image-acquisition semaphore and one render-finished binary semaphore per swapchain image; reconstruction requires an explicit device-idle contract |
| `KairoRenderer/VulkanRenderer.cppm` | `306bba0719948ecc4475a0de0112f15a00eee9a5` | RenderGraph flows Retire(wait fence) → Acquire(image, **reset fence**) → Record → vkQueueSubmit → vkQueuePresentKHR. The reset occurs before recording/submission; an error between reset and accepted submission requires recovery or next-frame fence wait may block indefinitely. Source-level issue, **not device-reproduced** |
| `Maveb/engine/metal/src/Renderer.cpp` | `cfe9669de8475995f82166e71dc407d0b39b3331` | Uses 3-frame dispatch semaphore, frameContexts_, MTL command-buffer commit/completed handler, readback capture; destructor waits for permits. Callback captures this but destructor's drain means **not** a proven use-after-free; investigate permit balance on all early exits |
| `Maveb/engine/metal/src/FrameContext.cpp` | `e610d5b2e4b0e467f4e93d3fed6a7b694a47a5e6` | beginFrame executes deferred cleanup and resets upload/readback cursors; safe only after all old GPU uses of these slices finish; allocation explicitly checks alignment and a form of overflow |

**Teaching impact:** CPU model `gpu_course/submission_retirement.hpp` now isolates frame-slot generation, WSI image, swapchain generation, accepted/unknown submission state, unsignaled fence recovery and explicit resource generations with CPU pins. `gpu_submit_retire_cpp23_lab.cpp` tests the discovered Kairo failure path and the Maveb frame-lifetime obligation using synthetic event inputs. The optional separate native `gpu_course/native_vulkan_submit_probe.cpp` uses real Vulkan 1.0 vkCmdFillBuffer, vkCmdPipelineBarrier to HOST, vkQueueSubmit and vkWaitForFences plus byte-for-byte mapped results only when explicitly requested with a Vulkan SDK and actual device. No GPU result was executed or verified by this source audit.

**External write boundary:** Kairo, Maveb, KairoPhysics and Kyoto repositories were **not modified** by Pass14. The source observations above establish design requirements and review targets; implementation fixes in the external repos need dedicated test/build evidence and clear instruction to apply there.

**Next project progression:** Chapter15 should use a real Metal 3 command buffer, drawable, storage-mode/resource-lifetime cases and validated graphics/compute output from Maveb/Kairo's Metal backend, alongside a platform-independent CPU oracle. The CUDA/Vulkan/Metal comparison must respect actual feature and ABI differences, not mechanically map identical flag values across APIs.


## Pass15 — actual Maveb/Kairo Metal execution and measurement source ledger

This pass advances the seven-book curriculum's **multi-project** policy. NES remains
a CPU/data-representation project; native GPU execution and measurement are
primarily taught through **Maveb and Kairo Metal**, with a collision-pair GPU
compute oracle and XR latency analysis. The sources below were actually fetched
from the separate main-branch repositories in this Pass15 turn. No edits were
made to those external repositories.

| Inspected file | Git blob observed | Grounded observation / open question |
| --- | --- | --- |
| `Maveb/engine/metal/src/Renderer.cpp` | `cfe9669de8475995f82166e71dc407d0b39b3331` | Triple frameSemaphore_ permits; commandBuffer render/compute/presentation; frame capture using 256-byte row pitch and completed handler; GPUEndTime-GPUStartTime sample stored without explicit nonzero availability filter. Terminal error/capture success deserve independent reporting |
| `Maveb/engine/metal/src/FrameContext.cpp` | `e610d5b2e4b0e467f4e93d3fed6a7b694a47a5e6` | 8 MiB shared upload, 256 KiB shared readback, 64 MiB private transient heap; beginFrame deferred cleanup/cursor reuse; explicit power-of-two and cursor overflow checks before alignUp |
| `Maveb/engine/metal/include/aether/metal/FrameContext.hpp` | `c0b9efbad52088ad89f5033510e6e1b8afd020c4` | FrameContext and BufferSlice actual resource/CPU-address ownership boundary |
| `KairoRenderer/MetalRenderer.cppm` | `78f8dd55116b860b43af17838c4eea161528702c` | Renderer-neutral C++23 Metal runtime wrapper around detail::MetalBackend, viewport capture/pick, camera transforms and scene/debug data |
| `KairoRenderer/detail/MetalBackend.mm` | `5d291ebc02578fbff222fdf67aacf8d270a945b9` | Actual ARC ObjC++ native MTL device/queue and encode/commit/present; capture waits for LastCommand before copying offscreen image; different CPU stall/lifetime tradeoff than Maveb |

**GPU-15M lab files:** `tools/labs/gpu_course/metal_frame_contracts.hpp`
implements a portable single-threaded C++23 model of three in-flight permits,
alignment/capacity-checked frame upload cursors, terminal success/error,
separately reported CPU readback errors, 256-byte-pitch BGRA8 compact copies,
strict positive GPU timestamp quality, no false completion under unknown
submission/device loss, and no slot-generation wraparound. The
`tools/labs/gpu_metal_frame_cpp23_lab.cpp` tests use Gaussian, Kairo scene,
Kyoto collision-pair and XR gaze workload tags and synthetic CPU observations.

**Real native route:** `tools/labs/gpu_course/native_metal_compute_probe.mm`
compiles *original* MSL compute and rendering programs using an actual
MTLDevice when available. It dispatches a 32×32 AABB collision-pair mask,
checks every GPU result against an independently implemented CPU half-open
AABB oracle and guard words, then draws an actual 3×2 solid-red triangle to
a private BGRA8 render target and reads it back through a 256-byte-pitch
shared blit buffer. After actual `waitUntilCompleted`, it checks the
terminal MTLCommandBufferStatusCompleted and every pixel. A separate
opt-in native benchmark mode runs five warmups and thirty independently
correctness-checked samples, distinguishes host encode/commit from GPU
command interval and refuses missing/zero GPU timestamp percentiles.
**No native execution or benchmark has been observed in this authoring pass.**

**Build portability:** Only Apple Objective-C++/Metal/Foundation builds
select the native `.mm` adapter; otherwise `native_metal_unavailable.cpp`
returns explicit **77 / unavailable**. The default CTest is CPU-only and never
runs either `--probe-native` or `--benchmark-native`. A missing GPU, failed
MSL compile, unsupported storage mode or invalid pixel output is never
presented as a fake passing Metal result.

**Cross-project follow-ups:** Apply native frame-permit/failed-Metal-status
hardening to the external Maveb renderer only when actual source edits are
authorized. For Kairo, measure synchronous capture stalls separately from
normal rendering and avoid comparing Kairo's CPU blocking capture to Maveb's
async capture without controlling scene, camera and device. For Kyoto,
replace the 32-box correctness oracle with actual SAP/DBVT/Guard/ML outputs,
large datasets and fair competing baselines; do not claim that this native
all-pairs kernel is an optimized broad phase. XR needs compositor/display and
gaze timestamps, not only command-buffer GPU start/end.


## Pass16 — GPU profiling and asynchronous Apple ownership

**Project selection:** Maveb's production Metal renderer/callback code and
KairoRenderer's real Objective-C++ bridge are primary. A tiny original Metal
GPU compute shader is a native correctness witness. Kyoto/KairoPhysics and
XR are *distinct measurement clients*, not claims that their research
worktrees were compiled. NES remains the earlier CPU/binary project, not a
forced visual/profiling frame.

### Actual connected GitHub source (read-only)

| Repo file | Observed main-branch Git blob SHA | Concrete evidence |
| --- | --- | --- |
| `Maveb/engine/metal/src/Renderer.cpp` | `cfe9669de8475995f82166e71dc407d0b39b3331` | Triple dispatch-semaphore frame permits; camera/Gaussian/scene rendering; capture completed handler captures `this` and buffer, compacts 256-byte-pitch rows, then releases; other completed handler calculates GPUStart/GPUEnd difference, increments completedFrames_ and signals permit |
| `Maveb/engine/metal/src/FrameContext.cpp` | `e610d5b2e4b0e467f4e93d3fed6a7b694a47a5e6` | 8 MiB shared upload, 256 KiB shared readback, 64 MiB private heap; `beginFrame()` deferred cleanup and cursor reset; explicit invalid-alignment/overflow/capacity checks |
| `Maveb/engine/metal/include/aether/metal/FrameContext.hpp` | `c0b9efbad52088ad89f5033510e6e1b8afd020c4` | Concrete BufferSlice owner/borrow boundary and explicit FrameContext move-only management |
| `KairoRenderer/MetalRenderer.cppm` | `78f8dd55116b860b43af17838c4eea161528702c` | C++23 renderer-neutral façade, pick/capture, objects and graphic backend conversion |
| `KairoRenderer/detail/MetalBackend.mm` | `5d291ebc02578fbff222fdf67aacf8d270a945b9` | Objective-C++ Metal command queue, command-buffer commit, wait-for-last-command before pick/capture/destruction, tracked native backend ownership |

**Source-level review findings, not reproduced runtime faults:** The Maveb
completed-handler capture of `this` requires a complete permit-drain and
shutdown/callback accounting proof, not a categorical assertion of use-after-
free. Its destructor waits for three frame permits, which is meaningful
lifetime coordination. The stored GPU duration calculation needs careful
handling of command terminal error, zero/unavailable GPU timestamps and
source identity before it can be used as a paper-grade performance metric.
Kairo's synchronous capture/readback simplifies a lifetime proof but can
introduce CPU stalls; those were not measured by this source read.

### New cross-project implementation and limits

- `gpu_course/gpu_profile_ownership.hpp`: a host callback mailbox whose
  `TerminalCallback` captures shared *internal state*, never a raw renderer;
  explicit owner/slot/generation and submitted/unknown status, one mutex
  for exact at-most-once completion/permit return, generation exhaustion,
  optional valid GPU timestamps and a fail-closed native-profile evidence
  summary keyed by device, source/shader/workload, SDK, tool, resolution and
  correctness result. GPU counter values require actual support and units.
- `gpu_profile_ownership_cpp23_lab.cpp`: **actual host C++23 jthreads** test
  concurrent completion, duplicates, stale frame/foreign owner rejection,
  renderer/UI owner disposal before late callback, unknown submission
  quarantine, terminal errors and missing/invalid GPU timing. The
  performance data are SYNTHETIC fixtures, not measured hardware.
- `gpu_course/native_metal_callback_probe.mm`: optional actual Apple ARC
  Objective-C++23 Metal kernel and native `addCompletedHandler:` block,
  with strong internal receipt and weak UI observer. The test deliberately
  destroys the renderer-like wrapper and UI before native GPU submission,
  waits for actual terminal status/callback delivery, and independently
  compares 64 GPU-written words with CPU arithmetic. A **single** native
  GPU interval is diagnostic only, and invalid/missing GPU timestamps are
  explicitly unavailable. This is authored, **not executed** here.
- `gpu_course/native_metal_callback_unavailable.cpp`: code 77 if Metal
  toolchain/runtime is not selected. Default CTest is CPU-only and cannot
  certify native GPU, Xcode capture, counter sampling or rendering.
- `GPU-16P - GPU Profiling and Apple Callback Ownership.md`: 44,409
  characters of additive long-form explanation and project reconstruction
  before the original Chapter16 host-allocation section. The original
  source manuscript stays intact and source-pinned.

### Real profiling workflow that is NOT yet executed

Primary Apple documentation consulted this pass: MTLCommandBuffer terminal
statuses/completed handler, GPUStartTime/GPUEndTime after completion,
`MTLCaptureManager` and GPU counter support/sampling. Actual
hardware counters and Xcode traces must be captured on a particular
compatible Mac with the exact source/scene/shader build and repeated,
correctness-gated runs. No `.gputrace`, counter values, profiler screenshots,
Maveb run or Kyoto/ILLIXR benchmark was produced by the connected GitHub
source edits.

**Future integration:** Maveb needs a real callback/error/permit audit under
actual Metal debug validation, not blanket ownership accusations. Kairo
should compare synchronous capture overhead with a bounded optional
asynchronous capture on matched scenes. Kyoto should compare exact pair
sets and timing across its real baselines and not extrapolate a 64-word
compute witness to collision performance. XR needs observed gaze sample,
prediction, GPU, compositor and display clocks.

**User workflow:** Continue staged additive commits and do not request any
local pull/LaTeX commands until the chosen consolidated checkpoint.


## Pass18 — Real C++23 filesystem, range and coroutine entry to typed engine assets

**Multiple projects, one legitimate reusable contract:** This lab is
intentionally NOT a new NES-only feature. The carried-forward data
representation is the previously tested TXR4 texture and MSH1 mesh codec
with typed generational publication, but its consumer architecture is
**Kairo/Maveb-style asset ingestion**. A separate reconstruction exercise
maps it to Maveb mesh/Gaussian datasets, Kairo scene replacement, Kyoto
broad-phase dataset manifests, Vulkax simulation inputs and XR revision
changes. None of those external engines is claimed patched or benchmarked.

| Real source / concept inspected | Observed source identity / role | What is now implemented |
| --- | --- | --- |
| `tools/labs/nes_course/typed_asset_pipeline.hpp` | Same repository's Pass05 `Spec<TextureTag/MeshTag>::decode`, TXR4/MSH1 schemas | New real temporary files use the original decoder; no copied fake codec |
| `tools/labs/nes_course/generational_asset_registry.hpp` | Same repository's Pass06 `GenerationalAssetRegistry::import/lookup/erase` with owner+generation typed handles | Full course's 17th CTest requires this header; file bytes actually reach typed publication and stale handle refusal in its executable |
| `Maveb/engine/metal/src/Renderer.cpp` and `FrameContext.cpp` | Previously inspected source: actual staged frame/allocator lifetimes | Model and prose explain why borrowed file data and coroutine frames cannot own in-flight GPU uploads after their lifetimes end; no Maveb edits |
| `KairoRenderer/MetalRenderer.cppm` and `detail/MetalBackend.mm` | Previously inspected real C++23/ObjC++ renderer boundary | Target consumer for owned async asset imports, scene generation and UI delivery; no Kairo edits |
| `tools/labs/foundations_course/modern_asset_ingestion.hpp` | New Pass18 portable C++23 source | Bounded `std::ifstream`, lexical/canonical filesystem checks, explicit TOCTOU warning, `std::ranges::sort`/`views::filter`, `std::format`/`std::println` feature gating, `co_yield` and `co_await`, typed result and real `std::jthread` |

**Executed local evidence:** GCC 14.2 and Clang 17 compiled and ran
the new standalone module with C++23 warnings; optimized GCC and Clang
Address/UndefinedBehaviorSanitizer also passed. This separate local
workspace does not contain the original Pass05/06 header, so its output
explicitly marks that registry subtest **not exercised locally**.
The repository CMake target now has compile-time fail-closed
`ALLMIGHT_COURSE_REQUIRE_REGISTRY`; a local negative compiler check
confirmed missing registry is an error rather than fake passing CTest.
Full 17-target GitHub CTest/Apple builds remain unobserved. See
`Book01/PASS18_CPP23_ASSET_PIPELINE_EVIDENCE.md`.

**Security and cancellation caveats:** `std::filesystem::symlink_status`
followed by `canonical` then `ifstream` is not an atomic hostile-directory
sandbox. The lab rejects stable symlinks and path traversal but claims no
OS-level race-free security. The generator is a single-resumer owned
coroutine, while the separate worker creates a real host thread; its
`std::stop_token` is cooperative and cannot force an in-progress file
read to stop. The worker's aggregate byte cap and caught exceptions
avoid specific runaway failures, but job-count limits and a bounded
consumer channel are production follow-ups.

**Book sequence:** Book 1 gains missing C++23 standard-library breadth
without forcing GPU/physics problems into NES. Book 2 numerical Python,
Book 3 geometry, Book 5 collision and Book 6 GPU, Book 7 XR then consume
a well-defined artifact/job interface rather than reteaching file
ownership and coroutine lifetimes in each domain. Final source-routing
ownership still needs a dedicated editorial release pass.


## Pass19 — Cross-pass source-to-evidence capstone

The [CPP-19E lesson](../../Supplementary/C++23%20Project%20Labs/CPP-19E%20-%20A%20Complete%20Asset%20to%20Evidence%20Trace.md)
connects the previously implemented Pass18 filesystem and worker, Pass05
real TXR4/MSH1 codec, Pass06 owner-aware registry, Pass08 task DAG, and
Pass09 CPU memory visibility/retirement oracle in one integrated portable
C++23 executable: `tools/labs/book01_end_to_end_cpp23_lab.cpp`.
The six host tasks produce independently checkable weighted RGB sum 297
and signed twice-area 65,536, a reproducible source-identity receipt,
and rejection tests for malformed/missing inputs and stale scene revision.

This is a multi-project **foundation**, not an NES-only application or proof
that engines were modified. In Kairo/Maveb, real scene/renderer submission
and thread-safe scene lifetime replace the fixture's single-threaded gate.
In Kyoto, pair-set/solver oracles replace the texture reduction. Vulkax
requires controlled physical residual validation and independent splits,
while XR requires measured gaze/render/display clocks and deadlines.
None of those native/research verification steps happened in this pass.

Eighteen source insertions, CMake targets, CTests and evidence entries
are now authored. **No full 18-target CTest or native Mac GPU results are
claimed**; the canonical continuous master, original chapters, and LaTeX
remain unchanged until the user's consolidated checkpoint.
