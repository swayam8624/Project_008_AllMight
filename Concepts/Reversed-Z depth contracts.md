# Reversed-Z depth contracts

A depth buffer stores an ordering coordinate, not Euclidean distance. Reversed-Z changes the mapping so near geometry receives larger depth values and far geometry approaches zero. The precision advantage comes from combining the reciprocal structure of perspective depth with floating-point numbers whose representable values are denser near zero.

The entire renderer has to agree on the convention. Reversing only the projection matrix while keeping old clear/comparison assumptions creates a system that is internally contradictory.

## Contract checklist

For a conventional zero-to-one API depth range, a common reversed-Z setup uses:

- near plane mapped near 1;
- far distance mapped toward 0;
- depth buffer cleared to 0;
- comparison set to GREATER or GREATER_EQUAL;
- hierarchical depth/occlusion reductions defined consistently with “larger is nearer”;
- screen-space reconstruction derived from the exact projection convention.

Infinite-far reversed projection can remove an explicit far-plane distance from the perspective matrix, but frustum/culling policy still needs a practical world-space bound when the application cannot process infinite geometry.

## Why floating point helps

Perspective depth is approximately affine in $1/z$. Conventional mapping spends many floating-point representable values near depth one, where far geometry is crowded. Reversed-Z places far geometry near zero, where floating-point exponent scaling gives much more useful relative spacing.

With integer/fixed-point depth, reversal alone does not create the same dramatic distribution advantage; the floating-point representation is part of the argument.

## Reconstruction must use the actual matrix

Do not paste a “linearize depth” formula from another API. Different clip-space ranges, handedness, finite/infinite far planes and reversed mappings change the coefficients.

A robust reconstruction derives camera-space position from the same projection matrix or stores explicit projection parameters and tests known near/far points.

## Occlusion and temporal systems

Hierarchical Z, SSAO, SSR, clustered depth slicing, picking and temporal rejection all consume depth meaning. Every reduction or comparison must be audited when the convention changes. A min-reduction hierarchy written for standard depth may need a max-reduction interpretation under reversed-Z.

## Tests

Project known camera-space depths, confirm monotonic ordering, quantize them to the real depth format, reconstruct them, and measure relative world-space error over logarithmically spaced distances. Also test clear-value behavior with an empty frame and a near geometry sample.

Teach and reconstruct: [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Give depth an explicit meaning]].

Related: [[Hardware depth-buffer linearization]], [[Rendering history validity]].
