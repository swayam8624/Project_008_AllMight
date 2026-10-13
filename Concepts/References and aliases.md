---
tags:
  - concept
  - cpp
---

# References and aliases

**Curriculum depth:** Introduced · **First seen:** C008 · **Latest development:** C008

## Definition and mechanism

A reference is an alias established by binding, not a reseatable pointer object. Assignment through it assigns the referent. In `int& r=a; r=b;`, r still names a. A reference definition needs a binding; an extern reference declaration can defer the definition. References can dangle and are non-owning.

## Prerequisites

- [[Storage duration and object lifetime]] — The referent and its backing storage must survive each alias use.
- [[Declarations and definitions]] — Declarations establish the entities and types whose uses are being classified.

## Used by

- [[Reference binding and converted temporaries]]
- [[Auto decltype and reference collapsing]]
- [[Const qualification and shallow constness]]
- [[Parameter passing and sink contracts]]

## Recall and next depth

Trace reference assignment versus reseating a reference-to-pointer. Pending: reference members and generated copy/assignment behavior. Written coverage does not establish learner mastery.

## Source and navigation

[[Continuous Notes/02 - C++ - From Objects to Reliable Programs#Giving another name to a live object|Teaching explanation and examples]] · [[Supplementary/Foundations#M008 - Parts 113-126|Exact coverage]] · [[Dependency Map]]
