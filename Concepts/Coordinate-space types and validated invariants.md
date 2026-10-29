---
tags:
  - concept
  - phase-6
---

# Coordinate-space types and validated invariants

**Curriculum depth:** Introduced · **First seen:** Phase6 · **Latest development:** Phase6

## Definition and mechanism

Compile-time space tags and affine-role types reject some meaningless operations without stored tag bytes. Validated wrappers carry finite/unit promises only through invariant-preserving operations. Broad local-space tags do not identify every dynamic object's frame.

Point difference returns a same-space vector. Scaling a unit direction by two returns an ordinary vector, not another unit direction.

## Prerequisites

- [[Vectors coordinates and affine points]] — Point, displacement and coordinate roles determine which geometric operations are meaningful.
- [[Unit directions and safe normalization]] — Validated finite unit directions establish the preconditions for simplified geometry formulas.
- [[Vector value types and component access]] — Primitive representation and access rules support layout choices and semantic wrappers.

## Used by

Later curriculum sources can deepen this same node; no future coverage is marked complete.

## Recall and next depth

Name the promise each constructor establishes; encode transform source/destination and separate normal transformation. Full production proofs, target-specific integration and learner mastery require further evidence.

## Source and navigation

[[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Types can remember the promises that three components forget|Teaching narrative]] · [[Supplementary/Foundations#Phase 6 - Parts 501-600|Coverage and source gap]] · [[Supplementary/Sources and Code Anchors#Phase 6 - Source analysis and verification|Evidence and limits]] · [[Dependency Map]]
