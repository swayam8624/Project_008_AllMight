# Phase 29 — From Model Bytes to a Running Transformer

This chapter closes the largest explicit gap in the current PROJECTALLMIGHT corpus. It is not a survey of machine-learning vocabulary. The objective is to make a small inference runtime reconstructable from memory: a model file becomes validated metadata and tensor views; tensor views become scheduled kernels; quantized weights become arithmetic; operators become a graph; and the graph ultimately becomes one causal transformer step with a persistent key/value cache.

The companion executable witness is `tools/labs/phase29_ml_runtime_lab.cpp`. That program intentionally stays dependency-free and CPU-only so that the contracts can be inspected without an ML framework. It proves selected shape, numerical, file-header, graph-order and attention invariants. It does **not** claim compatibility with every GGUF quantization type, complete ONNX operator coverage, tokenizer behavior, production Metal kernels, or current Nanoquant/Kairo runtime parity.

## 1. A tensor is storage plus an interpretation contract

Calling a tensor a “multidimensional array” is useful but incomplete. A runtime tensor needs at least an element type, rank, shape, strides, storage owner or storage view, byte offset, and enough lifetime information to know whether the referenced storage remains valid.

For a row-major contiguous tensor with shape

$$
S=(d_0,d_1,ldots,d_{r-1}),
$$

the number of scalar elements is

$$
N=\prod_{i=0}^{r-1} d_i.
$$

Its element strides, measured in elements rather than bytes, are

$$
s_{r-1}=1,\qquad s_i=d_{i+1}s_{i+1}.
$$

An index (I=(i_0,ldots,i_{r-1})) maps to the linear element offset

$$
o(I)=\sum_{k=0}^{r-1} i_k s_k.
$$

The important engineering point is that **shape does not imply contiguity**. A transpose can often be represented by changing shape and strides without moving any elements. A slice may have a nonzero base offset. A broadcast view may contain a stride of zero in a conceptual implementation because many logical indices refer to the same stored scalar. Therefore an operator that assumes contiguous rows must either reject a non-contiguous view, materialize it, or run a stride-aware kernel.

The companion lab uses only owned contiguous storage because it is the smallest trustworthy oracle. A production runtime should split `TensorStorage` from `TensorView`: storage owns bytes; a view owns shape/stride/offset metadata and borrows the storage.

### Overflow is part of the tensor contract

Before allocating (\prod d_i) elements, the product must be checked for integer overflow. Otherwise a malicious or corrupted model can request a shape whose product wraps to a small allocation and then drives out-of-bounds indexing. The same rule applies when converting element counts to byte counts.

The correct order is: validate rank and dimensions, checked-multiply dimensions, checked-multiply by element size, validate against runtime limits, then allocate or map.

## 2. Broadcasting is an indexing rule, not free replication

Broadcasting aligns dimensions from the trailing side. Two aligned dimensions are compatible when they are equal or one of them is one. The output dimension is their maximum.

Thus

$$
(4,1,8)quad\text{and}quad(1,7,8)
$$

broadcast to

$$
(4,7,8).
$$

No (4\times7\times8) copy of the second operand is required. The kernel maps an output coordinate back to each input; where an input dimension equals one, that input coordinate is forced to zero.

This distinction matters for performance and ownership. Broadcasting creates a repeated **logical read**, not repeated storage. A writeable broadcast view is dangerous because many logical output positions alias the same physical element. A simple engine runtime should therefore treat broadcast views as read-only unless it implements explicit reduction/scatter semantics.

## 3. Matvec and matmul are bandwidth contracts as much as arithmetic

A dense matrix-vector product

$$
y_i=\sum_{j=0}^{n-1}W_{ij}x_j
$$

performs (mn) multiply-add contributions for an (m\times n) matrix. In many inference workloads, especially batch-one autoregressive decoding, the limiting resource is not the number of floating-point operations but the number of weight bytes that must reach the execution units.

