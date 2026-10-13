---
tags:
  - concept
  - cpp
---

# Named members versus array elements

**Curriculum depth:** Developed · **First seen:** C007 · **Latest development:** C008

## Definition and mechanism

The x member's one-past pointer is not an authorized y access. A span describes an already-valid range; adjacency cannot create that range.

Adjacent x/y/z fields are separate subobjects, not T[3]. A pointer to x supports one-element-array pointer arithmetic; one-past cannot be dereferenced to access y. span(&x,3) does not create an array. Use explicit member dispatch or actual array storage; a ToArray result is a copy.

## Prerequisites

- [[Pointer arithmetic and provenance]] — Array-relative pointer bounds explain why adjacency of separate members cannot authorize indexing.

## Used by

- [[Vector value types and component access]] — Phase 6 uses this established foundation in its geometry narrative.



## Recall and next depth

Give an example, explain why the rule exists, and identify which guarantee would be missing if the prerequisite were ignored.

**Pending:** Deeper ownership, pointer/reference, generic programming and module rules from later sources. This is written coverage, not demonstrated personal mastery.

## Source and navigation

[[Supplementary/Foundations#M008 - Parts 113-126|C008 development and corrections]]

[[Continuous Notes/02 - C++ - From Objects to Reliable Programs#Bringing the object model back to bytes|C007 teaching chapter]] · [[Supplementary/Foundations#M007 - Parts 101-112|Source coverage]] · [[Dependency Map]]
