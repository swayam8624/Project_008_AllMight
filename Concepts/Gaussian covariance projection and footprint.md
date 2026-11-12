# Gaussian covariance projection and footprint

A 3D Gaussian does not project to screen by simply projecting its center and keeping the same radius. Perspective is locally nonlinear, so the usual rasterization approximation linearizes the camera projection at the Gaussian center and propagates the 3D covariance through that local Jacobian.

Let camera-space point be

$$
p=(x,y,z),
$$

with pinhole projection

$$
u=f_x\frac{x}{z},
\qquad
v=f_y\frac{y}{z}.
$$

The Jacobian with respect to $(x,y,z)$ is

$$
J=
\begin{bmatrix}
f_x/z & 0 & -f_xx/z^2\\
0 & f_y/z & -f_yy/z^2
\end{bmatrix}.
$$

If the 3D covariance in camera coordinates is $\Sigma_c$, first-order covariance propagation gives

$$
\Sigma_{2D}=J\Sigma_cJ^{\mathsf T}.
$$

This equation has useful invariants. If $\Sigma_c$ is positive semidefinite, $\Sigma_{2D}$ is also positive semidefinite because for any screen vector $q$,

$$
q^{\mathsf T}\Sigma_{2D}q
=
(J^{\mathsf T}q)^{\mathsf T}\Sigma_c(J^{\mathsf T}q)
\ge0.
$$

Symmetry should also be preserved numerically up to rounding.

## World covariance to camera covariance

If a Gaussian covariance is stored in world coordinates and the camera’s linear world-to-camera transform is $R$, then

$$
\Sigma_c=R\Sigma_wR^{\mathsf T}.
$$

For a rigid camera rotation, $R$ is orthogonal. For a similarity transform containing scale $s$, covariance scales by $s^2$ as well as rotating. Forgetting that square is a common alignment bug.

Many splat systems parameterize a positive covariance from scale and rotation rather than storing an arbitrary symmetric matrix. Whatever parameterization is used, the runtime should preserve positive-semidefinite structure instead of allowing an unconstrained matrix to become an invalid ellipse.

## Ellipse footprint

A symmetric 2×2 projected covariance has eigenvalues $\lambda_1,\lambda_2\ge0$. The square roots are standard deviations along the principal screen-space axes. A conservative raster footprint can bound a chosen multiple of those standard deviations.

Adding a low-pass variance floor can stabilize subpixel Gaussians, but it changes the effective footprint. That is a quality/stability trade-off and should be measured, especially temporally.

## Executable witness

tools/labs/gaussian_projection_reference_lab.cpp:

- finite-differences the pinhole projection and checks the analytic Jacobian;
- computes $J\Sigma J^T$;
- checks projected symmetry and PSD eigenvalues;
- verifies that doubling focal length scales covariance eigenvalues by four;
- includes low-order spherical-harmonic and compositing checks.

This is an independent CPU reference layer. It does not prove the current Metal/CUDA/Vulkan Gaussian implementation.

Related: [[Radiance fields and Gaussian splatting]], [[Captured-world similarity alignment]], [[Rendering history validity]].

## Conic inversion, tile duplication and depth order

Projection produces a 2D covariance, but a splat rasterizer normally consumes its inverse as a **conic**. For

$$
\Sigma=
\begin{bmatrix}
a&b\\
b&c
\end{bmatrix},
\qquad
\det\Sigma=ac-b^2>0,
$$

the inverse is

$$
\Sigma^{-1}
=
\frac{1}{ac-b^2}
\begin{bmatrix}
c&-b\\
-b&a
\end{bmatrix}.
$$

For a pixel displacement $d=(\Delta x,\Delta y)$, the Mahalanobis exponent uses $d^T\Sigma^{-1}d$. A conservative screen radius can be built from the largest eigenvalue of $\Sigma$; the resulting box determines which tiles receive a duplicate splat record. Those records must then be grouped by tile and ordered front-to-back before alpha compositing.

`tools/labs/gaussian_tile_conic_lab.cpp` now verifies the 2D inverse, positive conic evaluation, eigenvalue footprint bound, conservative tile duplication, stable tile/depth ordering, and the front-to-back transmittance identity. GPU radix sorting and exact ellipse-tile rejection remain separate native-performance layers.

