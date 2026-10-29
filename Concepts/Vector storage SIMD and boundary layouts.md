---
tags:
  - concept
  - phase-6
---

# Vector storage SIMD and boundary layouts

**Curriculum depth:** Introduced · **First seen:** Phase6 · **Latest development:** Phase6

## Definition and mechanism

Storage layout, compute lanes and external buffer layout are separate contracts. SIMD may operate within one vector or across objects. AoS, SoA and AoSoA change access patterns. Alignment and padding must be measured; a spare lane is not implicitly valid homogeneous data.

One million twelve-byte vectors occupy four million fewer payload bytes than sixteen-byte vectors. A sixteen-byte load still needs sixteen accessible bytes even if one lane is discarded.

## Prerequisites

- [[Vector value types and component access]] — Primitive representation and access rules support layout choices and semantic wrappers.
- [[Alignment and padding]] — Object alignment and array stride constrain storage and boundary conversions.
- [[SIMD and GPU data layouts]] — Lane access and external interface layouts require separate storage contracts.

## Used by

Later curriculum sources can deepen this same node; no future coverage is marked complete.

- [[GPU vertex specification]]
## Recall and next depth

Measure size/stride and shader offsets separately; benchmark before claiming an optimized representation wins. Full production proofs, target-specific integration and learner mastery require further evidence.

## Source and navigation

[[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Storage and computation can choose different layouts|Teaching narrative]] · [[Supplementary/Foundations#Phase 6 - Parts 501-600|Coverage and source gap]] · [[Supplementary/Sources and Code Anchors#Phase 6 - Source analysis and verification|Evidence and limits]] · [[Dependency Map]]
