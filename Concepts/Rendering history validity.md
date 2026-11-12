# Rendering history validity

Accumulated samples belong to a particular target image defined by camera, scene, materials, lighting and estimator settings. An engine must invalidate or deliberately transform history when that target changes.

Teach and reconstruct: [[Continuous Notes/Game Engine - From First Frame to Living Worlds#Let a renderer evolve without losing resource ownership]].

Related integration record: [[Supplementary/Manual Integration - Sources and Verification]].

## Mastery expansion — temporal data is a hypothesis about correspondence

A temporal history buffer is useful only when a current sample can be mapped to a previous sample representing the same underlying signal. History validity therefore depends on more than “a texture exists.”

Invalidation or reduced trust may be required after:

- camera cuts or projection changes;
- resize or dynamic-resolution changes;
- object spawn/despawn or topology changes;
- material/lighting discontinuities;
- depth disocclusion;
- motion-vector failure;
- changed jitter convention;
- changed exposure when history is stored in a non-compatible domain;
- backend/resource recreation.

A temporal system should carry explicit history metadata: previous transforms/camera, resolution, jitter index, content version and validity flags. Reprojection should compare current and reprojected depth/normal/material or other correspondence signals appropriate to the algorithm.

History clamping/variance clipping can limit ghosting but cannot make invalid correspondence correct. A hard reset is sometimes the right answer.

**Required next witness:** a deterministic sequence with camera cut, disocclusion, resize and moving object; assert reset/rejection behavior before judging visual smoothness.

