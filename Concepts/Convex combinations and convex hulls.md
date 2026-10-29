---
tags:
  - concept
  - phase-6
---

# Convex combinations and convex hulls

**Curriculum depth:** Introduced · **First seen:** Phase6 · **Latest development:** Phase6

## Definition and mechanism

Affine point combinations have weights summing to one. Convex combinations additionally have nonnegative weights. A convex set contains every segment between its members; a convex hull is the smallest such set containing supplied points.

The panel is the convex hull of its corners. A filled disk is convex; its boundary alone is not.

## Prerequisites

- [[Barycentric coordinates and interpolation]] — Affine weights establish origin-independent combinations and inside-hull restrictions.

## Used by

- [[Minkowski sums and support mappings]] — Builds on this mechanism in the connected geometry story.
- [[Subdivision surface masks]]

## Recall and next depth

Show origin invariance and explain why a hull can fill holes absent from the source object. Full production proofs, target-specific integration and learner mastery require further evidence.

## Source and navigation

[[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Weighted points lead from affine geometry to collision shapes|Teaching narrative]] · [[Supplementary/Foundations#Phase 6 - Parts 501-600|Coverage and source gap]] · [[Supplementary/Sources and Code Anchors#Phase 6 - Source analysis and verification|Evidence and limits]] · [[Dependency Map]]
