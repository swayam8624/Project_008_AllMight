# Transformer attention and KV cache runtime

A transformer runtime becomes understandable when tensor shapes remain visible. For one attention head, let a token representation produce query, key and value vectors

$$
q,k,v \in \mathbb{R}^{d}.
$$

For a query attending to keys $k_j$, the scaled score is

$$
\ell_j = \frac{q^T k_j}{\sqrt{d}},
$$

followed by a stable softmax

$$
p_j = \frac{e^{\ell_j-m}}{\sum_t e^{\ell_t-m}},
\qquad m=\max_t \ell_t,
$$

and the output is

$$
o = \sum_j p_j v_j.
$$

Subtracting $m$ does not change the distribution because the same factor appears in numerator and denominator, but it prevents overflow when logits are large.

## Causal masking

For autoregressive decoding at token position $t$, a query must not observe keys and values from future positions. In a batch attention matrix this is represented by a causal mask before softmax. During incremental decoding, a correctly managed KV cache gives the same structural guarantee: only keys and values through the current token have been appended.

The cache exists because recomputing all earlier K/V projections every token is wasteful. If each layer has key/value vectors of head width $d_h$, number of KV heads $h_{kv}$, sequence length $L$, and element size $b$, the rough storage per layer is

$$
2 L h_{kv} d_h b
$$

bytes before allocator and alignment overhead. Cache datatype, paging strategy and maximum context length are therefore memory-system decisions.

## Multi-head and grouped-query attention

Multiple heads perform the same mechanism with separate projected subspaces. Grouped-query attention may use fewer key/value heads than query heads; the mapping from query head to KV head must be explicit. GGUF metadata commonly carries head-count and key/value dimension information, so model loading and runtime shape validation meet at this boundary.

## Residual, normalization, and feed-forward structure

Attention is only one sublayer. A complete transformer block also needs residual paths, normalization, and a feed-forward network or architecture-specific alternative. The order—pre-norm versus post-norm, RMSNorm versus LayerNorm, activation and gating choice—is part of the model architecture and must come from the model contract rather than a generic “transformer” assumption.

## Executable witness

tools/labs/phase29_ml_runtime_lab.cpp implements stable softmax and a small incremental KV cache. Two keys with distinct values make the weighting observable; a single-token case proves that incremental causal attention cannot see a future token that has not been appended.

It intentionally does not claim multi-head batching, rotary position embeddings, grouped-query attention, tokenizer behavior, paged KV caches, fused kernels or production-model parity. Those are separate implementation milestones.

Related: [[Tensor runtime layout and views]], [[GGUF binary model loading]], [[ONNX graph import and lowering]], [[Quantized matvec and dequantization]].
