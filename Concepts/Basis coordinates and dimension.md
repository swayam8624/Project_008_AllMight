---
tags:
  - concept
  - phase-6
---

# Basis coordinates and dimension

**Curriculum depth:** Developed · **First seen:** Phase6 · **Latest development:** Phase7

## Definition and mechanism

Phase 7 stores basis images as matrix columns and derives coordinate conversion $[v]_C=C^{-1}B[v]_B$. The endpoint bases and whether motion is active or passive determine the product; row-major storage does not. [[Continuous Notes/07 - From Basis Images to Moving Worlds - Matrices, Transforms and Quaternions#Coordinates can change while the geometric object stays still|Reconstruction and active/passive example]].

A basis is an ordered independent spanning set. Independence makes coordinates unique; spanning makes every vector reachable. Dimension counts basis elements over the chosen field, not stored components or bytes. A basis change alters coordinates without necessarily moving the physical vector.

$(3,2)=1(1,0)+2(1,1)$, so its slanted-basis coordinates are $(1,2)$. A plane inside three-dimensional coordinates still has dimension two.

## Prerequisites

- [[Vector spaces subspaces and span]] — Reachability and closure establish the spanning and independence questions.

## Used by
- [[Matrices as linear maps and basis images]]

- [[Norms lengths and induced distances]] — Builds on this mechanism in the connected geometry story.
- [[Gram matrices and coordinate metrics]] — Builds on this mechanism in the connected geometry story.
- [[Covectors linear functionals and normal transforms]] — Builds on this mechanism in the connected geometry story.
- [[Linear systems and fundamental subspaces]] — Extends this foundation into linear-systems and decomposition analysis.
- [[Spline bases and curve evaluation]]

## Recall and next depth

Separate passive coordinate change from active transformation and explain near-dependent basis sensitivity. Full production proofs, target-specific integration and learner mastery require further evidence.

## Source and navigation

[[Continuous Notes/06 - From Movement to Meaning - Vectors, Geometry and Reliable Code#A basis is a measuring arrangement, not the vector itself|Teaching narrative]] · [[Supplementary/Foundations#Phase 6 - Parts 501-600|Coverage and source gap]] · [[Supplementary/Sources and Code Anchors#Phase 6 - Source analysis and verification|Evidence and limits]] · [[Dependency Map]]
