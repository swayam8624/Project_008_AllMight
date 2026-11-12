# Native GPU evidence protocol

A CPU model can prove algorithmic logic, but it cannot prove what a GPU API, driver or device actually did. PROJECTALLMIGHT therefore treats GPU evidence as a separate layer.

## Evidence tiers

**Specification evidence.** A claim is tied to the normative or vendor API contract.

**Compile evidence.** Real GLSL/HLSL/MSL/SPIR-V or host API code is accepted by the current toolchain.

**Validation evidence.** Vulkan validation layers or equivalent diagnostics observe the real API calls without reported contract violations.

**Execution evidence.** Work is submitted to an implementation, completes, and produces inspectable output.

**Data-parity evidence.** GPU output is compared to an independent CPU/reference result with exact or quantitative error metrics.

**Capture/counter evidence.** GPU debugger/profiler capture shows resource state, bandwidth, occupancy/timing or other hardware behavior relevant to the claim.

**Production evidence.** The same contract is verified in the current engine/research repository revision.

Do not promote compile evidence to execution evidence or software-Vulkan execution to physical-GPU performance evidence.

## Current branch

.github/workflows/native-graphics-contract.yml adds two concrete checks:

- a Vulkan 1.3 synchronization2 smoke program that records vkCmdPipelineBarrier2, submits with vkQueueSubmit2 and fails on validation errors when the layer is available;
- a macOS Metal job that compiles a real MSL kernel into a metallib.

These checks intentionally print their boundaries. The next milestones are resource-specific Vulkan transitions and a runnable Metal command-buffer witness on Apple GPU hardware.
