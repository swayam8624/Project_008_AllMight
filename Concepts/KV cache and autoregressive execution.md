# KV cache and autoregressive execution

Autoregressive decoding repeatedly evaluates a model over an ever-growing prefix. Recomputing every earlier key and value at every step wastes work, so decoder runtimes commonly retain per-layer key/value state.

For a simplified cache with $L$ layers, sequence length $T$, key/value width $D$ and element size $B$, memory scales proportionally to $2LTDB$. Real architectures modify the constants through heads, grouped-query attention, paging and quantization, but the scaling explains why context length becomes a memory problem.

A KV cache is not “just a vector.” It needs sequence/request identity, token-position identity, capacity/growth policy, layout, layer/head indexing, reset/truncation semantics, device residency, synchronization and invalidation rules. Reusing cache state for the wrong prompt or position is a semantic error even when every memory access is legal.

A fixed-capacity reference cache is valuable because it makes overflow explicit. Production systems may use paged or sliding-window policies, but those policies must preserve the attention semantics they claim.

**Executable witness:** the Phase-29 lab appends consistent-width key/value vectors and rejects capacity overflow.

**Next depth:** per-layer/head layouts, rotary-position interaction, paged KV allocation, speculative decoding, cache quantization and cross-device scheduling.
