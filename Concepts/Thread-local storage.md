---
tags:
  - concept
  - cpp
---

# Thread-local storage

**Curriculum depth:** Introduced · **First seen:** C007 · **Latest development:** C007

## Definition and mechanism

thread_local gives a separate object/storage instance per thread; block scope does not make it automatic. Initialization, access costs and destruction follow implementation/language rules. Thread-local state does not make shared pointees thread-safe, and escaped addresses need a valid thread/object lifetime.

## Prerequisites

- [[Storage duration and object lifetime]] — Thread duration is a distinct backing-storage category, not a synonym for a local name's scope.

## Used by

Later chunks extend this same node; dependent concepts are listed here as they are integrated.

## Recall and next depth

Give an example, explain why the rule exists, and identify which guarantee would be missing if the prerequisite were ignored.

**Pending:** Deeper ownership, pointer/reference, generic programming and module rules from later sources. This is written coverage, not demonstrated personal mastery.

## Source and navigation

[[Continuous Notes/02 - C++ - From Objects to Reliable Programs#A name does not decide how long storage lasts|C007 teaching chapter]] · [[Supplementary/Foundations#M007 - Parts 101-112|Source coverage]] · [[Dependency Map]]
