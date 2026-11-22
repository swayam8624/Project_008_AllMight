# Screen-space ambient occlusion and thickness

Ambient occlusion estimates how much of the local hemisphere around a surface point is blocked by nearby geometry. Screen-space ambient occlusion (SSAO) approximates that visibility using the depth and often normal buffers from the current camera.

## The sample question

For each sample direction around the reconstructed surface, project a test point into screen space and compare the depth at which that test point is expected with the scene depth found there. If the scene contains geometry closer to the camera than the expected sample, that direction may be treated as occluded.

A **thickness** or bias term is necessary because finite depth precision, curved surfaces and the fact that a depth buffer stores only the first visible layer make exact equality meaningless. Too little thickness self-occludes the surface; too much allows true nearby blockers to leak through.

## Visibility, not darkness

AO is a geometric visibility factor. It should modulate an appropriate indirect/ambient lighting model rather than being multiplied indiscriminately into direct light or emissive terms. Radius is also a world/view-space modeling choice: changing resolution should not silently change the physical size of the neighborhood.

Noise kernels and sample rotation trade structured banding for noise. Bilateral filtering should respect depth/normal discontinuities so occlusion does not bleed across silhouettes.

## Executable reference

`tools/labs/realtime_effects_reference_lab.cpp` evaluates a small set of expected sample depths against observed scene depths with an explicit thickness allowance and checks the resulting visible fraction. This is the binary core of the decision, not a claim of GTAO/HBAO quality.

A production oracle should reconstruct 3D positions, rotate a hemisphere kernel, project every sample, compare against a ray-traced local-visibility reference, and test radius/thickness behavior across resolution and depth discontinuities.

Related: [[Hardware depth-buffer linearization]], [[Surface tangents normals and bitangents]], [[Sampling aliasing and reconstruction]].
