#include <metal_stdlib>
using namespace metal;

// Compile-time Metal witness. Runtime hazard/visibility behavior requires a real Metal execution capture.
kernel void allmight_prefix_offset(
    device uint* values [[buffer(0)]],
    constant uint& offset [[buffer(1)]],
    uint id [[thread_position_in_grid]]) {
    values[id] += offset;
}

// Native compressed-inference witness. One invocation performs a small q8 dequantized dot;
// this is correctness evidence, not a claim about an optimized SIMD/threadgroup kernel.
kernel void allmight_q8_dot(
    device const char* qweights [[buffer(0)]],
    device const float* input [[buffer(1)]],
    constant uint& count [[buffer(2)]],
    constant float& scale [[buffer(3)]],
    device float* output [[buffer(4)]],
    uint id [[thread_position_in_grid]]) {
    if (id != 0) {
        return;
    }

    float sum = 0.0f;
    for (uint i = 0; i < count; ++i) {
        sum += float(qweights[i]) * scale * input[i];
    }
    output[0] = sum;
}

