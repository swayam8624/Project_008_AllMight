---
tags:
  - concept
  - phase-6
---

# Vector tolerances and finite-state diagnostics

**Curriculum depth:** Introduced · **First seen:** Phase6 · **Latest development:** Phase6

## Definition and mechanism

Closeness depends on a declared distance or angular criterion. Combined absolute/relative scalar rules differ from component boxes and Euclidean balls. NaN/infinity policies are explicit. Diagnose the first invalid stage rather than only the final visible symptom.

Errors $(0.09,0.09)$ pass component tolerance $0.1$ but fail Euclidean tolerance $0.1$. Near $10^8$, binary32 can lose a one-unit displacement at storage.

## Prerequisites

- [[Numerical tolerance policies]] — A declared scale and domain make closeness and rejection thresholds meaningful.
- [[Dot and cross-product rounding]] — Cancellation and rounded inputs explain when a determinant sign or finite measurement is uncertain.

## Used by

Later curriculum sources can deepen this same node; no future coverage is marked complete.

## Recall and next depth

Show nontransitivity; separate measurement noise, arithmetic error and input quantization. Full production proofs, target-specific integration and learner mastery require further evidence.

## Source and navigation

[[Continuous Notes/06 - From Movement to Meaning - Vectors, Geometry and Reliable Code#Reliability begins at the boundary of the calculation|Teaching narrative]] · [[Supplementary/Foundations#Phase 6 - Parts 501-600|Coverage and source gap]] · [[Supplementary/Sources and Code Anchors#Phase 6 - Source analysis and verification|Evidence and limits]] · [[Dependency Map]]
