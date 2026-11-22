# Quantized matvec and dequantization

Quantization trades representation precision for storage and bandwidth. The runtime question is not merely “how many bits per weight?” It is what mathematical value each stored code represents, where its scale or zero-point metadata lives, and how the dot product is accumulated without silently changing the model.

For a simple symmetric per-row signed 8-bit scheme,

$$
s = \frac{\max_i |w_i|}{127},
\qquad
q_i = \operatorname{round}\!\left(\frac{w_i}{s}\right),
$$

with clamping to the representable range. The reconstructed value is

$$
\hat w_i = s q_i.
$$

A reference matrix-vector product can therefore compute

$$
y = \sum_i (s q_i)x_i.
$$

That scalar implementation is not necessarily fast, but it creates an oracle for SIMD or GPU fused dequantization-and-dot kernels.

Before clamping, the scalar reconstruction error for ordinary round-to-nearest is bounded by roughly half a quantization step. The output error is workload-dependent because the dot product combines weight errors with input magnitudes. An accuracy report therefore needs representative activations or a model-level metric, not only weight reconstruction error.

## Zero rows and exceptional values

When all weights in a block are zero, dividing by the maximum absolute value would produce an invalid scale. The format and runtime need an explicit zero-block representation. Non-finite input weights should be rejected or handled by a documented conversion policy instead of entering integer casts accidentally.

Groupwise 4-bit formats add another trade-off. Smaller groups let the scale adapt more locally but spend more bytes on metadata and may increase metadata bandwidth. Larger groups reduce metadata overhead but make each scale describe a wider distribution.

## Reference versus optimized execution

tools/labs/phase29_ml_runtime_lab.cpp implements a checked per-row Q8 reference and compares its dot product against the original floating-point row under a stated error budget. The point is not to advertise Q8 as the final format; it is to establish the proof pattern:

stored codes plus metadata → scalar interpretation → reference result → optimized kernel → measured error and performance.

A Metal or SIMD kernel should be judged against that reference on adversarial widths, tails, zero blocks, extreme scales and random vectors. SIMD lane packing is an implementation detail; the dequantized mathematical contract remains the same.

Related: [[Quantization and compressed weights]], [[GGUF binary model loading]], [[Tensor runtime layout and views]].
