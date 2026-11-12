#include <metal_stdlib>
using namespace metal;

// Compile-time Metal witness. Runtime hazard/visibility behavior requires a real Metal execution capture.
kernel void allmight_prefix_offset(
    device uint* values [[buffer(0)]],
    constant uint& offset [[buffer(1)]],
    uint id [[thread_position_in_grid]]) {
    values[id] += offset;
}
