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

[[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#The sensor needs coordinates inside the triangle|Teaching narrative]] · [[Supplementary/Foundations#Phase 6 - Parts 501-600|Coverage and source gap]] · [[Supplementary/Sources and Code Anchors#Phase 6 - Source analysis and verification|Evidence and limits]] · [[Dependency Map]]

## Phase 11 camera development

Adaptive orientation and incircle predicates protect discrete sign decisions from floating-point error by refining arithmetic when an error bound cannot certify the result. Camera estimation has a different uncertainty: noisy pixels and weak observability can leave pose uncertain even when every sign is exact. Predicate robustness does not improve measurement accuracy, and a low reprojection residual does not guarantee every combinatorial decision was reliable. Keep layers distinct: use robust predicates for topology, uncertainty weights for image residuals, and Jacobian spectra for weak directions. Link: [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#A camera model must close the loop|A camera model must close the loop]]; research: [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Four deeper links from the TUM lecture review|Four deeper links from the TUM lecture review]].
