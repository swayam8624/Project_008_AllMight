---
tags:
  - concept
  - cpp
---

# Scoped enums and validated domains

**Curriculum depth:** Introduced · **First seen:** C007 · **Latest development:** C007

## Definition and mechanism

enum class scopes enumerators and disallows implicit conversion to integers. A fixed underlying type establishes representation, not membership in the named application domain. Validate external codes before constructing semantic states; explicitly define bitmask operations when combination is intended.

## Prerequisites

- [[Fixed-width integer types]] — An enum's underlying integer type establishes representation but not the application's valid code set.

## Used by

Later chunks extend this same node; dependent concepts are listed here as they are integrated.

## Recall and next depth

Give an example, explain why the rule exists, and identify which guarantee would be missing if the prerequisite were ignored.

**Pending:** Deeper ownership, pointer/reference, generic programming and module rules from later sources. This is written coverage, not demonstrated personal mastery.

## Source and navigation

[[Continuous Notes/02 - C++ - From Objects to Reliable Programs#Give a domain its own vocabulary|C007 teaching chapter]] · [[Supplementary/Foundations#M007 - Parts 101-112|Source coverage]] · [[Dependency Map]]
