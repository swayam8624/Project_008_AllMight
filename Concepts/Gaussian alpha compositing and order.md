# Gaussian alpha compositing and order

A splat rasterizer typically evaluates an opacity contribution for each projected Gaussian and composites visible contributors in depth order. Front-to-back compositing keeps a running transmittance

$$
T_0=1,
$$

and for layer $i$ with opacity $\alpha_i$,

$$
w_i=T_i\alpha_i,
\qquad
C \leftarrow C+w_i c_i,
\qquad
T_{i+1}=T_i(1-\alpha_i).
$$

After all layers, accumulated opacity is

$$
A=1-T
=
1-\prod_i(1-\alpha_i).
$$

Opacity accumulation is commutative under this formula, but color is generally **not** because earlier colors receive larger transmittance weights. Therefore sorting or another order-correct method is part of the image definition, not just an optimization.

## Early termination

When $T$ becomes small, later layers can contribute at most a small amount if color values are bounded. An early-out threshold is therefore an approximation with a measurable error bound. The threshold should not be mixed into the layer before deciding whether that layer contributes; candidate-transmittance and post-accumulation stopping can differ by one visible contributor.

## Numerical stability

Large numbers of tiny alpha values can accumulate rounding error. Clamping alpha should follow a stated kernel/model rule rather than hide negative or greater-than-one values produced by an earlier bug.

Sorting precision also matters. If depth keys quantize several splats to the same value, the tie policy should be deterministic if reproducible images are required.

## Executable witness

tools/labs/gaussian_projection_reference_lab.cpp composites two colored layers in both orders. It proves equal final opacity but different color, making the ordering dependency observable.

Related: [[Gaussian covariance projection and footprint]], [[Sampling support and missing strata]].
