---
tags:
  - concept
aliases: []
---

# Stable quadratic root evaluation

**Curriculum depth:** Developed · **First seen:** Phase 10 · **Latest development:** Phase 10

## Definition and mechanism

The direct quadratic formula can lose nearly all significant digits in one root when it subtracts nearly equal values. For real coefficients with nonzero leading coefficient and a valid nonnegative discriminant, compute a signed large-magnitude numerator first: $q=-\frac12(b+\operatorname{copysign}(\sqrt{b^2-4ac},b))$, then obtain roots as $q/a$ and $c/q$. The second root follows from the product relation $x_1x_2=c/a$. This avoids cancellation in the small root, but it does not prevent overflow in the discriminant or cover degenerate coefficients, nonfinite inputs, or $q=0$; a robust solver also scales inputs and handles those cases explicitly. For example, the small root of $x^2+10^8x+1=0$ is near $-10^{-8}$, exactly the root most endangered by direct subtraction.

## Prerequisites

- [[Stable reformulation]]
- [[Unit roundoff]]
- [[Conditioning and singularity]]
- [[Algorithmic stability]]

## Used by

- [[Bracketed and open root solvers]]

## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Floating-point answers: error, conditioning, stability, and stable arithmetic|Floating-point answers: error, conditioning, stability, and stable arithmetic]]

## Source coverage

Supplied lesson parts: 974.

## Future keywords

Vieta relation, discriminant scaling, cancellation