This is why quantization matters operationally. Reducing a weight from 16 or 32 bits to 8, 4, or fewer bits can reduce memory traffic enough that the extra dequantization arithmetic is cheaper than reading a larger representation.

The proper benchmark therefore reports at least:

- tensor dimensions and batch size;
- representation and group size;
- bytes read;
- elapsed time after warmup;
- effective bandwidth;
- arithmetic result error against a higher-precision oracle;
- whether packing/dequantization overhead was included;
- CPU instruction set or GPU device used.

A “4-bit kernel is faster” statement without those boundaries is not evidence.

## 4. Quantization is a representation transform with an error model

For simple symmetric int8 quantization, choose

$$
s=\frac{\max_i |x_i|}{127},
$$

then store

$$
q_i=\operatorname{clamp}\left(\operatorname{round}\left(\frac{x_i}{s}\right),-127,127\right).
$$

Reconstruction uses

$$
\hat x_i=sq_i.
$$

The zero vector is a special case because its maximum magnitude is zero; an implementation can choose a harmless scale such as one while storing all-zero quantized values.

Asymmetric quantization adds a zero point (z),

$$
q_i=\operatorname{clamp}\left(\operatorname{round}\left(\frac{x_i}{s}\right)+z,q_{min},q_{max}\right),
qquad
\hat x_i=s(q_i-z).
$$

Groupwise quantization estimates scale, and possibly zero point or codebook parameters, per block. Smaller groups adapt better to local ranges but spend more metadata and can complicate kernels.

Do not confuse **packing** with **quantization**. Quantization chooses discrete representable values. Packing decides how those integer/code values are arranged into bytes and machine words.

The companion lab implements symmetric int8 because it is easy to inspect. A serious Nanoquant path should add differential tests for each supported block type against a scalar reference decoder before vectorizing it.

## 5. GGUF is a model container, not a tensor algorithm

GGUF is designed as a self-contained, extensible model format that supports fast loading and memory mapping. Its current specification defines a header containing magic, version, tensor count and metadata key-value count, followed by metadata, tensor descriptors, required padding and aligned tensor data. Tensor descriptors carry a name, dimensions, a GGML type and an offset relative to the tensor-data region.

A loader should conceptually run in four stages.

**Stage A — structural parse.** Read fixed-width fields with explicit endianness, bounds-check every read, reject impossible counts before allocating, and preserve the exact file offset of each parsed record.

**Stage B — metadata parse.** Validate string lengths and metadata types. Treat metadata as untrusted input. Unknown extensible metadata can often be preserved or ignored according to policy; malformed required metadata cannot.

**Stage C — tensor directory validation.** Validate each tensor name, rank, dimensions, type code, data alignment, byte extent and overlap policy. Computing a tensor extent requires the block geometry of the selected GGML type, not merely multiplying dimensions by a scalar element size.

**Stage D — publication.** Only after the entire directory is coherent should the runtime publish tensor views backed by an owned mapping or buffer.

The lab parses only the fixed header. That is intentional: a \partial parser should state its boundary rather than silently look like a full GGUF implementation. A future production-grade parser must follow the authoritative `ggml_type` definitions because type codes and block layouts evolve.

## 6. Memory mapping changes lifetime, not the mathematical tensor

Memory mapping allows file pages to become addressable without first copying every weight into an application-owned heap buffer. The mathematical tensor is unchanged, but its lifetime contract changes. Tensor views now depend on a mapping object, file descriptor/handle state, page-fault behavior and operating-system paging.

A clean ownership structure is:

`MappedFile` owns the OS mapping → `ModelStorage` owns or shares `MappedFile` → tensor views borrow validated ranges from `ModelStorage`.

Never let a tensor outlive the mapping whose bytes it references.

Lazy paging also changes benchmarking. The first traversal may include page faults; a warmed traversal may not. Report which one you measured.

## 7. ONNX is a graph IR with naming and ordering invariants

ONNX models represent computation as graphs containing inputs, outputs, initializers, nodes and attributes. For the top-level dataflow graph, node outputs obey single-static-assignment naming: an output name is defined once. The node list is topologically ordered, so a node cannot consume a value produced by a later node.

