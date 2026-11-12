# Shader buffer ABI

A shader buffer is a byte-level interface with explicit offsets, strides, widths and layout rules. Alignment is not payload size. Verify the compiled shader layout and the CPU representation together.

Teach and reconstruct: [[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#Make a shader record an explicit binary contract]].

Related integration record: [[Supplementary/Manual Integration - Sources and Verification]].

## Mastery expansion — shader data is a binary interface

A CPU struct and shader struct that look textually similar are not guaranteed to share offsets, alignment, padding or matrix layout. The ABI contract must state the shader language/layout rules, host-side representation, field offsets, total size, array stride, matrix orientation/stride and resource binding identity.

Do not serialize a native C++ struct merely because `sizeof` seems plausible. Padding bytes, bool width, enum width and compiler-specific layout can make the representation unstable.

For cross-backend data, define one canonical packed record and generate or assert both sides where possible. At minimum, use compile-time `static_assert` checks for host offsets/sizes and shader reflection or validation for the compiled shader interface.

Matrices require special care: storage order, mathematical multiplication convention and shader matrix-major layout are separate decisions.

**Required next witness:** one record shared between CPU, Vulkan shader and Metal shader with reflected/declared offsets, deliberate mismatch negative test and frame-capture inspection.

