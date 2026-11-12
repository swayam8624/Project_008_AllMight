#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
NOTE = ROOT / "Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography.md"

def code_block(path, language):
    return "\n\n~~~" + language + "\n" + path.read_text().rstrip() + "\n~~~\n"

def insert_or_refresh_before(text, anchor, marker, body):
    if anchor not in text:
        raise SystemExit("missing anchor: " + anchor)
    normalized = body.strip("\n")
    if marker in text:
        start = text.index(marker)
        while start > 0 and text[start - 1] == "\n":
            start -= 1
        end = text.index(anchor, start)
        return text[:start] + "\n\n" + normalized + "\n\n" + text[end:]
    return text.replace(anchor, normalized + "\n\n" + anchor, 1)

text = NOTE.read_text()

imu_code = code_block(ROOT / "tools/labs/xr_3d_imu_preintegration_lab.py", "python")
nerf_code = code_block(ROOT / "tools/labs/nerf_volume_rendering_lab.py", "python")

imu = """
<!-- Bible-final-pass: XR 3D VIO spine -->
#### From raw 3-D IMU samples to the pose constraint used by VIO

The planar EKF above is useful because every state variable and Jacobian can be inspected. A real headset, however, rotates around all three axes and accelerates in three dimensions. We therefore need the next bridge: not a second list of VIO vocabulary, but the exact path from high-rate IMU samples to a constraint that a filter or sliding-window optimizer can use.

Keep the frame convention from the previous section. Let R_i be body-to-world orientation at keyframe i, p_i world position, v_i world velocity, and g world gravity. The IMU reports

~~~text
omega_m = omega + b_g + n_g
f_m     = R^T (a_world - g) + b_a + n_a
~~~

The accelerometer measures specific force, not world acceleration. That one sentence explains a common debugging surprise: a level, stationary accelerometer can report roughly +9.81 m/s^2 along its upward body axis. After rotating that specific force into the world and adding world gravity, the net translational acceleration is zero.

If every IMU sample were integrated from the beginning of the trajectory each time an optimizer changed an old pose, optimization would be wasteful. **Preintegration** compresses all IMU samples between two keyframes into a relative motion summary.

For samples between i and j, define three quantities in frame i:

~~~text
Delta R_ij   relative rotation
Delta v_ij   gravity-free relative velocity change
Delta p_ij   gravity-free relative position change
~~~

Under a zero-order-hold discretization with one sample interval dt,

~~~text
Delta R <- Delta R Exp((omega_m - b_g) dt)

a_local <- Delta R_before (f_m - b_a)

Delta p <- Delta p + Delta v dt + 1/2 a_local dt^2
Delta v <- Delta v + a_local dt
~~~

The order matters. The force sample for that interval is rotated by the relative orientation at the start of the interval in this simple discretization.

After the whole interval has been compressed, the later navigation state is predicted by

~~~text
R_j = R_i Delta R_ij

v_j = v_i + g Delta t + R_i Delta v_ij

p_j = p_i + v_i Delta t
            + 1/2 g Delta t^2
            + R_i Delta p_ij
~~~

Notice where gravity appears: not inside the stored preintegrated delta above, but when the delta is connected to the world state. This separation is what lets one relative IMU constraint be reused while the optimizer changes R_i, p_i, and v_i.

Two consecutive preintegrated intervals also compose without replaying the raw samples. If a is i->j and b is j->k,

~~~text
Delta R_ik = Delta R_ij Delta R_jk

Delta v_ik = Delta v_ij
           + Delta R_ij Delta v_jk

Delta p_ik = Delta p_ij
           + Delta v_ij Delta t_jk
           + Delta R_ij Delta p_jk
~~~

This is not merely an optimization trick. It is a testable algebraic contract. Integrating 200 samples in one pass should agree with integrating the first 100, integrating the second 100 in the intermediate frame, and composing the two deltas.

##### Bias changes the relative motion, so production preintegration carries bias sensitivity

The compact recurrence above assumes a particular gyro bias b_g and accelerometer bias b_a. If the optimizer later changes those bias estimates, the preintegrated delta is no longer exactly correct.

A production preintegrator therefore carries first-order Jacobians such as

~~~text
d(Delta R) / d b_g
d(Delta v) / d b_g
d(Delta v) / d b_a
d(Delta p) / d b_g
d(Delta p) / d b_a
~~~

and a covariance for the preintegrated measurement. Small bias changes can then correct the delta without replaying every IMU sample. Large changes may require reintegration. The exact Jacobian recurrence depends on the left/right perturbation convention and SO(3) Jacobians; copying formulas from a paper while using the opposite rotation convention is a reliable way to build a filter that looks plausible but is inconsistent.

The executable witness below intentionally stops one level before that production complexity. It verifies the 3-D geometry first: SO(3) exp/log, a stationary gravity-cancellation case, constant yaw integration, interval composition, and visible drift when a known gyro bias is ignored.
""" + imu_code + """
This witness reaches reconstructable depth for the **3-D inertial propagation and preintegration subproblem**. It does not claim that a full VIO system is finished.

##### How the preintegrated delta enters a sliding-window optimizer

A keyframe optimizer keeps several camera/IMU states. Between adjacent states it adds an IMU residual. In simplified form,

~~~text
r_R = Log( Delta R_ij^-1 (R_i^T R_j) )

r_v = R_i^T (v_j - v_i - g Delta t) - Delta v_ij

r_p = R_i^T (p_j - p_i - v_i Delta t
             - 1/2 g Delta t^2) - Delta p_ij
~~~

The rotation residual is a three-vector in a tangent space, not nine elementwise matrix differences. Velocity and position residuals are expressed in the same local frame as the preintegrated quantities. Bias random-walk residuals connect b_g and b_a across the interval.

Stack those residuals with visual reprojection residuals. Then a Gauss-Newton or Levenberg-Marquardt iteration repeatedly:

1. evaluates residuals at the current state;
2. builds Jacobians using the declared perturbation convention;
3. whitens residuals using their covariance or information;
4. applies a robust loss where justified;
5. solves the sparse normal-equation or square-root system;
6. retracts rotation increments back onto SO(3);
7. repeats until a defensible stopping rule is met.

The word **factor graph** is simply a useful representation of this sparse dependency structure. Variable nodes hold unknown poses, velocities, biases, landmarks, calibration values, and possibly other states. Factor nodes hold measurements or priors that depend on a small subset of those variables. The graph does not solve the problem by itself; it exposes which blocks of the Jacobian can be nonzero.

##### Filter VIO and sliding-window VIO remove old information differently

An MSCKF-style filter keeps a state estimate and covariance, augments it with a short history of camera poses, and uses feature tracks without permanently adding every feature as a state. Unknown feature position can be eliminated from the measurement constraint, leaving a residual that updates the navigation state.

A sliding-window optimizer instead keeps a recent set of states and often selected landmarks, then jointly minimizes visual and inertial residuals. When the window grows, old variables are **marginalized**. Marginalization does not mean deleting them and forgetting their information; their local information is folded into a prior on the variables that remain.

That prior is tied to the linearization point used when marginalization happened. Repeatedly treating it as if it were a fresh nonlinear measurement can create inconsistency. This is one reason VIO papers discuss first-estimate Jacobians, observability, square-root marginalization, or other consistency choices.

##### Observability tells us which errors measurements can actually remove

With only inertial data, position and velocity drift because small bias errors integrate over time. Vision constrains relative motion, but not every global degree of freedom is automatically observable.

In a typical monocular visual-inertial system with gravity estimated, global translation and global yaw remain gauge freedoms unless an external reference fixes them. Scale becomes observable through the metric accelerometer only when motion provides enough excitation and the calibration/model is adequate. A stationary or weakly excited initialization can therefore be mathematically ambiguous even with perfectly written code.

This changes how we test the estimator. A singular direction caused by a real gauge freedom should not be “fixed” by secretly adding numerical damping and then presented as newly observed information. Choose a gauge explicitly, add a stated prior, or compare quantities invariant to that gauge.

##### SLAM adds map identity, loop closure, and relocalization

VIO answers: “How did I move recently?” SLAM also asks: “Have I been here before, and can old map evidence reduce accumulated drift?”

A loop-closure pipeline needs a place-recognition candidate, geometric verification, a constraint between distant poses, and an optimization strategy that can distribute the correction without tearing the local map apart. Relocalization is related but asks for the current camera pose in an existing map after tracking was lost.

A false loop is usually more damaging than a missed loop because it adds a strong incorrect global constraint. Descriptor similarity alone is therefore not enough; geometric verification and robust graph optimization are part of the correctness path.

##### XR adds one final state: the pose at display time

Even a mathematically consistent VIO estimate is usually an estimate at a sensor timestamp. The renderer needs a pose for predicted display time. Keep those operations separate:

~~~text
sensor samples
    -> estimator state at estimation time
    -> short-horizon prediction to display time
    -> render view
    -> optional later compositor reprojection
    -> photons
~~~

Prediction should propagate not only a pose value but also an age/uncertainty story. A six-millisecond prediction during smooth head motion is a different evidence regime from a long prediction during an abrupt turn. OpenXR's predicted display time is therefore part of the pose contract, not a convenience timestamp.

**Reconstruct without reading:** derive the three preintegration recurrences, derive the state reconstruction equations containing gravity, compose two preintegrated intervals, explain why bias Jacobians are needed, write the three simplified IMU residuals, and distinguish MSCKF elimination, sliding-window marginalization, loop closure, relocalization, and display-time prediction. If those five ideas collapse into the word “tracking,” the pipeline is not yet understood.
"""

