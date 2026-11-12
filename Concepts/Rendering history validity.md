# Rendering history validity

A temporal renderer reuses information from earlier frames only if that information still estimates the same target quantity at the reprojected location. “Previous frame exists” is therefore not enough. History needs a validity contract.

A history sample may become invalid because of:

- camera cuts or projection changes;
- resize or dynamic-resolution changes;
- object transform discontinuities;
- newly visible or disoccluded surfaces;
- material or lighting changes;
- estimator changes such as sample pattern, exposure domain or denoiser configuration;
- resource reallocation or version replacement.

## Reprojection is a hypothesis

Motion vectors or previous transforms propose a correspondence from current sample to previous sample. The renderer should test that hypothesis with available evidence such as depth, normal, object/material identity, velocity magnitude or neighborhood statistics.

A common failure is using a valid coordinate that points to the wrong surface. Bounds checking prevents memory errors, not temporal ghosting.

## Versioned history

Treat history like a cache with a key. Relevant state changes advance a version or invalidate specific history resources. The render graph can then express that a temporal pass consumes history version N and produces version N+1.

This is also important for progressive path tracing. Accumulation belongs to a camera/scene/material/light/estimator state. If any target-defining state changes and old samples remain, the output converges to a mixture of different images.

## Failure-oriented tests

Useful temporal tests include a hard camera cut, a moving foreground object revealing background, an emissive material changing abruptly, resize, FOV change, jitter-sequence restart, and an object whose ID remains but geometry changes. For each case, record which history samples should survive and why.

The existing render-graph witness proves logical resource dependencies, not temporal validity itself. Native renderer evidence should capture motion/depth/ID buffers and show the acceptance/rejection mask.

Teach and reconstruct: [[Continuous Notes/Game Engine - From First Frame to Living Worlds#Let a renderer evolve without losing resource ownership]].

Related: [[Temporal reprojection and history validation]], [[Reversed-Z depth contracts]].
