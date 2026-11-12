# Gaussian spherical harmonics appearance

A 3D Gaussian can carry more than geometry and opacity. In view-dependent Gaussian splatting, each primitive may store coefficients of **spherical harmonics (SH)** so its emitted/displayed color changes with viewing direction without storing an arbitrary function over the sphere.

## A directional function in a finite orthonormal basis

For a unit direction $omega$, a scalar directional signal can be approximated by

$$
f(omega)
=
sum_{ell=0}^{L}
sum_{m=-ell}^{ell}
c_{ell m}Y_{ell m}(omega).
$$

For degree $L$, there are $(L+1)^2$ coefficients. Degree 0 carries one direction-independent term. Degrees 1, 2 and 3 add progressively finer angular variation, giving 4, 9 and 16 total basis functions respectively.

The basis convention matters. Real SH formulas can differ by signs, ordering and normalization conventions while spanning the same function space. A file format, training pipeline, CPU oracle and GPU shader therefore need one explicit coefficient order and one explicit view-direction convention. "SH degree 3" alone does not define interoperable bytes.

## Why orthonormality is the useful invariant

For an orthonormal real basis,

$$
int_{S^2}
Y_i(omega)Y_j(omega),domega
=
delta_{ij}.
$$

This means a coefficient measures the projection of the directional function onto one basis mode. It also gives a powerful independent test: numerically integrate pairwise products over many nearly uniform sphere directions and require diagonal entries near one and cross terms near zero.

The constant basis value

$$
Y_{00}=rac{1}{2sqrt{pi}}
$$

lets a direction-independent scalar $k$ be encoded with coefficient $c_{00}=k/Y_{00}$ when all higher coefficients are zero.

## Gaussian-renderer boundary

The direction fed into the SH basis must be declared. A common renderer evaluates a normalized direction related to camera position and Gaussian center, but whether that direction is camera-to-point or point-to-camera changes odd-degree signs. Color-space/clamping policy also sits outside the basis math: SH can reconstruct negative values or values above one, and the renderer must define what happens next.

Higher SH degree increases storage and evaluation cost per splat. It may model view-dependent appearance better, but it does not fix geometry, exposure, opacity ordering, or insufficient training views.

## Executable oracle

`tools/labs/gaussian_spherical_harmonics_lab.cpp` states one explicit real-SH convention through degree 3, checks known pole values, numerically integrates the Gram matrix of all 16 basis functions over a large Fibonacci-sphere sample set, and verifies direction-independent reconstruction from the $l=0$ coefficient.

That establishes basis mathematics independently of Maveb/Vulkax. It does **not** claim coefficient-order/sign parity with a particular trained 3DGS checkpoint, GPU shader parity, learned coefficient quality, color post-processing behavior, or current production integration.

Related: [[Radiance fields and Gaussian splatting]], [[Gaussian covariance projection and footprint]], [[Sampling aliasing and reconstruction]].
