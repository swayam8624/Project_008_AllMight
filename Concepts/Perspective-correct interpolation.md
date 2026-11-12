# Perspective-correct interpolation

Rasterization computes barycentric coordinates in screen space, but many vertex attributes are defined as linear functions on the original 3D primitive before perspective division. Directly interpolating those attributes with screen-space barycentrics therefore produces the wrong result.

Let clip-space vertex $i$ have homogeneous coordinate $w_i$ and attribute $a_i$. If screen-space barycentric weights at a sample are $\lambda_i$, perspective-correct interpolation is

$$
a=
\frac{
\sum_i \lambda_i a_i/w_i
}{
\sum_i \lambda_i /w_i
}.
$$

The denominator reconstructs the projective scaling that was lost during perspective division.

## Why affine screen interpolation fails

Imagine two vertices at different depths but carrying texture coordinates that vary linearly across the original triangle. Perspective makes equal screen distances correspond to unequal distances on the surface. Affine screen interpolation assumes the opposite, so textures visibly “swim” or warp across polygons.

The derivation can be viewed by interpolating homogeneous quantities before division: both $a/w$ and $1/w$ are affine in screen barycentrics, then dividing recovers $a$.

## Depth is a separate contract

Do not blindly apply the same formula to every quantity named “depth.” Hardware depth after projection/divide is designed to interpolate affinely in screen space under the pipeline’s projective construction. Camera-space $z$, Euclidean range and reciprocal depth are different quantities.

The shader/runtime must know which one it is carrying.

## Derivatives

Texture gradients used for mip/anisotropic filtering are derivatives of the perspective-correct attribute with respect to screen position. Finite differences across neighboring fragments approximate that quotient behavior; manually reconstructing derivatives requires differentiating the quotient rather than differentiating the raw affine numerator alone.

## Executable reference

tools/labs/raster_reference_lab.cpp chooses a triangle with unequal reciprocal-w values and verifies that affine interpolation differs from the perspective-correct result. The same lab tests top-left edge ownership and depth ordering.

Teach and reconstruct: [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Decide which samples a triangle owns]].

Related: [[Homogeneous clipping and perspective interpolation]], [[Reversed-Z depth contracts]].
