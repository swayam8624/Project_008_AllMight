# 28 — Reliable Optimization and Root Finding

[[Chapter Edition/01 - Computing Foundations/00 - Reading Map|Reading map]] · [[Chapter Edition/01 - Computing Foundations/27 - Gauss–Newton for Geometry and Pose|Previous chapter]] · [[Chapter Edition/01 - Computing Foundations/29 - Discretization, Convergence, and Experimental Design|Next chapter]]

A practical optimizer must report why it stopped, whether derivatives agree with independent checks, and how a line search or linear solve failed. This chapter turns optimization into an auditable implementation and extends the same evidence discipline to root-finding certificates.

**By the end:** Implement and assess optimization and root-finding procedures with finite checks, stopping criteria, failure statuses, and independent numerical witnesses.

**Recall question:** What evidence distinguishes true convergence from a tiny step, failed line search, or inaccurate derivative?

<!-- Derived from Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md; source lines 66237–69719; source-span SHA256 3d25a4f2f3817ceaecfe61e6825d7467cf99feaca3333d4a92974bc19d84b4aa. -->

<!-- CHAPTER-SOURCE-BEGIN -->
## Reliable optimization in practice
### Building a numerically tested gradient-based optimizer

This final part brings together the entire phase.

We will construct and verify a practical optimizer using principles from M049–M054.

The implementation should not merely return a parameter vector. It should also report whether the result is finite, which stopping condition was met, whether a line search or linear solve failed, and whether gradient calculations agree with independent numerical checks.

The source claims that it built and compiled a complete standalone C++23 optimization laboratory. Its downloadable source is not present in the supplied attachment, and the result has not been reproduced here.

Phase 9 C++23 optimization laboratory

Verified

The source implements safeguarded gradient descent, an educational full-memory BFGS solver, scaled-damping Levenberg–Marquardt, projected gradient descent, independent finite-difference gradient tests, and runtime assertions.

Download the complete C++23 implementation

Standalone source file, designed for reconstruction and experimentation before adapting the relevant pieces to KairoMath.

Compile it with:

```cpp
c++ -std=c++23 -O2 -Wall -Wextra -Wpedantic \
  phase9_m053_m054_optimization.cpp \
  -o phase9_optimization

./phase9_optimization
```

The cited full source was not present in the supplied attachment. The available excerpts are retained as code fragments; no run result is asserted in this draft.

Its output from the executed tests was:

EXECUTION RESULTS

| Algorithm                    | Result                    | Iterations / accepted steps |
| ---------------------------- | ------------------------- | --------------------------- |
| Gradient descent, Rosenbrock | (0.99999922, 0.99999843)  | 13,756                      |
| BFGS, Rosenbrock             | (1.00000000, 0.99999999)  | 34                          |
| LM camera calibration        | (0.12000000, -0.20000000) | 5 accepted                  |
| Projected gradient           | (1.00000000, -0.50000000) | Converged                   |

Source-reported result; not independently reproduced in this pass.

These results are from deliberately controlled examples. They demonstrate that the implementations behave as intended on those tests, not that their performance ranking generalizes to every objective.

In particular, the educational BFGS implementation uses Armijo backtracking with explicit curvature checks and reset logic. It is not a replacement for a mature strong-Wolfe or damped-BFGS implementation.

#### How the validated gradient-descent implementation works

The essential iteration is:

```cpp
// Current state:
Vec2 x;
double currentValue = objective(x);

// Calculate the exact supplied analytical gradient.
const Vec2 g = gradient(x);

// Ensure the gradient is finite before using it.
if (!finite(g))
    return numericalFailure;

// For an unconstrained smooth objective, the negative
// gradient is a descending direction unless g == 0.
const Vec2 direction = -1.0 * g;

// Armijo backtracking searches for an acceptable step.
double alpha = 1.0;

while (true) {
    Vec2 candidate = x + alpha * direction;

    // The actual objective must satisfy sufficient decrease.
    if (objective(candidate) <= currentValue
        + 1e-4 * alpha * dot(g, direction)) {
        x = candidate;
        break;
    }

    alpha *= 0.5;
}
```

The downloadable source adds finite-value checks, maximum trial counts, representable-step checks, and explicit failure statuses.

These safeguards are important because a mathematical update formula is not enough to define a reliable solver.

A solver is a state machine: it must handle successful updates, rejected updates, invalid models, and termination.

#### The nonlinear camera calibration experiment

The camera experiment is especially relevant to reconstruction.

Suppose the observed horizontal pixel coordinate is:

$$
u_i^{\mathrm{obs}} = f_x\frac{X_i-t_x^\star}{Z_i-t_z^\star}+c_x.
$$

We generate observations from known parameters:

$$
\boxed{ t_x^\star=0.12\ \mathrm m, \qquad t_z^\star=-0.20\ \mathrm m }.
$$

The optimizer does not receive those target parameter values directly. It receives only the known 3D points and generated pixel observations.

Its unknown parameter vector is:

$$
\mathbf x= \begin{bmatrix} t_x\\t_z \end{bmatrix}.
$$

The residual for observation $i$ is:

$$
\boxed{ r_i(\mathbf x) = f_x\frac{X_i-t_x}{Z_i-t_z} +c_x-u_i^{\mathrm{obs}} }.
$$

The objective is:

$$
E(\mathbf x)=\frac12\sum_i r_i^2.
$$

##### Deriving the camera Jacobian

Differentiate with respect to horizontal translation:

$$
\boxed{ \frac{\partial r_i}{\partial t_x} = -\frac{f_x}{Z_i-t_z} }.
$$

Differentiate with respect to depth translation:

$$
\boxed{ \frac{\partial r_i}{\partial t_z} = \frac{f_x(X_i-t_x)}{(Z_i-t_z)^2} }.
$$

The positive sign in the second derivative is important.

It comes from:

$$
\frac d{dt_z}(Z_i-t_z)^{-1} = (Z_i-t_z)^{-2}.
$$

The code evaluates these derivatives for every observation, forming gradient and Gauss–Newton curvature contributions without needing to explicitly allocate a large Jacobian matrix.

For each residual $r_i$ with Jacobian row:

$$
J_i= \begin{bmatrix} j\_{ix}&j\_{iz} \end{bmatrix},
$$

the gradient contribution is:

$$
J_i^Tr_i= \begin{bmatrix} j\_{ix}r_i\\ j\_{iz}r_i \end{bmatrix}.
$$

The normal-matrix contribution is:

$$
J_i^TJ_i= \begin{bmatrix} j\_{ix}^2&j\_{ix}j\_{iz}\\ j\_{ix}j\_{iz}&j\_{iz}^2 \end{bmatrix}.
$$

Accumulate these over all observations:

$$
\mathbf g=\sum_iJ_i^Tr_i,
$$

$$
H\_{\mathrm{GN}}=\sum_iJ_i^TJ_i.
$$

Then solve:

$$
(H\_{\mathrm{GN}}+\lambda D)\Delta\mathbf x=-\mathbf g.
$$

The implementation uses a diagonal scaling matrix $D$ and a small positive-definite linear solve tailored to the two-parameter teaching example.

For larger production systems, the same mathematics would be implemented using an appropriate sparse factorization, QR method, or iterative solver.

#### The exact parameter recovery

The test initializes the camera translations away from the true values.

After five accepted LM updates, the output is:

$$
\boxed{ (t_x,t_z)=(0.12,-0.20) }
$$

to the displayed numerical precision.

It also verifies the analytical gradient against central finite differences at a separate test point.

The recovered values are meaningful because the generated observations are noise-free, the camera model matches the data-generation model, and the chosen geometry provides sufficient information to identify the two parameters locally in this experiment.

A real calibration system must handle noisy observations, outliers, lens distortion, uncertain 3D points, possible degeneracy, and more parameters.

### A compact standalone numerical workbench

The downloadable quadrature, ODE, and broad optimization programs cited by the supplied source were not present in the supplied attachments, so their code and reported results remain unavailable here. The following is a newly authored, small C++23 teaching program; it is not a transcription of those missing downloads and is not a production numerical library.

It checks composite trapezoid and Simpson quadrature for $e^x$ on $[0,1]$ (with an even Simpson interval count), compares a central-difference derivative with the analytic derivative, and compares Euler and RK4 for $y'=-2y$ at $t=1$. For the harmonic oscillator, it shows explicit Euler energy growth and bounded oscillation under semi-implicit Euler over the stated finite interval; the latter does not conserve energy exactly. It also performs backtracking Newton on $f(x)=\log\cosh x$ from $x_0=3$. This example chooses the conservative Armijo constant $c_1=0.5$ to make the first accepted dyadic fraction $1/32$ visible; that is an illustrative choice, not a universal default.

The assertions are quantitative checks for these fixed inputs. They are not a general accuracy or stability guarantee. The same complete program appears as a standalone file at [`calculus-workbench.cpp`](</Users/swayamsingal/.codex/source-cache/math-cpp-expansion-2026-10-28/verification/calculus-workbench.cpp>).

Compile with assertions enabled:

```bash
/opt/homebrew/bin/clang++ -std=c++23 -O2 -Wall -Wextra -Wpedantic calculus-workbench.cpp -o calculus-workbench
./calculus-workbench
```

```cpp
#include <algorithm>
#include <cassert>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>

namespace {

bool close(double a, double b, double tolerance) {
    return std::abs(a - b) <= tolerance;
}

double trapezoid(double a, double b, int n) {
    assert(n > 0);
    const double h = (b - a) / n;
    double sum = 0.5 * (std::exp(a) + std::exp(b));
    for (int i = 1; i < n; ++i)
        sum += std::exp(a + i * h);
    return h * sum;
}

double simpson(double a, double b, int n) {
    assert(n > 0 && n % 2 == 0);
    const double h = (b - a) / n;
    double sum = std::exp(a) + std::exp(b);
    for (int i = 1; i < n; ++i)
        sum += (i % 2 == 0 ? 2.0 : 4.0) * std::exp(a + i * h);
    return h * sum / 3.0;
}

double centralDerivative(double (*f)(double), double x, double h) {
    return (f(x + h) - f(x - h)) / (2.0 * h);
}

double sine(double x) { return std::sin(x); }

double eulerDecay(double h, int steps) {
    double y = 1.0;
    for (int i = 0; i < steps; ++i)
        y += h * (-2.0 * y);
    return y;
}

double rk4Decay(double h, int steps) {
    double y = 1.0;
    for (int i = 0; i < steps; ++i) {
        const double k1 = -2.0 * y;
        const double k2 = -2.0 * (y + 0.5 * h * k1);
        const double k3 = -2.0 * (y + 0.5 * h * k2);
        const double k4 = -2.0 * (y + h * k3);
        y += h * (k1 + 2.0 * k2 + 2.0 * k3 + k4) / 6.0;
    }
    return y;
}

struct State { double q; double p; };

double energy(State s) { return 0.5 * (s.q * s.q + s.p * s.p); }

State explicitEulerOscillator(State s, double h) {
    return {s.q + h * s.p, s.p - h * s.q};
}

State semiImplicitEulerOscillator(State s, double h) {
    const double qNext = s.q + h * s.p;
    const double pNext = s.p - h * qNext;
    return {qNext, pNext};
}

double logCosh(double x) {
    const double ax = std::abs(x);
    return ax + std::log1p(std::exp(-2.0 * ax)) - std::log(2.0);
}

double firstArmijoNewtonStep(double x, int& backtracks) {
    const double value = logCosh(x);
    const double gradient = std::tanh(x);
    const double hessian = 1.0 / std::pow(std::cosh(x), 2.0);
    assert(std::isfinite(value) && hessian > 0.0);
    const double direction = -gradient / hessian;
    assert(gradient * direction < 0.0);

    // A deliberately conservative valid Armijo constant makes the first
    // accepted dyadic trial easy to inspect in this teaching example.
    constexpr double c1 = 0.5;
    double alpha = 1.0;
    backtracks = 0;
    for (int trial = 0; trial < 64; ++trial) {
        const double candidate = x + alpha * direction;
        if (std::isfinite(candidate)
            && logCosh(candidate) <= value + c1 * alpha * gradient * direction)
            return alpha;
        alpha *= 0.5;
        ++backtracks;
    }
    return 0.0;
}
} // namespace

int main() {
    std::cout << std::setprecision(12);

    // Integrate exp(x) on [0,1]; the exact answer is e - 1.
    constexpr int n = 100;
    const double exactIntegral = std::exp(1.0) - 1.0;
    const double trap = trapezoid(0.0, 1.0, n);
    const double simp = simpson(0.0, 1.0, n);
    assert(std::abs(trap - exactIntegral) < 1.5e-5);
    assert(std::abs(simp - exactIntegral) < 1.0e-9);
    assert(std::abs(simp - exactIntegral) < std::abs(trap - exactIntegral));
    std::cout << "quadrature exact=" << exactIntegral << " trapezoid=" << trap
              << " Simpson=" << simp << '\n';

    // Check a numerical derivative against an independent analytic value.
    constexpr double x = 0.7;
    constexpr double hDiff = 1.0e-4;
    const double fd = centralDerivative(sine, x, hDiff);
    assert(std::abs(fd - std::cos(x)) < 1.0e-8);
    std::cout << "central derivative=" << fd << " exact=" << std::cos(x) << '\n';

    // Solve y'=-2y from y(0)=1 to t=1 with h=0.001.
    constexpr int odeSteps = 1000;
    constexpr double hOde = 1.0 / odeSteps;
    const double exactDecay = std::exp(-2.0);
    const double euler = eulerDecay(hOde, odeSteps);
    const double rk4 = rk4Decay(hOde, odeSteps);
    assert(std::abs(euler - exactDecay) < 3.0e-4);
    assert(std::abs(euler - exactDecay) > 1.0e-5);
    assert(std::abs(rk4 - exactDecay) < 1.0e-12);
    assert(std::abs(rk4 - exactDecay) < std::abs(euler - exactDecay));
    std::cout << "decay exact=" << exactDecay << " Euler=" << euler
              << " RK4=" << rk4 << '\n';

    // Compare energy behavior for q'=p, p'=-q over t=100.
    constexpr double hOsc = 0.1;
    constexpr int oscSteps = 1000;
    State explicitState{1.0, 0.0};
    State semiState{1.0, 0.0};
    double explicitFinalEnergy = energy(explicitState);
    double semiMinEnergy = energy(semiState);
    double semiMaxEnergy = energy(semiState);
    for (int i = 0; i < oscSteps; ++i) {
        explicitState = explicitEulerOscillator(explicitState, hOsc);
        semiState = semiImplicitEulerOscillator(semiState, hOsc);
        explicitFinalEnergy = energy(explicitState);
        semiMinEnergy = std::min(semiMinEnergy, energy(semiState));
        semiMaxEnergy = std::max(semiMaxEnergy, energy(semiState));
    }
    assert(explicitFinalEnergy > 100.0);
    assert(semiMinEnergy > 0.46 && semiMaxEnergy < 0.54);
    assert(semiMaxEnergy > semiMinEnergy);
    std::cout << "oscillator E0=0.5 explicit E(100)=" << explicitFinalEnergy
              << " semi-implicit range=[" << semiMinEnergy << ',' << semiMaxEnergy
              << "] (bounded here, not exact)\n";

    // A safeguarded Newton step for f(x)=log(cosh(x)), starting at x=3.
    int backtracks = 0;
    const double alpha = firstArmijoNewtonStep(3.0, backtracks);
    assert(close(alpha, 1.0 / 32.0, 0.0));
    assert(backtracks == 5);
    const double gradientAtStart = std::tanh(3.0);
    const double hessianAtStart = 1.0 / std::pow(std::cosh(3.0), 2.0);
    const double newtonDirection = -gradientAtStart / hessianAtStart;
    const double nextX = 3.0 + alpha * newtonDirection;
    assert(logCosh(nextX) < logCosh(3.0));
    std::cout << "Newton x0=3 alpha=" << alpha << " backtracks=" << backtracks
              << " x1=" << nextX << " f1=" << logCosh(nextX) << '\n';

    std::cout << "all quantitative assertions passed\n";
}
```

