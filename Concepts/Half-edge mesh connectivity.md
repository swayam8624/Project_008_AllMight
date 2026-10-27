---
tags:
  - concept
  - university-series
  - independent-series
---

# Half-edge mesh connectivity

**Curriculum depth:** Developed · **First seen:** University courses · **Latest development:** University courses

## Definition and mechanism

A half-edge data structure stores each undirected edge as two opposite directed records. Each record points to its twin, the next directed edge around its face, its origin vertex, its edge, and its face; vertices, edges, and faces keep one representative half-edge. Repeatedly following next visits a face boundary. Following twin then next walks around a vertex fan. This makes local neighborhoods and edits proportional to local degree rather than requiring a scan of every face. Correctness depends on invariants: twin of twin returns to the original, a half-edge is not its own twin, and next pointers form closed face cycles. The representation naturally assumes manifold connectivity; it can encode flips, splits, and collapses by changing a bounded set of pointers, but careless updates can break the cycles and corrupt subsequent traversal.

## Prerequisites

- [[Manifold polygon meshes and boundary structure]]

## Used by

- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography|Worlds, Images and Inference]]
- [[Subdivision surface masks]]
- [[Quadric error mesh simplification]]

## Full teaching explanation

[[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Meshes, connectivity, and geometry processing|Meshes, connectivity, and geometry processing]]
