# Shadow filtering: PCF and variance bounds

A shadow map stores visibility information from a light-space camera. At shading time the receiver is projected into shadow-map coordinates and compared against stored depth.

The raw binary comparison is discontinuous. Filtering changes the estimator, and different methods filter different mathematical quantities.

## Percentage-closer filtering

PCF performs several depth comparisons and averages the binary visibility results:

$$
V=
\frac{1}{N}
\sum_i
\mathbf 1[z_{receiver}\le z_i+b],
$$

where $b$ is a declared bias under the shadow-map depth convention.

Filtering depths first and comparing once is not equivalent to averaging comparisons.

Bias is not a free visual knob. Constant/slope/normal-offset policies trade acne against detached shadows. The correct sign also depends on depth convention.

## Variance shadow maps

VSM stores moments such as

$$
\mu=E[Z],
\qquad
E[Z^2].
$$

Variance is

$$
\sigma^2=E[Z^2]-\mu^2.
$$

For receiver depth $t>\mu$, a one-sided Chebyshev/Cantelli-style bound commonly used by VSM gives a maximum probability estimate of being on the visible side:

$$
p_{max}
=
\frac{\sigma^2}
{\sigma^2+(t-\mu)^2}.
$$

The moment representation can be filtered efficiently, but the probabilistic bound can cause light bleeding. Minimum variance clamps and light-bleeding reduction modify behavior and should be treated as approximations.

## Executable reference

tools/labs/realtime_rendering_reference_lab.cpp computes a 3×3 PCF fraction on a hand-checkable map and checks that the VSM upper bound stays in [0,1] and decreases as a receiver moves farther beyond the mean blocker depth.

This does not prove cascade selection, perspective aliasing control, receiver-plane bias, EVSM, GPU sampling state or production shadow quality.

Related: [[Reversed-Z depth contracts]], [[Sampling support and missing strata]].
