# Prefix scan and stream compaction

Exclusive prefix sums turn local output counts into unique output offsets. Predicate, scan and scatter can compact data without an atomic increment per output. Hierarchical scans require correct dependencies between phases.

Teach and reconstruct: [[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#Scan turns local counts into nonoverlapping output regions]].

Related integration record: [[Supplementary/Manual Integration - Sources and Verification]].

## Mastery expansion — scan converts local counts into global addresses

For inputs (x_0,\ldots,x_{n-1}), an exclusive prefix sum produces

[
y_i=\sum_{j<i}x_j.
]

When (x_i) is a 0/1 predicate saying whether item (i) survives, (y_i) is exactly the output address for a surviving item. The final output count is the last prefix plus the last predicate.

That turns stream compaction into three phases: classify, scan, scatter.

A serial scan is trivial, but a parallel scan must preserve the associative operator while reducing depth. Common approaches perform per-block scans, scan block totals, then add block offsets. GPU implementations must define workgroup barriers, shared-memory indexing, partial-block masks and cross-dispatch dependencies.

The operator must be associative enough for the required semantics. Floating-point addition is mathematically associative but not bitwise associative, so a parallel tree can change rounding and determinism.

**Required next witness:** CPU serial oracle plus GPU/block simulation, partial-block tests, zero/all-selected cases, overflow policy and deterministic integer compaction.

