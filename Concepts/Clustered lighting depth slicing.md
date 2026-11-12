# Clustered lighting depth slicing

Clustered lighting partitions the camera frustum into three-dimensional cells and assigns lights to the cells they can influence. A fragment then tests only the lights attached to its cell rather than every light in the scene.

The screen dimensions are naturally tiled in x and y. Depth needs more care because perspective scenes can span orders of magnitude. Uniform linear z slices often waste many clusters far from the camera or give poor resolution near it.

A common logarithmic mapping for positive camera-space depth $z$ between near $n$ and far $f$ is

$$
t=
\frac{\log(z/n)}{\log(f/n)},
$$

then

$$
slice=
\left\lfloor tN_z\right\rfloor,
$$

clamped to $[0,N_z-1]$.

Equal multiplicative depth ratios occupy equal widths in log space. With $n=0.1$ and $f=1000$, the decades 1→10 and 10→100 span the same number of ideal log slices.

## Light assignment

Depth slicing alone does not make a clustered renderer. For each point/spot light, determine a conservative cluster range from its influence volume. Then append the light ID to every overlapping cluster.

The GPU implementation usually has two allocation problems:

1. count how many light references each cluster needs;
2. prefix-scan those counts to obtain compact non-overlapping list offsets;
3. fill the light-index array.

Atomic append alternatives exist, but they require overflow policy and contention measurement.

## False positives versus false negatives

A conservative cluster test may include a light that ultimately contributes nothing to a fragment. That costs shading work but preserves correctness. A false negative drops illumination and is a correctness bug.

Cluster bounds therefore need to match the exact camera projection/depth convention, including reversed-Z if hardware depth is used to derive z.

## Executable reference

tools/labs/realtime_rendering_reference_lab.cpp validates the logarithmic slice mapping, near/far clamping, monotonicity and equal-decade behavior. It does not yet implement full 3D light-volume/cluster intersection or GPU compaction.

Related: [[Prefix scan and stream compaction]], [[Reversed-Z depth contracts]].
