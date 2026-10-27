---
tags:
  - concept
  - phase-6
---

# Robust geometric predicates

**Curriculum depth:** Introduced · **First seen:** Phase6 · **Latest development:** Phase6

## Definition and mechanism

A geometric predicate decides a category from a sign, such as orientation. Near cancellation, ordinary floating arithmetic may not certify that sign. Adaptive arithmetic adds work only when justified error bounds leave uncertainty. Exact stored-input decisions cannot restore rounded-away physical information.

A triangle can have an accurately small area estimate yet an uncertified orientation sign. Declaring every small value zero changes topology instead of proving a decision.

## Prerequisites

- [[Cross products and oriented area]] — Oriented area supplies face normals, frame signs and geometric determinants.
- [[Dot and cross-product rounding]] — Cancellation and rounded inputs explain when a determinant sign or finite measurement is uncertain.

## Used by

Later curriculum sources can deepen this same node; no future coverage is marked complete.

- [[Collision barriers and continuous detection]]
## Recall and next depth

Distinguish tolerance-based geometry, measurements and certified predicates; the Phase 6 lab implements only ordinary tests. Full production proofs, target-specific integration and learner mastery require further evidence.

## Source and navigation

[[Continuous Notes/06 - From Movement to Meaning - Vectors, Geometry and Reliable Code#The sensor needs coordinates inside the triangle|Teaching narrative]] · [[Supplementary/Foundations#Phase 6 - Parts 501-600|Coverage and source gap]] · [[Supplementary/Sources and Code Anchors#Phase 6 - Source analysis and verification|Evidence and limits]] · [[Dependency Map]]
