---
tags:
  - concept
aliases: []
---

# Tensor-product interpolation

**Curriculum depth:** Developed · **First seen:** Phase 10 · **Latest development:** Phase 10

## Definition and mechanism

Bilinear and trilinear interpolation apply one-dimensional linear weights independently along each coordinate axis. In a rectangular cell, each corner contributes the product of its axis weights; the weights are nonnegative and sum to one for coordinates inside the cell. This preserves the range of scalar corner values, but the interpolated surface can be twisted rather than planar, and derivatives may jump across cell boundaries. For a voxel field, trilinear interpolation provides a continuous in-cell estimate; differentiating it yields a local gradient, which need not have unit length even when samples approximate signed distance. Keep coordinates normalized to the cell and define behavior outside the sampled grid explicitly.

## Prerequisites

- [[Interpolation versus approximation]]
- [[Sampling aliasing and reconstruction]]

## Used by

- No downstream concept node is added in this stage.

## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Reconstructing values and curves between samples|Reconstructing values and curves between samples]]

## Source coverage

Supplied lesson parts: 978–979.

## Future keywords

bilinear weights, trilinear fields, tensor-product basis
