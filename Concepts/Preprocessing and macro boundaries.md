---
tags:
  - concept
  - cpp
---

# Preprocessing and macro boundaries

**Curriculum depth:** Introduced · **First seen:** C007 · **Latest development:** C007

## Definition and mechanism

The preprocessor replaces tokens, includes text and selects conditional branches before ordinary type analysis. A square macro can change precedence or duplicate side effects. Header guards prevent repeat inclusion in one translation unit; differing macro environments can still produce an ODR mismatch.

## Prerequisites

- [[Translation units and linking]] — The compilation boundary explains where token replacement stops and typed semantic analysis begins.

## Used by

Later chunks extend this same node; dependent concepts are listed here as they are integrated.

## Recall and next depth

Give an example, explain why the rule exists, and identify which guarantee would be missing if the prerequisite were ignored.

**Pending:** Deeper ownership, pointer/reference, generic programming and module rules from later sources. This is written coverage, not demonstrated personal mastery.

## Source and navigation

[[Continuous Notes/02 - C++ - From Objects to Reliable Programs#Giving a program a buildable shape|C007 teaching chapter]] · [[Supplementary/Foundations#M007 - Parts 101-112|Source coverage]] · [[Dependency Map]]