### Repository audit: what KairoMath currently implements — source claims pending independent repository verification

The relevant source is [KairoMath/Optimization.cppm](https://github.com/swayam8624/KairoMath/blob/master/Optimization.cppm?utm_source=chatgpt.com).

The current module includes gradient descent, momentum, a Nesterov-style method, Adam, conjugate-gradient solving, Newton, Gauss–Newton, and Levenberg–Marquardt.

This is a substantial collection of numerical algorithms, but the available APIs and safeguards are not identical to those of a production nonlinear optimization library.

##### Gradient descent and line search

The module's `OptimizationSettings<T>` includes parameters such as `LearningRate`, `GradientTolerance`, `StepTolerance`, `ValueTolerance`, `ArmijoC1`, `BacktrackingShrink`, and `MinimumStepScale`.

Its internal `BacktrackingLineSearch` returns a step scale of zero when it cannot find an acceptable candidate above the minimum threshold.

In `GradientDescent`, that returned scale is multiplied into the learning rate.

If the resulting step is zero, the step-tolerance stopping condition can be satisfied, and the result can report convergence even though the gradient was not sufficiently small.

That is a real failure mode suggested by the inspected control flow.

A more reliable design would report line-search failure explicitly instead of interpreting a failed search as a successful zero-sized update.

##### Newton's method

The supplied source claims that the current Newton implementation obtains a Hessian from the caller and solves:

$$
H\mathbf p=-\mathbf g
$$

using `LinearSolve`.

This is the correct mathematical Newton system.

However, the inspected implementation does not explicitly require the Hessian to be positive definite or the resulting direction to satisfy:

$$
\mathbf g^T\mathbf p<0
$$

before attempting the line search.

For an indefinite Hessian, the computed Newton direction may be an ascent direction.

A mature implementation should incorporate a suitable curvature safeguard, trust region, modified Hessian, or fallback strategy.

##### Gauss–Newton · V09.08.11

The existing `GaussNewton` validates that the Jacobian dimensions match residual count and parameter count.

It computes:

$$
J^T\mathbf r
$$

and:

$$
J^TJ,
$$

then solves the normal equations for the direction.

This is mathematically correct for the Gauss–Newton approximation.

But directly solving $J^TJ$ can be numerically problematic when the Jacobian is badly conditioned or rank deficient.

A production extension could support rank-revealing QR or SVD, with solver selection based on dimensions and rank diagnostics.

##### Levenberg–Marquardt · V09.08.12

The supplied source claims `LevenbergMarquardt` implements the damped system:

$$
(J^TJ+\lambda I)\Delta=-J^T\mathbf r.
$$

It decreases damping after improving steps and increases it after rejected steps or certain linear-solve failures.

That is a valid simplified LM strategy.

It does not implement the full actual-versus-predicted reduction ratio used by more sophisticated trust-region LM implementations, and it uses unscaled identity damping rather than a problem-dependent diagonal metric.

Both can be worthwhile improvements for problems containing parameters with different sensitivities.

The implementation also uses exception handling around the linear solve. A production solver should distinguish numerical singularity from unrelated programming or allocation errors rather than indiscriminately treating every exception as a reason to increase damping.

##### Why this audit matters

These findings do not imply that KairoMath is unusable or that the mathematical algorithms are conceptually wrong.

They distinguish the canonical mathematical method from the currently implemented solver contract and identify specific testable weaknesses.

A useful repository improvement would be to introduce structured termination statuses, line-search failure propagation, descent-direction checks, scale-aware tolerances, and robust rank-deficiency handling.

These are concrete engineering changes with measurable effects.

### Further essential optimization concepts

The original 45 parts provide a strong foundation, but several adjacent concepts should be connected explicitly because they repeatedly appear in graphics research.

#### Robust losses

Squared loss is highly sensitive to outliers because residual influence grows linearly with residual magnitude:

$$
\frac d{dr}\frac12r^2=r.
$$

A robust loss reduces the influence of sufficiently large residuals.

For example, Huber loss with threshold $\delta>0$ is:

$$
\rho\_\delta(r)= \begin{cases} \frac12r^2,&|r|\le\delta,\\ \delta(|r|-\frac12\delta),&|r|>\delta. \end{cases}
$$

Its derivative is:

$$
\rho\_\delta'(r)= \begin{cases} r,&|r|\le\delta,\\ \delta\\,\operatorname{sign}(r),&|r|>\delta. \end{cases}
$$

Small residuals receive quadratic treatment. Large residuals receive a linear penalty, limiting their influence.

In camera calibration, this can help prevent a small number of incorrect feature matches from dominating the objective.

However, robust losses can introduce additional nonconvexity depending on the chosen formulation, and a robust objective is not a substitute for handling invalid correspondences.

#### Iteratively reweighted least squares

Certain robust objectives can be approached through iteratively reweighted least squares, or IRLS.

For a scalar residual $r$, define an influence function:

$$
\psi(r)=\rho'(r).
$$

An associated weight may be:

$$
w(r)=\frac{\psi(r)}r
$$

for $r\ne0$, with a suitable limiting value at zero.

At each iteration, we construct a weighted least-squares approximation using weights derived from the current residuals.

This allows some robust objectives to reuse least-squares machinery.

But weights depend on the current state, so the algorithm must update them consistently. The approximation and convergence behavior depend on the exact loss and IRLS formulation.

#### Preconditioning

Suppose we need to solve:

$$
A\mathbf x=\mathbf b.
$$

If $A$ is ill-conditioned, iterative methods may converge slowly.

A preconditioner $M$ approximates useful inverse structure so that the transformed problem is easier to solve.

For example, left preconditioning gives:

$$
M^{-1}A\mathbf x=M^{-1}\mathbf b.
$$

The objective is not necessarily to approximate $A^{-1}$ perfectly. A useful preconditioner should be inexpensive to construct and apply while improving the effective spectrum or clustering of the linear system.

For optimization, parameter scaling is a simple form of preconditioning. More sophisticated methods use Hessian approximations, block diagonals, incomplete factorizations, or geometric structure.

#### Matrix-free optimization

A large Hessian may be impossible or wasteful to form explicitly.

But an algorithm may need only:

$$
H\mathbf v.
$$

A Hessian-vector product can be computed through suitable automatic differentiation techniques without materializing the entire Hessian matrix.

This allows truncated Newton and certain Krylov-based trust-region methods to operate on high-dimensional problems.

For a least-squares objective, the Gauss–Newton matrix-vector product is:

$$
\boxed{ (J^TJ)\mathbf v = J^T(J\mathbf v) }.
$$

We can compute this using a JVP followed by a VJP.

No explicit $J^TJ$ matrix is necessary.

This is a direct and important connection between M052's automatic differentiation and M053's curvature-based optimization.

#### Schur complements in bundle adjustment

Suppose the unknowns are divided into camera variables $\mathbf c$ and 3D point variables $\mathbf p$.

A linearized normal system may have block structure:

$$
\begin{bmatrix} H\_{cc}&H\_{cp}\\ H\_{pc}&H\_{pp} \end{bmatrix} \begin{bmatrix} \Delta\mathbf c\\ \Delta\mathbf p \end{bmatrix} = - \begin{bmatrix} \mathbf g_c\\ \mathbf g_p \end{bmatrix}.
$$

If the point block $H\_{pp}$ can be solved efficiently, eliminate $\Delta\mathbf p$.

From the second equation:

$$
H\_{pc}\Delta\mathbf c+ H\_{pp}\Delta\mathbf p=-\mathbf g_p.
$$

Thus:

$$
\Delta\mathbf p= -H\_{pp}^{-1} (\mathbf g_p+H\_{pc}\Delta\mathbf c).
$$

Substitute into the first equation:

$$
\boxed{ \left( H\_{cc}-H\_{cp}H\_{pp}^{-1}H\_{pc} \right) \Delta\mathbf c = -\mathbf g_c+ H\_{cp}H\_{pp}^{-1}\mathbf g_p }.
$$

The matrix:

$$
\boxed{ S=H\_{cc}-H\_{cp}H\_{pp}^{-1}H\_{pc} }
$$

is the Schur complement.

The implementation should use solves involving $H\_{pp}$, not necessarily explicit inverse matrices.

This block reduction is fundamental in large-scale bundle adjustment and similar problems where some variable blocks can be eliminated efficiently.

#### Gauge freedom and identifiability

A reconstruction objective may possess transformations that do not change its predictions.

For instance, in certain monocular reconstruction settings, global similarity transformations of cameras and geometry leave the image measurements unchanged.

These transformations create nullspace directions in the Jacobian.

An optimizer may then encounter singular normal matrices, arbitrary drift, or ambiguous parameter estimates.

Fixing one reference camera, fixing scale, introducing suitable priors, or explicitly representing the gauge can make the system better posed.

A small final reprojection error does not prove that the recovered scene has a unique physical scale or coordinate frame.

This distinction should be part of any serious inverse-graphics validation.

### Phase 9 research synthesis

There is one particularly valuable combined research question emerging from M049–M054:

How do parameterization, conditioning, differentiation accuracy, and optimization globalization interact in geometric inverse problems?

Consider a camera-pose estimation experiment. We can vary the camera parameterization, scene depth distribution, feature noise, and initialization distance.

Then compare multiple solvers under the same objective.

Potential solver configurations include gradient descent with line search, BFGS or L-BFGS, Gauss–Newton, and LM.

The measurements should include final reprojection error, ground-truth pose error where available, runtime, Jacobian evaluation cost, linear-solver cost, rejected steps, gradient-check error, and convergence or failure rates.

An important ablation would isolate scaling.

For example, run the same physical problem using translation expressed in metres and millimetres, with and without parameter normalization.

If the algorithm is sensitive to this change despite representing the same physical problem, that reveals the effect of solver coordinates and scaling.

Another ablation would isolate derivative accuracy by comparing analytical Jacobians, automatic differentiation, and finite differences with carefully selected step sizes.

A third would isolate globalization by comparing unguarded Gauss–Newton, line-search Gauss–Newton, and trust-region LM.

The goal would not be to claim that one optimizer universally dominates another. It would be to identify which mathematical properties and problem regimes cause each method to succeed or fail.

This is a realistic research methodology for reconstruction, calibration, physics simulation, or differentiable rendering.

### Phase 9 reconstruction and mastery gate

You should now be able to move through the entire causal chain:

$$
\text{Physical or geometric model} \rightarrow \text{objective} \rightarrow \text{derivatives} \rightarrow \text{local approximation} \rightarrow \text{candidate step} \rightarrow \text{acceptance} \rightarrow \text{verification}.
$$

To test that understanding, reconstruct the following independently.

1. Starting from the Taylor expansion of a differentiable scalar function, derive gradient descent, its local descent property, and the fixed-step convergence condition for a positive-definite quadratic objective.
2. Derive multidimensional Newton's method from the linearized gradient and explain why an indefinite Hessian can produce an ascent direction.
3. Starting from a residual vector $\mathbf r(\mathbf x)$, derive the least-squares gradient, exact Hessian, Gauss–Newton approximation, and LM damping equation. Explain the different meanings of regularization and step damping.
4. Derive the KKT conditions for an inequality-constrained optimization problem. Solve a quadratic problem whose optimum lies on an active bound, and explain why its ordinary gradient need not vanish.
5. Derive the normalization Jacobian and the left-applied $SO(3)$ rotation perturbation Jacobian. Explain their rank, nullspace, coordinate conventions, and singular or degenerate cases.
6. Implement a gradient-based optimizer with explicit failure statuses, scale-aware stopping criteria, line-search rejection, finite-difference derivative checks, and a final report distinguishing convergence from stagnation.

The downloaded C++23 laboratory provides a concrete reference against which to verify your reconstruction.


<!-- end V09.08 -->

<!-- begin V10.17 -->
## Floating-point answers: error, conditioning, stability, and stable arithmetic

```mermaid
flowchart LR
 M["Model and data scale"] --> D["Discretization choice"] --> A["Stable arithmetic"] --> S["Sampling and reduction"] --> E["Error budget and tests"]
 K["Problem conditioning"] -. "limits attainable accuracy" .-> E
 class M,D,A,S blue;class K amber;class E green;
classDef blue fill:#e5eff9,stroke:#2463a6,color:#18354c;
classDef green fill:#e3f3ed,stroke:#16806a,color:#174b3c;
classDef amber fill:#fff0dc,stroke:#c57820,color:#64400d;
```



Let's start with the most important misconception in numerical computing: the assumption that a floating-point variable is simply a real number with a limited number of decimal places.

A binary floating-point number is not an arbitrary real number. It is represented using a finite sign, exponent, and significand.

A normalized binary number has the conceptual form:

$$ \boxed{x=(-1)^s(1.f)_2\,2^e} $$

where $s$ is the sign, $f$ contains the fractional significand bits, and $e$ is the exponent.

For IEEE 754 binary32, commonly represented by C++ `float`, the usual format has one sign bit, eight exponent bits, and 23 explicitly stored fraction bits, giving 24 bits of significand precision for normal numbers.

For IEEE 754 binary64, commonly represented by C++ `double`, the format has one sign bit, eleven exponent bits, and 52 explicitly stored fraction bits, giving 53 bits of significand precision for normal numbers.

The machine cannot represent most real numbers exactly.

For example:

$$ 0.1_{10} $$

has an infinitely repeating binary representation.

Thus, even a simple assignment such as:

```
double distance = 0.1;
```

ordinarily stores the nearest representable binary64 approximation to $0.1$, not the exact real value.

### 1.1 The standard floating-point model

For a basic arithmetic operation $\circ$, a common mathematical model is:

$$ \boxed{ \operatorname{fl}(a\circ b) = (a\circ b)(1+\delta), \qquad |\delta|\le u } $$

where $u$ is unit roundoff.

The bound applies under its standard assumptions: the exact result is nonzero, normal and within range, and the operation is correctly rounded according to the relevant model.

For binary64 with round-to-nearest:

$$ \boxed{u=2^{-53}\approx1.1102230246251565\times10^{-16}}. $$

For binary32:

$$ \boxed{u=2^{-24}\approx5.9604644775\times10^{-8}}. $$

Be careful with terminology. In C++:

```
std::numeric_limits<double>::epsilon()
```

is:

$$ \boxed{\epsilon=2^{-52}}, $$

the distance from 1 to the next larger representable binary64 number.

Under round-to-nearest, unit roundoff is:

$$ u=\epsilon/2. $$

Some numerical literature uses the expression machine epsilon for different closely related quantities. Always state your definition.

### 1.2 How local errors propagate

Suppose we calculate:

$$ y=f(x). $$

But the stored input is:

$$ \hat x=x+\Delta x. $$

We want to determine how much the output changes.

By first-order Taylor expansion:

$$ f(x+\Delta x) = f(x)+f'(x)\Delta x+ O(\Delta x^2). $$

Therefore:

$$ \boxed{ \Delta y\approx f'(x)\Delta x }. $$

This is the basic differential model of error propagation.

For multiple inputs:

$$ y=f(x_1,\ldots,x_n), $$

a small perturbation gives:

$$ \boxed{ \Delta y\approx \sum_i \frac{\partial f}{\partial x_i} \Delta x_i }. $$

In vector form:

$$ \boxed{ \Delta\mathbf y\approx J_f(\mathbf x)\Delta\mathbf x }. $$

The same Jacobian that describes local geometric transformation also describes how small input errors affect output errors.

This is why conditioning, derivatives, and uncertainty propagation are so tightly connected.

### 1.3 Absolute versus relative error propagation

Suppose:

$$ y=ab. $$

Let:

$$ \hat a=a+\Delta a,\qquad \hat b=b+\Delta b. $$

Then:

$$ \hat y=(a+\Delta a)(b+\Delta b). $$

Expand:

$$ \hat y = ab+b\Delta a+a\Delta b+\Delta a\Delta b. $$

Ignoring second-order perturbation products:

$$ \Delta y\approx b\Delta a+a\Delta b. $$

Divide by $y=ab$, assuming $a,b\ne0$:

$$ \boxed{ \frac{\Delta y}{y} \approx \frac{\Delta a}{a}+ \frac{\Delta b}{b} }. $$

Thus relative errors in multiplication approximately add to first order.

Now consider subtraction:

$$ y=a-b. $$

Its absolute perturbation is:

$$ \Delta y=\Delta a-\Delta b. $$

Its relative perturbation is:

$$ \boxed{ \frac{\Delta y}{y} = \frac{\Delta a-\Delta b}{a-b} }. $$

If $a$ and $b$ are nearly equal, the denominator is tiny.

Even small absolute input errors can become enormous relative output errors.

This is the mechanism behind catastrophic cancellation.

### 1.4 A geometric example

Imagine computing the difference between two camera-space positions:

$$ \mathbf d=\mathbf p_2-\mathbf p_1. $$

Suppose both positions are around $10^8$ metres in world coordinates, but their true separation is only $0.01$ metres.

In binary32, the spacing between representable numbers near $10^8$ is much larger than one centimetre.

The stored positions may therefore become identical even if the original physical positions differ.

Subtracting those stored values produces zero.

No later normalization, vector-length calculation, or optimizer can reconstruct the lost centimetre-scale detail from those two stored numbers.

A common remedy in rendering is to maintain world positions in a sufficiently precise representation and transform them into a local coordinate frame near the camera or object before using lower-precision computations.

The local-frame transformation can reduce the magnitude of coordinates and preserve differences that would otherwise disappear.

But the subtraction must occur before precision is lost. Subtracting two already-rounded `float` positions and then converting to `double` cannot recover missing information.


Numerical analysis distinguishes two different ways of judging an algorithm.

Suppose the exact mathematical problem is:

$$ y=f(x). $$

The computer produces:

$$ \hat y. $$

### 2.1 Forward error

Forward error compares the computed output with the exact output of the original problem:

$$ \boxed{ e_{\mathrm{forward}}= |\hat y-f(x)| }. $$

For vector outputs, we often use a norm:

$$ \boxed{ e_{\mathrm{forward}} = \|\hat{\mathbf y}-f(\mathbf x)\| }. $$

This answers:

> How far is the computed answer from the mathematically correct answer for the specified input?

### 2.2 Backward error

Backward error asks:

> Is the computed result the exact answer to a nearby problem?

We seek a perturbed input $x+\Delta x$ satisfying:

$$ \boxed{ \hat y=f(x+\Delta x) }. $$

The backward error measures the size of the smallest permitted perturbation $\Delta x$ that explains the computed output exactly.

For a linear system:

$$ A\mathbf x=\mathbf b, $$

suppose the numerical algorithm produces $\hat{\mathbf x}$.

We may ask whether:

$$ (A+\Delta A)\hat{\mathbf x} = \mathbf b+\Delta\mathbf b $$

for small perturbations $\Delta A,\Delta\mathbf b$.

An algorithm is backward stable in a specified sense if its result can be interpreted as the exact solution of a nearby problem with appropriately small perturbations.

### 2.3 A small backward error can coexist with large forward error

Consider:

$$ A= \begin{bmatrix} 1&0\\ 0&10^{-8} \end{bmatrix}. $$

Let:

$$ \mathbf b= \begin{bmatrix} 1\\ 10^{-8} \end{bmatrix}. $$

The exact solution is:

$$ \mathbf x^\star= \begin{bmatrix} 1\\ 1 \end{bmatrix}. $$

Now suppose the computed result is:

$$ \hat{\mathbf x}= \begin{bmatrix} 1\\ 1.01 \end{bmatrix}. $$

The forward error is:

$$ \|\hat{\mathbf x}-\mathbf x^\star\| =0.01 $$

in the Euclidean norm.

But its residual is:

$$ \mathbf r=\mathbf b-A\hat{\mathbf x}. $$

Compute:

$$ A\hat{\mathbf x} = \begin{bmatrix} 1\\ 1.01\times10^{-8} \end{bmatrix}. $$

Therefore:

$$ \mathbf r= \begin{bmatrix} 0\\ -10^{-10} \end{bmatrix}. $$

Its norm is only:

$$ \boxed{\|\mathbf r\|=10^{-10}}. $$

A very small residual coexists with a much larger solution error.

Why?

Because the second coordinate is weakly constrained by a very small diagonal coefficient.

A perturbation in that direction has little effect on the residual.

This is a conditioning problem, not necessarily evidence that the numerical algorithm used the wrong equation.

### 2.4 The residual is not the solution error

For a nonsingular linear system:

$$ A\mathbf x^\star=\mathbf b, $$

and approximate solution $\hat{\mathbf x}$, define:

$$ \mathbf r=\mathbf b-A\hat{\mathbf x}. $$

Then:

$$ A(\mathbf x^\star-\hat{\mathbf x})=\mathbf r. $$

Thus:

$$ \mathbf x^\star-\hat{\mathbf x} = A^{-1}\mathbf r. $$

Therefore:

$$ \boxed{ \|\mathbf x^\star-\hat{\mathbf x}\| \le \|A^{-1}\|\|\mathbf r\| }. $$

A small residual gives a small forward-error bound only if $\|A^{-1}\|$ is suitably controlled.

This is a fundamental bridge between residual verification and conditioning.


Conditioning describes sensitivity inherent in the mathematical problem.

It is not primarily a property of the algorithm.

For a differentiable scalar function $y=f(x)$, a local absolute condition measure is:

$$ \boxed{ \kappa_{\mathrm{abs}}(x)=|f'(x)| }. $$

It describes approximate output change per unit absolute input change:

$$ |\Delta y|\approx|f'(x)||\Delta x|. $$

When $x\ne0$ and $f(x)\ne0$, a relative condition number is:

$$ \boxed{ \kappa_{\mathrm{rel}}(x) = \left| \frac{xf'(x)}{f(x)} \right| }. $$

This follows from:

$$ \frac{\Delta y}{y} \approx \frac{f'(x)\Delta x}{f(x)}. $$

Multiply and divide by $x$:

$$ \frac{\Delta y}{y} \approx \frac{xf'(x)}{f(x)} \frac{\Delta x}{x}. $$

Thus the relative output error is approximately the relative input error multiplied by the relative condition number.

### 3.1 Worked conditioning example

Consider:

$$ f(x)=\frac1{1-x}. $$

Its derivative is:

$$ f'(x)=\frac1{(1-x)^2}. $$

The relative condition number is:

$$ \kappa_{\mathrm{rel}}(x) = \left| \frac{x}{1-x} \right|. $$

At:

$$ x=0.5, $$

we have:

$$ \kappa_{\mathrm{rel}}=1. $$

At:

$$ x=0.999, $$

we have:

$$ \boxed{\kappa_{\mathrm{rel}}=999}. $$

A small relative input perturbation can produce a much larger relative output change near the singularity $x=1$.

Even an ideal, exact implementation cannot make that function insensitive to perturbations near its singularity.

### 3.2 Matrix conditioning

For a nonsingular matrix $A$, the condition number under a consistent operator norm is:

$$ \boxed{ \kappa(A)=\|A\|\|A^{-1}\| }. $$

For the spectral 2-norm:

$$ \boxed{ \kappa_2(A) = \frac{\sigma_{\max}(A)}{\sigma_{\min}(A)} }. $$

Here $\sigma_{\max}$ and $\sigma_{\min}$ are the largest and smallest singular values.

For:

$$ A= \begin{bmatrix} 1&0\\ 0&10^{-8} \end{bmatrix}, $$

the singular values are:

$$ 1,\quad10^{-8}. $$

Therefore:

$$ \boxed{\kappa_2(A)=10^8}. $$

The linear system is highly sensitive to perturbations along the weak direction.

### 3.3 Conditioning of a root

Suppose we want to solve:

$$ f(x)=0 $$

and the exact root is $r$, with:

$$ f'(r)\ne0. $$

If the function is perturbed by a small additive amount $\delta f$, the root changes approximately according to:

$$ f(r+\delta r)+\delta f\approx0. $$

Linearize:

$$ f(r)+f'(r)\delta r+\delta f\approx0. $$

Because $f(r)=0$:

$$ f'(r)\delta r+\delta f\approx0. $$

Therefore:

$$ \boxed{ \delta r\approx-\frac{\delta f}{f'(r)} }. $$

If $|f'(r)|$ is small, the root is sensitive to errors in function evaluation.

This is important for tangential or multiple roots.

A solver may reduce the residual dramatically without locating the root accurately when the root is badly conditioned.


Conditioning describes the problem. Stability describes the algorithm's behavior when implemented in finite precision.

A stable algorithm does not unnecessarily amplify numerical errors beyond what the problem's conditioning requires, under a specified stability notion.

Different notions exist: forward stability, backward stability, mixed stability, componentwise stability, and more.

We must identify which one we mean.

### 4.1 Stable algorithm versus stable problem

Imagine two algorithms for calculating the same mathematical quantity.

Both are given the same floating-point input.

Algorithm A uses a sequence of operations that causes catastrophic cancellation.

Algorithm B uses a mathematically equivalent formula that avoids subtracting nearly equal large quantities.

The problem itself has not changed, but the algorithms can have very different accuracy.

That is algorithmic stability.

Conversely, if a problem is extremely ill-conditioned, even a backward-stable algorithm may produce a large forward error.

The problem is sensitive, so nearby inputs can have substantially different exact solutions.

### 4.2 Stability of linear-system solving

For a well-designed direct linear solver, we often analyze the computed solution as satisfying:

$$ (A+\Delta A)\hat{\mathbf x} = \mathbf b+\Delta\mathbf b, $$

where the perturbations are small relative to suitable data scales.

That is a backward-error perspective.

The resulting forward error depends on conditioning:

$$ \boxed{ \text{forward error} \approx \text{conditioning}\times \text{backward error} } $$

as a conceptual first-order relationship, not a universal exact equality.

### 4.3 Stability is conditional on assumptions

A factorization algorithm may be stable for positive-definite matrices but not applicable to indefinite matrices.

A root solver may converge reliably on continuous functions with a sign-changing bracket but fail on discontinuous functions.

An ODE integrator may be stable only for step sizes within a particular region of the complex plane.

Thus the phrase numerically stable is incomplete unless we state the algorithm, problem class, arithmetic assumptions, and required accuracy.


Individual floating-point operations introduce rounding errors. Repeated operations accumulate them.

Suppose we add $n$ numbers:

$$ S=\sum_{i=1}^{n}x_i. $$

If every addition is rounded, the computed result may differ from the exact sum.

For a conventional sequential summation under standard floating-point assumptions, an error bound can be expressed using:

$$ \boxed{ \gamma_n= \frac{nu}{1-nu} } $$

when $nu<1$, with the index adjusted according to the number of operations and precise algorithm being analyzed.

For sequential summation, one common absolute-error bound is:

$$ \boxed{ |\hat S-S| \le \gamma_{n-1} \sum_{i=1}^{n}|x_i| } $$

under the usual floating-point model and absence of exceptional arithmetic behavior.

### 5.1 Why the sum of magnitudes appears

If all values have the same sign, the true sum is relatively large compared with the sum of absolute values.

But if positive and negative values cancel:

$$ \left|\sum_ix_i\right| \ll \sum_i|x_i|. $$

Then even a small absolute error bound can correspond to a large relative error.

The ratio:

$$ \boxed{ \kappa_{\mathrm{sum}} = \frac{\sum_i|x_i|} {\left|\sum_ix_i\right|} } $$

is a condition measure for summation when the denominator is nonzero.

A sum containing severe cancellation can be ill-conditioned.

### 5.2 Numerical example

Consider the exact arithmetic sum:

$$ 10^{16}+1-10^{16}=1. $$

In binary64, adding one to $10^{16}$ can round back to $10^{16}$ because the spacing of representable values is larger than one.

Thus sequential evaluation:

```
double total = 1.0e16;
total += 1.0;
total -= 1.0e16;
```

can produce:

$$ \boxed{0} $$

rather than one.

The problem is not that subtraction is always inaccurate. It is that the earlier addition has already lost the small increment.

Changing the grouping:

$$ 10^{16}+(-10^{16})+1 $$

can produce one exactly in this example.

Floating-point arithmetic is not associative:

$$ \boxed{ \operatorname{fl}(\operatorname{fl}(a+b)+c) \ne \operatorname{fl}(a+\operatorname{fl}(b+c)) } $$

in general.

### 5.3 Why GPUs expose this problem

A parallel reduction can combine values in different orders depending on its reduction tree, workgroup structure, or execution strategy.

Different aggregation orders produce different rounding histories.

Consequently, floating-point reductions can produce slightly different answers even when every thread processes the same values.

For deterministic scientific computation, define a stable reduction order or algorithm and test reproducibility across target configurations.

For rendering, this matters when accumulating radiance over many samples, combining tiles, and calculating statistical moments.


Kahan summation attempts to recover small contributions that would otherwise be lost when adding numbers of very different magnitudes.

It maintains both a running sum and a correction.

A conventional implementation is:

```
struct KahanSum {
    double sum = 0.0;
    double correction = 0.0;

    void add(double x) {
        // Compensate for rounding error retained
        // from previous additions.
        const double y = x - correction;

        // Add the corrected contribution.
        const double t = sum + y;

        // Estimate the low-order error introduced
        // by the floating-point addition.
        correction = (t - sum) - y;

        sum = t;
    }
};
```

The variable `correction` stores an estimate of information lost during previous additions.

It is not an exact symbolic record of every rounding error, but it often improves results substantially.

### 6.1 Worked example

Take:

$$ x_1=10^{16},\quad x_2=1,\quad x_3=1,\quad x_4=-10^{16}. $$

The exact sum is:

$$ \boxed{2}. $$

Ordinary left-to-right binary64 summation can lose both unit contributions and return zero.

Kahan summation compensates for some lost low-order bits. But an important subtlety is that classic Kahan summation is not guaranteed to recover the exact result for every adversarial sequence, including sequences in which a large cancellation occurs after multiple small corrections.

A more robust compensated alternative is Neumaier summation, which handles certain mixed-magnitude and mixed-sign patterns better.

### 6.2 Neumaier compensated summation

Neumaier's algorithm maintains an explicit correction to the running sum:

```
struct NeumaierSum {
    double sum = 0.0;
    double correction = 0.0;

    void add(double x) {
        const double t = sum + x;

        if (std::abs(sum) >= std::abs(x)) {
            correction += (sum - t) + x;
        } else {
            correction += (x - t) + sum;
        }

        sum = t;
    }

    [[nodiscard]]
    double value() const {
        return sum + correction;
    }
};
```

This often improves behavior when values have mixed signs and widely different magnitudes.

However, compensation still uses finite-precision arithmetic. It is not an arbitrary-precision exact summation algorithm.

### 6.3 Application to Monte Carlo rendering

A pixel may accumulate millions of radiance contributions.

Most contributions might be small, while a few are very large.

A naive accumulator may lose small contributions when adding them to a large running total.

Compensated summation can reduce accumulation error.

But it also has computational and state overhead.

On a GPU, the performance impact depends on register pressure, memory layout, vectorization, and reduction structure.

A scientifically useful benchmark should measure both numerical improvement and execution cost.


Pairwise summation changes the order in which values are combined.

Instead of adding sequentially:

$$ (((x_1+x_2)+x_3)+\cdots)+x_n, $$

we construct a balanced reduction tree.

For eight values, first compute four pairwise sums, then two sums, then one final sum.

The tree depth is:

$$ \log_2 n $$

rather than $n-1$.

### 7.1 Why balancing helps

In sequential summation, early values participate in many rounded additions.

In a balanced pairwise tree, each original value participates in approximately $\log_2 n$ additions along its path to the root.

Under standard assumptions, pairwise summation often has a worst-case error bound involving:

$$ \gamma_{\lceil\log_2n\rceil} $$

rather than a bound proportional to $\gamma_{n-1}$.

This can reduce worst-case rounding-error growth substantially.

### 7.2 Parallel implementation

```
double pairwiseSum(
    const double* data,
    std::size_t count
) {
    if (count == 0)
        return 0.0;

    if (count == 1)
        return data[0];

    const std::size_t middle = count / 2;

    const double left =
        pairwiseSum(data, middle);

    const double right =
        pairwiseSum(data + middle, count - middle);

    return left + right;
}
```

The recurrence is:

$$ T(n)=2T(n/2)+O(1). $$

Therefore the total arithmetic work is:

$$ \boxed{O(n)}. $$

The recursive depth is:

$$ \boxed{O(\log n)}. $$

The implementation above uses recursive function calls, which may be inefficient for tiny partitions. A production version commonly uses iterative blocks or stops recursion at a base-case size.

Pairwise summation is naturally compatible with parallel reduction.

But a deterministic reduction tree should be fixed if bitwise reproducibility is required.

### 7.3 Pairwise versus compensated summation

Neither method universally dominates the other.

Kahan-style compensation can be highly accurate for many sequential sums.

Pairwise summation offers efficient parallel structure and favorable error growth.

A hybrid can combine pairwise reduction with compensated accumulation within local blocks.

The appropriate choice depends on input distribution, required error guarantees, performance constraints, and reproducibility requirements.


The Euclidean norm is:

$$ \boxed{ \|\mathbf x\|_2= \sqrt{\sum_ix_i^2} }. $$

The formula looks harmless.

But directly computing:

```
std::sqrt(x * x + y * y)
```

can overflow even when the final norm is representable.

### 8.1 Overflow example

Suppose:

$$ x=10^{200},\qquad y=10^{200}. $$

The true norm is:

$$ \sqrt{2}\times10^{200}. $$

This is representable in binary64.

But:

$$ x^2=10^{400} $$

is far beyond the maximum finite binary64 value.

The intermediate multiplication overflows.

The naive formula may return infinity, even though the correct answer is finite.

### 8.2 Scaled norm derivation

Let:

$$ m=\max(|x|,|y|). $$

If $m>0$, factor it out:

$$ \sqrt{x^2+y^2} = m\sqrt{\left(\frac xm\right)^2+\left(\frac ym\right)^2}. $$

Because:

$$ \left|\frac xm\right|\le1, \qquad \left|\frac ym\right|\le1, $$

the squared normalized terms cannot overflow under ordinary finite inputs.

Thus:

$$ \boxed{ \operatorname{hypot}(x,y) = m\sqrt{ (x/m)^2+(y/m)^2 } }. $$

For very small numbers, scaling also helps avoid underflow in intermediate squares.

### 8.3 Standard-library support

C++ provides:

```
std::hypot(x, y)
```

and an overload for three arguments on supported standard libraries.

For 2D or 3D vector lengths, this is often preferable to manually squaring components.

For arbitrary vector dimensions, a scalable norm algorithm can maintain a running scale and scaled sum of squares.

One common pattern tracks:

$$ \text{scale}=\max_i|x_i| $$

and accumulates appropriately scaled squared terms without constructing $x_i^2$ when $x_i$ is dangerously large or small.

### 8.4 Stable normalization

To normalize a vector:

$$ \hat{\mathbf x}= \frac{\mathbf x}{\|\mathbf x\|}, $$

first compute the norm robustly.

Then handle zero and near-zero norms explicitly.

But be careful: introducing an arbitrary threshold changes the mathematical function.

For example:

```
if (length < 1e-6)
    return Vec3{0, 0, 0};
```

may be a reasonable application-specific policy, but it introduces a discontinuity and an absolute scale.

A more principled threshold can depend on expected physical units and acceptable error.

We should not assume that every vector shorter than $10^{-6}$ has no meaningful direction in every domain.


Consider:

$$ \boxed{ax^2+bx+c=0}, \qquad a\ne0. $$

The familiar quadratic formula is:

$$ \boxed{ x_\pm= \frac{-b\pm\sqrt{b^2-4ac}}{2a} }. $$

It is mathematically exact, but its direct floating-point implementation can be unstable.

### 9.1 Where cancellation occurs

Suppose $b>0$ and:

$$ \sqrt{b^2-4ac}\approx b. $$

Then:

$$ -b+\sqrt{b^2-4ac} $$

subtracts two nearly equal numbers.

The numerator for one root may be tiny relative to either term.

Significant digits can be lost.

### 9.2 A worked example

Consider:

$$ x^2+10^8x+1=0. $$

Here:

$$ a=1,\quad b=10^8,\quad c=1. $$

The discriminant is:

$$ D=b^2-4ac=10^{16}-4. $$

The two roots are approximately:

$$ x_1\approx-10^8, $$

$$ x_2\approx-10^{-8}. $$

The small root comes from:

$$ \frac{-10^8+\sqrt{10^{16}-4}}2. $$

The two numerator terms are nearly equal in magnitude.

The subtraction loses relative accuracy.

In some floating-point configurations, the discriminant may round to $10^{16}$, so the square root rounds to $10^8$.

Then the small-root numerator becomes exactly zero.

The algorithm returns zero instead of approximately $-10^{-8}$.

### 9.3 Derive a stable alternative

Define:

$$ \boxed{ q= -\frac12\left(b+\operatorname{copysign}(\sqrt D,b)\right) }. $$

This chooses the sign so the large-magnitude terms add rather than cancel.

One root is:

$$ \boxed{x_1=\frac qa}. $$

For the other, use Vieta's relation:

$$ x_1x_2=\frac ca. $$

Therefore:

$$ x_2=\frac{c/a}{x_1}. $$

Substitute $x_1=q/a$:

$$ \boxed{x_2=\frac cq}. $$

The pair becomes:

$$ \boxed{ x_1=\frac qa, \qquad x_2=\frac cq }. $$

### 9.4 Why Vieta helps

Instead of calculating the small root using a subtraction of nearly equal large numbers, we infer it from the product of roots.

For our example, the large root is approximately $-10^8$.

Because:

$$ x_1x_2=\frac ca=1, $$

the other root is:

$$ x_2\approx\frac1{-10^8}=-10^{-8}. $$

This avoids the dangerous subtraction.

### 9.5 Robust C++ implementation issues

A complete solver should also handle:

- $a=0$, which reduces the equation to linear form.
- Negative discriminant, if only real roots are desired.
- Zero discriminant, producing a repeated root.
- $c=0$, where zero is a known root.
- Overflow in $b^2$ or $4ac$.
- Nonfinite coefficients.
- Cases where $q=0$.
- Coefficient scaling before discriminant evaluation.

The stable $q$-formula addresses cancellation, but does not by itself solve every overflow and underflow problem.

A robust implementation may scale coefficients by their largest magnitude before calculating the discriminant and then exploit fused multiply-add where appropriate.

For ray tracing, quadratic solutions appear in ray–sphere intersections, analytic primitives, and collision calculations. Incorrectly computing a near root can cause missed intersections or self-intersection artifacts.


Catastrophic cancellation occurs when subtracting nearby quantities causes substantial relative information loss.

But we must distinguish two situations.

First, the inputs may already be inaccurate. Subtracting them can expose the input error.

Second, the subtraction may be part of an unstable evaluation formula that could be algebraically rearranged.

The appropriate remedy depends on which situation applies.

### 10.1 Algebraic reformulation

Consider:

$$ f(x)=\sqrt{x+1}-\sqrt x $$

for large positive $x$.

Both square roots are approximately $\sqrt x$, so direct subtraction loses accuracy.

Multiply by the conjugate:

$$ f(x)= \frac{ (\sqrt{x+1}-\sqrt x)(\sqrt{x+1}+\sqrt x) }{ \sqrt{x+1}+\sqrt x }. $$

The numerator simplifies:

$$ (x+1)-x=1. $$

Thus:

$$ \boxed{ f(x)= \frac1{\sqrt{x+1}+\sqrt x} }. $$

This formula avoids subtracting nearly equal square roots.

### 10.2 `log1p` for small changes

Consider:

$$ \log(1+x) $$

when $|x|$ is tiny.

If we first compute:

```
std::log(1.0 + x)
```

the expression `1.0 + x` may round to one, losing the small increment.

C++ provides:

```
std::log1p(x)
```

to evaluate $\log(1+x)$ accurately when $x$ is small.

For example, with:

$$ x=10^{-16}, $$

the stored result of $1+x$ may equal one in binary64 under round-to-nearest, causing the naive expression to return zero.

`std::log1p(x)` can preserve the small nonzero result.

### 10.3 `expm1` for small exponentials

Similarly:

$$ e^x-1 $$

can lose accuracy when $x$ is near zero.

Use:

```
std::expm1(x)
```

rather than:

```
std::exp(x) - 1.0
```

in numerically sensitive contexts.

The exponential-distribution CDF from the earlier probability chapter is a direct example:

$$ F(t)=1-e^{-\lambda t}. $$

A stable form is:

$$ \boxed{ F(t)=-\operatorname{expm1}(-\lambda t) }. $$

### 10.4 Fused multiply-add

The fused multiply-add operation computes:

$$ ab+c $$

with one final rounding rather than rounding the product separately and then rounding the addition, subject to the implementation's floating-point semantics.

C++ provides:

```
std::fma(a, b, c)
```

A fused operation can improve precision when calculating expressions such as:

$$ ab+c, $$

including discriminant-like expressions and polynomial evaluations.

But FMA does not automatically make every cancellation problem disappear.

It can improve the evaluation of one expression while other parts of the algorithm remain ill-conditioned.

### 10.5 Higher precision and local coordinate frames

Sometimes algebraic reformulation is insufficient.

If a geometric calculation fundamentally needs more significant digits than binary32 can provide at the chosen coordinate scale, use a higher-precision representation or move to a better-scaled coordinate frame.

For example, a large open-world engine may store global positions in binary64 and construct camera-relative binary32 positions for GPU rendering.

This approach separates large-world positioning from local rendering precision.

But coordinate-frame changes must preserve geometric consistency across transforms, culling, motion vectors, temporal accumulation, and physics.

### 10.6 The central numerical principle

The best response to cancellation is not always “increase precision.”

Often it is:

$$ \boxed{ \text{Choose a mathematically equivalent formulation with better numerical behavior.} } $$

But when the underlying problem is genuinely ill-conditioned, reformulation cannot eliminate its inherent sensitivity.

That distinction—stable algorithm versus well-conditioned problem—should now be explicit.



### Figure N10-7 — Summation is an algorithm, not just a plus sign

![[Visual Recall/Math Foundations/N10-7.png]]

Exact real sum is 10000. The three-term counterexample shows why Kahan is not an exact-arithmetic guarantee. Strict IEEE-style evaluation is assumed; reassociation can invalidate compensation.


### Figure N10-12 — Algebraically equal formulas can have unequal numerical error

![[Visual Recall/Math Foundations/N10-12.png]]

The reference uses 90-digit Decimal arithmetic and compares binary64 formulas for b from 100 to 10¹⁶. This bounded example illustrates cancellation, not a complete robust quadratic solver.


<!-- end V10.17 -->

<!-- begin V10.18 -->
## Reconstructing values and curves between samples


We now move from floating-point arithmetic into a different numerical question: how to reconstruct a function or geometric shape from finitely many known values.

Suppose we know:

$$ f(x_0)=y_0,\qquad f(x_1)=y_1,\qquad\ldots,\qquad f(x_n)=y_n. $$

We want to estimate $f(x)$ at positions between or beyond the known samples.

There are two related but distinct approaches.

Interpolation constructs a function that passes through the specified data points:

$$ \boxed{p(x_i)=y_i}. $$

Approximation constructs a function that represents the data according to an error criterion but does not necessarily pass exactly through every observation.

For example, least-squares regression minimizes:

$$ \min_\theta\sum_i(f_\theta(x_i)-y_i)^2. $$

The fitted function might not satisfy $f_\theta(x_i)=y_i$ for any particular sample.

### 11.1 Why the distinction matters

Suppose a camera calibration dataset contains noisy pixel measurements. Forcing a high-degree interpolating polynomial through every observation can reproduce the noise rather than the underlying signal.

A lower-complexity approximation may generalize better.

In contrast, when interpolating a voxel grid or texture image, the stored samples may define the discrete representation. Interpolation is then used to reconstruct values between those grid points.

The appropriate goal depends on the application.

### 11.2 Interpolation inside versus extrapolation outside

If:

$$ x_0\le x\le x_n, $$

we interpolate within the sampled range.

If:

$$ x\<x_0 \quad\text{or}\quad x>x_n, $$

we extrapolate beyond it.

Extrapolation can be much less reliable because the known samples do not directly constrain behavior outside their range.

For example, a polynomial can fit several nearby values and then grow extremely rapidly outside the observed interval.

A renderer's sampling and filtering code should specify whether parameters outside the intended domain are clamped, wrapped, mirrored, extrapolated, or rejected.

These choices change the mathematical function.

### 11.3 High-degree interpolation and Runge's phenomenon

A common misconception is that higher polynomial degree always gives a better interpolation.

High-degree global interpolation through equally spaced nodes can oscillate severely, particularly near interval endpoints.

This is known as Runge's phenomenon.

The underlying issue involves both polynomial behavior and interpolation-node placement.

Using piecewise low-degree interpolation, splines, or better node distributions such as Chebyshev nodes can often improve accuracy and stability.

In engines, locally supported splines are especially valuable because editing one control region should not necessarily disturb the entire curve.


Linear interpolation constructs a straight line between two values.

Let the endpoints be:

$$ (x_0,y_0),\qquad(x_1,y_1) $$

with $x_1\ne x_0$.

Define the normalized coordinate:

$$ \boxed{ t=\frac{x-x_0}{x_1-x_0} }. $$

When $x=x_0$, $t=0$. When $x=x_1$, $t=1$.

The interpolated value is:

$$ \boxed{ y(t)=(1-t)y_0+ty_1 }. $$

Equivalently:

$$ \boxed{ y(t)=y_0+t(y_1-y_0) }. $$

This operation is commonly called `lerp`.

### 12.1 Deriving lerp from a straight-line equation

Suppose the desired function has the form:

$$ y(t)=at+b. $$

At $t=0$:

$$ b=y_0. $$

At $t=1$:

$$ a+b=y_1. $$

Thus:

$$ a=y_1-y_0. $$

Therefore:

$$ y(t)=y_0+t(y_1-y_0). $$

### 12.2 Worked example

Suppose camera exposure is animated from:

$$ y_0=2 $$

to:

$$ y_1=10. $$

At:

$$ t=0.25, $$

we have:

$$ y=2+0.25(10-2) =2+2 =\boxed4. $$

At $t=0.75$:

$$ y=2+0.75(8)=8. $$

### 12.3 Important properties

For $t\in[0,1]$, the interpolation weights:

$$ 1-t,\quad t $$

are nonnegative and sum to one.

Therefore, for scalar values:

$$ \boxed{ \min(y_0,y_1)\le y(t)\le\max(y_0,y_1) }. $$

Linear interpolation does not overshoot the endpoint range.

For vector endpoints:

$$ \mathbf p(t)=(1-t)\mathbf p_0+t\mathbf p_1, $$

the result traces the straight segment joining them.

Its derivative with respect to $t$ is:

$$ \boxed{ \frac{d\mathbf p}{dt} = \mathbf p_1-\mathbf p_0 }. $$

For physical coordinate $x$:

$$ \boxed{ \frac{dy}{dx} = \frac{y_1-y_0}{x_1-x_0} }. $$

This distinction matters: the derivative with respect to normalized parameter $t$ differs from the derivative with respect to physical position or time.

### 12.4 Interpolation error

Suppose the true underlying function $f$ is twice continuously differentiable on the interpolation interval.

For the linear interpolant $L(x)$, the pointwise error is:

$$ \boxed{ f(x)-L(x) = \frac{f''(\xi_x)}2 (x-x_0)(x-x_1) } $$

for some $\xi_x$ in the interval.

Let:

$$ h=x_1-x_0. $$

The maximum absolute magnitude of:

$$ (x-x_0)(x-x_1) $$

inside the interval is $h^2/4$.

Therefore:

$$ \boxed{ |f(x)-L(x)| \le \frac{h^2}{8} \max_{\xi\in[x_0,x_1]}|f''(\xi)| }. $$

This gives an $O(h^2)$ interpolation error for sufficiently smooth functions.

Reducing grid spacing by half can reduce the leading linear-interpolation error by approximately a factor of four, provided the asymptotic assumptions apply.

This is a mathematical approximation error, independent of floating-point rounding.

### 12.5 Numerically robust lerp

A common implementation is:

```
double lerp(double a, double b, double t) {
    return a + t * (b - a);
}
```

Using FMA:

```
double lerp(double a, double b, double t) {
    return std::fma(t, b - a, a);
}
```

can reduce one rounding step when FMA is supported with the intended semantics.

But the subtraction `b-a` can still overflow when endpoints are extreme, even if the interpolated result is finite.

The alternative:

```
(1.0 - t) * a + t * b
```

can avoid that particular large-difference overflow in some situations but introduces other rounding behavior.

A general-purpose interpolation API should specify its intended range, precision requirements, handling of infinities and NaNs, and endpoint behavior.


Bilinear interpolation extends linear interpolation to two dimensions.

Suppose we know four scalar values on the corners of a rectangular cell:

$$ f_{00},\quad f_{10},\quad f_{01},\quad f_{11}. $$

Let:

$$ u,v\in[0,1] $$

be the normalized horizontal and vertical coordinates.

First interpolate horizontally on the bottom edge:

$$ a(u)=(1-u)f_{00}+uf_{10}. $$

On the top edge:

$$ b(u)=(1-u)f_{01}+uf_{11}. $$

Then interpolate vertically:

$$ f(u,v)=(1-v)a(u)+vb(u). $$

Substitute:

$$ \boxed{ \begin{aligned} f(u,v)=& (1-u)(1-v)f_{00} +u(1-v)f_{10}\\ &+(1-u)vf_{01} +uvf_{11}. \end{aligned} } $$

### 13.1 Worked example

Let the four values be:

$$ f_{00}=1,\quad f_{10}=3, $$

$$ f_{01}=5,\quad f_{11}=7. $$

Choose:

$$ u=0.25,\qquad v=0.75. $$

Bottom interpolation:

$$ a=1+0.25(3-1)=1.5. $$

Top interpolation:

$$ b=5+0.25(7-5)=5.5. $$

Vertical interpolation:

$$ f=1.5+0.75(5.5-1.5) =1.5+3 =\boxed{4.5}. $$

### 13.2 Why the name is bilinear

For fixed $v$, the function is linear in $u$.

For fixed $u$, it is linear in $v$.

But it can contain a product term $uv$, so it is not generally an affine function of the pair $(u,v)$ jointly.

Expand:

$$ f(u,v) = f_{00} +(f_{10}-f_{00})u +(f_{01}-f_{00})v +(f_{00}-f_{10}-f_{01}+f_{11})uv. $$

The mixed term is:

$$ uv. $$

If its coefficient is nonzero, the surface can twist.

A bilinear surface patch is not necessarily planar.

### 13.3 Derivatives

Differentiate with respect to $u$:

$$ \boxed{ \frac{\partial f}{\partial u} = (1-v)(f_{10}-f_{00}) + v(f_{11}-f_{01}) }. $$

With respect to $v$:

$$ \boxed{ \frac{\partial f}{\partial v} = (1-u)(f_{01}-f_{00}) + u(f_{11}-f_{10}) }. $$

These derivatives are useful when reconstructing image gradients, height-field slopes, or texture sensitivities.

### 13.4 Separable filters

Bilinear interpolation can be implemented as two horizontal lerps followed by one vertical lerp.

The order can also be reversed in exact arithmetic.

This separability is computationally useful.

The same idea generalizes to higher-dimensional tensor-product interpolation.

In image processing, bilinear interpolation is frequently used for texture sampling and image resizing, but its smoothness properties have limitations: values are continuous across properly matched grid-cell boundaries, while derivatives can be discontinuous.


Trilinear interpolation extends bilinear interpolation to three dimensions.

Imagine a voxel cell with eight corner values:

$$ f_{ijk}, \qquad i,j,k\in\{0,1\}. $$

Let:

$$ u,v,w\in[0,1] $$

represent normalized coordinates within the cell.

The interpolation formula is:

$$ \boxed{ f(u,v,w)= \sum_{i=0}^{1} \sum_{j=0}^{1} \sum_{k=0}^{1} f_{ijk} B_i(u)B_j(v)B_k(w) } $$

where:

$$ B_0(t)=1-t, \qquad B_1(t)=t. $$

Each corner contributes a product of three one-dimensional weights.

### 14.1 Geometric interpretation

Every corner receives a weight based on its relative proximity in each coordinate direction.

For example, the corner $(0,0,0)$ has weight:

$$ (1-u)(1-v)(1-w). $$

The corner $(1,1,1)$ has weight:

$$ uvw. $$

The weights sum to one:

$$ \sum_{i,j,k}B_i(u)B_j(v)B_k(w)=1. $$

For $u,v,w\in[0,1]$, all weights are nonnegative.

Thus scalar trilinear interpolation stays within the range of the eight corner values.

### 14.2 Worked voxel example

Suppose the eight corner values are:

$$ 0,1,2,3,4,5,6,7 $$

with a consistent ordering.

At the centre:

$$ u=v=w=0.5. $$

Every corner receives weight:

$$ (0.5)^3=\frac18. $$

Thus:

$$ f(0.5,0.5,0.5) = \frac{0+1+2+3+4+5+6+7}{8}. $$

Therefore:

$$ \boxed{f=3.5}. $$

### 14.3 Application to TSDF sampling

A truncated signed-distance field stores scalar distance estimates at discrete grid locations.

To evaluate the field between grid points, a renderer or mesher can use trilinear interpolation.

Suppose the field is:

$$ \phi(\mathbf x). $$

The interpolated field gives an approximate distance value at a continuous position.

Its gradient can be approximated by differentiating the trilinear polynomial within the current cell.

The gradient direction is often used to estimate a surface normal:

$$ \mathbf n= \frac{\nabla\phi}{\|\nabla\phi\|}. $$

But a trilinearly interpolated function is generally not a true signed-distance field everywhere between samples, even if grid values came from accurate distances.

Its gradient need not have unit magnitude.

Likewise, gradients may be discontinuous when crossing cell boundaries.

For reconstruction, these are important limitations rather than implementation bugs.


Linear interpolation uses a degree-one polynomial.

Cubic interpolation uses a polynomial of degree at most three:

$$ \boxed{ p(t)=at^3+bt^2+ct+d. } $$

A cubic polynomial has four coefficients.

Therefore four independent constraints can determine it uniquely, assuming the resulting constraint system is nonsingular.

One choice is to interpolate four data values.

Another is to constrain two endpoint values and two endpoint derivatives.

The second construction leads to cubic Hermite interpolation.

### 15.1 Why cubics are so useful

A cubic has enough flexibility to control endpoint positions and tangents.

This makes it possible to join curve segments with specified first-derivative continuity.

It also remains relatively inexpensive to evaluate.

Cubic splines are common in animation curves, camera paths, trajectory planning, geometry processing, and interpolation between time samples.

However, cubic interpolation is not automatically monotonic, bounded, or free of overshoot.

The choice of endpoint tangents and parameterization is crucial.

### 15.2 Continuity classes

Suppose two curve segments meet at time $t_0$.

$C^0$ continuity means their positions agree at the join.

$C^1$ continuity means their positions and first derivatives agree.

$C^2$ continuity means their positions, first derivatives, and second derivatives agree.

These are parametric continuity conditions.

Geometric continuity, often denoted $G^k$, allows some derivative differences related to reparameterization while preserving geometric smoothness.

For camera and animation paths, $C^1$ avoids instantaneous velocity jumps, while $C^2$ continuity avoids instantaneous acceleration jumps under a suitable time parameterization.

But visual smoothness and physical smoothness are not always equivalent. A curve can be geometrically smooth while its time parameterization produces abrupt speed changes.


Hermite interpolation constructs a cubic from endpoint values and endpoint derivatives.

Let:

$$ p(0)=p_0, $$

$$ p(1)=p_1, $$

$$ p'(0)=m_0, $$

$$ p'(1)=m_1. $$

Here $m_0,m_1$ are derivatives with respect to the local parameter $t$, not necessarily derivatives with respect to world-space distance or physical time.

Assume:

$$ p(t)=at^3+bt^2+ct+d. $$

### 16.1 Solve the four constraints

At $t=0$:

$$ p(0)=d=p_0. $$

Differentiate:

$$ p'(t)=3at^2+2bt+c. $$

At $t=0$:

$$ p'(0)=c=m_0. $$

At $t=1$:

$$ a+b+c+d=p_1. $$

Therefore:

$$ a+b=p_1-p_0-m_0. $$

The final derivative constraint is:

$$ 3a+2b+c=m_1. $$

Thus:

$$ 3a+2b=m_1-m_0. $$

Solve these two equations.

Multiply the first equation by two:

$$ 2a+2b=2(p_1-p_0-m_0). $$

Subtract from the second:

$$ a=m_1-m_0-2(p_1-p_0-m_0). $$

Simplify:

$$ \boxed{ a=2p_0-2p_1+m_0+m_1 }. $$

Then:

$$ b=p_1-p_0-m_0-a. $$

Substitute:

$$ \boxed{ b=-3p_0+3p_1-2m_0-m_1 }. $$

And:

$$ c=m_0,\qquad d=p_0. $$

### 16.2 Hermite basis functions

Collect terms by endpoint values and tangents:

$$ \boxed{ p(t)= h_{00}(t)p_0+ h_{10}(t)m_0+ h_{01}(t)p_1+ h_{11}(t)m_1 }. $$

The basis functions are:

$$ \boxed{ \begin{aligned} h_{00}(t)&=2t^3-3t^2+1,\\ h_{10}(t)&=t^3-2t^2+t,\\ h_{01}(t)&=-2t^3+3t^2,\\ h_{11}(t)&=t^3-t^2. \end{aligned} } $$

The naming convention is historical: the first subscript identifies the endpoint, and the second indicates value or derivative basis information in a common notation.

Check the endpoint values:

$$ h_{00}(0)=1,\quad h_{01}(0)=0, $$

$$ h_{00}(1)=0,\quad h_{01}(1)=1. $$

The tangent basis terms vanish at both endpoints, so they do not change endpoint positions.

Differentiating the basis shows:

$$ p'(0)=m_0,\qquad p'(1)=m_1. $$

### 16.3 Worked example

Take:

$$ p_0=0,\quad p_1=1, $$

$$ m_0=1,\quad m_1=1. $$

These constraints define the straight line:

$$ \boxed{p(t)=t}. $$

At $t=0.5$:

$$ p(0.5)=0.5. $$

Now change the endpoint tangents:

$$ m_0=3,\quad m_1=-1. $$

The cubic still passes through $p_0=0$ and $p_1=1$, but it leaves the first endpoint rapidly and approaches the second with negative tangent.

This can introduce overshoot or nonmonotonic behavior.

Endpoint interpolation alone does not determine how the curve behaves between endpoints.

### 16.4 Physical time scaling

Suppose a segment covers the physical time interval:

$$ [t_0,t_1], \qquad \Delta t=t_1-t_0. $$

Define normalized parameter:

$$ u=\frac{t-t_0}{\Delta t}. $$

If the supplied physical velocities are:

$$ v_0=\frac{dp}{dt}(t_0), \qquad v_1=\frac{dp}{dt}(t_1), $$

then the local Hermite tangents must be:

$$ \boxed{ m_0=\Delta t\,v_0, \qquad m_1=\Delta t\,v_1 }. $$

Why?

By the chain rule:

$$ \frac{dp}{du} = \frac{dp}{dt}\frac{dt}{du} = \Delta t\frac{dp}{dt}. $$

Using physical velocities directly as local derivatives without multiplying by $\Delta t$ gives incorrect interpolation when segment duration differs from one time unit.

This is a common bug in animation and trajectory interpolation.


Catmull–Rom splines construct interpolating curves through a sequence of control points while estimating tangents from neighbouring points.

Consider four consecutive scalar or vector points:

$$ p_0,p_1,p_2,p_3. $$

We want a cubic segment from $p_1$ to $p_2$.

A common uniform Catmull–Rom choice estimates the local tangents as:

$$ \boxed{ m_1=\frac{p_2-p_0}{2} } $$

and:

$$ \boxed{ m_2=\frac{p_3-p_1}{2} }. $$

Then use cubic Hermite interpolation:

$$ \boxed{ C(t)= h_{00}p_1+ h_{10}m_1+ h_{01}p_2+ h_{11}m_2 }. $$

### 17.1 Why the neighbouring points appear

If we only know $p_1$ and $p_2$, we know the segment endpoints but not its desired entry and exit directions.

The preceding point $p_0$ suggests how the curve approaches $p_1$.

The following point $p_3$ suggests how it leaves $p_2$.

The centred differences provide tangent estimates.

Thus Catmull–Rom turns position samples into a smooth interpolating path without requiring artists or algorithms to specify tangent vectors manually.

### 17.2 Numerical example

Take:

$$ p_0=0,\quad p_1=1,\quad p_2=2,\quad p_3=3. $$

The tangents are:

$$ m_1=\frac{2-0}{2}=1, $$

$$ m_2=\frac{3-1}{2}=1. $$

The Hermite segment goes from one to two with constant tangent one.

Therefore:

$$ \boxed{C(t)=1+t}. $$

At $t=0.5$:

$$ \boxed{C(0.5)=1.5}. $$

### 17.3 Uniform, chordal, and centripetal parameterizations

Not all Catmull–Rom splines use equally spaced parameter values.

Uniform Catmull–Rom uses constant parameter intervals.

Chordal parameterization chooses intervals proportional to chord lengths:

$$ t_{i+1}-t_i=\|p_{i+1}-p_i\|. $$

Centripetal parameterization uses:

$$ \boxed{ t_{i+1}-t_i=\sqrt{\|p_{i+1}-p_i\|} }. $$

More generally:

$$ t_{i+1}-t_i= \|p_{i+1}-p_i\|^\alpha, $$

with typical choices:

$$ \alpha=0 \text{ (uniform)}, $$

$$ \alpha=\frac12 \text{ (centripetal)}, $$

$$ \alpha=1 \text{ (chordal)}. $$

Centripetal Catmull–Rom is often preferred for irregularly spaced points because it can avoid certain local cusps and self-intersections under its standard conditions.

But the simple uniform tangent formula cannot be reused unchanged with arbitrary nonuniform parameter intervals.

The tangent estimates must account for the knot spacing.

### 17.4 Application to camera paths

Suppose a cinematic camera follows sampled waypoints.

Using independent linear segments produces velocity-direction discontinuities at joins.

Catmull–Rom provides a smoother interpolated path through the points.

However, a smooth curve parameter does not automatically produce constant camera speed.

If the camera position is:

$$ \mathbf p(t), $$

its speed is:

$$ \boxed{ v(t)=\|\mathbf p'(t)\| }. $$

That speed generally varies with $t$.

For approximately constant-speed movement, use arc-length parameterization or an arc-length lookup table.

The arc length is:

$$ \boxed{ s(t)=\int_0^t\|\mathbf p'(\tau)\|\,d\tau }. $$

To move at constant speed, invert the relation $s(t)$ approximately and evaluate the curve at the corresponding parameter.

This combines interpolation, numerical integration, and root finding.


A cubic Bézier curve is defined by four control points:

$$ P_0,P_1,P_2,P_3. $$

Its mathematical form is:

$$ \boxed{ B(t)= (1-t)^3P_0+ 3(1-t)^2tP_1+ 3(1-t)t^2P_2+ t^3P_3 } $$

for:

$$ t\in[0,1]. $$

The coefficients are the cubic Bernstein basis functions.

### 18.1 General Bernstein basis

For degree $n$:

$$ \boxed{ B_{i,n}(t)= \binom ni t^i(1-t)^{n-i} }. $$

A degree-$n$ Bézier curve is:

$$ \boxed{ P(t)=\sum_{i=0}^{n}B_{i,n}(t)P_i }. $$

### 18.2 Partition of unity

The Bernstein basis functions sum to one:

$$ \sum_{i=0}^{n} \binom ni t^i(1-t)^{n-i}. $$

By the binomial theorem:

$$ =(t+(1-t))^n=1. $$

Thus:

$$ \boxed{ \sum_iB_{i,n}(t)=1 }. $$

For $t\in[0,1]$, each basis function is nonnegative.

Therefore a Bézier curve lies within the convex hull of its control points.

This property is extremely useful for geometric bounding and subdivision.

### 18.3 Endpoints

At $t=0$:

$$ B(0)=P_0. $$

At $t=1$:

$$ B(1)=P_3. $$

The intermediate control points generally do not lie on the curve.

They shape its tangents and curvature.

This differs from Catmull–Rom, which interpolates the defining waypoints.

### 18.4 Endpoint derivatives

Differentiate the cubic Bézier expression.

At the beginning:

$$ \boxed{ B'(0)=3(P_1-P_0) }. $$

At the end:

$$ \boxed{ B'(1)=3(P_3-P_2) }. $$

Therefore the first control handle determines the initial tangent direction and magnitude.

The final control handle determines the ending tangent.

The second derivative at the start is:

$$ \boxed{ B''(0)=6(P_0-2P_1+P_2) }. $$

At the end:

$$ \boxed{ B''(1)=6(P_3-2P_2+P_1) }. $$

These quantities are important for curvature and continuity matching between segments.

### 18.5 De Casteljau's algorithm

Instead of evaluating the Bernstein polynomial directly, we can construct a Bézier point through repeated linear interpolation.

First level:

$$ A_0=(1-t)P_0+tP_1, $$

$$ A_1=(1-t)P_1+tP_2, $$

$$ A_2=(1-t)P_2+tP_3. $$

Second level:

$$ C_0=(1-t)A_0+tA_1, $$

$$ C_1=(1-t)A_1+tA_2. $$

Third level:

$$ \boxed{ B(t)=(1-t)C_0+tC_1 }. $$

This is de Casteljau's algorithm.

It uses the affine structure of the control points directly.

It is valuable for stable evaluation, curve subdivision, and geometric reasoning.

### 18.6 C++ implementation

```
double cubicBezier(
    double p0,
    double p1,
    double p2,
    double p3,
    double t
) {
    // First interpolation layer.
    const double a = std::lerp(p0, p1, t);
    const double b = std::lerp(p1, p2, t);
    const double c = std::lerp(p2, p3, t);

    // Second layer.
    const double d = std::lerp(a, b, t);
    const double e = std::lerp(b, c, t);

    // Final point.
    return std::lerp(d, e, t);
}
```

C++20's `std::lerp` is specified to provide useful numerical guarantees for floating-point interpolation, including exact endpoints and boundedness properties under its stated conditions.

The downloadable laboratory also includes direct Bernstein evaluation so the two formulations can be compared.

### 18.7 Geometry application

Bézier curves appear in vector graphics, font outlines, animation easing, camera controls, surface design, and patch-based geometry.

Their convex-hull property is useful for geometric pruning and subdivision.

Their derivative control is useful for joining segments smoothly.

But a cubic Bézier curve does not always pass through $P_1$ or $P_2$. If a tool promises to interpolate all given waypoints, Bézier control points and Catmull–Rom interpolation points cannot be treated as interchangeable.



### Figure N10-8 — More polynomial degree does not guarantee a better picture

![[Visual Recall/Math Foundations/N10-8.png]]

Both interpolants match every node. Their behaviour between nodes differs. Chebyshev extrema reduce this example’s endpoint oscillation; this comparison is not a universal interpolation error bound.


### Figure N10-9 — Bézier and Hermite can tell the same curve story

![[Visual Recall/Math Foundations/N10-9.png]]

Use Hermite tangents 3(P1−P0) and 3(P3−P2) for this cubic Bézier curve. The convex hull of the control points bounds the curve through nonnegative weights summing to one; inner control points need not lie on the curve.


<!-- end V10.18 -->

<!-- begin V10.19 -->
## Finding roots with guarantees and convergence checks


We now shift from reconstructing functions to solving equations.

Suppose we want:

$$ \boxed{f(x)=0}. $$

A root is a value $r$ satisfying that equation.

Bisection is a bracketing root-finding method.

Assume:

1. $f$ is continuous on $[a,b]$.
2. $f(a)$ and $f(b)$ have opposite signs, or an endpoint is already a root.

Then the intermediate value theorem guarantees at least one root in the interval.

### 19.1 The bisection update

Calculate the midpoint:

$$ \boxed{ m=\frac{a+b}{2} }. $$

Evaluate $f(m)$.

If $f(m)=0$, we found a root.

Otherwise, choose the half-interval whose endpoints have opposite signs.

Repeat.

Each iteration halves the bracket width.

### 19.2 Worked example: square root of two

Solve:

$$ f(x)=x^2-2=0. $$

The positive root is:

$$ r=\sqrt2. $$

Choose:

$$ a_0=1,\qquad b_0=2. $$

We have:

$$ f(1)=-1, $$

$$ f(2)=2. $$

The signs differ.

First midpoint:

$$ m_1=1.5. $$

Evaluate:

$$ f(1.5)=2.25-2=0.25. $$

The sign is positive, so the root lies in:

$$ [1,1.5]. $$

Second midpoint:

$$ m_2=1.25. $$

Evaluate:

$$ f(1.25)=1.5625-2=-0.4375. $$

The sign is negative, so the root lies in:

$$ [1.25,1.5]. $$

Third midpoint:

$$ m_3=1.375. $$

Evaluate:

$$ f(1.375)=1.890625-2=-0.109375. $$

The root lies in:

$$ [1.375,1.5]. $$

Fourth midpoint:

$$ m_4=1.4375. $$

Evaluate:

$$ f(1.4375)=2.06640625-2=0.06640625. $$

The root lies in:

$$ [1.375,1.4375]. $$

The interval steadily shrinks around $\sqrt2$.

### 19.3 Error bound

The initial bracket width is:

$$ W_0=b_0-a_0. $$

After $n$ bisection updates:

$$ \boxed{ W_n=\frac{W_0}{2^n} }. $$

The true root remains somewhere inside the maintained bracket.

If we return the midpoint of the final bracket, its absolute error is bounded by half the bracket width:

$$ \boxed{ |m_n-r| \le \frac{W_0}{2^{n+1}} } $$

when $n$ denotes the number of completed halving updates before the final midpoint is returned.

This is a genuine location-error certificate derived from the bracket.

### 19.4 How many iterations?

Suppose we want the bracket width to be at most $\varepsilon$:

$$ \frac{W_0}{2^n}\le\varepsilon. $$

Rearrange:

$$ 2^n\ge\frac{W_0}{\varepsilon}. $$

Take base-two logarithms:

$$ n\ge \log_2\left(\frac{W_0}{\varepsilon}\right). $$

Thus:

$$ \boxed{ n\ge \left\lceil \log_2\frac{W_0}{\varepsilon} \right\rceil }. $$

Bisection has predictable worst-case progress under its assumptions.

### 19.5 Practical limitations

A sign-changing bracket is a sufficient condition for the existence of an odd-crossing root under continuity, but it is not necessary for all roots.

For example:

$$ f(x)=(x-2)^2 $$

has a root at $x=2$, but the function is nonnegative on both sides.

A strict sign-change test may fail to bracket it.

Also, a discontinuous function can change sign without having a root, such as:

$$ f(x)= \begin{cases} -1,&x<0,\\ 1,&x\ge0. \end{cases} $$

Bisection may repeatedly narrow around the discontinuity, but no root exists.

The continuity assumption is essential.

### 19.6 Numerically stable midpoint

A naive midpoint calculation:

```
double middle = (a + b) / 2.0;
```

may overflow if $a+b$ overflows, even when the midpoint is finite.

C++20 provides:

```
std::midpoint(a, b)
```

which avoids such intermediate overflow under its specified conditions.

For robust numerical solvers, seemingly minor helper operations can affect correctness.


Bisection maintains a bracket.

The secant method instead approximates the derivative from two recent function evaluations and uses that slope to predict the root.

Let the current estimates be:

$$ x_{n-1},\qquad x_n. $$

The slope of the secant line through the corresponding function values is:

$$ \boxed{ m_n= \frac{ f(x_n)-f(x_{n-1}) }{ x_n-x_{n-1} } }. $$

Approximate the function near $x_n$ by:

$$ f(x)\approx f(x_n)+m_n(x-x_n). $$

Set the approximation to zero:

$$ 0=f(x_n)+m_n(x_{n+1}-x_n). $$

Solve:

$$ x_{n+1}=x_n-\frac{f(x_n)}{m_n}. $$

Substitute the secant slope:

$$ \boxed{ x_{n+1} = x_n- f(x_n) \frac{x_n-x_{n-1}} {f(x_n)-f(x_{n-1})} }. $$

### 20.1 Numerical example

Again solve:

$$ f(x)=x^2-2. $$

Start with:

$$ x_0=1,\qquad x_1=2. $$

Their function values are:

$$ f(x_0)=-1,\qquad f(x_1)=2. $$

The next iterate is:

$$ x_2= 2- 2\frac{2-1}{2-(-1)}. $$

Thus:

$$ \boxed{x_2=\frac43\approx1.333333}. $$

Evaluate:

$$ f(4/3)=\frac{16}{9}-2=-\frac29. $$

Next:

$$ x_3= \frac43- \left(-\frac29\right) \frac{\frac43-2}{-\frac29-2}. $$

Simplifying:

$$ \boxed{x_3=1.4}. $$

The iterates approach $\sqrt2$.

### 20.2 Convergence order

Under appropriate smoothness assumptions near a simple root, the secant method has superlinear convergence order:

$$ \boxed{ p=\frac{1+\sqrt5}{2}\approx1.618034 }. $$

The number is the golden ratio.

This means the asymptotic error behaves approximately like:

$$ e_{n+1}\approx C|e_n|^p $$

for a suitable constant $C$, near the root.

The secant method can converge faster than bisection in favorable cases, without requiring an analytical derivative.

But it does not provide a guaranteed bracketing interval.

### 20.3 Failure modes

The denominator:

$$ f(x_n)-f(x_{n-1}) $$

may be zero or extremely small.

The resulting update can be undefined or enormous.

The method can move outside the function's domain, oscillate, or diverge.

It can also converge to an unintended root.

Therefore a production implementation needs finite-value checks, denominator safeguards, iteration limits, and meaningful stopping tests.


Newton's method for root finding is:

$$ \boxed{ x_{n+1} = x_n-\frac{f(x_n)}{f'(x_n)} }. $$

Optimization also uses root finding, but here the goal is explicitly to solve $f(x)=0$, not necessarily to minimize a function.

### 21.1 Derivation from a tangent line

Near $x_n$, approximate:

$$ f(x_n+\Delta x) \approx f(x_n)+f'(x_n)\Delta x. $$

Set this local linear model to zero:

$$ 0=f(x_n)+f'(x_n)\Delta x. $$

Solve:

$$ \Delta x=-\frac{f(x_n)}{f'(x_n)}. $$

Thus:

$$ x_{n+1}=x_n+\Delta x = x_n-\frac{f(x_n)}{f'(x_n)}. $$

### 21.2 Numerical example

Solve:

$$ f(x)=x^2-2. $$

Then:

$$ f'(x)=2x. $$

The Newton update is:

$$ x_{n+1} = x_n-\frac{x_n^2-2}{2x_n}. $$

Simplify:

$$ \boxed{ x_{n+1}= \frac12\left(x_n+\frac2{x_n}\right) }. $$

Start at:

$$ x_0=1.5. $$

First update:

$$ x_1= \frac12\left(1.5+\frac2{1.5}\right) = \boxed{1.4166666667}. $$

Second update:

$$ x_2= \frac12\left(1.4166666667+\frac2{1.4166666667}\right) \approx \boxed{1.4142156863}. $$

Third update:

$$ x_3\approx1.4142135624. $$

The result rapidly approaches:

$$ \sqrt2\approx1.4142135623730951. $$

### 21.3 Deriving quadratic convergence

Let $r$ be a simple root:

$$ f(r)=0,\qquad f'(r)\ne0. $$

Define the error:

$$ e_n=x_n-r. $$

Expand $f(x_n)$ around $r$:

$$ f(x_n) = f'(r)e_n+ \frac12f''(r)e_n^2+ O(e_n^3). $$

Likewise:

$$ f'(x_n) = f'(r)+f''(r)e_n+O(e_n^2). $$

Substitute into Newton's update:

$$ e_{n+1} = e_n- \frac{ f'(r)e_n+\frac12f''(r)e_n^2+O(e_n^3) }{ f'(r)+f''(r)e_n+O(e_n^2) }. $$

Expand the quotient to second order.

The first-order terms cancel, leaving:

$$ \boxed{ e_{n+1} = \frac{f''(r)}{2f'(r)}e_n^2 + O(e_n^3) }. $$

Thus, when the iterate is sufficiently near a simple root and the necessary smoothness conditions hold:

$$ \boxed{ |e_{n+1}|\approx C|e_n|^2 }. $$

This is quadratic convergence.

Roughly speaking, once the method enters the asymptotic region, the number of correct digits can approximately double each iteration.

### 21.4 Multiple roots change convergence

Consider:

$$ f(x)=(x-r)^m $$

for integer $m>1$.

Then:

$$ f'(x)=m(x-r)^{m-1}. $$

Newton's update becomes:

$$ x_{n+1} = x_n- \frac{(x_n-r)^m}{m(x_n-r)^{m-1}}. $$

Simplify:

$$ x_{n+1} = x_n-\frac{x_n-r}{m}. $$

Subtract $r$:

$$ \boxed{ e_{n+1} = \left(1-\frac1m\right)e_n }. $$

This is linear convergence, not quadratic.

For a double root:

$$ m=2, $$

so:

$$ e_{n+1}=\frac12e_n. $$

This shows that Newton's celebrated quadratic convergence requires more than simply having a differentiable function. A simple root and suitable local conditions matter.

### 21.5 Newton can diverge

Consider:

$$ f(x)=x^3-2x+2. $$

Its derivative is:

$$ f'(x)=3x^2-2. $$

Start at $x_0=0$.

Then:

$$ f(0)=2,\qquad f'(0)=-2. $$

Newton gives:

$$ x_1=0-\frac2{-2}=1. $$

At $x_1=1$:

$$ f(1)=1,\qquad f'(1)=1. $$

Thus:

$$ x_2=1-\frac11=0. $$

The method cycles:

$$ \boxed{0\rightarrow1\rightarrow0\rightarrow1\rightarrow\cdots}. $$

Newton's method is locally fast but not globally guaranteed.


The solver comparison closes this discussion of root-finding methods.

This is not simply a speed comparison. It is a comparison of what information and guarantees the algorithm retains.

| Property                                      | Bisection                              | Secant            | Newton                     |
| --------------------------------------------- | -------------------------------------- | ----------------- | -------------------------- |
| Requires sign-changing bracket                | Yes                                    | No                | No                         |
| Requires analytical derivative                | No                                     | No                | Yes, in standard form      |
| Typical local convergence order               | Linear                                 | \~1.618           | Quadratic at a simple root |
| Maintains a root-containing interval          | Yes                                    | No                | No                         |
| Can diverge or leave domain                   | Not under valid bracketing assumptions | Yes               | Yes                        |
| Can exploit local curvature/slope efficiently | Limited                                | Approximate slope | Exact supplied derivative  |

### 22.1 Why hybrid solvers are effective

A hybrid solver tries to combine the safety of bracketing with the speed of open methods.

For example, it may maintain:

$$ f(a)f(b)<0 $$

while proposing a faster candidate using secant or inverse interpolation.

If the candidate is unsuitable, it falls back to a safe bracketing step.

Brent's root-finding method combines bracketing with interpolation-based acceleration.

SciPy's `brentq` documentation describes this combination of interval bracketing, bisection, and inverse quadratic extrapolation, and requires a sign-changing interval for the standard continuous-function guarantee.&#x20;

[image](https://www.google.com/s2/favicons?domain=https://docs.scipy.org\&sz=32)

SciPy v1.18.0 Manual+1



### 22.2 A safe design policy

For general scalar root finding, if a valid sign-changing bracket is available, a safeguarded bracketing method is often a strong default.

If a trustworthy derivative and good initial estimate are available, Newton can be substantially faster.

If derivatives are expensive or unavailable, secant methods may be attractive.

But a solver should report why it stopped.

Possible statuses include successful residual convergence, successful bracket-width convergence, maximum iterations, invalid function evaluation, singular derivative, lost bracket, and numerical stagnation.

A tiny step by itself is not proof that the root has been found.

### 22.3 Error bounds for the root

If we maintain:

$$ r\in[a,b], $$

then returning the midpoint gives a certified absolute location bound:

$$ \boxed{ |\hat r-r|\le\frac{b-a}{2} }. $$

An open solver generally lacks this bracket-based certificate.

It can instead use local residual and derivative information.

For a root $r$, the mean value theorem gives:

$$ f(x)-f(r)=f'(\xi)(x-r) $$

for some $\xi$ between $x$ and $r$.

If we know:

$$ |f'(t)|\ge m>0 $$

throughout the relevant interval, then:

$$ |f(x)|\ge m|x-r|. $$

Therefore:

$$ \boxed{ |x-r|\le\frac{|f(x)|}{m} }. $$

The derivative lower bound is crucial.

Without it, a small residual does not guarantee an accurate root.


The preceding chapters examined algorithms and their local failure modes. We now measure error across an entire computation and verify that an implementation behaves as its mathematical specification predicts.

The goal is to construct a numerical error budget.

Suppose an engine estimates a physical quantity $Q$. Several distinct sources of error may be present:

$$ \boxed{ \hat Q-Q = \text{modeling contribution} + \text{discretization contribution} + \text{solver contribution} + \text{arithmetic contribution} } $$

This is a conceptual decomposition; the contributions can interact and are not necessarily uniquely separable or exactly additive in a nonlinear pipeline.

For Monte Carlo calculations, sampling error adds another component.

For experiments, uncertainty in the reference data may also matter.

A useful engineering workflow must identify which source dominates before deciding how to improve accuracy.


An iterative numerical algorithm repeatedly applies an update:

$$ \boxed{ x_{n+1}=G(x_n) }. $$

A solution $x^\star$ is a fixed point when:

$$ \boxed{ G(x^\star)=x^\star }. $$

Root finding can often be rewritten as a fixed-point problem. So can many linear solvers and optimization methods.

We want to know how far $x_n$ is from the solution without necessarily knowing $x^\star$.

### 23.1 Contraction mappings

Suppose $G$ is a contraction on a complete metric space, or an appropriate closed invariant subset, with contraction factor:

$$ 0\le q<1. $$

This means:

$$ \boxed{ \|G(x)-G(y)\| \le q\|x-y\| } $$

for all relevant $x,y$.

Because $x^\star=G(x^\star)$:

$$ \begin{aligned} \|x_{n+1}-x^\star\| &= \|G(x_n)-G(x^\star)\|\\ &\le q\|x_n-x^\star\|. \end{aligned} $$

Therefore:

$$ \boxed{ e_{n+1}\le qe_n } $$

where:

$$ e_n=\|x_n-x^\star\|. $$

Applying repeatedly:

$$ \boxed{ e_n\le q^ne_0 }. $$

This establishes geometric convergence.

### 23.2 A posteriori error bound

The previous bound depends on $e_0$, which may not be known.

Can we estimate the current error from successive iterates?

Using a telescoping sum:

$$ x^\star-x_n = (x_{n+1}-x_n) + (x_{n+2}-x_{n+1}) +\cdots $$

under convergence.

Take norms and use the triangle inequality:

$$ \|x^\star-x_n\| \le \sum_{k=n}^{\infty} \|x_{k+1}-x_k\|. $$

The contraction property implies successive differences decrease by at least the factor $q$:

$$ \|x_{n+j+1}-x_{n+j}\| \le q^j\|x_{n+1}-x_n\|. $$

Therefore:

$$ \|x^\star-x_n\| \le \|x_{n+1}-x_n\| \sum_{j=0}^{\infty}q^j. $$

The geometric series is:

$$ \sum_{j=0}^{\infty}q^j=\frac1{1-q}. $$

Thus:

$$ \boxed{ \|x^\star-x_n\| \le \frac{\|x_{n+1}-x_n\|}{1-q} }. $$

A related form expressed using the previous step is:

$$ \boxed{ \|x^\star-x_n\| \le \frac{q}{1-q}\|x_n-x_{n-1}\| }. $$

These bounds are useful only when the contraction property and factor $q$ are justified.

### 23.3 Why a tiny update is not automatically convergence

Suppose:

$$ q=0.9999. $$

Then:

$$ \frac1{1-q}=10{,}000. $$

Even if:

$$ |x_{n+1}-x_n|=10^{-6}, $$

the first bound permits an error as large as:

$$ 10^{-2}. $$

A small step does not necessarily indicate proximity to the solution when contraction is weak.

This is closely related to the false-convergence risk we identified in KairoMath's line-search implementation discussed earlier.

### 23.4 Residual-based bounds

For a linear system:

$$ A\mathbf x=\mathbf b, $$

let the approximate solution be $\hat{\mathbf x}$, and define:

$$ \mathbf r=\mathbf b-A\hat{\mathbf x}. $$

We have:

$$ \mathbf x^\star-\hat{\mathbf x}=A^{-1}\mathbf r. $$

Therefore:

$$ \boxed{ \|\mathbf x^\star-\hat{\mathbf x}\| \le \|A^{-1}\|\|\mathbf r\| }. $$

This is a posteriori in the sense that the residual is computed from the approximate solution.

But the usefulness of the bound depends on controlling $\|A^{-1}\|$.

In an ill-conditioned system, a very small residual may give only a weak solution-error guarantee.

### 23.5 Root-finding certification

For bisection, the root lies inside the maintained bracket:

$$ [a_n,b_n]. $$

The midpoint error is bounded by:

$$ \boxed{ |\hat r-r|\le\frac{b_n-a_n}{2} }. $$

This bound does not require knowing the derivative at the root.

For Newton and secant methods, local convergence can be much faster, but a corresponding error certificate usually requires more assumptions.

This is why numerical solver output should include both the approximate result and evidence about its accuracy.



### Figure N10-10 — A root solver needs both progress and a safety argument

![[Visual Recall/Math Foundations/N10-10.png]]

The error comparison uses cos(x)−x with specified starting values. Bisection has a shrinking bracket; Newton and secant need additional hypotheses. The plotting floor is explicitly shown rather than interpreted as continuing convergence.


<!-- end V10.19 -->

<!-- begin V10.20 -->
<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/01 - Computing Foundations/00 - Reading Map|Reading map]] · [[Chapter Edition/01 - Computing Foundations/27 - Gauss–Newton for Geometry and Pose|Previous chapter]] · [[Chapter Edition/01 - Computing Foundations/29 - Discretization, Convergence, and Experimental Design|Next chapter]]
