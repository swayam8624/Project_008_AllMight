---
tags:
  - concept
  - cpp
---

# Temporary lifetime extension

**Curriculum depth:** Introduced · **First seen:** C008 · **Latest development:** C008

## Definition and mechanism

Ordinary temporary destruction occurs at the full-expression's end unless specified extension applies. Direct local binding can extend lifetime, but a returned parameter reference cannot transitively extend it. Aggregate braces and parentheses differ for reference members; new-initializer extension does not last as long as the allocated holder.

## Prerequisites

- [[Reference binding and converted temporaries]] — The chosen original or converted referent determines the destruction deadline.
- [[Storage duration and object lifetime]] — The referent and its backing storage must survive each alias use.

## Used by

- [[Deferred captures and borrowed views]]
- [[Ref-qualified accessors]]

## Recall and next depth

Draw the destruction point for pass(std::string{"x"}). Pending: detailed subobject extension and C++26 diagnostic changes. Written coverage does not establish learner mastery.

## Source and navigation

[[Continuous Notes/02 - C++ - From Objects to Reliable Programs#A temporary needs an explicit lifetime story|Teaching explanation and examples]] · [[Supplementary/Foundations#M008 - Parts 113-126|Exact coverage]] · [[Dependency Map]]