nerf = """
<!-- Bible-final-pass: neural volume rendering -->
## A NeRF pixel is an integral before it is a neural-network output

The phrase “NeRF uses an MLP to synthesize new views” skips the part that makes the method understandable. The network does **not** directly have to output a finished pixel. It represents a field that is sampled along a camera ray, and a differentiable volume-rendering equation turns those samples into the pixel used by the loss.

Let a camera ray be

~~~text
r(t) = o + t d
~~~

with origin o, direction d, and distance parameter t. At a sample position x = r(t), a basic NeRF-style field predicts

~~~text
sigma(x) >= 0              volume density
c(x, d)                     emitted/view-dependent RGB color
~~~

Density says how quickly light traveling along the ray is terminated. Color says what radiance is contributed if the ray terminates there.

### Continuous transmittance explains the discrete alpha formula

Suppose density along the ray is sigma(s). The probability-like fraction of light that survives from the ray start t_n to a later point t is the **transmittance**

~~~text
T(t) = exp( - integral[t_n to t] sigma(s) ds )
~~~

The rendered color is

~~~text
C(r) = integral T(t) sigma(r(t)) c(r(t), d) dt
       + T(t_f) C_background
~~~

This equation says something simple: a sample contributes strongly only if the ray first survives to that sample and then terminates there.

A computer uses finite intervals. If sample i represents an interval of length delta_i and density is treated as constant over that interval,

~~~text
alpha_i = 1 - exp(-sigma_i delta_i)
~~~

is the opacity of that interval. The transmittance reaching sample i is

~~~text
T_i = product over j<i of (1 - alpha_j)
~~~

and its rendering weight is

~~~text
w_i = T_i alpha_i.
~~~

The discrete pixel is therefore

~~~text
C = sum_i w_i c_i + T_end C_background.
~~~

The weights plus the surviving background transmittance sum to one, up to floating-point error. That gives us an immediate implementation invariant.

### Why density is not the same thing as opacity

Density sigma has units of inverse distance. Opacity alpha is dimensionless and depends on both density and interval length.

If the same density is sampled using a larger delta, alpha becomes larger. This is why changing ray-sampling spacing without changing the rendering equation changes the model. A paper that reports “128 samples per ray” is also telling us something about numerical integration and training cost.

The exponential form also keeps opacity in [0,1) for finite nonnegative density. Implementations often produce a raw neural value and map it through a nonnegative activation or another density parameterization rather than allowing negative physical density.

### The renderer is differentiable because every weight is a smooth chain of operations

For a scalar-color version of the ray,

~~~text
C = sum_i T_i alpha_i c_i + T_end c_bg.
~~~

The derivative with respect to color is especially clear:

~~~text
dC / dc_i = w_i.
~~~

A color at a fully occluded sample receives almost no gradient because its weight is tiny.

Density is more interesting. Increasing sigma_i increases alpha_i, which makes sample i contribute more but makes every later sample and the background contribute less. Let C_after_i be the color the remaining tail would produce if its incoming transmittance were reset to one. Then

~~~text
dC / d sigma_i
    = T_i delta_i (1 - alpha_i) (c_i - C_after_i).
~~~

The sign has a physical interpretation. If the current sample is brighter than what lies behind it, increasing its density raises the pixel. If it is darker, increasing density hides a brighter tail and lowers the pixel.

This is the missing bridge between rendering and learning: the photometric loss sends gradients through transmittance into density, color, and eventually the field parameters.

The standalone witness below implements the compositing equation, checks the weight partition, derives analytic gradients, verifies every density/color derivative against finite differences, performs a tiny inverse-rendering optimization, demonstrates one-pixel non-identifiability, and builds a positional encoding.
""" + nerf_code + """
The tiny optimization should not be mistaken for “training NeRF.” Its purpose is to prove the differentiable rendering path on an inspectable problem.

### One pixel is not enough to recover a unique scene

The witness deliberately constructs two different density/color pairs that produce the same pixel. This is not a bug in gradient descent. It is an **inverse-problem ambiguity**.

Real NeRF training reduces that ambiguity by using many calibrated images whose rays intersect the same scene from different viewpoints. Even then, the result depends on camera accuracy, coverage, appearance assumptions, regularization, network capacity, and the sampled training distribution. A model can reproduce training views while holding geometry that is poor for collision or relighting.

This is why image reconstruction quality and geometric correctness must be evaluated separately.

### Positional encoding changes the function class the network can learn efficiently

A plain coordinate MLP tends to learn low-frequency variation first. The original NeRF maps a coordinate through sinusoidal functions at multiple frequencies before the MLP sees it. In one dimension a simple form is

~~~text
gamma(x) =
[x,
 sin(pi x), cos(pi x),
 sin(2 pi x), cos(2 pi x),
 ...].
~~~

The encoding does not create missing scene information. It gives the network basis features that make high-frequency spatial variation easier to represent.

Instant-NGP changes the parameterization again. Instead of a large fixed sinusoidal expansion feeding a deep MLP, it learns multiresolution grid features stored through hash tables and uses a much smaller network. The rendering equation can remain NeRF-like while the representation and training speed change dramatically.

This distinction is important:

~~~text
field quantity
    != field parameterization
    != ray sampler
    != volume compositor
    != training objective
~~~

A paper may innovate in one of these layers while leaving the others nearly unchanged.

### Sampling is part of both bias and cost

A basic ray sampler places points between near and far bounds. If important density lies between poorly chosen samples, the discrete integral can miss it.

Hierarchical sampling uses an earlier pass to estimate where ray weight is concentrated, then spends more samples there. Occupancy grids and empty-space skipping avoid querying regions believed to contain no useful density. Instant-NGP relies heavily on this systems side of the problem.

Mip-NeRF changes a different assumption: one pixel corresponds to a footprint, not an infinitesimal line. It integrates encoded features over a conical frustum region so distant high-frequency detail is prefiltered rather than point-sampled into aliasing.

No acceleration method earns correctness merely by reducing network evaluations. It must preserve the quantity the renderer is trying to estimate closely enough for the claim being made.

### A complete NeRF training loop has several contracts

A minimal research implementation needs to make each of these explicit:

1. **camera model:** intrinsics, extrinsics, distortion handling, near/far bounds;
2. **ray generation:** which world-space origin and direction correspond to each training pixel;
3. **sampling:** stratified, hierarchical, occupancy-guided, or another declared policy;
4. **field query:** position/view encoding and the mapping to density/color;
5. **compositing:** alpha, transmittance, weights, background policy;
6. **loss:** usually some photometric term plus any regularizers;
7. **optimization:** optimizer, learning-rate schedule, precision, batch/ray count;
8. **validation:** held-out views and metrics not used to choose every training decision;
9. **geometry claim:** separate surface/depth tests if the paper claims geometry;
10. **systems claim:** training time, memory, render latency, hardware, and precision.

If any one of those is hidden behind “we trained a NeRF,” reproduction becomes guesswork.

### 3D Gaussian Splatting changes the representation and rasterization path, not the need for training discipline

A 3-D Gaussian primitive stores a world-space mean, covariance/scale-and-rotation, opacity, and appearance coefficients such as spherical harmonics. Rendering projects the Gaussian into screen space, determines which pixels it overlaps, sorts or otherwise orders visible contributions, and alpha-composites them.

Training commonly begins from a Structure-from-Motion point cloud when available. Then gradient descent adjusts positions, anisotropic shape, opacity, and appearance so rendered training views match captured images.

The representation has a special problem: the right number and placement of primitives are not known in advance. **Densification** changes model structure during optimization. Underfit regions may clone or split Gaussians according to gradient/size criteria; useless or nearly transparent primitives may be pruned. Some implementations periodically reset or control opacity so early saturation does not permanently block useful gradients.

That means a faithful 3DGS reproduction needs more than the rasterizer. It needs the training lifecycle:

~~~text
camera/SfM initialization
 -> differentiable splat rasterization
 -> photometric loss
 -> parameter update
 -> densification / split / clone
 -> pruning / opacity policy
 -> repeat
 -> evaluate held-out views and geometry/system behavior
~~~

Changing the densification thresholds, schedule, opacity policy, image resolution, or initialization can change quality, memory, and training time. These are algorithmic parameters, not housekeeping.

The later 2DGS, Mip-Splatting, 3DGUT and surface-oriented families should now be read against this baseline. They change geometry bias, filtering, projection, or robustness, but they still need an explicit optimization and evaluation contract.

**Reconstruct without reading:** derive alpha from constant density over an interval; derive transmittance and w_i; explain why dC/dc_i equals w_i; explain why density affects all later samples; identify the ambiguity demonstrated by the lab; then write the ten contracts of a complete NeRF experiment and the training lifecycle of 3DGS. If you can only name architectures, you do not yet understand neural rendering as an algorithm.
"""

text = insert_or_refresh_before(
    text,
    "### Calibration errors can masquerade as rendering errors",
    "<!-- Bible-final-pass: XR 3D VIO spine -->",
    imu,
)
text = insert_or_refresh_before(
    text,
    "## Radiance fields are representations, not necessarily MLPs",
    "<!-- Bible-final-pass: neural volume rendering -->",
    nerf,
)

text = insert_or_refresh_before(
    text,
    "## Neural shading keeps an explicit renderer and learns a local function",
    "<!-- Bible-final-pass: score distillation -->",
    sds,
)
text = insert_or_refresh_before(
    text,
    "## Spatial audio is a geometry-and-filtering pipeline",
    "<!-- Bible-final-pass: XR human study design -->",
    human,
)

NOTE.write_text(text)
print("graphics final-pass patch applied")
