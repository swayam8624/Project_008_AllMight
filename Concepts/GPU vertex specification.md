---
tags:
  - concept
  - university-series
  - independent-series
aliases:
  - "VAO"
  - "VBO"
  - "vertex attribute"
  - "vertex format"
  - "interleaved vertex data"
  - "stride"
  - "byte offset"
  - "attribute binding"
---

# GPU vertex specification

**Curriculum depth:** Developed · **First seen:** University courses · **Latest development:** University courses

## Definition and mechanism

A GPU vertex-input specification explains how bytes in one or more buffers become per-vertex shader inputs. A buffer object owns bytes; a vertex array object records their interpretation: enabled attributes, component count and type, normalization, stride, byte offset, and the buffer binding used for fetch. Stride is the number of bytes between successive vertices; offset is the first component's byte position. For vertex i, the address is base + offset + i × stride. For interleaved records containing position (3 floats), normal (3), and UV (2), the stride is 8 × 4 = 32 bytes and the UV offset is 24 bytes. Modern binding APIs separate attribute format from the buffer binding; older pointer-style calls can capture the buffer bound at setup time. A wrong stride may still address valid memory, so the symptom is often plausible but scrambled geometry. The contract must agree with CPU packing, shader input types, and the API's state-capture rules.

## Prerequisites

- [[Vector storage SIMD and boundary layouts]]
- [[Matrix storage indexing and strides]]
- [[Camera view and clip-space pipeline]]
- [[Pointer arithmetic and provenance]]

## Used by

- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Rasterization, coverage, and depth|Rasterization, coverage, and depth]]
- [[Homogeneous clipping and perspective interpolation]]

## Full teaching explanation

- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Rasterization, coverage, and depth|Rasterization, coverage, and depth]]

## Recall and next depth

- Trace the fetched address for vertex 3 when stride is 32 bytes and offset is 24 bytes.
- Why can an incorrect stride produce plausible but scrambled geometry without a memory fault?

## Source and navigation

Teaching narrative: [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Rasterization, coverage, and depth|open the main explanation]]. Browse the [[Keyword Index#University graphics vision and photography - independent series|University graphics and vision index]]. Additional chapter connections: [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#A scene description becomes an image|A scene description becomes an image]]; [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Rasterization, coverage, and depth|Rasterization, coverage, and depth]]; [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#The mathematical table needs an explicit storage agreement|The mathematical table needs an explicit storage agreement]]; [[Continuous Notes/02 - C++ - From Objects to Reliable Programs#A pointer carries an access contract|A pointer carries an access contract]].

Caption/source records: [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#P1-01 - Module 1.1: Course Intro & Administrivia Part 1 Graphics & Visual Computing 2026|P1-01]], [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#P3-03 - Moving Objects in Space|P3-03]], [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#P3-04 - Rotations About an Arbitrary Axis|P3-04]].
