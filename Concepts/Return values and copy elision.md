---
tags:
  - concept
  - cpp
---

# Return values and copy elision

**Curriculum depth:** Introduced · **First seen:** C009 · **Latest development:** C009

## Definition and mechanism

Appropriate same-type prvalues directly initialize result objects since C++17. NRVO for eligible named locals is optional in C++23 and needs a usable fallback. Move-eligible local/parameter return expressions have special xvalue treatment; return std::move(local) loses the ordinary NRVO form.

## Prerequisites

- [[Value categories and materialization]] — The return expression category distinguishes result construction from a bound path or move candidate.
- [[Object construction and storage reuse]] — A new destination requires a valid lifetime, independently of available bytes.

## Used by

Later sources will extend this node when their guarantees are developed.

## Recall and next depth

Return an immovable type directly, then explain why a named-local return fails. Pending: overlapping-subobject cases and detailed return deduction. Written coverage does not demonstrate personal mastery.

## Source and navigation

[[Continuous Notes/02 - C++ - From Objects to Reliable Programs#Return a result without handing out a lifetime problem|Teaching mechanism and examples]] · [[Supplementary/Foundations#M009 - Parts 127-147|Exact coverage]] · [[Dependency Map]]
