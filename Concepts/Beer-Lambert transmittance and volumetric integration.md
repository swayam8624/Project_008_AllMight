# Beer–Lambert transmittance and volumetric integration

A participating medium changes radiance continuously along a ray rather than only at surfaces. The simplest useful quantity is **transmittance**: the fraction of radiance that survives extinction over distance.

For homogeneous extinction coefficient $sigma_tge0$ and distance $dge0$,

$$
T(d)=e^{-sigma_t d}.
$$

This has an important composition invariant:

$$
T(d_1+d_2)=T(d_1)T(d_2).
$$

That identity is a powerful implementation test for homogeneous media and explains why optical depth adds while transmittance multiplies.

## Extinction is not the whole volume equation

$sigma_t$ combines absorption and out-scattering. Real participating-media rendering also needs emission and in-scattering, usually integrated along the ray. A froxel renderer discretizes space and estimates those terms over finite cells; step size, phase functions, shadowing and temporal reprojection then become part of the error model.

A naive linear attenuation such as $1-sigma_t d$ can become negative and does not compose correctly. The exponential law is the correct homogeneous solution of

$$
rac{dL}{ds}=-sigma_t L.
$$

## Executable reference

`tools/labs/realtime_effects_reference_lab.cpp` checks zero-extinction and zero-distance limits, a known $e^{-1}$ case, and multiplicative composition over concatenated segments. These tests establish the attenuation contract independently of any particular GPU volume grid.

The next layer is numerical integration of spatially varying $sigma_t$, single scattering with a declared phase function, shadow transmittance, froxel reconstruction and comparison against a higher-resolution or path-traced reference.

Related: [[Definite integrals and the fundamental theorem]], [[Numerical quadrature and error]], [[Light spectra and radiometry]].