Those properties are valuable even if your engine does not execute ONNX directly. An importer can lower ONNX into a smaller internal IR.

A robust lowering pipeline is:

serialized model → structural validation → opset/domain resolution → symbolic/static shape reasoning → constant/initializer registration → operator validation → internal node creation → capability analysis → backend lowering → memory planning → executable schedule.

Do not fuse during the first parse. First create a trustworthy graph whose semantics are inspectable. Then transform it through explicit passes whose before/after graphs can be tested.

### Attributes and runtime values are different

An attribute is a static property of a node established in the graph, such as an axis or kernel shape. Inputs and outputs are values that participate in runtime dataflow. Treating an attribute as if it were a runtime tensor—or the reverse—creates confused importer APIs and makes constant folding harder to reason about.

## 8. Internal IR needs more than an enum of operators

A useful internal node records:

- operator kind and versioned semantic contract;
- ordered input value IDs;
- ordered output value IDs;
- static attributes;
- inferred or partially known types/shapes;
- diagnostic source identity;
- backend capability/fallback state.

A useful value record contains:

- unique ID/name;
- element type;
- shape information;
- whether it is constant, external input, temporary or output;
- storage assignment after planning.

The importer should reject duplicate definitions, undefined reads and impossible operator signatures before scheduling. The companion lab models exactly this SSA/topological core.

## 9. Scheduling is a resource problem after it is a graph problem

Topological order only proves dependency legality. A runtime scheduler must also decide when values are materialized, which buffers can be reused, where kernels execute and how CPU/GPU transfers are synchronized.

A simple planner can compute each temporary value's first definition and last use. Non-overlapping lifetimes can share an arena region when alignment and device constraints permit. That turns liveness analysis into a memory-allocation problem.

Backend selection should be explicit:

$$
\text{node}\rightarrow
\begin{cases}
\text{Metal kernel}, & \text{supported and profitable}\
\text{SIMD CPU kernel}, & \text{supported}\
\text{scalar reference}, & \text{fallback}
\end{cases}
$$

A fallback is a feature only if the runtime records that it happened. Silent fallback makes benchmark results uninterpretable.

## 10. Stable softmax is a required numerical transformation

Naively computing

$$
\operatorname{softmax}(x_i)=\frac{e^{x_i}}{\sum_j e^{x_j}}
$$

can overflow for large positive logits. Since adding or subtracting a common scalar does not change the normalized distribution,

$$
\operatorname{softmax}(x_i)=
\frac{e^{x_i-m}}{\sum_j e^{x_j-m}},
\qquad m=\max_j x_j.
$$

Now the largest exponent is (e^0=1), and every other exponent is at most one. The lab explicitly tests logits around 1000 to prove this is not cosmetic.

For masked attention, invalid positions should be excluded before normalization, commonly by assigning them an effective (-infty) logit. Do not compute a normal probability and then zero it afterward without renormalizing.

## 11. Scaled dot-product attention is an explicit dataflow

For a sequence matrix (X), learned projections produce

$$
Q=XW_Q,\qquad K=XW_K,\qquad V=XW_V.
$$

For one query vector (q_t), attention scores against visible keys are

$$
\ell_j=\frac{q_tcdot k_j}{\sqrt{d_k}}.
$$

The (\sqrt{d_k}) scaling keeps dot-product magnitude from growing with dimension under common initialization assumptions.

After applying the causal mask,

$$
p=\operatorname{softmax}(ell),
qquad
o_t=\sum_j p_jv_j.
$$

The causal rule is semantic, not merely an optimization: token (t) must not read keys/values belonging to future tokens.

The lab traces a two-dimensional toy case so every score and weighted value remains inspectable.

## 12. The KV cache is persistent derived state

During autoregressive generation, earlier keys and values do not need to be recomputed every step if the model architecture permits caching them. Each new token appends the current layer's key/value vectors; subsequent queries attend over the cached prefix.

