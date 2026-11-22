# Temporal history validation and reprojection

Temporal rendering reuses information from earlier frames, but the old sample is useful only when it still estimates the same current surface and the same target quantity. Reprojection therefore proposes a correspondence; validation decides whether to trust it.

Under one common screen-space convention, a motion vector stores

$$
m = uv_{previous}-uv_{current},
$$

so the history lookup is

$$
uv_{history}=uv_{current}+m.
$$

Other engines store the opposite sign. The important requirement is that the convention is stated once and tested by projecting the same point in current and previous camera transforms.

## Validation signals

A history sample should be rejected when evidence says the reprojected location belongs to a different target. Useful signals include:

- out-of-bounds previous UV;
- camera-cut/history-generation mismatch;
- depth disagreement;
- normal disagreement;
- object/material/primitive identity mismatch;
- invalid velocity;
- resize or projection change;
- estimator/material/lighting changes that invalidate the accumulated quantity.

Depth thresholds should be scale-aware. A fixed absolute depth epsilon that works at one metre may be useless at one kilometre.

## Disocclusion

Disocclusion is the classic temporal failure. A foreground object moves away and reveals background. The motion vector can point to a valid previous pixel, but that pixel represented the foreground. Bounds checks succeed while semantics fail. Depth/ID/normal validation is therefore about *surface correspondence*, not memory safety.

## Accumulation and clamping

Once history is accepted, a temporal filter blends current and historical estimates. A high history weight reduces noise but increases lag and ghosting. Neighborhood clipping, variance clipping, reactive masks and confidence weighting are ways to constrain history when local evidence changes.

Those techniques should be tested on controlled sequences: static scene, camera pan, hard cut, moving foreground revealing background, emissive change, thin geometry and high-frequency texture.

## Executable reference

tools/labs/realtime_rendering_reference_lab.cpp defines an explicit history predicate using bounds, history generation, object identity, relative depth and normal agreement. Tests cover a stable sample, camera cut, disocclusion, depth jump and off-screen reprojection. It also verifies the declared motion-vector sign and that invalid history contributes zero weight.

This is a CPU semantic oracle. It does not yet implement TAA neighborhood clipping, jitter, full screen reprojection or GPU parity.

Related: [[Rendering history validity]], [[Reversed-Z depth contracts]].
