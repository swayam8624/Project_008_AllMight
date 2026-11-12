# Tensor shapes, strides, views and broadcasting

A tensor runtime cannot treat shape as a synonym for storage layout. A tensor needs an element type, rank, shape, strides, storage identity, offset and lifetime. For a row-major contiguous shape $(d_0,\ldots,d_{r-1})$, strides satisfy $s_{r-1}=1$ and $s_i=d_{i+1}s_{i+1}$, so an index maps to $o=\sum_i i_i s_i$.

A **view** changes interpretation without necessarily moving bytes. Transposes permute shape/stride metadata; slices change offsets and possibly strides; broadcasts make one stored value serve many logical coordinates. That means kernels must either be stride-aware or explicitly require/materialize contiguity.

Broadcast dimensions align from the trailing side and are compatible when equal or when one side is one. The output dimension is the maximum. Broadcasting should usually be a read-only logical view because writing through a repeated dimension can alias many logical coordinates onto one physical element.

Before allocating, checked-multiply dimensions and element size. A wrapped shape product is a memory-safety bug, not merely a numerical mistake.

**Executable witness:** [[Supplementary/Phase 29 - ML Runtime Reconstruction]] and `tools/labs/phase29_ml_runtime_lab.cpp`.

**Used by:** quantized matvec, ONNX lowering, attention, KV caches, GPU tensor kernels.

**Next depth:** non-contiguous slicing, zero-stride broadcasting, ownership-safe views, vectorized kernels and memory planning.
