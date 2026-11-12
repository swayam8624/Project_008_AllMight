---
tags:
  - concept
  - foundation
---

# Stable reformulation

**Curriculum depth:** Introduced · **First seen:** C005 · **Latest development:** C005

## Definition and mechanism

A mathematically equivalent expression can avoid unnecessary numerical loss. sqrt(x+1)−sqrt(x)=1/(sqrt(x+1)+sqrt(x)) for x≥0; at double x=1e16 the direct result is zero while the reciprocal form remains near 5e-9.

## Prerequisites

- [[Cancellation and loss of significance]] — Near-equal operand errors motivate exact-subtraction distinctions and reformulation.

## Used by

Connect later dependent concepts here when their lessons arrive.

- [[Stable probability normalization and log-sum-exp]]
- [[Stable quadratic root evaluation]]
## Recall and next depth

Reconstruct the definition's example and name the assumptions that make it valid.

**Pending:** Every reformulation still needs domain, range, conditioning and accuracy checks.

## Source and navigation

[[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#When arithmetic meets uncertainty|M005 teaching chapter]] · [[Supplementary/Foundations#M005 - Parts 77-88|Part definitions]] · [[Dependency Map]]

## Algebraic equivalence is not numerical equivalence

Two formulas can be identical over real arithmetic and behave very differently in finite precision. Stable reformulation changes the computational path while preserving the intended mathematical quantity.

For the quadratic equation, the textbook root expression can subtract nearly equal numbers and lose most significant digits. Computing one root with a sign-aware expression such as

$$
q=-\tfrac12\left(b+\operatorname{sign}(b)\sqrt{b^2-4ac}\right)
$$

and obtaining the other as $c/q$ avoids that cancellation in many cases.

The same idea appears in log-sum-exp, `hypot`-style norms, rationalizing expressions, compensated sums, and solving least squares with QR rather than normal equations. A reformulation is not automatically stable: its overflow/underflow regions and exceptional inputs still need analysis.

Tests should compare against a higher-precision reference across adversarial inputs, not only ordinary random values. The goal is to show that the reformulated algorithm reduces forward/backward error where the naive expression is fragile.

