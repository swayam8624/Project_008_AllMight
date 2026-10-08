---
tags:
  - concept
  - cpp
---

# Standard-layout and trivial copyability

**Curriculum depth:** Introduced · **First seen:** C007 · **Latest development:** C007

## Definition and mechanism

Standard-layout is a structural language property supporting useful low-level layout reasoning; it does not promise no padding or a portable ABI. Trivial copyability permits particular byte-copy operations on valid objects/representations, not arbitrary byte validity or serialization. Aggregate, trivial and trivially-copyable are separate traits.

## Prerequisites

- [[ABI and object layout]] — ABI offsets and padding must be separated from standard-layout and byte-copy language eligibility.

## Used by

Later chunks extend this same node; dependent concepts are listed here as they are integrated.

## Recall and next depth

Give an example, explain why the rule exists, and identify which guarantee would be missing if the prerequisite were ignored.

**Pending:** Deeper ownership, pointer/reference, generic programming and module rules from later sources. This is written coverage, not demonstrated personal mastery.

## Source and navigation

[[Continuous Notes/02 - C++ - From Objects to Reliable Programs#Bringing the object model back to bytes|C007 teaching chapter]] · [[Supplementary/Foundations#M007 - Parts 101-112|Source coverage]] · [[Dependency Map]]
