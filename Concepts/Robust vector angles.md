---
tags:
  - concept
  - phase-6
---

# Robust vector angles

**Curriculum depth:** Introduced · **First seen:** Phase6 · **Latest development:** Phase6

## Definition and mechanism

The unsigned angle uses nonzero finite directions. An atan2 expression combines cross magnitude and dot, retaining sine- and cosine-like information. Scale inputs before evaluating to reduce range failures. A signed angle requires a specified oriented plane.

Parallel directions give zero, antiparallel directions give $\pi$, and perpendicular directions give $\pi/2$. Zero input has no angle.

## Prerequisites

- [[Cross products and oriented area]] — Oriented area supplies face normals, frame signs and geometric determinants.
- [[Unit directions and safe normalization]] — Validated finite unit directions establish the preconditions for simplified geometry formulas.

## Used by

Later curriculum sources can deepen this same node; no future coverage is marked complete.

## Recall and next depth

Separate acos domain clamping from input validation, and explain when no angle needs computing. Full production proofs, target-specific integration and learner mastery require further evidence.

## Source and navigation

[[Continuous Notes/06 - From Movement to Meaning - Vectors, Geometry and Reliable Code#An angle is a question with a domain|Teaching narrative]] · [[Supplementary/Foundations#Phase 6 - Parts 501-600|Coverage and source gap]] · [[Supplementary/Sources and Code Anchors#Phase 6 - Source analysis and verification|Evidence and limits]] · [[Dependency Map]]