That converts repeated prefix recomputation into persistent memory usage. For (L) layers, sequence length (T), key/value width (D), and element size (B), a simplified cache-size model is proportional to

$$
2LTDB.
$$

Real architectures introduce heads, grouped-query attention and layout details, but the scaling argument explains why long contexts create memory pressure.

A KV cache therefore needs:

- capacity and growth policy;
- shape/layout invariant;
- token-position identity;
- reset/truncation behavior;
- ownership per sequence/request;
- device residency and synchronization;
- a policy for sliding-window or paged attention if supported.

The companion lab deliberately uses a fixed-capacity cache and rejects overflow rather than silently reallocating. That makes the lifetime contract obvious.

## 13. A transformer block is more than attention

A minimal decoder block commonly contains normalization, projected self-attention, residual addition, a feed-forward network and another residual path. Exact ordering differs by architecture.

Therefore a model runtime must read architecture metadata rather than assume one universal transformer. Differences can include normalization type/order, rotary or other positional encoding, activation function, grouped-query attention, bias presence, expert routing and tensor naming.

PROJECTALLMIGHT should teach the reusable execution principles while keeping architecture-specific assumptions explicit.

## 14. From compressed bytes to one generated-token step

The complete mental trace should be reconstructable as follows.

A GGUF file is opened or mapped. The loader validates its header and metadata. Tensor descriptors are checked for shape, type, alignment and bounds. Quantized weight tensors remain compressed in storage.

The tokenizer, which is intentionally outside the current executable lab, maps \text to token IDs. An embedding lookup turns one token ID into a vector. Positional machinery modifies or accompanies that representation according to the architecture.

For each layer, the runtime obtains the required quantized projection weights, runs a dequantizing matvec or other compressed kernel, forms query/key/value vectors, appends the new key/value state to the cache, computes causally masked attention, runs the output projection and feed-forward path, and preserves residual state.

The final hidden vector is projected to vocabulary logits. Stable normalization or sampling logic converts logits into the next-token choice. The selected token becomes the next step's input while the KV cache persists.

If you cannot identify every representation boundary in that paragraph—file bytes, tensor descriptors, packed weights, dequantized accumulators, temporary activations, persistent KV state and output logits—the runtime has not yet been reconstructed.

## 15. What production verification still requires

The new laboratory establishes a reference layer, not production completion. The following must remain marked open until separately evidenced:

1. Full GGUF metadata and tensor-directory parser with all selected type/block extents.
2. Differential dequantization tests for every Nanoquant block format.
3. SIMD dispatch with scalar-oracle parity.
4. Native Metal quantized matvec and GPU timing.
5. Real ONNX protobuf import, opset validation and supported-operator lowering.
6. Shape inference and dynamic-shape policy.
7. Memory planner with overlapping-lifetime tests and alignment constraints.
8. Tokenizer implementation and exact round-trip fixtures.
9. Architecture-specific transformer execution, including positional encoding and normalization.
10. End-to-\end logits comparison against a trusted runtime on a small public model.

The acceptance bar is not “the model produced text.” It is that each layer can be independently tested and disagreements can be localized.

## 16. Reconstruction questions

Close the note and answer these from memory.

Why can shape and storage order differ? Derive row-major strides for ((2,3,4)). Why is a broadcast usually a view rather than a copy? Why can quantization make a matvec faster even though it adds arithmetic? What does the GGUF tensor offset measure relative to? Why must an importer distinguish attributes from runtime values? What invariant does SSA give the internal graph? Why does subtracting the maximum preserve softmax probabilities? Why is the attention score divided by (\sqrt{d_k})? What exactly persists in a KV cache, and what invalidates it? Finally, trace one quantized weight block from mapped file bytes to its contribution to one output logit.

## Primary specification anchors

The current GGUF format documentation is maintained in the ggml repository under `docs/gguf.md`. The current ONNX intermediate-representation rules are maintained by ONNX under its IR specification. These specifications define file/graph contracts; they do not substitute for the runtime's own tests, numerical oracles or backend measurements.
