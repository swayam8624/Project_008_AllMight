# Raster sample coverage, stencil, and blending

Rasterization is not finished when a triangle has screen-space vertices. A graphics pipeline still has to decide which **samples** the primitive owns, how per-sample tests update depth/stencil state, and how a surviving source contribution combines with the destination.

## Multisample coverage

MSAA places several sample locations inside a pixel. Coverage is evaluated at those locations, so one pixel can have a partial sample mask even when the shader ultimately runs at pixel frequency. The sample pattern is part of the raster contract; using the pixel center alone is not equivalent to four-sample coverage.

A reference CPU rasterizer should therefore keep three ideas separate: primitive coverage, shader evaluation frequency, and final resolve. Coverage answers which stored samples exist for the primitive. Shading may be shared among samples depending on the API/state. Resolve combines multisample storage into the single-sample target.

## Stencil as stateful classification

Stencil testing compares a stored integer value with a reference under a declared compare function and then applies different operations for stencil-fail, depth-fail, or pass. It is therefore a small state machine, not merely a mask. Saturating increment/decrement, replace, keep, bit masks and front/back face state can all change behavior.

## Premultiplied source-over

For premultiplied source color $C_s$ with alpha $a_s$ over destination $C_d$,

$$
C_{out}=C_s+(1-a_s)C_d,
$$

and

$$
a_{out}=a_s+(1-a_s)a_d.
$$

Premultiplication makes the RGB equation match the physical idea that source RGB already carries its own coverage/opacity weight. Feeding straight-alpha RGB into this equation produces dark or bright fringes.

## Executable reference

`tools/labs/raster_pipeline_state_lab.cpp` clips one triangle against the homogeneous left plane, checks the two generated intersections, evaluates an explicit rotated 4x sample pattern, exercises stencil equal/pass increment with saturation, and verifies premultiplied source-over numerically.

It complements `raster_reference_lab.cpp`, which owns top-left shared-edge coverage, barycentrics, perspective-correct interpolation and depth ordering. Together they form a CPU semantic oracle; exact API sample locations, early/late tests, conservative rasterization and GPU parity remain native-pipeline evidence.

Related: [[Homogeneous clipping and perspective interpolation]], [[Triangle rasterization and coverage tests]], [[Perspective-correct interpolation]].
