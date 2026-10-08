---
tags:
  - concept
  - cpp
---

# One Definition Rule

**Curriculum depth:** Introduced · **First seen:** C007 · **Latest development:** C007

## Definition and mechanism

ODR requires coherent definitions. A non-inline externally linked function normally needs one program definition when odr-used. Certain class, template and inline definitions can appear in multiple translation units only under the rule's conditions, including matching tokens and lookup; named-module attachment changes the permitted cases. Header guards operate within one translation unit, not across the program.

## Prerequisites

- [[Declarations and definitions]] — Agreement about which declarations define entities is necessary to state the one-definition constraint.

## Used by

Later chunks extend this same node; dependent concepts are listed here as they are integrated.

## Recall and next depth

Give an example, explain why the rule exists, and identify which guarantee would be missing if the prerequisite were ignored.

**Pending:** Deeper ownership, pointer/reference, generic programming and module rules from later sources. This is written coverage, not demonstrated personal mastery.

## Source and navigation

[[Continuous Notes/02 - C++ - From Objects to Reliable Programs#Many files must describe one coherent program|C007 teaching chapter]] · [[Supplementary/Foundations#M007 - Parts 101-112|Source coverage]] · [[Dependency Map]]
