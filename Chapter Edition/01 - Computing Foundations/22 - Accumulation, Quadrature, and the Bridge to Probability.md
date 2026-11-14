# 22 — Accumulation, Quadrature, and the Bridge to Probability

[[Chapter Edition/01 - Computing Foundations/00 - Reading Map|Reading map]] · [[Chapter Edition/01 - Computing Foundations/21 - Multivariable Derivatives and Sensitivity|Previous chapter]] · [[Chapter Edition/01 - Computing Foundations/23 - Probability Models and Measurement Error|Next chapter]]

Integration accumulates changing quantities and is not limited to geometric area; numerical quadrature approximates that accumulation from samples. The chapter carries the idea into numerical simulation and prepares the transition from deterministic models to uncertain measurements.

**By the end:** Relate integrals to accumulation, derive simple antiderivatives, and compare exact quantities with numerical approximations.

**Recall question:** What quantity is being accumulated in a chosen integral, and what assumptions make a quadrature rule accurate enough?

<!-- Derived from Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md; source lines 49358–52396; source-span SHA256 febb1514dfc9c7a3628c940baf2a680485acd99576522f7e25756967a0559c67. -->

<!-- CHAPTER-SOURCE-BEGIN -->
## Integration and quadrature
### Accumulation and numerical simulation

#### Integration means accumulating infinitesimal contributions

Suppose an object moves in one dimension with velocity:

$$
v(t)=3t^2
$$

where $t$ is measured in seconds and $v$ in metres per second.

What distance does it travel between $t=0$ and $t=2$?

Velocity tells us how much position changes per second. If velocity were constant, displacement would be:

$$
\Delta x=v\Delta t.
$$

But our velocity changes continuously. Using only the initial or final velocity for the entire interval would be inaccurate.

We could divide the interval into small pieces. During each sufficiently short interval, the velocity is approximately constant, and the displacement is approximately:

$$
\Delta x_i\approx v(t_i)\Delta t.
$$

Add all the displacements:

$$
\Delta x\approx\sum\_{i=0}^{N-1}v(t_i)\Delta t.
$$

If we make the intervals increasingly small, their accumulated contribution approaches the definite integral:

$$
\boxed{\Delta x=\int_0^2v(t)\\,dt}.
$$

For our velocity:

$$
\int_0^2 3t^2\\,dt = \left[t^3\right]\_0^2=8.
$$

The object travels 8 metres.

The expression $[t^3]\_0^2$ means evaluate $t^3$ at the upper boundary and subtract its value at the lower boundary.

This is the fundamental meaning of integration: it calculates the net accumulation of a continuously varying quantity.

##### Integration is not inherently an area calculation

We often introduce integration as the area beneath a curve. That is a useful geometric model, but it is not the most general interpretation.

An integral can represent accumulated displacement, work, mass, electric charge, probability, radiance, or energy. The physical meaning depends on the quantity being integrated and the variable of integration.

For instance, work done by a variable force along a one-dimensional path is:

$$
W=\int\_{x_0}^{x_1}F(x)\\,dx.
$$

If force is measured in newtons and position in metres, the integral has units of newton-metres, or joules.

The impulse delivered by a time-varying force is:

$$
\mathbf J=\int\_{t_0}^{t_1}\mathbf F(t)\\,dt,
$$

with units of newton-seconds, equal to the change in linear momentum.

In volume rendering, radiance contributions accumulate along a ray, modulated by absorption and transmittance. In a physically based renderer, illumination involves integrals over directions on a hemisphere.

The geometric interpretation differs across these applications, but the principle is identical: many local contributions combine into one total quantity.

##### Definite versus indefinite integration

A definite integral has specified boundaries:

$$
\int_a^b f(x)\\,dx.
$$

It produces a scalar when $f$ is scalar-valued and integrable, or a vector when the integrand is vector-valued and the integral is defined componentwise.

An indefinite integral denotes a family of antiderivatives:

$$
\int f(x)\\,dx=F(x)+C,
$$

where:

$$
F'(x)=f(x).
$$

The constant $C$ exists because differentiation eliminates additive constants.

For example:

$$
\int 6x\\,dx=3x^2+C.
$$

There is no single unique antiderivative unless an additional condition fixes $C$.

That distinction will reappear in differential equations: knowing a derivative law does not uniquely determine a trajectory until an initial condition is supplied.

##### Signed accumulation versus total distance

One subtlety deserves attention.

The integral of velocity gives displacement, not necessarily total distance traveled.

If an object moves forward and then backward, positive and negative velocity contributions cancel in displacement.

For a differentiable one-dimensional trajectory:

$$
x(b)-x(a)=\int_a^b v(t)\\,dt.
$$

Total distance is:

$$
\boxed{D=\int_a^b|v(t)|\\,dt}.
$$

For a three-dimensional path $\mathbf x(t)$, its arc length is:

