# Calculus and optimization — visual recall

## One-page quick recall

- **Scalar change:** $f(a+h)=f(a)+f'(a)h+o(|h|)$.
- **Vector output:** $F(x+h)=F(x)+J_F(x)h+o(\|h\|)$; $J$ has output rows and input columns.
- **Scalar curvature:** $f(x+h)\approx f(x)+\nabla f^T h+\tfrac12h^THh$.
- **Accumulation:** $y(t)=y_0+\int_{t_0}^t f(s,y(s))\,ds$; a numerical solver approximates this evolution.
- **Forward sensitivity:** $Jv$; **reverse sensitivity:** $J^T\lambda$.
- **Least squares:** $E=\tfrac12\|r\|^2$, $\nabla E=J^Tr$, $H_E=J^TJ+\sum_i r_iH_{r_i}$.
- **Gauss–Newton:** $J^TJp=-J^Tr$; **LM:** $(J^TJ+\lambda D)p=-J^Tr$.
- **Constrained stationarity:** use KKT/projected-gradient conditions, not a raw zero-gradient test.
- **Always diagnose:** domains, units/scales, coordinate frames, conditioning, nonfinite values, line-search/solver status, and whether the derivative exists.

## Figure guide

1. **Local change ladder (horizontal):** secant slopes converge to derivative; derivative defines tangent prediction; multivariate derivative becomes $J\Delta x$; Hessian adds quadratic curvature. Annotate the remainder orders and assumptions.
2. **Camera differential frame map:** object-space perturbation → world → camera → perspective projection → pixel change, with multiplication order $J_\pi R A$ and a coordinate-convention callout. Include the ideal positive-forward projection Jacobian only as a declared convention.
3. **Integration comparison strip:** left/right rectangle, midpoint, trapezoid, Simpson over $x^2$ on $[0,2]$, showing the exact $8/3$ and the four-subinterval estimates.
4. **[[ODE initial-value problems and stiffness|ODE stability]] plane:** Euler amplification $R(z)=1+z$ disk, damped scalar eigenvalue on negative real axis, oscillator eigenvalues on imaginary axis; contrast explicit Euler growth and semi-implicit bounded regime.
5. **AD graph:** forward primal/tangent arrows and reverse adjoint arrows for $(xy+\sin x)^2$; shared $x$ paths merge by addition.
6. **Solver decision map:** objective/residual structure → gradient descent, Newton/BFGS, Gauss–Newton/LM; wrap each with scaling, globalization, constraints and explicit termination statuses.
7. **Optimization path figures:** narrow quadratic valley and projected-bound optimum. Show Euclidean steepest direction changes under parameter scaling and why active-bound gradients need not vanish.

The six figure assets have been rendered as PNG and SVG files and embedded in the staged volume above. Final vault link resolution remains for the parent integration pass.

## Topic route

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Functions, limits, continuity, and scalar change|Functions and local change]]
- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Jacobians, curvature, and local approximation|Jacobians and curvature]]
- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Integration and quadrature|Integration]]
- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#ODEs and numerical simulation|Simulation]]
- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Numerical differentiation and automatic differentiation|Automatic differentiation]]
- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Optimization and inverse problems|Optimization]]

## Six figures

![[Visual Recall/Math Foundations/F09-1_tangent-linearization.png]]

**F09-1. Tangent prediction and local residual.** At h=0.2, the exact residual for the shown exponential is about 0.03529 and the quadratic leading term about 0.03297; this illustrates local-order behavior.

![[Visual Recall/Math Foundations/F09-2_ftc-accumulation.png]]

**F09-2. Accumulated area and the derivative that recovers the integrand.** For A(x)=x cubed/3 at x=1.8, A=1.944 and A prime=3.24=x squared.

![[Visual Recall/Math Foundations/F09-3_quadrature-convergence.png]]

**F09-3. Quadrature error as sample intervals increase.** For exp(x) on [0,1] at N=16, left-rule error is about 0.053137, trapezoid error 5.593e-4, and Simpson error 1.456e-7; Simpson uses an even interval count.

![[Visual Recall/Math Foundations/F09-4_oscillator-energy.png]]

**F09-4. Energy behavior for the stated harmonic-oscillator step size.** At h=0.1 for 80 periods, explicit Euler energy reaches about 2.62e21, [[Time integrators and symplectic updates|semi-implicit Euler]] stays in [0.47619,0.526316], and RK4 ends near 0.499965. This is the plotted setup, not a universal ranking.

![[Visual Recall/Math Foundations/F09-5_damped-newton.png]]

**F09-5. Full Newton step versus Armijo backtracking for the plotted log-cosh example.** At x=3, the full step is about -100.8566; backtracking first accepts alpha=0.03125 in this setup.

![[Visual Recall/Math Foundations/F09-6_finite-difference-error.png]]

**F09-6. Central-difference truncation and roundoff for sin(x) at x=0.7.** The analytic derivative is cos(0.7), about 0.7648422; the best sampled step has error about 8.94e-13. Guide curves are qualitative.

## Phase 9 concept key map

Use each keyword for the concept note and the adjacent section link for the full derivation or example.

| Concept keyword | Full teaching section |
|---|---|
| [[Function contracts: domains, codomains, and images|domain and codomain]] | [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Functions, limits, continuity, and scalar change\|Phase 9 section]] |
| [[Limits and continuity|epsilon-delta limit]] | [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Functions, limits, continuity, and scalar change\|Phase 9 section]] |
| [[Derivative as a local linear map|differential]] | [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Functions, limits, continuity, and scalar change\|Phase 9 section]] |
| [[Differentiation rules and chain rule|product rule]] | [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Functions, limits, continuity, and scalar change\|Phase 9 section]] |
| [[Partial derivatives and gradients|gradient vector]] | [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Multivariable derivatives and sensitivity\|Phase 9 section]] |
| [[Directional and total derivatives|directional derivative]] | [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Multivariable derivatives and sensitivity\|Phase 9 section]] |
| [[Jacobians, pushforwards, and pullbacks|Jacobian matrix]] | [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Jacobians, curvature, and local approximation\|Phase 9 section]] |
| [[Hessians and local curvature|second derivative matrix]] | [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Jacobians, curvature, and local approximation\|Phase 9 section]] |
| [[Taylor models and approximation error|Taylor polynomial]] | [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Jacobians, curvature, and local approximation\|Phase 9 section]] |
| [[Finite differences and step-size balance|finite-difference derivative]] | [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Numerical differentiation and automatic differentiation\|Phase 9 section]] |
| [[Definite integrals and the fundamental theorem|Riemann integral]] | [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Integration and quadrature\|Phase 9 section]] |
| [[Numerical quadrature and error|integration rule]] | [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Integration and quadrature\|Phase 9 section]] |
| [[ODE initial-value problems and stiffness|initial-value problem]] | [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#ODEs and numerical simulation\|Phase 9 section]] |
| [[Time integrators and symplectic updates|symplectic integrator]] | [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#ODEs and numerical simulation\|Phase 9 section]] |
| [[Newton and quasi-Newton optimization|Newton method]] | [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Optimization and inverse problems\|Phase 9 section]] |
| [[Nonlinear least squares: Gauss-Newton and Levenberg-Marquardt|Gauss-Newton]] | [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Optimization and inverse problems\|Phase 9 section]] |
| [[Implicit differentiation and parameter sensitivities|implicit differentiation]] | [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Numerical differentiation and automatic differentiation\|Phase 9 section]] |

