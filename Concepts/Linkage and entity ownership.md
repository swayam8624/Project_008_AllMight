---
tags:
  - concept
  - cpp
---

# Linkage and entity ownership

**Curriculum depth:** Introduced · **First seen:** C007 · **Latest development:** C007

## Definition and mechanism

Linkage determines when declarations can denote the same entity across scopes or translation units. An unnamed namespace gives internal linkage to suitable names; named modules also introduce module linkage. Linkage is not scope, storage duration or export visibility.

## Prerequisites

- [[Declarations and definitions]] — Declarations must be related to determine whether they denote one entity or distinct private entities.

## Used by

Later chunks extend this same node; dependent concepts are listed here as they are integrated.

## Recall and next depth

Give an example, explain why the rule exists, and identify which guarantee would be missing if the prerequisite were ignored.

**Pending:** Deeper ownership, pointer/reference, generic programming and module rules from later sources. This is written coverage, not demonstrated personal mastery.

## Source and navigation

[[Continuous Notes/02 - C++ - From Objects to Reliable Programs#Many files must describe one coherent program|C007 teaching chapter]] · [[Supplementary/Foundations#M007 - Parts 101-112|Source coverage]] · [[Dependency Map]]