$$
\boxed{ L=\int_a^b\|\mathbf x'(t)\|\\,dt }.
$$

The norm converts velocity into nonnegative speed. This distinction matters in animation path lengths, odometry, physical travel distances, and path-following systems.

#### Riemann sums: constructing an integral from finite computations

A computer cannot literally sum an infinite number of infinitesimal contributions. It can only perform a finite sequence of operations.

To understand numerical integration, we need to reconstruct the integral using finite partitions.

Suppose we want:

$$
I=\int_a^b f(x)\\,dx.
$$

Divide the interval into $N$ subintervals:

$$
a=x_0\<x_1<\cdots\<x_N=b.
$$

Define the width of the $i$-th interval as:

$$
\Delta x_i=x\_{i+1}-x_i.
$$

Choose a sampling point $\xi_i\in[x_i,x\_{i+1}]$.

Then the Riemann sum is:

$$
S_N=\sum\_{i=0}^{N-1}f(\xi_i)\Delta x_i.
$$

For a Riemann-integrable function, as the maximum subinterval width tends to zero, the sum approaches the integral:

$$
\boxed{ \int_a^b f(x)\\,dx = \lim\_{\|\mathcal P\|\to0} \sum\_{i=0}^{N-1}f(\xi_i)\Delta x_i }.
$$

Here $\mathcal P$ denotes the partition, and its mesh width is:

$$
\|\mathcal P\|=\max_i\Delta x_i.
$$

The limit is taken over partitions whose maximum interval width tends to zero. We do not require every interval to have the same width.

For a continuous function on a closed bounded interval, the [[Chapter Edition/01 - Computing Foundations/22 - Accumulation, Quadrature, and the Bridge to Probability#Integration and quadrature|Riemann integral]] exists.

[Calculus: Left, Right, and Midpoint Riemann Sums Infographic | LivePhysics™](https://images.openai.com/static-rsc-4/t5MkAz-2qFGici8ndHfZur9gQtfdWJYL32F6n8G88ZHPGmAsnDRse5qe5xfYJJ4GJcZqh8uYwSaYHOu9cPkLr8aB6FomkHL4VOpXoOryLK4Ee1f_NR5LPcOGaSAMPsLd58tQ6P4H7c9PoHrfy0mY3unWO2wadUnb798uxKmCL0Y?purpose=inline)

[livephysics.com](https://livephysics.com/infographics/calculus-left-right-and-midpoint-riemann-sums/?utm_source=chatgpt.com)

##### Uniform partitions

For a uniform partition:

$$
h=\frac{b-a}{N},
$$

and:

$$
x_i=a+ih.
$$

A left-endpoint Riemann sum is:

$$
L_N=h\sum\_{i=0}^{N-1}f(x_i).
$$

A right-endpoint sum is:

$$
R_N=h\sum\_{i=1}^{N}f(x_i).
$$

A midpoint sum is:

$$
M_N=h\sum\_{i=0}^{N-1}f\left(x_i+\frac h2\right).
$$

All three approximate the same integral, but their error differs because they choose different representatives of the function within each interval.

##### Worked trace: integrating $x^2$

Consider:

$$
I=\int_0^2x^2\\,dx=\frac83\approx2.6666667.
$$

Take four equal intervals:

$$
N=4,\qquad h=0.5.
$$

The partition points are $0,0.5,1,1.5,2$.

For the left rule:

$$
\begin{aligned} L_4 &=0.5[f(0)+f(0.5)+f(1)+f(1.5)]\\ &=0.5[0+0.25+1+2.25]\\ &=\boxed{1.75}. \end{aligned}
$$

For the right rule:

$$
\begin{aligned} R_4 &=0.5[0.25+1+2.25+4]\\ &=\boxed{3.75}. \end{aligned}
$$

The left rule underestimates and the right rule overestimates because $x^2$ increases throughout the interval.

For the midpoint rule, the sample points are:

$$
0.25,\quad0.75,\quad1.25,\quad1.75.
$$

Then:

$$
\begin{aligned} M_4 &=0.5[0.0625+0.5625+1.5625+3.0625]\\ &=\boxed{2.625}. \end{aligned}
$$

The midpoint result is significantly closer to the exact value.

This is not merely because it looks more visually balanced. For sufficiently smooth functions, the midpoint rule cancels leading first-order error terms across each symmetric interval.

We will derive that behavior shortly.

#### The fundamental theorem of calculus

The relationship between integration and differentiation is much deeper than saying that one is the reverse of the other.

The fundamental theorem establishes two related statements.

##### Part I: Differentiating an accumulation function

Define an accumulation function:

$$
A(x)=\int_a^x f(t)\\,dt.
$$

The upper boundary changes with $x$, so $A(x)$ represents everything accumulated from $a$ to the current location $x$.

If $f$ is continuous, the fundamental theorem states:

$$
\boxed{A'(x)=f(x)}.
$$

Why?

For a small increment $h$:

$$
A(x+h)-A(x)=\int_x^{x+h}f(t)\\,dt.
$$

Divide by $h$:

$$
\frac{A(x+h)-A(x)}h = \frac1h\int_x^{x+h}f(t)\\,dt.
$$

The right side is the average value of $f$ over the tiny interval.

As $h\to0$, continuity ensures that the average approaches $f(x)$. Thus:

$$
A'(x)=f(x).
$$

The derivative of accumulated quantity is the current accumulation rate.

The dummy variable $t$ names the points being accumulated; the upper endpoint $x$ is the input to the accumulation function. Keeping these roles distinct makes the Fundamental Theorem easy to read: if $f$ is continuous and $G(x)=\int_a^x f(t)\,dt$, then $G'(x)=f(x)$. This also constructs the solution of the initial-value problem $y'=f(x)$, $y(a)=0$. The supplied MIT 18.01SC Session 51 notes make this exact connection between accumulation and an ODE: [MIT 18.01SC, Session 51a notes](https://ocw.mit.edu/courses/18-01sc-single-variable-calculus-fall-2010/2459faee7ec3308afd5cca2b95fad48e_MIT18_01SCF10_Ses51a.pdf).



For a physical trajectory:

$$
x(t)=x(t_0)+\int\_{t_0}^{t}v(s)\\,ds,
$$

we recover:

$$
\frac{dx}{dt}=v(t).
$$

For velocity obtained by accumulating acceleration:

$$
v(t)=v(t_0)+\int\_{t_0}^{t}a(s)\\,ds,
$$

differentiating recovers acceleration.

##### Part II: Evaluating an integral using an antiderivative

If $F'(x)=f(x)$, and $f$ satisfies the usual integrability assumptions, then:

$$
\boxed{\int_a^b f(x)\\,dx=F(b)-F(a)}.
$$

This converts an accumulation problem into two evaluations of an antiderivative.

For our earlier example:

$$
f(x)=x^2,\qquad F(x)=\frac{x^3}{3}.
$$

Therefore:

$$
\int_0^2x^2\\,dx = \frac{2^3}{3}-0 =\frac83.
$$

##### When the theorem is insufficient computationally

The theorem is exact, but it does not imply that every integral can be evaluated by a convenient elementary formula.

For example:

$$
\int e^{-x^2}\\,dx
$$

does not have an antiderivative expressible using only elementary functions.

Other integrals involve data sampled from a simulation, a procedural shader, a measured sensor signal, or a computationally expensive black-box model. In these cases, even if the integral mathematically exists, a symbolic antiderivative may not be available or useful.

That is why numerical quadrature is a fundamental engineering technique rather than an inferior workaround for symbolic calculus.

![[Visual Recall/Math Foundations/F09-2_ftc-accumulation.png]]

**F09-2. Accumulated area and the derivative that recovers the integrand.** For A(x)=x cubed/3 at x=1.8, A=1.944 and A prime=3.24=x squared.

#### Rectangle rules: the first numerical quadrature methods

Quadrature is the numerical approximation of definite integrals.

Rectangle rules assume that the function can be approximated as constant within each interval.

##### Deriving the left rule from Taylor expansion

Within one interval $[x_i,x_i+h]$, write:

$$
f(x_i+s)=f(x_i)+f'(x_i)s+\frac12f''(x_i)s^2+\cdots.
$$

Integrate over $s\in[0,h]$:

$$
\int\_{x_i}^{x_i+h}f(x)\\,dx = hf(x_i)+\frac{h^2}{2}f'(x_i)+O(h^3)
$$

under sufficient smoothness assumptions.

The left rectangle uses only:

$$
I_i^{\mathrm{left}}=hf(x_i).
$$

Its leading local error is therefore:

$$
\frac{h^2}{2}f'(x_i).
$$

There are approximately $N=(b-a)/h$ intervals. Accumulating $N$ errors of order $h^2$ gives a global error of order:

$$
\boxed{O(h)}.
$$

This is why halving the step size tends to halve the error in a typical first-order rectangle approximation.

##### Why the midpoint method is more accurate

For midpoint integration, expand about the interval centre $m_i$:

$$
f(m_i+s)= f(m_i)+f'(m_i)s+\frac12f''(m_i)s^2+\cdots
$$

for $s\in[-h/2,h/2]$.

Integrating the linear term:

$$
\int\_{-h/2}^{h/2}f'(m_i)s\\,ds=0.
$$

The positive and negative contributions cancel.

Consequently:

$$
\int\_{x_i}^{x\_{i+1}}f(x)\\,dx = hf(m_i)+\frac{h^3}{24}f''(m_i)+O(h^5)
$$

for sufficiently smooth $f$.

The leading local error is order $h^3$, and the global composite midpoint error is order:

$$
\boxed{O(h^2)}.
$$

This is a recurring numerical-analysis strategy: choose a symmetric approximation to cancel leading error terms.

##### A caveat about error orders

An order statement such as $O(h^2)$ depends on regularity and the problem being approximated.

If the integrand has discontinuities, singularities, severe oscillations, or insufficient smoothness, the expected convergence rate may not apply.

Likewise, reducing $h$ can increase floating-point accumulation error. Mathematical truncation error and arithmetic rounding error must be treated separately.

![[Visual Recall/Math Foundations/F09-3_quadrature-convergence.png]]

**F09-3. Quadrature error as sample intervals increase.** For exp(x) on [0,1] at N=16, left-rule error is about 0.053137, trapezoid error 5.593e-4, and Simpson error 1.456e-7; Simpson uses an even interval count.

#### Trapezoidal integration

A rectangle approximates the integrand as constant over each interval. The trapezoidal rule approximates it using the straight line connecting the endpoint values.

On $[a,b]$, let:

$$
h=b-a.
$$

The linear interpolant is:

$$
p(x)= f(a)+ \frac{f(b)-f(a)}h(x-a).
$$

Integrate:

$$
\begin{aligned} \int_a^b p(x)\\,dx &=hf(a)+ \frac{f(b)-f(a)}h\frac{h^2}{2}\\ &=\frac h2[f(a)+f(b)]. \end{aligned}
$$

Therefore the single-interval trapezoidal rule is:

$$
\boxed{ T=\frac{b-a}{2}[f(a)+f(b)] }.
$$

For a uniform partition, adding the trapezoids gives:

$$
\boxed{ T_N= h\left[ \frac{f(a)}2+ \sum\_{i=1}^{N-1}f(x_i)+ \frac{f(b)}2 \right] }.
$$

Interior sample points have weight one because each is shared by neighboring trapezoids. The two boundary points appear only once, so they have weight one-half.

For $f(x)=x^2$, over $[0,2]$ with $N=4$:

$$
\begin{aligned} T_4 &=0.5[ 0+0.25+1+2.25+2 ]\\ &=\boxed{2.75}. \end{aligned}
$$

The exact value is $2.6666667$.

##### Error and exactness

For a twice continuously differentiable function, the global error of the composite trapezoidal rule is bounded by:

$$
\boxed{ |I-T_N| \le \frac{(b-a)h^2}{12} \max\_{x\in[a,b]}|f''(x)| }.
$$

Thus, trapezoidal integration is globally second-order accurate under these assumptions.

If $f$ is affine, its second derivative is zero. The trapezoidal approximation is then exact regardless of the interval width.

This has an important interpretation: a quadrature rule is exact for every function belonging to the polynomial family it can represent perfectly.

#### Simpson's rule: integrating a local quadratic approximation

The trapezoidal rule uses a degree-one polynomial. Simpson's rule uses a degree-two interpolating polynomial over two adjacent subintervals.

Let three equally spaced nodes be:

$$
x_0=a,\quad x_1=a+h,\quad x_2=a+2h.
$$

Use the quadratic polynomial passing through the three sampled function values. Integrating that interpolating polynomial over $[a,a+2h]$ produces:

$$
\boxed{ S=\frac h3[f(x_0)+4f(x_1)+f(x_2)] }.
$$

The weights $1,4,1$ are not arbitrary. They arise from integrating the Lagrange polynomial basis.

##### Deriving the weights explicitly

Introduce a normalized coordinate:

$$
u=\frac{x-x_0}{h},
$$

so the sample nodes are $u=0,1,2$.

The corresponding Lagrange basis polynomials are:

$$
L_0(u)=\frac{(u-1)(u-2)}2,
$$

$$
L_1(u)=-u(u-2),
$$

$$
L_2(u)=\frac{u(u-1)}2.
$$

The quadratic interpolant is:

$$
p(u)=f(x_0)L_0(u)+f(x_1)L_1(u)+f(x_2)L_2(u).
$$

Because $dx=h\\,du$:

$$
\int\_{x_0}^{x_2}p(x)\\,dx = h\int_0^2p(u)\\,du.
$$

Evaluate each basis integral:

$$
\int_0^2L_0(u)\\,du=\frac13,
$$

$$
\int_0^2L_1(u)\\,du=\frac43,
$$

$$
\int_0^2L_2(u)\\,du=\frac13.
$$

Therefore:

$$
S=\frac h3[f(x_0)+4f(x_1)+f(x_2)].
$$

##### Composite Simpson's rule

For an even number $N$ of uniformly spaced intervals:

$$
\boxed{ S_N= \frac h3 \left[ f_0+f_N+ 4\sum\_{\substack{i=1\\\i\text{ odd}}}^{N-1}f_i+ 2\sum\_{\substack{i=2\\\i\text{ even}}}^{N-2}f_i \right] }.
$$

The number of subintervals must be even for the standard composite Simpson's $1/3$ rule because each quadratic interpolant spans two intervals.

For $x^2$ on $[0,2]$, taking $N=4$ gives:

$$
\begin{aligned} S_4 &=\frac{0.5}{3} [0+4(0.25)+2(1)+4(2.25)+4]\\ &=\frac{16}{6}\\ &=\boxed{\frac83}. \end{aligned}
$$

It is exact.

An interesting result is that Simpson's rule is exact for polynomials through degree three, even though the interpolation polynomial has degree two. Symmetry cancels the error contribution that would otherwise affect cubic terms.

Under sufficient smoothness:

$$
\boxed{ |I-S_N| \le \frac{(b-a)h^4}{180} \max\_{x\in[a,b]}|f^{(4)}(x)| }.
$$

Simpson's rule is therefore globally fourth-order accurate, compared with second order for midpoint and trapezoidal rules and first order for left/right rectangle rules.

##### Compare all five methods on the same problem

| Method          | Estimate | Absolute error | Global order |
| --------------- | -------- | -------------- | ------------ |
| Left rectangle  | 1.750000 | 0.916667       | $O(h)$   |
| Right rectangle | 3.750000 | 1.083333       | $O(h)$   |
| Midpoint        | 2.625000 | 0.041667       | $O(h^2)$ |
| Trapezoidal     | 2.750000 | 0.083333       | $O(h^2)$ |
| Simpson         | 2.666667 | 0              | $O(h^4)$ |

All estimates use four equal subintervals for $\int_0^2x^2\\,dx$. Orders assume sufficient smoothness and a fixed interval as $h\to0$.

Higher-order methods are not automatically better for every workload. Simpson's rule benefits from smoothness and regularly structured sampling. For discontinuous or noisy measurements, the extra polynomial approximation may have little advantage, and irregularly spaced data requires a modified formulation.

#### Ordinary differential equations: representing laws of continuous evolution

We now move from integrating a known function to a more difficult problem: the rate of change may itself depend on the state whose evolution we want to calculate.

Suppose a mass is attached to a spring.

If displaced from its equilibrium position, the spring produces a restoring force. Hooke's law is:

$$
F=-kx,
$$

where $x$ is displacement from equilibrium in metres, $k$ is spring stiffness in newtons per metre, and the minus sign means the force points opposite the displacement.

Newton's second law gives:

$$
F=ma=m\frac{d^2x}{dt^2}.
$$

Combining them:

$$
\boxed{m\frac{d^2x}{dt^2}=-kx}.
$$

This is an ordinary differential equation, abbreviated ODE.

It does not directly state the object's future position. Instead, it constrains how its position must evolve over time.

##### What makes an equation differential?

A differential equation relates an unknown function to one or more of its derivatives.

For example:

$$
\frac{dy}{dt}=-2y
$$

says the instantaneous rate of change of $y$ equals negative twice its current value.

If $y$ is large and positive, it decreases rapidly. As it approaches zero, its rate of decrease becomes smaller.

The function:

$$
y(t)=Ce^{-2t}
$$

satisfies this equation because:

$$
\frac{dy}{dt}=-2Ce^{-2t}=-2y.
$$

The constant $C$ is not determined by the differential equation alone.

Other examples include constant acceleration:

$$
\frac{d^2x}{dt^2}=g,
$$

linear damping:

$$
\frac{dv}{dt}=-\lambda v,
$$

and the damped spring:

$$
m\frac{d^2x}{dt^2}+c\frac{dx}{dt}+kx=0.
$$

Here $c$ is a damping coefficient in $\mathrm{kg/s}$.

##### Order, linearity, autonomy, and dimensions

The order of an ODE is the highest derivative appearing in it. The spring equation uses a second derivative, so it is second-order.

An ODE is linear in the unknown function when that function and its derivatives appear linearly, with coefficients that may depend on the independent variable. Thus:

$$
y''+a(t)y'+b(t)y=g(t)
$$

is a linear ODE, even when $a,b,g$ vary with time.

By contrast:

$$
y'=-y^3
$$

is nonlinear.

An ODE is autonomous if its derivative law has no explicit dependence on the independent variable:

$$
\dot y=f(y).
$$

It is nonautonomous if time appears explicitly:

$$
\dot y=f(t,y).
$$

This distinction matters for integrator APIs. A solver receiving only `derivative(y)` cannot directly represent every time-dependent forcing law. A general-purpose solver should accept `derivative(t, y)`.

##### Converting higher-order equations to first-order systems

Most general ODE solvers work with a first-order vector equation:

$$
\boxed{ \frac{d\mathbf y}{dt}=f(t,\mathbf y) }.
$$

How can a second-order spring equation fit this form?

Introduce velocity as an additional state variable:

$$
v=\frac{dx}{dt}.
$$

Then:

$$
\frac{dx}{dt}=v,
$$

$$
\frac{dv}{dt}=-\frac{k}{m}x.
$$

Combine position and velocity into one state vector:

$$
\mathbf y= \begin{bmatrix} x\\v \end{bmatrix}.
$$

Its derivative is:

$$
\boxed{ \frac{d\mathbf y}{dt} = \begin{bmatrix} v\\ -\frac{k}{m}x \end{bmatrix} }.
$$

For a damped spring receiving an external force $F\_{\mathrm{ext}}(t)$:

$$
\boxed{ \frac{d}{dt} \begin{bmatrix} x\\v \end{bmatrix} = \begin{bmatrix} v\\ \frac{F\_{\mathrm{ext}}(t)-cv-kx}{m} \end{bmatrix} }.
$$

This representation is fundamental to simulation architecture.

A rigid body's state can contain position, orientation, linear momentum, and angular momentum. A soft body's state can contain many particle positions and velocities. The governing equations determine the rate of change of those state variables.

The ODE solver advances the complete state through time.

##### State-space matrix representation

For an undamped spring:

$$
\dot{\mathbf y}=A\mathbf y,
$$

where:

$$
\boxed{ A= \begin{bmatrix} 0&1\\ -\omega^2&0 \end{bmatrix}, \qquad \omega=\sqrt{\frac{k}{m}} }.
$$

Here $\omega$ is the natural angular frequency in radians per second.

The eigenvalues satisfy:

$$
\det(A-\lambda I) = \lambda^2+\omega^2=0.
$$

Therefore:

$$
\lambda=\pm i\omega.
$$

These imaginary eigenvalues describe oscillatory behavior rather than exponential growth or decay.

This connects the ODE to Phase 8's eigenanalysis. Soon we will use the same eigenvalues to determine whether a numerical integration method is stable.

#### Initial-value problems: why a law does not determine a unique history

A differential equation tells us the permitted evolution of a system. We also need a starting state.

An initial-value problem, or IVP, consists of:

$$
\boxed{ \dot{\mathbf y}=f(t,\mathbf y), \qquad \mathbf y(t_0)=\mathbf y_0 }.
$$

The initial time is $t_0$, the initial state is $\mathbf y_0$, and the objective is to find a function $\mathbf y(t)$ satisfying the differential equation and the initial condition.

For:

$$
y'=-2y,
$$

the general solution is:

$$
y(t)=Ce^{-2t}.
$$

If:

$$
y(0)=5,
$$

then $C=5$, giving:

$$
\boxed{y(t)=5e^{-2t}}.
$$

A different initial state produces a different trajectory even though the governing differential equation remains unchanged.

For the harmonic oscillator, we generally need both an initial position and an initial velocity because the original equation is second-order.

##### Existence and uniqueness are separate questions

Does every differential equation have a solution? Is that solution unique?

Not necessarily.

A standard sufficient local condition is that $f(t,\mathbf y)$ is continuous in time and locally Lipschitz-continuous in the state. Roughly speaking, the Lipschitz condition bounds how rapidly the derivative law can change with state:

$$
\|f(t,\mathbf x)-f(t,\mathbf y)\| \le L\|\mathbf x-\mathbf y\|
$$

in a suitable neighborhood.

The constant $L\ge0$ is a Lipschitz bound.

Under the standard hypotheses of the Picard–Lindelöf theorem, the IVP has a unique local solution.

But continuity alone does not ensure uniqueness.

Consider:

$$
y'=\sqrt{|y|}, \qquad y(0)=0.
$$

The constant function $y(t)=0$ satisfies the IVP.

Yet we can also construct solutions that remain at zero until an arbitrary time $a\ge0$, then follow:

$$
y(t)=\frac{(t-a)^2}{4}, \qquad t\ge a.
$$

All these trajectories satisfy the same derivative law and initial condition for $t\ge0$.

The derivative law fails the relevant Lipschitz condition near zero.

This issue has practical analogues in discontinuous contact laws, friction models, nonsmooth optimization dynamics, and systems with switching behavior. A simulation algorithm cannot guarantee physical or mathematical uniqueness when the model itself does not provide it.

##### Integral form of an ODE

From the fundamental theorem of calculus:

$$
\boxed{ \mathbf y(t) = \mathbf y_0+ \int\_{t_0}^{t}f(s,\mathbf y(s))\\,ds }.
$$

This is a particularly powerful expression.

It says that future state equals initial state plus all the instantaneous changes accumulated along the trajectory.

But there is a complication: the integrand depends on $\mathbf y(s)$, the trajectory we are trying to find.

That is precisely why solving an ODE is more difficult than integrating a fully known function.

A numerical integrator must approximate both the evolving trajectory and the accumulated derivative.

For continuous forcing $f$, define
$$
G(x)=\int_a^x f(t)\,dt.
$$
The fundamental theorem gives $G'(x)=f(x)$ and $G(a)=0$, so $G$ solves $y'=f(x)$ with $y(a)=0$. For initial value $y(a)=y_0$,
$$
y(x)=y_0+\int_a^x f(t)\,dt.
$$
Here $t$ is a dummy integration variable and $x$ is the upper-limit input. This is the specific continuous-forcing construction in the brief official MIT 18.01SC Session 51 PDF; it does not establish existence or uniqueness for arbitrary nonlinear ODEs. [MIT 18.01SC Session 51 PDF](https://ocw.mit.edu/courses/18-01sc-single-variable-calculus-fall-2010/2459faee7ec3308afd5cca2b95fad48e_MIT18_01SCF10_Ses51a.pdf)

**Concept link:** [[Chapter Edition/01 - Computing Foundations/22 - Accumulation, Quadrature, and the Bridge to Probability#Integration and quadrature|integration rule]] — Quadrature approximates a definite integral from finitely many function….



<!-- end V09.04 -->


## Derivative, finite difference, and numerical error are different ideas

A mathematical derivative is defined by a limit. A numerical derivative estimates it from finite precision evaluations. The forward-difference estimate is

$$
f'(x)\approx\frac{f(x+h)-f(x)}{h}.
$$

A smaller $h$ reduces truncation error for a smooth function, but subtraction can cancel most significant digits when $f(x+h)$ and $f(x)$ are nearly equal. Dividing the remaining roundoff by a tiny $h$ magnifies that noise. A centered difference,

$$
f'(x)\approx\frac{f(x+h)-f(x-h)}{2h},
$$

has smaller leading truncation error under suitable smoothness, yet still faces roundoff. There is no rule that “smaller step is always better”; step size balances approximation and floating-point error and depends on scale.

Automatic differentiation (AD) uses the chain rule on the operations of a program. It can compute derivatives accurate to floating-point rounding for the executed computation, but it does not prove that the program expresses the intended mathematics, that the model is smooth, or that the output is well-conditioned. A branch, clamp, discontinuity, or unstable operation still has its mathematical consequences. Finite differences can independently check an AD implementation at selected points, but they are themselves approximate and can fail near nonsmooth points or under poor step choices.

Conditioning describes sensitivity of the mathematical problem to perturbed inputs. Stability describes whether the algorithm introduces errors beyond what the problem's sensitivity would cause. Roundoff is the finite-precision arithmetic error. Truncation or discretization error comes from replacing a continuous or infinite process with a finite approximation. These categories should not be collapsed into one “numerical error.” For a computed result $\hat y$ from input $x$, a perturbation may be amplified approximately according to the derivative: $\Delta y\approx Df(x)\Delta x$. If the derivative has a large operator norm in the chosen units and metric, small input errors can produce large output changes. A stable algorithm cannot make an ill-conditioned question insensitive; it can avoid adding unnecessary error.

An iterative solver's residual illustrates the distinction. If $A\hat x=b-r$, a small residual $r$ means $\hat x$ nearly solves a nearby right-hand side. It does not ensure $\hat x$ is near the exact solution when $A$ has a tiny singular value. Conversely, backward stability can mean the algorithm returns the exact solution to a slightly perturbed problem, while forward error may still be large because the original problem is sensitive. Always state scale-aware residuals, condition estimates, and the units or norms used.

<!-- begin V09.06 -->
## Numerical differentiation and automatic differentiation
### Numerical gradient checking

Suppose an objective depends on $n$ parameters:

$$
L:\mathbb R^n\rightarrow\mathbb R.
$$

An analytical or automatic differentiation implementation returns a gradient:

$$
\mathbf g= \nabla L(\mathbf x).
$$

How do we know that gradient is correct?

We can independently estimate each component using central finite differences:

$$
\boxed{ g_i^{\mathrm{numeric}} \approx \frac{ L(\mathbf x+h_i\mathbf e_i)- L(\mathbf x-h_i\mathbf e_i) }{2h_i} }.
$$

Here $\mathbf e_i$ is the $i$-th standard basis vector.

This produces an approximate numerical gradient.

We then compare it with the analytical or automatic gradient.

##### Why checking an entire gradient can be expensive

A full central-difference gradient requires approximately $2n$ objective evaluations.

For an optimizer with 10 parameters, this may be manageable.

For a neural network with millions of parameters, it is generally impractical.

For a renderer, each objective evaluation might require generating an entire image.

Therefore, gradient validation often uses a subset of variables or randomized directional checks.

##### Directional gradient checking

Choose a direction:

$$
\mathbf d\in\mathbb R^n.
$$

The chain rule predicts:

$$
\frac{d}{dh}L(\mathbf x+h\mathbf d)\bigg|\_{h=0} = \nabla L(\mathbf x)^T\mathbf d.
$$

Estimate the same quantity numerically:

$$
\boxed{ D_h= \frac{ L(\mathbf x+h\mathbf d)- L(\mathbf x-h\mathbf d) }{2h} }.
$$

Compare:

$$
D_h \quad\text{against}\quad \mathbf g^T\mathbf d.
$$

This requires only two objective evaluations regardless of parameter count.

It verifies the gradient's action along one direction, rather than individually checking every component.

Randomized directions provide broad coverage. However, a finite collection of checks cannot prove an arbitrary high-dimensional gradient is correct.

##### How should disagreement be measured?

Absolute error is:

$$
E\_{\mathrm{abs}}= |D_h-\mathbf g^T\mathbf d|.
$$

A scaled comparison can use:

$$
\boxed{ E\_{\mathrm{scaled}} = \frac{ |D_h-\mathbf g^T\mathbf d| }{ \max(s,\ |D_h|,\ |\mathbf g^T\mathbf d|) } }
$$

where $s>0$ is a scale chosen for the problem.

Do not choose the scale without considering units and expected magnitudes.

Near a true derivative of zero, a purely relative comparison can be misleading because its denominator becomes tiny.

Gradient checking should also use multiple step sizes. A wrong gradient may accidentally agree at one step size because truncation and implementation errors cancel.

##### Gradient-checking failure cases

A [[Chapter Edition/01 - Computing Foundations/22 - Accumulation, Quadrature, and the Bridge to Probability#Numerical differentiation and automatic differentiation|gradient check]] may fail even if the differentiation implementation is internally consistent.

If a perturbation crosses a visibility boundary, clipping threshold, collision event, `max` branch, or discontinuous LOD switch, a central difference samples different computational regimes on opposite sides of the point.

The classical derivative may not exist there.

If an objective includes randomness, evaluations at nearby parameters must be compared under controlled random streams or appropriate statistical methodology. Otherwise sampling noise can overwhelm the finite-difference signal.

For a differentiable renderer, it is often useful to first validate gradients on a smooth, fixed-visibility reference scene before introducing discontinuous visibility or stochastic sampling.

### Automatic differentiation: differentiating the program's operations

Automatic differentiation is neither symbolic algebra in the usual sense nor numerical finite differentiation.

It evaluates derivatives by applying the chain rule to the elementary operations executed by a program.

Consider:

$$
L(x,y)=\left(xy+\sin x\right)^2.
$$

Instead of manipulating this expression symbolically as one object, decompose its computation into elementary steps:

$$
a=xy,
$$

$$
b=\sin x,
$$

$$
c=a+b,
$$

$$
L=c^2.
$$

Each operation has a known local derivative.

Multiplication has derivative contributions from both inputs.

Sine contributes cosine.

Addition passes sensitivities unchanged.

Squaring contributes $2c$.

Automatic differentiation combines these local rules along the actual computation.

##### Differentiation as composition of linear maps

Suppose:

$$
F=F_k\circ F\_{k-1}\circ\cdots\circ F_1.
$$

The Jacobian chain rule gives:

$$
J_F = J\_{F_k}\cdots J\_{F_2}J\_{F_1}.
$$

There are two major strategies for computing the resulting derivative.

We can propagate an input perturbation forward through each local Jacobian.

Or we can propagate an output sensitivity backward through each transposed local Jacobian.

These are forward-mode and reverse-mode automatic differentiation.

##### Automatic differentiation and numerical precision

AD avoids the artificial perturbation step used by finite differences. It does not need to subtract nearly identical function values to estimate a derivative.

Under smooth operations, it computes derivatives of the executed computational composition using local derivative rules, subject to the floating-point behavior of those operations.

But it is not mathematically infallible.

Overflow, underflow, unstable formulas, branch boundaries, discontinuities, mutable state, inaccurate derivative rules for custom operations, and nondifferentiable primitives can still cause problems.

AD differentiates the mathematical operations represented by the program's differentiation rules. It does not automatically correct an inaccurate physical model or a numerically unstable underlying algorithm.

### Forward-mode automatic differentiation

Forward-mode AD answers:

> If the inputs change in a specified direction, how does every intermediate result change?

Each intermediate value carries two pieces of information.

The first is its ordinary value, often called its primal.

The second is its derivative in the selected input direction, called its tangent.

For one scalar variable:

$$
x\longrightarrow(x,\dot x).
$$

The dot here represents differentiation with respect to a chosen abstract perturbation parameter, not necessarily physical time.

For a function:

$$
z=f(x,y),
$$

the tangent is:

$$
\dot z=f_x\dot x+f_y\dot y.
$$

Choosing:

$$
\dot x=1,\qquad \dot y=0
$$

computes sensitivity with respect to $x$.

Choosing:

$$
\dot x=0,\qquad \dot y=1
$$

computes sensitivity with respect to $y$.

Choosing an arbitrary vector of input tangents computes a directional derivative.

#### Dual numbers: the mathematical foundation

A useful way to implement forward-mode AD is with dual numbers.

Introduce a special algebraic quantity $\varepsilon$ satisfying:

$$
\boxed{\varepsilon^2=0}
$$

while $\varepsilon\ne0$.

This is not an ordinary real number. It is a formal nilpotent element of the dual-number algebra.

Represent:

$$
a+b\varepsilon.
$$

The coefficient $a$ stores the primal value and $b$ stores its tangent.

##### Deriving dual-number addition

Let:

$$
u=a+b\varepsilon, \qquad v=c+d\varepsilon.
$$

Then:

$$
u+v=(a+c)+(b+d)\varepsilon.
$$

Therefore:

$$
\boxed{ (a,b)+(c,d)=(a+c,b+d) }.
$$

This is exactly the derivative rule for addition.

##### Deriving multiplication

Multiply:

$$
\begin{aligned} uv &=(a+b\varepsilon)(c+d\varepsilon)\\ &=ac+(ad+bc)\varepsilon+bd\varepsilon^2. \end{aligned}
$$

Because $\varepsilon^2=0$:

$$
\boxed{ uv=ac+(ad+bc)\varepsilon }.
$$

The tangent is:

$$
ad+bc.
$$

That is the product rule.

Notice that the derivative arises automatically from the algebra of the representation.

##### Deriving division

For:

$$
u=a+b\varepsilon,\qquad v=c+d\varepsilon,
$$

with $c\ne0$, seek a dual number $r+s\varepsilon$ satisfying:

$$
(c+d\varepsilon)(r+s\varepsilon) = a+b\varepsilon.
$$

Matching primal terms:

$$
cr=a \quad\Longrightarrow\quad r=\frac ac.
$$

Matching tangent terms:

$$
cs+dr=b.
$$

Therefore:

$$
s=\frac{b-dr}{c} =\frac{bc-ad}{c^2}.
$$

Thus:

$$
\boxed{ \frac{a+b\varepsilon}{c+d\varepsilon} = \frac ac+ \frac{bc-ad}{c^2}\varepsilon }.
$$

This is the quotient rule.

##### Elementary functions

For a sufficiently smooth scalar function:

$$
f(a+b\varepsilon) = f(a)+f'(a)b\varepsilon,
$$

because higher-order terms contain $\varepsilon^2$ and vanish.

Therefore:

$$
\boxed{ \sin(a+b\varepsilon) = \sin a+b\cos a\\,\varepsilon }
$$

and:

$$
\boxed{ e^{a+b\varepsilon} = e^a+be^a\varepsilon }.
$$

The same pattern applies to logarithms, square roots, and other differentiable primitives, subject to their domains.

#### Fully documented C++23 dual-number implementation

This is a compact but genuine forward-mode AD implementation.

```cpp
#include <cmath>
#include <stdexcept>

// A dual number represents a primal scalar value plus a tangent:
//      primal + tangent * epsilon
// where epsilon^2 = 0.
//
// This implementation propagates ONE directional derivative.
// For a function of n inputs, selecting a basis seed computes
// one column of the Jacobian at a time.
struct Dual {
    double primal{};
    double tangent{};
};

// Addition: (a + bε) + (c + dε)
Dual operator+(Dual a, Dual b) {
    return {
        a.primal + b.primal,
        a.tangent + b.tangent
    };
}

// Subtraction: derivative is the difference of tangents.
Dual operator-(Dual a, Dual b) {
    return {
        a.primal - b.primal,
        a.tangent - b.tangent
    };
}

// Product rule:
// d(ab) = (da)b + a(db)
Dual operator*(Dual a, Dual b) {
    return {
        a.primal * b.primal,
        a.tangent * b.primal +
        a.primal * b.tangent
    };
}

// Quotient rule, requiring a nonzero primal denominator.
Dual operator/(Dual a, Dual b) {
    if (b.primal == 0.0)
        throw std::domain_error("Division by zero");

    return {
        a.primal / b.primal,

        (a.tangent * b.primal -
         a.primal * b.tangent) /
        (b.primal * b.primal)
    };
}

// Chain rule for sine:
// d sin(x) = cos(x) dx
Dual sin(Dual x) {
    return {
        std::sin(x.primal),
        std::cos(x.primal) * x.tangent
    };
}

// Chain rule for exponential:
// d exp(x) = exp(x) dx
Dual exp(Dual x) {
    const double value = std::exp(x.primal);

    return {
        value,
        value * x.tangent
    };
}
```

##### Worked computational trace

Consider:

$$
L(x,y)=(xy+\sin x)^2.
$$

Evaluate it at:

$$
x=2,\qquad y=3.
$$

We want:

$$
\frac{\partial L}{\partial x}.
$$

Seed the dual inputs:

$$
x=(2,1),\qquad y=(3,0).
$$

The first tuple element is the primal, and the second is the tangent.

Multiplication:

$$
a=xy=(6,3).
$$

The primal is $6$. Its tangent is:

$$
1(3)+2(0)=3.
$$

Next:

$$
b=\sin x = (\sin2,\cos2).
$$

Approximately:

$$
b=(0.909297427,-0.416146837).
$$

Add:

$$
c=a+b.
$$

Its primal is:

$$
c_p=6.909297427.
$$

Its tangent is:

$$
c_d=3-0.416146837 =2.583853163.
$$

Finally:

$$
L=c^2.
$$

The primal is:

$$
L_p\approx47.738390932.
$$

The tangent is:

$$
L_d=2c_pc_d.
$$

Numerically:

$$
\boxed{ \frac{\partial L}{\partial x} \approx35.705220027 }.
$$

To compute the derivative with respect to $y$, use a different seed:

$$
x=(2,0),\qquad y=(3,1).
$$

Then:

$$
\boxed{ \frac{\partial L}{\partial y} \approx27.637189707 }.
$$

The complete gradient is:

$$
\boxed{ \nabla L(2,3) \approx \begin{bmatrix} 35.705220027\\ 27.637189707 \end{bmatrix} }.
$$

##### Generalization: Jacobian-vector products

For:

$$
F:\mathbb R^n\rightarrow\mathbb R^m,
$$

forward-mode differentiation propagates:

$$
\boxed{ J_F(\mathbf x)\mathbf v }
$$

for a chosen tangent direction $\mathbf v\in\mathbb R^n$.

This operation is called a Jacobian-vector product, or JVP.

You do not need to explicitly construct the entire Jacobian matrix to calculate it.

This matters in graphics, physics, and numerical optimization. A Jacobian may be enormous but sparse or only needed through its action on a vector.

If you want the full Jacobian, you can evaluate forward mode with each basis vector $\mathbf e_i$. Each evaluation produces the corresponding Jacobian column.

For a function with relatively few inputs and many outputs, forward mode is often attractive.

For a function with millions of inputs and one scalar output, computing the full gradient one input at a time becomes inefficient.

That motivates reverse mode.

The [JAX documentation on JVPs and VJPs](https://docs.jax.dev/en/latest/jacobian-vector-products.html?utm_source=chatgpt.com) develops the same linear-map interpretation and explains the practical trade-off between forward and reverse differentiation.&#x20;

[image](https://www.google.com/s2/favicons?domain=https://docs.jax.dev\&sz=32)

JAX documentation



### Reverse-mode automatic differentiation

Forward mode propagates input perturbations toward the outputs.

Reverse mode propagates output sensitivities back toward the inputs.

This direction is especially useful for optimization, where an enormous parameter vector produces one scalar loss.

Suppose:

$$
L:\mathbb R^n\rightarrow\mathbb R.
$$

We want:

$$
\nabla L.
$$

Reverse mode can obtain the entire gradient in one reverse traversal of a suitable computational graph, with arithmetic cost often proportional to the original computation, although actual memory and execution costs depend on the implementation.

#### Deriving reverse accumulation

Let:

$$
z=f(x,y).
$$

Suppose the final scalar loss $L$ depends on $z$.

By the chain rule:

$$
\frac{\partial L}{\partial x} = \frac{\partial L}{\partial z} \frac{\partial z}{\partial x},
$$

$$
\frac{\partial L}{\partial y} = \frac{\partial L}{\partial z} \frac{\partial z}{\partial y}.
$$

Define the adjoint:

$$
\bar z=\frac{\partial L}{\partial z}.
$$

Then:

$$
\boxed{ \bar x\mathrel{+}= \bar z\frac{\partial z}{\partial x} }
$$

and:

$$
\boxed{ \bar y\mathrel{+}= \bar z\frac{\partial z}{\partial y} }.
$$

The `+=` notation matters.

A variable may influence the output through several downstream paths, and the chain rule requires summing contributions from every path.

This is the reason reverse AD systems accumulate gradients rather than overwrite them.

##### Local reverse rules

For addition:

$$
z=x+y,
$$

the local derivatives are both one:

$$
\bar x\mathrel{+}=\bar z, \qquad \bar y\mathrel{+}=\bar z.
$$

For multiplication:

$$
z=xy,
$$

the local derivatives are:

$$
\frac{\partial z}{\partial x}=y, \qquad \frac{\partial z}{\partial y}=x.
$$

Thus:

$$
\boxed{ \bar x\mathrel{+}=\bar z\\,y, \qquad \bar y\mathrel{+}=\bar z\\,x }.
$$

For sine:

$$
z=\sin x,
$$

we propagate:

$$
\boxed{ \bar x\mathrel{+}=\bar z\cos x }.
$$

For a squared scalar:

$$
z=x^2,
$$

we propagate:

$$
\boxed{ \bar x\mathrel{+}=2x\bar z }.
$$

These local operations are sufficient to differentiate a complex program composed of supported primitives.

#### Reverse-mode example: manually backpropagating the complete graph

Use the same function:

$$
L(x,y)=(xy+\sin x)^2.
$$

Introduce the intermediate variables:

$$
a=xy,\qquad b=\sin x,\qquad c=a+b,\qquad L=c^2.
$$

The forward computation at $(2,3)$ is:

$$
a=6,
$$

$$
b\approx0.909297427,
$$

$$
c\approx6.909297427,
$$

$$
L\approx47.738390932.
$$

The computation graph is:

##### Start at the scalar loss

We initialize:

$$
\boxed{\bar L=1}
$$

because:

$$
\frac{\partial L}{\partial L}=1.
$$

From:

$$
L=c^2,
$$

we obtain:

$$
\bar c=\bar L(2c).
$$

Thus:

$$
\boxed{\bar c\approx13.818594853}.
$$

##### Backpropagate through addition

Because:

$$
c=a+b,
$$

the local derivatives are both one.

Therefore:

$$
\boxed{\bar a=13.818594853},
$$

$$
\boxed{\bar b=13.818594853}.
$$

##### Backpropagate through multiplication

The first branch is:

$$
a=xy.
$$

Thus:

$$
\bar x\_{\text{product}} = \bar a\\,y = 13.818594853(3).
$$

So:

$$
\bar x\_{\text{product}} \approx41.455784560.
$$

The gradient contribution to $y$ is:

$$
\bar y=\bar a\\,x.
$$

Therefore:

$$
\boxed{\bar y\approx27.637189707}.
$$

##### Backpropagate through sine

The other branch is:

$$
b=\sin x.
$$

Its contribution to $x$ is:

$$
\bar x\_{\text{sine}} = \bar b\cos x.
$$

At $x=2$, $\cos2\approx-0.416146837$, producing a negative contribution of approximately $-5.750565$.

##### Accumulate the two paths

The variable $x$ influences the loss through multiplication and sine.

Therefore:

$$
\bar x = \bar x\_{\text{product}} + \bar x\_{\text{sine}}.
$$

The final result is:

$$
\boxed{\bar x\approx35.705220027}.
$$

Together:

$$
\boxed{ \nabla L(2,3)= \begin{bmatrix} 35.705220027\\ 27.637189707 \end{bmatrix} }.
$$

This matches the forward-mode result.

The fact that both paths contribute to $x$ is central. An implementation that assigns rather than accumulates gradients can silently discard one contribution and return an incorrect derivative.

### Computational graphs and reverse traversal

A computational graph represents intermediate values as nodes and data dependencies as directed edges.

For a normal pure computation without feedback, the execution graph is a directed acyclic graph, or DAG.

The graph is acyclic because an operation depends on values computed earlier in the execution.

A node can contain the following information:

```cpp
struct Node {
    double value;       // Forward computed scalar value
    double adjoint;     // Accumulated reverse sensitivity
    size_t parentA;     // Index of first input node
    size_t parentB;     // Index of second input node
    double localA;      // d(node)/d(parentA)
    double localB;      // d(node)/d(parentB)
};
```

This is one possible representation, not a universal AD architecture.

More general systems may store operation types, tensor shapes, backward callbacks, saved tensors, recomputation metadata, device placement, and memory-management information.

#### Why reverse topological order is necessary

A node's gradient should be propagated to its parents only after the node has received all relevant downstream contributions.

If:

$$
x\rightarrow a\rightarrow L
$$

and:

$$
x\rightarrow b\rightarrow L,
$$

then $x$ must receive both contributions.

If a reverse traversal propagates a node's adjoint too early, before all its children have contributed, some gradient paths can be missed.

Topological ordering provides a systematic solution.

In a tape where every operation is appended after its operands, reversing insertion order is already a reverse topological order.

That is the approach used by the compact implementation below.

#### A working scalar reverse-mode tape in C++23

This implementation stores graph nodes in a contiguous `std::vector`, addressing parents by stable integer indices rather than raw pointers.

```cpp
#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <vector>

class Tape {
public:
    static constexpr std::size_t noParent =
        std::numeric_limits<std::size_t>::max();

    struct Node {
        double value{};
        double adjoint{};

        std::size_t a = noParent;
        std::size_t b = noParent;

        // Local derivatives saved from the forward computation.
        double da{};
        double db{};
    };

    // A variable is a leaf with no parent operations.
    std::size_t variable(double value) {
        return append({value, 0.0, noParent, noParent, 0, 0});
    }

    // z = a + b
    // dz/da = 1, dz/db = 1
    std::size_t add(std::size_t a, std::size_t b) {
        return append({
            nodes_.at(a).value + nodes_.at(b).value,
            0.0, a, b, 1.0, 1.0
        });
    }

    // z = a*b
    // dz/da = b, dz/db = a
    std::size_t multiply(std::size_t a, std::size_t b) {
        const double va = nodes_.at(a).value;
        const double vb = nodes_.at(b).value;

        return append({
            va * vb,
            0.0, a, b, vb, va
        });
    }

    // z = sin(a)
    // dz/da = cos(a)
    std::size_t sine(std::size_t a) {
        const double va = nodes_.at(a).value;

        return append({
            std::sin(va),
            0.0, a, noParent,
            std::cos(va), 0.0
        });
    }

    // Execute one reverse differentiation pass from a scalar output.
    void backward(std::size_t output) {
        nodes_.at(output);  // Validate the requested output.

        // Clear gradients from any previous backward traversal.
        for (Node& node : nodes_)
            node.adjoint = 0.0;

        // d(output)/d(output) = 1.
        nodes_[output].adjoint = 1.0;

        // Reverse insertion order is reverse topological order
        // because an operation is always appended after its parents.
        for (std::size_t i = nodes_.size(); i-- > 0;) {
            // Copy the current node so parent accumulation cannot
            // invalidate the values being read in this iteration.
            const Node node = nodes_[i];

            if (node.a != noParent) {
                nodes_[node.a].adjoint +=
                    node.adjoint * node.da;
            }

            if (node.b != noParent) {
                nodes_[node.b].adjoint +=
                    node.adjoint * node.db;
            }
        }
    }

    double value(std::size_t index) const {
        return nodes_.at(index).value;
    }

    double gradient(std::size_t index) const {
        return nodes_.at(index).adjoint;
    }

private:
    std::vector<Node> nodes_;

    std::size_t append(Node node) {
        nodes_.push_back(node);
        return nodes_.size() - 1;
    }
};
```

##### Construct the actual graph

Use:

```cpp
Tape tape;

// Leaf variables: x = 2, y = 3.
const auto x = tape.variable(2.0);
const auto y = tape.variable(3.0);

// a = x*y
const auto product = tape.multiply(x, y);

// b = sin(x)
const auto trig = tape.sine(x);

// c = a+b
const auto sum = tape.add(product, trig);

// L = c*c
// Passing the same node twice is intentional.
// Both incoming derivative paths must accumulate.
const auto loss = tape.multiply(sum, sum);

// Perform reverse differentiation.
tape.backward(loss);

// tape.gradient(x) gives dL/dx.
// tape.gradient(y) gives dL/dy.
```

An important case is:

```cpp
tape.multiply(sum, sum);
```

The same input node appears twice in the multiplication.

For:

$$
L=c\cdot c,
$$

the multiplication node stores two local derivatives, each equal to $c$.

During the reverse pass, both derivative paths contribute to the same parent:

$$
\bar c\mathrel{+}=\bar Lc,
$$

$$
\bar c\mathrel{+}=\bar Lc.
$$

Together:

$$
\bar c=2c\bar L.
$$

This is correct.

An implementation that uses a map keyed by parent identity and accidentally overwrites one duplicate edge would return only half the correct gradient for this operation.

#### Memory ownership and graph lifetime

The tape owns the entire node array.

The integer indices are identifiers into that array. A `std::vector` reallocation may move the node objects in memory, but the indices remain valid as long as the nodes retain their positions.

This avoids storing pointers to elements that might become dangling after reallocation.

However, the graph handles are meaningful only while the corresponding tape exists and has not been structurally reset in an incompatible way.

The minimal implementation is intentionally not thread-safe. Concurrent graph construction or adjoint accumulation would require synchronization, partitioning, or a different design.

Its reverse pass uses:

$$
O(N)
$$

time for $N$ graph nodes, assuming constant-cost scalar primitive operations, and:

$$
O(N)
$$

storage for saved node records.

For tensor operations, each node can represent an entire tensor operation, so storage and work costs also depend on tensor sizes and saved intermediate activations. Node count alone is insufficient.

### Full validated C++23 reference program

I prepared a complete standalone program incorporating the methods taught across M051 and M052.

It includes composite rectangle, midpoint, trapezoidal, and Simpson quadrature; explicit Euler, semi-implicit Euler, midpoint RK2, Heun RK2, and classical RK4; a mass-spring energy comparison; dual-number forward differentiation; a reverse-mode tape; and central finite-difference gradient validation.

M051 + M052 — Reference implementation

The source reports passing tests and points to a downloadable complete source file; that file is absent from the supplied attachment and the result is not independently verified here.

The supplied source reports it was compiled using (not reproduced here):

```bash
g++ -std=c++23 -O2 -Wall -Wextra -Wpedantic \
  phase9_m051_m052_reference.cpp \
  -o phase9_demo

./phase9_demo
```

The source reports the following executable output (not independently reproduced here):

Quadrature over $x^2$ from 0 to 2

| Exact           | 2.666666667 |
| --------------- | ----------- |
| Left rectangle  | 1.750000000 |
| Right rectangle | 3.750000000 |
| Midpoint        | 2.625000000 |
| Trapezoidal     | 2.750000000 |
| Simpson         | 2.666666667 |

Spring after one $0.1$-second step

| Exact position          | 0.980066578   |
| ----------------------- | ------------- |
| Explicit Euler position | 1.000000000   |
| Explicit Euler energy   | 2.080000000 J |
| Semi-implicit position  | 0.960000000   |
| Semi-implicit energy    | 1.923200000 J |
| Midpoint position       | 0.980000000   |
| RK4 position            | 0.980066667   |

Differentiation of $L=(xy+\sin x)^2$ at $(2,3)$

| Method             | $dL/dx$  | $dL/dy$  |
| ------------------ | ------------ | ------------ |
| Forward AD         | 35.705220027 | 27.637189707 |
| Reverse AD         | 35.705220027 | 27.637189707 |
| Central difference | 35.705220028 | 27.637189707 |

The supplied source reports that its assertions passed; that result has not been independently reproduced here.

The agreement is meaningful because the techniques use different derivative computations.

Forward mode propagates tangents through operations.

Reverse mode propagates adjoints through the graph in reverse order.

Central differences evaluate the original objective at perturbed inputs.

All three agree to the expected precision for this smooth test case.

That does not constitute proof that a general AD library is correct, but it is a valuable independent validation mechanism.

### Advanced AD: Jacobian-vector products versus vector-Jacobian products

We now need to understand the deeper linear-algebra structure behind forward and reverse differentiation.

Suppose:

$$
F:\mathbb R^n\rightarrow\mathbb R^m
$$

has Jacobian:

$$
J\in\mathbb R^{m\times n}.
$$

Forward-mode AD computes:

$$
\boxed{\mathbf u=J\mathbf v}
$$

for a chosen input tangent $\mathbf v\in\mathbb R^n$.

Reverse-mode AD computes:

$$
\boxed{\mathbf w=J^T\boldsymbol\lambda}
$$

for a chosen output sensitivity $\boldsymbol\lambda\in\mathbb R^m$.

This is the fundamental distinction.

Forward mode pushes changes through the computation.

Reverse mode pulls sensitivities backward through the computation.

The mathematical terminology is pushforward for a tangent vector and pullback for a cotangent vector.

##### Why reverse mode is efficient for scalar losses

Suppose a neural network or differentiable renderer has one million adjustable parameters and produces one scalar loss.

Then:

$$
J\in\mathbb R^{1\times1{,}000{,}000}.
$$

Forward mode with one tangent seed produces one directional derivative. Recovering every partial derivative separately would require one million basis-direction evaluations, barring additional batching or structural optimizations.

Reverse mode begins with one output sensitivity:

$$
\boldsymbol\lambda=[1].
$$

Then:

$$
J^T\boldsymbol\lambda
$$

produces the full one-million-component gradient in one reverse differentiation pass.

The cost is not independent of network size, but it is generally proportional to the executed computation rather than requiring one separately seeded execution per parameter.

Conversely, if a function has three inputs and one million outputs, forward mode may be advantageous for constructing its complete Jacobian.

The broad selection rule is:

| Situation                               | Often suitable                                           |
| --------------------------------------- | -------------------------------------------------------- |
| Few inputs, many outputs                | Forward mode                                             |
| Many inputs, few outputs                | Reverse mode                                             |
| One desired directional derivative      | Forward JVP                                              |
| One desired weighted output sensitivity | Reverse VJP                                              |
| Full Jacobian                           | Choose based on dimensions, sparsity, batching, and cost |
| Hessian-vector product                  | Mixed-mode or other matrix-free differentiation          |

This comparison assumes the operations support the chosen AD mode. Hardware, available kernels, memory, sparsity, and implementation details can change the best choice.

#### Differentiating matrix multiplication

Let:

$$
C=AB,
$$

where:

$$
A\in\mathbb R^{m\times k},\qquad B\in\mathbb R^{k\times n}.
$$

Then:

$$
C\in\mathbb R^{m\times n}.
$$

For small perturbations $dA,dB$, the product rule gives:

$$
\boxed{ dC=(dA)B+A(dB) }.
$$

Suppose a scalar loss $L$ depends on $C$, and its upstream gradient is:

$$
G=\frac{\partial L}{\partial C}.
$$

The scalar differential is:

$$
dL=\langle G,dC\rangle_F,
$$

where the Frobenius inner product is:

$$
\langle X,Y\rangle_F =\operatorname{tr}(X^TY).
$$

Substitute the product differential:

$$
dL=\langle G,(dA)B\rangle_F+ \langle G,A(dB)\rangle_F.
$$

Using trace cyclicity:

$$
\langle G,(dA)B\rangle_F =\langle GB^T,dA\rangle_F,
$$

and:

$$
\langle G,A(dB)\rangle_F =\langle A^TG,dB\rangle_F.
$$

Therefore:

$$
\boxed{ \frac{\partial L}{\partial A}=GB^T, \qquad \frac{\partial L}{\partial B}=A^TG }.
$$

These are the reverse-mode matrix-multiplication rules.

They explain a major performance consideration: gradient calculations often require matrix multiplications comparable in complexity to the forward matrix multiplication.

This is why modern deep-learning systems invest so heavily in efficient matrix kernels for both forward and backward execution.

#### Broadcasting requires gradient reduction

Suppose a matrix $X\in\mathbb R^{N\times D}$ receives a row-wise bias:

$$
Y\_{ij}=X\_{ij}+b_j.
$$

Here:

$$
b\in\mathbb R^D.
$$

Every row uses the same bias element.

Let the upstream gradient be:

$$
G\_{ij}=\frac{\partial L}{\partial Y\_{ij}}.
$$

The input gradient is:

$$
\frac{\partial L}{\partial X\_{ij}}=G\_{ij}.
$$

But the bias gradient is:

$$
\boxed{ \frac{\partial L}{\partial b_j} = \sum\_{i=1}^{N}G\_{ij} }.
$$

Why do we sum over rows?

Because each $b_j$ influences $N$ separate output elements. The chain rule accumulates all those paths.

A common automatic differentiation bug is forgetting to reduce a broadcasted gradient back to the original parameter shape.

This can produce wrong values, wrong tensor shapes, or accidental gradient replication.

### Differentiating a simulation through time

We can now connect the two chunks completely.

Suppose a physical system satisfies:

$$
\dot{\mathbf y}=f(t,\mathbf y,\boldsymbol\theta),
$$

where $\boldsymbol\theta$ contains parameters such as spring stiffness, damping, mass, or initial conditions.

We run a numerical simulation and obtain:

$$
\mathbf y(T;\boldsymbol\theta).
$$

We then evaluate a loss:

$$
L(\boldsymbol\theta) = \ell(\mathbf y(T;\boldsymbol\theta)).
$$

For example, the loss could measure the disagreement between simulated and observed object positions.

We want:

$$
\boxed{\frac{dL}{d\boldsymbol\theta}}.
$$

There are two conceptually distinct approaches.

We can differentiate the discrete numerical integrator that was executed.

Or we can derive continuous sensitivity equations and integrate those alongside the physical state.

These are related, but not always numerically identical.

#### Differentiating an Euler step

Suppose:

$$
\mathbf y\_{n+1} = \mathbf y_n+ h f(t_n,\mathbf y_n,\boldsymbol\theta).
$$

Differentiate with respect to one parameter $\theta$.

Define:

$$
S_n=\frac{\partial\mathbf y_n}{\partial\theta}.
$$

Assume, for now, that the timestep is constant and independent of $\theta$.

Then:

$$
\frac{\partial\mathbf y\_{n+1}}{\partial\theta} = \frac{\partial\mathbf y_n}{\partial\theta} + h\frac{d}{d\theta} f(t_n,\mathbf y_n,\theta).
$$

The total derivative of $f$ is:

$$
\frac{df}{d\theta} = \frac{\partial f}{\partial\mathbf y} \frac{\partial\mathbf y_n}{\partial\theta} + \frac{\partial f}{\partial\theta}.
$$

Therefore:

$$
\boxed{ S\_{n+1} = S_n+ h\left(J_fS_n+f\_\theta\right) }.
$$

Here $J_f=\partial f/\partial\mathbf y$ is the Jacobian of the dynamics with respect to state, and $f\_\theta=\partial f/\partial\theta$ is the direct parameter derivative.

This equation is the differentiated Euler update.

It propagates both the simulated state and its sensitivity.

#### Continuous sensitivity equations

For the exact continuous system:

$$
\dot{\mathbf y}=f(t,\mathbf y,\theta),
$$

differentiate with respect to $\theta$:

$$
\frac{d}{dt} \left( \frac{\partial\mathbf y}{\partial\theta} \right) = J_f \frac{\partial\mathbf y}{\partial\theta} + f\_\theta.
$$

Thus:

$$
\boxed{ \dot S=J_fS+f\_\theta }.
$$

This is an ODE in its own right.

We can integrate the physical state and sensitivity together:

$$
\frac d{dt} \begin{bmatrix} \mathbf y\\S \end{bmatrix} = \begin{bmatrix} f(t,\mathbf y,\theta)\\ J_fS+f\_\theta \end{bmatrix}.
$$

The extra sensitivity state can be large if there are many parameters.

For $n$ state variables and $p$ parameters, a full forward sensitivity matrix has $np$ elements.

This explains why forward sensitivity integration becomes expensive for models with many parameters.

#### Worked example: exponential damping

Consider:

$$
\dot y=-\lambda y, \qquad y(0)=y_0,
$$

where $\lambda>0$ is the damping parameter.

The exact solution is:

$$
y(t)=y_0e^{-\lambda t}.
$$

Differentiate with respect to $\lambda$:

$$
\boxed{ \frac{\partial y(t)}{\partial\lambda} = -ty_0e^{-\lambda t} }.
$$

Now derive the sensitivity equation independently.

Let:

$$
S=\frac{\partial y}{\partial\lambda}.
$$

The dynamics are:

$$
f(y,\lambda)=-\lambda y.
$$

Its derivatives are:

$$
f_y=-\lambda, \qquad f\_\lambda=-y.
$$

Therefore:

$$
\boxed{ \dot S=-\lambda S-y }.
$$

If the initial state is independent of $\lambda$:

$$
S(0)=0.
$$

The solution is:

$$
S(t)=-ty_0e^{-\lambda t}.
$$

Both derivations agree.

##### Differentiating the discrete Euler approximation

Explicit Euler gives:

$$
y\_{n+1}=(1-\lambda h)y_n.
$$

After $N$ steps:

$$
y_N=y_0(1-\lambda h)^N.
$$

Differentiate this discrete expression:

$$
\boxed{ \frac{\partial y_N}{\partial\lambda} = -Nhy_0(1-\lambda h)^{N-1} }.
$$

Compare this with the continuous derivative:

$$
\frac{\partial y(T)}{\partial\lambda} = -Ty_0e^{-\lambda T}, \qquad T=Nh.
$$

They are generally different at finite timestep.

As $h\to0$, under the usual convergence conditions, the discrete sensitivity approaches the continuous sensitivity.

This distinction is fundamental in differentiable simulation: AD through a numerical solver differentiates the implemented discrete computation. Continuous sensitivity analysis differentiates the mathematical differential equation and then numerically integrates the resulting sensitivity equations.

They need not agree exactly at a finite resolution.

#### Reverse adjoint sensitivity for continuous systems

Forward sensitivities are attractive when there are few parameters. But what if the simulation depends on millions of parameters and produces one scalar objective?

A reverse adjoint formulation may be more appropriate.

Suppose:

$$
\dot{\mathbf y}=f(t,\mathbf y,\boldsymbol\theta)
$$

and the objective is a terminal loss:

$$
L=\ell(\mathbf y(T)).
$$

Introduce an adjoint variable $\boldsymbol\lambda(t)$ with terminal condition:

$$
\boxed{ \boldsymbol\lambda(T)= \nabla\_{\mathbf y}\ell(\mathbf y(T)) }.
$$

The continuous adjoint satisfies:

$$
\boxed{ \dot{\boldsymbol\lambda} = -J_f(t,\mathbf y,\boldsymbol\theta)^T \boldsymbol\lambda }.
$$

Under appropriate smoothness assumptions and when the initial state does not depend on parameters, the parameter gradient is:

$$
\boxed{ \frac{dL}{d\boldsymbol\theta} = \int_0^T \left(\frac{\partial f}{\partial\boldsymbol\theta}\right)^T \boldsymbol\lambda(t)\\,dt }.
$$

If the initial state depends on parameters, an additional initial-condition sensitivity term appears. Explicit parameter dependence in the terminal loss also contributes directly, and running costs require further terms.

This is the continuous counterpart of reverse-mode differentiation through a sequence of integration steps.

The reverse computation depends on the forward trajectory, because $J_f$ must be evaluated along that trajectory.

That creates a memory problem: should the solver store every forward state or recompute states during the backward pass?

This motivates checkpointing, where selected forward states are preserved and intermediate states are reconstructed when needed.

For chaotic, stiff, discontinuous, or adaptively integrated systems, adjoint sensitivities require particular numerical care. Reconstructing the trajectory inaccurately can corrupt the reverse gradients.

The SUNDIALS CVODES package implements forward and [[Chapter Edition/01 - Computing Foundations/22 - Accumulation, Quadrature, and the Bridge to Probability#Numerical differentiation and automatic differentiation|adjoint sensitivity]] workflows for ODE systems, making it a useful production reference beyond this teaching implementation.&#x20;

[image](https://www.google.com/s2/favicons?domain=https://github.com\&sz=32)

GitHub



### Actual KairoMath automatic differentiation architecture

Your [KairoMath/TensorAutograd.cppm](https://github.com/swayam8624/KairoMath/blob/master/TensorAutograd.cppm?utm_source=chatgpt.com) provides a more sophisticated tensor-based implementation than the scalar tape we constructed.

The supplied source claims it inspected graph-node storage, gradient accumulation, `Backward`, addition, multiplication, matrix multiplication, checkpointing, and selected tensor operations.

##### Graph representation

The internal `Node` contains:

```cpp
Tensor<float> value;
Tensor<float> gradient;

bool requiresGradient = false;
bool hasGradient = false;

std::vector<std::shared_ptr<Node>> parents;

std::function<void(const Tensor<float>&)> backward;
```

The graph therefore stores tensor values rather than only scalars.

Parent relationships use shared ownership, and backward behavior is implemented using callable objects.

This has several consequences.

The architecture can define specialized backward rules for tensor operations without storing an individual graph node for every scalar arithmetic operation.

A matrix multiplication can be represented as one high-level operation while its backward function performs appropriately shaped matrix-gradient calculations.

However, shared ownership, heap allocations, callable storage, tensor temporaries, and graph traversal all affect memory use and performance.

The fact that an implementation uses automatic differentiation does not mean it has zero runtime overhead.

##### Actual gradient accumulation

Your `autograd_detail::Accumulate` validates the rank and shape of gradient contributions against the receiving variable.

If the node has no gradient storage yet, it constructs a zero-initialized gradient tensor.

It then adds incoming gradient values elementwise.

This is the tensor equivalent of:

$$
\bar x\mathrel{+}=\text{contribution}.
$$

The shape validation is important because broadcasting and reduction rules must be handled by the operation-specific backward functions before accumulation into a given input shape.

##### Actual backward execution

`Variable::Backward` constructs a topological ordering through the parent graph and processes nodes in reverse.

This is conceptually the same algorithm we implemented using a contiguous scalar tape.

The source supports both a scalar-output backward interface and a tensor-valued upstream-gradient interface.

The latter is the foundation for computing vector-Jacobian products, not merely gradients of scalar losses.

One important lifecycle distinction is that the implementation exposes `ZeroGradient()` explicitly, and its documentation notes gradient accumulation across calls.

For a training loop, the caller must manage gradient reset deliberately. Repeated backward execution through an existing graph also deserves regression testing to ensure intermediate adjoints do not produce unintended repeated propagation.

This is a property to test under the actual intended reuse pattern, rather than assuming every autograd engine has the same graph-retention and reset semantics.

##### Matrix multiplication in your repository

The source says the `AutogradMatMul` implementation computes gradients corresponding to:

$$
\frac{\partial L}{\partial A}=GB^T,
$$

$$
\frac{\partial L}{\partial B}=A^TG.
$$

Rather than express these derivatives using the higher-level matrix operators, the inspected implementation accumulates them through nested loops.

That gives us a direct connection between the mathematics and physical tensor accesses.

For example, the left operand's gradient entry is accumulated using:

$$
(\nabla_A L)\_{ri} = \sum_cG\_{rc}B\_{ic}.
$$

This is exactly the corresponding element of $GB^T$.

The right operand's gradient is:

$$
(\nabla_B L)\_{ic} = \sum_rA\_{ri}G\_{rc}.
$$

That is the corresponding element of $A^TG$.

A production optimization opportunity would be to benchmark these explicit loops against appropriate optimized tensor or matrix kernels, accounting for temporary allocations and tensor layouts rather than assuming one implementation must be faster.

##### Checkpointing

The source describes `AutogradCheckpoint` as evaluating a forward callback, stores its output value, and arranges for the callback to be recomputed when the backward operation receives an upstream gradient.

The intended trade-off is:

$$
\text{less saved activation memory} \quad\longleftrightarrow\quad \text{more recomputation}.
$$

This can be valuable when the forward computation produces large temporary tensors.

But recomputation must preserve the relevant forward semantics. Mutable parameters, changing random streams, non-deterministic execution, side effects, and stateful operations can invalidate a naive recomputation strategy.

Thus, checkpointing is not merely a memory-management technique. It changes the execution schedule and introduces additional invariants for reproducibility and differentiation correctness.

### Production numerical behavior: what can break?

We should collect the most important deeper failure mechanisms across both chunks.

##### Integrating the wrong state

A simulation may contain positions, velocities, accelerations, and forces stored in structures with similar scalar types.

Using an acceleration directly as a displacement, or using a timestep twice, produces physically incorrect units.

For example:

$$
\Delta v=a\Delta t,
$$

while:

$$
\Delta x=v\Delta t.
$$

The first quantity has units of metres per second; the second has units of metres.

Dimensional analysis should accompany equation derivation and code review.

##### Updating a coupled system in place incorrectly

For explicit Euler, every state derivative must be evaluated using the same old state.

Suppose the code does:

```cpp
state.v += acceleration(state.x) * dt;
state.x += state.v * dt;
```

This is not the fully simultaneous explicit Euler update. It is velocity-first semi-implicit Euler for the specified mechanical system.

That may be intentional, but it must be understood and documented.

For a large vector system, mutating one component before evaluating another component's derivative can accidentally create an inconsistent mixture of old and new state.

Double-buffering or explicitly staged state updates can prevent this.

##### Accuracy versus physical structure

RK4 may produce excellent short-term accuracy, but standard classical RK4 is not symplectic.

Semi-implicit Euler may be less accurate per step but preserve useful long-term phase-space structure in appropriate mechanical systems.

Implicit Euler can strongly damp high-frequency dynamics.

The method choice depends on what must be preserved: trajectory accuracy, energy behavior, constraints, stability, cost, or reproducibility.

No single integrator optimizes all these properties.

##### Nondifferentiable events

A physical simulator may include:

$$
\text{if penetration}>0: \quad\text{apply contact force}.
$$

At the activation boundary, the derivative of the complete update may change abruptly or fail to exist.

Reverse-mode AD can differentiate the branch taken during the actual execution, but that result need not represent a unique derivative at the switching surface.

For collision events, differentiating event time and reset maps may require additional hybrid-system sensitivity analysis.

Smooth penalty contacts and differentiable approximations can help in some research settings, but they also change the physical model and may allow penetration.

##### GPU implementation and memory cost

An RK4 GPU implementation may require several intermediate state buffers.

A reverse-mode differentiable simulation over thousands of steps may require storing or reconstructing a large number of forward states.

If a state has $N$ elements and there are $T$ simulation steps, naive full trajectory storage scales as:

$$
O(NT).
$$

Checkpointing reduces retained state storage at the cost of recomputation.

For a GPU implementation, we must also account for kernel launch costs, synchronization between RK stages, memory bandwidth, intermediate buffer lifetime, and register pressure.

Within an RK4 step, later stages depend on earlier stage results. These dependencies cannot be removed by treating every stage as independent.

However, derivative evaluation across many independent particles or spatial elements may be parallelized when its data dependencies permit it.

##### Floating-point determinism

Numerical integration accumulates changes over time.

Even small differences in arithmetic order or intermediate precision can affect trajectories, particularly in chaotic or collision-heavy systems.

Floating-point reassociation, fused multiply-add behavior, parallel reduction order, and hardware-specific math functions can produce differences between platforms.

This does not necessarily imply a broken implementation.

For research validation, distinguish bitwise reproducibility from tolerance-based numerical agreement and from agreement in statistical or physical observables.

A simulation may be scientifically reproducible within defined tolerances without producing bit-identical state arrays on every architecture.

### Research bridge: differentiable soft-body reconstruction

We can use M051 and M052 to formulate a concrete research problem relevant to physics simulation and inverse reconstruction.

Suppose a soft body has unknown stiffness $k$ and damping $c$.

We can observe its positions at selected times:

$$
\mathbf x\_{\mathrm{observed}}(t_1),\ldots, \mathbf x\_{\mathrm{observed}}(t_N).
$$

We simulate the soft body using parameters:

$$
\boldsymbol\theta= \begin{bmatrix} k\\c \end{bmatrix}.
$$

The simulator produces:

$$
\mathbf x\_{\mathrm{simulated}}(t_i;\boldsymbol\theta).
$$

Define an objective:

$$
\boxed{ L(\boldsymbol\theta)= \frac12\sum\_{i=1}^{N} \left\| \mathbf x\_{\mathrm{simulated}}(t_i;\boldsymbol\theta) - \mathbf x\_{\mathrm{observed}}(t_i) \right\|^2 }.
$$

The forward problem is to simulate motion from material parameters.

The inverse problem is to recover material parameters from observed motion.

The combined pipeline is:

$$
\boldsymbol\theta \rightarrow \text{forces} \rightarrow \text{ODE integration} \rightarrow \text{trajectory} \rightarrow L \rightarrow \nabla\_{\boldsymbol\theta}L.
$$

Forward simulation and differentiation are now connected.

#### A falsifiable research question

How does the choice of time integrator affect the accuracy and reliability of gradients used to recover material parameters?

A useful experiment would compare explicit Euler, semi-implicit Euler, RK4, and an appropriate implicit solver under controlled conditions.

The independent variables would include timestep, spring stiffness, damping, and integration method.

The dependent measurements would include trajectory error, energy behavior, parameter-gradient agreement against a high-accuracy reference, optimization convergence, runtime, and memory use.

The experiment should distinguish gradient correctness with respect to the discrete solver from gradient agreement with the underlying continuous model.

A finite-difference check of the discrete objective may show excellent agreement with reverse AD through the same discretization, even when both disagree with the gradient of the continuous physical system.

That distinction is central to evaluating differentiable simulation research.

For a valid comparison, all methods should use the same physical model, initial state, external forces, observation times, and loss definition.

If the problem includes contacts, those events should initially be excluded or carefully controlled before studying their separate derivative effects.

#### Connection to your Gaussian reconstruction work

The same idea applies to a reconstruction system.

Suppose projected observations depend on Gaussian means, covariance parameters, and camera pose:

$$
\mathbf s_i= \pi(\boldsymbol\mu_i,\Sigma_i,\mathbf T).
$$

A loss may compare predicted projected features with observed image features.

Automatic differentiation can provide gradients through the implemented projection and rendering pipeline.

But visibility changes, sorting decisions, clipping, opacity saturation, covariance conditioning, and numerical approximation can create important differentiation difficulties.

One experiment could compare analytical projection Jacobians, AD-generated derivatives, and central finite differences on smooth fixed-visibility scenes.

Another could quantify where gradients become unreliable as Gaussians approach clipping surfaces, become highly anisotropic, or cross visibility-ordering boundaries.

The objective would be to locate specific regimes where an approximation is inadequate, not merely report average gradient agreement over easy configurations.

### Final reconstruction: M051 + M052

The two chunks form one coherent mathematical-to-computational chain.


#### Reconstruction challenges

To establish mastery, try reconstructing the following without using the formulas above.

1\. Derive the trapezoidal rule by integrating a line between two endpoints, then derive Simpson's $1,4,1$ weights by integrating the quadratic Lagrange basis. Explain why midpoint and trapezoidal integration are second-order while Simpson's rule is fourth-order under suitable assumptions.

2\. Starting from $y'=f(t,y)$, derive explicit Euler, explicit midpoint RK2, and RK4. For $y'=-2y$, manually compute one step of each with $h=0.1$ and compare against the exact exponential solution.

3\. For the oscillator $x''=-\omega^2x$, derive the explicit and semi-implicit Euler update matrices. Use their eigenvalues to explain why their stability behavior differs.

4\. Write a dual-number type implementing addition, multiplication, division, sine, and exponential. Use it to differentiate $(xy+\sin x)^2$ in both input directions without deriving the final expression symbolically.

5\. Implement reverse differentiation for the same expression using a graph that stores primal values, local derivatives, parents, and adjoints. Explain why the graph must accumulate contributions and why reverse topological processing is required.

6\. Differentiate the discrete Euler update for $y'=-\lambda y$ with respect to $\lambda$, and explain why that derivative differs from the exact continuous sensitivity at finite timestep.

The practical capstone is to take the reference C++23 program, replace the constant-stiffness spring with a damped, externally forced oscillator, and make the spring stiffness a differentiable parameter. Independently verify its trajectory and parameter gradient against a high-accuracy baseline.

That single extension forces you to connect physical modeling, ODE state design, time integration, Jacobian propagation, numerical verification, and optimization.


<!-- end V09.06 -->

<!-- begin V10.00 -->
## From Uncertainty to Trustworthy Samples: Probability and Monte Carlo


<!-- end V10.00 -->


## Random measurements, expectation, and covariance

PCA and many estimation methods require a statistical description of a cloud of measurements. A random variable is a quantity whose value is not known until an outcome is observed. Its probability law assigns probabilities to possible values. For a scalar random variable $X$ with finite mean, its expectation is the probability-weighted average

$$
\mathbb E[X]=\sum_x x\,P(X=x)
$$

in the discrete case, or $\mathbb E[X]=\int x p(x)\,dx$ when it has density $p$. Expectation is a population property of the model. The average of a finite observed sample is an estimate of that property, not the same object.

For a random vector $X\in\mathbb R^d$ with mean $\mu=\mathbb E[X]$, define its centered form as $X-\mu$. Its covariance matrix is

$$
\Sigma=\mathbb E\big[(X-\mu)(X-\mu)^T\big].
$$

Entry $\Sigma_{ij}$ records how deviations in components $i$ and $j$ vary together. Diagonal entries are variances; off-diagonal entries are covariances. For any direction $v$, the variance of the projection $v^TX$ is

$$
\operatorname{Var}(v^TX)=v^T\Sigma v.
$$

This equation connects a cloud of data to a quadratic form: covariance measures spread as seen along every direction. Because $v^T\Sigma v\ge 0$, covariance is symmetric positive semidefinite. It therefore has an orthonormal eigenbasis, and its eigenvalues are nonnegative.

A worked two-point distribution makes the definition tangible. Let $X$ be $(1,0)^T$ or $(3,2)^T$, each with probability $1/2$. Then $\mu=(2,1)^T$. The centered outcomes are $(-1,-1)^T$ and $(1,1)^T$, so

$$
\Sigma=\tfrac12\begin{bmatrix}1&1\\1&1\end{bmatrix}
+\tfrac12\begin{bmatrix}1&1\\1&1\end{bmatrix}
=\begin{bmatrix}1&1\\1&1\end{bmatrix}.
$$

The eigenvector $(1,1)^T/\sqrt2$ has eigenvalue $2$, while $(1,-1)^T/\sqrt2$ has eigenvalue $0$. All variation lies along the diagonal direction; there is no variation perpendicular to it. The covariance matrix describes that fact without needing to inspect every sample individually.

Given $N$ observations $x_1,\ldots,x_N$, first compute the sample mean $\bar x=N^{-1}\sum_i x_i$, then center each observation. The empirical covariance often uses denominator $N$ when it describes the observed finite cloud, and denominator $N-1$ when estimating an unknown population covariance under the usual independent, identically distributed sampling assumptions. State which convention is used. Forgetting to center produces a second-moment matrix $N^{-1}\sum_i x_ix_i^T$, which mixes the cloud's offset from the origin with its variation around its center.

PCA takes the eigenvectors of a centered covariance matrix as principal directions. The eigenvalue gives variance along its eigenvector. Sorting eigenvalues from largest to smallest orders directions by captured variation. For a centered data matrix $X_c$ whose rows are observations, covariance is proportional to $X_c^TX_c$; the right singular vectors of $X_c$ are the PCA directions, and squared singular values are proportional to covariance eigenvalues. This connects PCA to SVD while often avoiding the numerical squaring of condition number that can occur when explicitly forming a Gram matrix.

PCA is not unit-free truth. If one axis records meters and another records millimeters, raw covariance gives the second axis a scale one thousand times larger in numerical coordinates. Standardizing variables or using a metric changes the question. Eigenvectors can also be unstable when neighboring eigenvalues are close: a small perturbation may rotate the corresponding subspace substantially even if the total variance captured is stable. Report the eigengap and measurement scale when interpreting principal directions. For a line or plane fit, the smallest-variance direction often supplies a normal, but if the smallest eigenvalues are nearly tied, that normal is poorly determined.

<!-- begin V10.02 -->
<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/01 - Computing Foundations/00 - Reading Map|Reading map]] · [[Chapter Edition/01 - Computing Foundations/21 - Multivariable Derivatives and Sensitivity|Previous chapter]] · [[Chapter Edition/01 - Computing Foundations/23 - Probability Models and Measurement Error|Next chapter]]
