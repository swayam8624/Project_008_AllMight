---
tags:
  - concept
  - phase-6
---

# Vector value types and component access

**Curriculum depth:** Introduced · **First seen:** Phase6 · **Latest development:** Phase6

## Definition and mechanism

A vector value type stores components directly with coherent arithmetic and initialization. Named members are not an array. Named dot, cross and Hadamard operations avoid ambiguous vector multiplication; writable swizzles need explicit alias policies.

A checked component accessor selects one of three real members and rejects out-of-range indices. It does not traverse from the address of the first member.

## Prerequisites

- [[Vectors coordinates and affine points]] — Point, displacement and coordinate roles determine which geometric operations are meaningful.
- [[Named members versus array elements]] — Array-relative pointer rules constrain safe named-component access.

## Used by

- [[Vector storage SIMD and boundary layouts]] — Builds on this mechanism in the connected geometry story.
- [[Coordinate-space types and validated invariants]] — Builds on this mechanism in the connected geometry story.

## Recall and next depth

Distinguish constexpr, noexcept, finite results, trivial copying and serialization. Full production proofs, target-specific integration and learner mastery require further evidence.

## Source and navigation

[[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Three stored numbers are not the complete contract|Teaching narrative]] · [[Supplementary/Foundations#Phase 6 - Parts 501-600|Coverage and source gap]] · [[Supplementary/Sources and Code Anchors#Phase 6 - Source analysis and verification|Evidence and limits]] · [[Dependency Map]]
