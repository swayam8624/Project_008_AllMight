---
tags:
  - concept
  - foundation
---

# Capped weighted updates

**Curriculum depth:** Introduced · **First seen:** C005 · **Latest development:** C005

## Definition and mechanism

A cap can discard old influence mathematically. With cap 2 and unit-weight observations [0,1,−1] versus [0,−1,1], exact distances end at −0.25 versus +0.25. This is not merely floating summation order.

## Prerequisites

- [[Mixed-precision state updates]] — Persistent rounding must be separated from mathematical history-weight policy.
- [[Arithmetic policies]] — Capping changes which influence is retained rather than only the representation.

## Used by

Connect later dependent concepts here when their lessons arrive.

## Recall and next depth

Reconstruct the definition's example and name the assumptions that make it valid.

**Pending:** Changing to batch averaging changes the algorithm; separate history policy from rounding.

## Source and navigation

[[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#When arithmetic meets uncertainty|M005 teaching chapter]] · [[Supplementary/Foundations#M005 - Parts 77-88|Part definitions]] · [[Dependency Map]]

## Why clipping a contribution changes the estimator

A capped weighted update replaces an unbounded contribution $w x$ with something like

$$
\tilde w = \min(w,w_{max}),
\qquad
\Delta = \tilde w x.
$$

The cap protects an online system from one extreme weight dominating state, overflowing finite precision, or creating intolerable variance. But unless the cap is part of the intended mathematical model, it introduces bias: contributions above the threshold are deliberately underrepresented.

This trade appears in importance weighting, robust online statistics, adaptive reconstruction and control. The correct question is not “does clipping stabilize it?” but how bias changes as the cap moves and whether the uncapped tail is real signal, model mismatch, or corrupted data.

A useful experiment sweeps $w_{max}$, records variance, bias against an uncapped/high-precision reference, effective sample size and downstream failure rate. Tests should include weights exactly at the cap, negative/invalid weights when the domain forbids them, and extreme values that would overflow the uncapped path.

