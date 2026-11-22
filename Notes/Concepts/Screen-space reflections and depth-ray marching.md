# Screen-space reflections and depth-ray marching

Screen-space reflection (SSR) searches the **already rendered depth field** for evidence that a reflected camera ray meets geometry visible in the current frame. It is therefore an image-space visibility approximation, not a replacement for geometric ray tracing.

## From a reflected direction to a depth crossing

After reconstructing a shaded point from the depth buffer, reflect the view direction about the surface normal and project points along that ray back into screen coordinates. At each screen sample, compare the ray's expected camera-space or linear depth with the scene depth stored at that pixel.

A simple crossing detector looks for a sign change in

$$
Delta = z_{ray}-z_{scene}.
$$

If the ray was in front of visible geometry and later moves behind it, a surface crossing lies between the samples. A production method refines that interval and applies a thickness tolerance rather than accepting arbitrary depth differences.

## What screen space cannot know

SSR can miss geometry that is off-screen, hidden behind the first visible layer, absent from the depth pyramid, or projected into undersampled regions. Rays near silhouettes may walk through holes; rough reflections require cone/footprint reasoning rather than one sharp ray; temporal accumulation can stabilize noise but can also preserve stale hits.

Hierarchical-Z acceleration replaces fixed pixel steps with coarse depth bounds so empty regions can be skipped. That changes traversal efficiency, not the fundamental visibility limitation.

## Executable reference

`tools/labs/realtime_effects_reference_lab.cpp` contains a deliberately simplified one-dimensional depth march. It proves the crossing logic against a depth step and proves the no-hit case against a uniformly distant field. The same lab keeps this oracle separate from TAA, SSAO, volumetrics and particles so one effect's assumptions are not silently reused by another.

The next evidence layer is a 2D projected ray with mip-selected hierarchical-Z traversal, binary refinement, thickness policy and comparison against a geometric ray-tracing oracle.

Related: [[Hardware depth-buffer linearization]], [[Screen-space ray generation and mouse picking]], [[Temporal reprojection and history validation]].
