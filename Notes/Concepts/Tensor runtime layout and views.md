# Tensor runtime layout and views

A tensor runtime does not begin with neural-network vocabulary. It begins with a byte-addressing contract. A tensor value needs an element type, a rank, a shape, a mapping from logical indices to storage offsets, and an ownership/lifetime rule for the backing storage. If any of those are implicit, later operations such as broadcasting, transposition, slicing, matrix multiplication, device transfer, or quantized kernels can silently disagree about what the same bytes mean.

For a contiguous row-major tensor with shape

$$
(d_0,d_1,\ldots,d_{r-1}),
$$

the canonical stride of axis $i$ is

$$
s_i = \prod_{j=i+1}^{r-1} d_j,
$$

so an index $(i_0,\ldots,i_{r-1})$ addresses

$$
\operatorname{offset}=\sum_{k=0}^{r-1} i_k s_k.
$$

Shape describes the logical domain while strides describe the storage map. A transposed or sliced view may have the same elements without being contiguous. A runtime that assumes offset = row × width + column for every two-dimensional tensor will eventually misread a view.

A rank-zero tensor is a scalar and has shape []; it is not the same object shape as a rank-one tensor of shape [1]. A zero-sized dimension is also meaningful: shape [4,0,8] has zero elements but still has rank three. These distinctions matter when importing external graph formats because shape information is part of the interface contract rather than decoration.

## Broadcasting is an index transformation

Broadcasting does not physically duplicate the smaller tensor. Align the input and output shapes at the trailing axes. For every aligned axis, the input size must either equal the output size or equal one. If the input size is one, every output index on that axis maps to input index zero.

For a bias of shape [3] applied to a matrix [2,3], output coordinates across both rows map back to bias indices 0,1,2 repeatedly. This is why some runtimes can represent a broadcast axis with a zero stride: advancing the logical output coordinate does not advance the input address.

## Views, aliases, and mutation

A view is cheap because it shares storage. That also means it shares lifetime and mutation hazards. A runtime must decide whether an operation accepts arbitrary strided inputs, requests a contiguous materialization, or dispatches to separate kernels. Reshape is metadata-only only when the requested logical order can be represented by the existing storage arrangement; otherwise it requires a copy.

The same question appears on the GPU. A kernel receiving shape and stride metadata can support arbitrary views, but the extra integer arithmetic and irregular memory access can cost more than making a compact copy. The correct choice is workload-dependent and should be measured.

## Executable reconstruction

tools/labs/phase29_ml_runtime_lab.cpp contains a standalone C++20 tensor representation with checked element counts, contiguous strides, multi-index addressing, and trailing-axis broadcasting. Its negative test rejects incompatible shapes rather than allowing an accidental indexing interpretation.

The witness is intentionally small enough to rebuild from memory. It does not claim to implement a production tensor allocator, autograd engine, sparse tensor representation, or GPU device-memory manager.

## External contract

The ONNX IR specification defines tensors by values plus shape, distinguishes scalar shape [] from a one-element vector, and requires top-level graph inputs and outputs to carry rank information even when individual dimensions are symbolic. When an ONNX model is imported, the internal tensor representation must preserve those distinctions instead of flattening everything into an untyped byte blob.

Primary reference: https://onnx.ai/onnx/repo-docs/IR.html

## Failure probes

A tensor-runtime implementation should deliberately test integer overflow while multiplying dimensions, out-of-range indexing, zero-sized dimensions, rank-zero scalars, incompatible broadcasting, views whose strides are not contiguous, aliased mutation, negative or unsupported strides if the API exposes them, and CPU/GPU disagreement over element size or alignment.

The central reconstruction question is: given only shape, strides, element size and an index tuple, can you prove which byte is being accessed and whether that byte remains alive?

Related: [[Shader buffer ABI]], [[Quantization and compressed weights]], [[ONNX graph import and lowering]], [[Transformer attention and KV cache runtime]].
