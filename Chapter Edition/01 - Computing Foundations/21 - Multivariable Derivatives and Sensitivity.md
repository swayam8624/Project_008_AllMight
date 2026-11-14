# 21 — Multivariable Derivatives and Sensitivity

[[Chapter Edition/01 - Computing Foundations/00 - Reading Map|Reading map]] · [[Chapter Edition/01 - Computing Foundations/20 - Choose Matrix Factorizations from Structure|Previous chapter]] · [[Chapter Edition/01 - Computing Foundations/22 - Accumulation, Quadrature, and the Bridge to Probability|Next chapter]]

Scalar calculus extends to vector-valued quantities through Jacobians, chain rules, and implicit relationships. This chapter uses derivatives to quantify how geometry and model outputs respond to changing inputs, preparing for optimization and simulation.

**By the end:** Compute and interpret derivatives of vector functions, normalization, and implicit constraints as local sensitivity information.

**Recall question:** How does a Jacobian describe the first-order change of a vector output when several inputs change?

<!-- Derived from Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md; source lines 46370–49357; source-span SHA256 282a6d0da34c917c63ad2ea7725fa7123ddb20e6e00166bc5fda6464c6dbfbb0. -->

<!-- CHAPTER-SOURCE-BEGIN -->
## Multivariable derivatives and sensitivity
###  Additional foundations: derivatives of vector operations

These are especially relevant because the next chunk uses vectors rather than individual scalar inputs.

For differentiable vector functions $\mathbf a(t)$ and $\mathbf b(t)$, differentiation acts componentwise.

The derivative of their dot product is:

$$
\frac d{dt}(\mathbf a\cdot\mathbf b) =\mathbf a'\cdot\mathbf b+\mathbf a\cdot\mathbf b'.
$$

The derivative of their cross product is:

$$
\frac d{dt}(\mathbf a\times\mathbf b) =\mathbf a'\times\mathbf b+\mathbf a\times\mathbf b'.
$$

If $A(t)$ is a matrix and $\mathbf x(t)$ a vector:

$$
\frac d{dt}(A\mathbf x) =A'\mathbf x+A\mathbf x'.
$$

Matrix multiplication is not generally commutative, so when differentiating a product $A(t)B(t)$, preserve factor order:

$$
\frac d{dt}(AB)=A'B+AB'.
$$

##### The derivative of a vector's length

Let:

$$
r(t)=\|\mathbf v(t)\| =\sqrt{\mathbf v(t)\cdot\mathbf v(t)}.
$$

By the chain and dot-product rules:

$$
r'(t)=\frac1{2\|\mathbf v\|} \left(2\mathbf v\cdot\mathbf v'\right).
$$

Thus, whenever $\mathbf v\ne0$:

$$
\boxed{\frac d{dt}\|\mathbf v\| =\frac{\mathbf v\cdot\mathbf v'}{\|\mathbf v\|}}.
$$

This has a physical interpretation: only the component of velocity change aligned with the vector contributes to the change of its length. A change perpendicular to the vector rotates it without changing its length to first order.

##### The derivative of normalization

Define:

$$
\mathbf n(t)=\frac{\mathbf v(t)}{\|\mathbf v(t)\|}.
$$

Using the product and chain rules:

$$
\mathbf n' = \frac{\mathbf v'}{\|\mathbf v\|} - \frac{\mathbf v(\mathbf v\cdot\mathbf v')}{\|\mathbf v\|^3}.
$$

Since $\mathbf n=\mathbf v/\|\mathbf v\|$:

$$
\boxed{ \mathbf n' = \frac1{\|\mathbf v\|} \left(I-\mathbf n\mathbf n^T\right)\mathbf v' }.
$$

Here $I$ is the identity matrix, and $\mathbf n\mathbf n^T$ is the outer product of the unit direction with itself.

The matrix:

$$
I-\mathbf n\mathbf n^T
$$

projects a vector onto the plane perpendicular to $\mathbf n$.

This is a remarkable connection: Differentiating normalization produces a projection matrix.

Why? A unit vector is constrained to remain on the unit sphere. Its instantaneous motion must therefore lie in the tangent plane of that sphere, not point radially outward.

At $\mathbf v=0$, normalization is undefined. Near zero, the factor $1/\|\mathbf v\|$ can become very large, revealing numerical sensitivity.

This formula later appears in normal-vector optimization, shading derivatives, geometry processing, and differentiable rendering.

##### Implicit differentiation as an additional bridge

Sometimes an output is not given by an explicit formula $y=f(x)$. Instead, it is defined by an equation:

$$
F(x,y)=0.
$$

If $y$ depends differentiably on $x$, differentiate both sides:

$$
F_x+F_y\frac{dy}{dx}=0.
$$

Provided $F_y\ne0$:

$$
\boxed{\frac{dy}{dx}=-\frac{F_x}{F_y}}.
$$

For a circle:

$$
F(x,y)=x^2+y^2-R^2=0,
$$

we obtain:

$$
2x+2y\frac{dy}{dx}=0, \qquad \frac{dy}{dx}=-\frac xy.
$$

At a point where $y=0$, this particular representation has a vertical tangent; it cannot locally express $y$ as a differentiable function of $x$.

Implicit differentiation is used extensively in constraint solvers, collision geometry, equilibrium problems, root-finding derivatives, and differentiable simulation.



We have established that the derivative of a scalar function measures its local response to a small input change.

Now we must generalize this idea.

A renderer does not usually receive a single scalar. A point has three coordinates. A material has multiple parameters. A camera has position, orientation, intrinsic parameters, and potentially distortion parameters. An optimization objective may depend on thousands or millions of variables.

We need a mathematical description of how all these inputs affect all the outputs.

Our underlying objective is to understand the general transformation:

$$
\mathbf y=F(\mathbf x), \qquad F:\mathbb R^n\rightarrow\mathbb R^m,
$$

and what happens when:

$$
\mathbf x\longrightarrow\mathbf x+\Delta\mathbf x.
$$

The answer will connect partial derivatives, gradients, total derivatives, Jacobians, Hessians, and Taylor expansions into one coherent system.

#### Multivariable functions and partial derivatives

Consider a function:

$$
f(x,y)=x^2y+3y^2.
$$

This function depends on two independent scalar inputs and produces one scalar output.

Geometrically, the function defines a surface in three-dimensional space:

$$
z=f(x,y).
$$

For every point $(x,y)$ on the input plane, the surface has a height $z$.

[Understanding Partial Differential Equations: From Surfaces to Relationships | by Tomio Kobayashi | May, 2026 | Medium](https://images.openai.com/static-rsc-4/GEWWaskcvDs-k2bsfVrRmlEzSQf7QE6m-7riYWFEmjdlQQz374f8l37I0H_qhF8skoNnV5akPnFXPgxsB0R5XVBdkVoZ4mggd_v0u-1BSl56lHDmRf8nQjxd5CDCdry1u_oubL3XSLuw5LwhNU2Y7wf0_B7wqYeZegMjquLa938?purpose=inline)

[MATH2111 Higher Several Variable Calculus: Partial derivatives definition](https://images.openai.com/static-rsc-4/e1nonwR6E_jbX1gzC3b2FD5d-QIIrLwkQhy-t7IFdjG_hoMsT-A2Sk6F_QM8Jwz3-lgOqkHr03ugrEHklNd7B963N8uKfSViTHdmD7gf3W2lUH6JaOU7qDr2e4h-BapYF6XNLyBGReGHnwMLCB0kubuwT4Ec80aioW9MUPw5IoA?purpose=inline)

[Directional Derivatives and the Gradient · Calculus](https://images.openai.com/static-rsc-4/MtStZWdgED9mKU6EnkX1k_zlh4L3TWo-GNfhGqa5OirCu2UqgHU1RDOc_KWqWGx4LvoxpyNeJD9le0CUU_moeDOftB-Tv5JU_33TqQg3AYkU_y2mzNrrzsQlPUtGln7B37nN0ug6FVtEWCW8T5tMoWrpR9uHgd9H2YPZXXUQm74?purpose=inline)

7

With one input, a graph is a curve. With two inputs and one output, it is a surface. With three inputs and one output, it is a scalar field in three-dimensional space, which we often visualize using contour surfaces rather than adding a fourth spatial axis.

A signed-distance field is an example:

$$
d:\mathbb R^3\rightarrow\mathbb R.
$$

A 3D velocity field is vector-valued:

$$
\mathbf v:\mathbb R^3\rightarrow\mathbb R^3.
$$

##### What does it mean to differentiate with respect to one variable?

Suppose we change $x$ but hold $y$ fixed.

The [[Chapter Edition/01 - Computing Foundations/21 - Multivariable Derivatives and Sensitivity#Multivariable derivatives and sensitivity|partial derivative]] with respect to $x$ is:

$$
\frac{\partial f}{\partial x} = \lim\_{h\to0} \frac{f(x+h,y)-f(x,y)}h.
$$

Likewise, the partial derivative with respect to $y$ is:

$$
\frac{\partial f}{\partial y} = \lim\_{h\to0} \frac{f(x,y+h)-f(x,y)}h.
$$

The symbol $\partial$ is used because we are differentiating with respect to only one of several input coordinates.

For:

$$
f(x,y)=x^2y+3y^2,
$$

differentiate with respect to $x$, treating $y$ as constant:

$$
\boxed{f_x=2xy}.
$$

Differentiate with respect to $y$, treating $x$ as constant:

$$
\boxed{f_y=x^2+6y}.
$$

Here $f_x$ and $f_y$ are shorthand for the respective partial derivatives.

At $(x,y)=(2,1)$:

$$
f_x(2,1)=4, \qquad f_y(2,1)=10.
$$

This means that near this point, increasing $x$ by a small amount $h$, with $y$ unchanged, changes the output by approximately $4h$.

Increasing $y$ by the same amount, with $x$ unchanged, changes the output by approximately $10h$.

Notice that the partial derivatives can themselves be functions of all the input variables. Holding a variable constant during differentiation does not mean the derivative is globally independent of it.

##### Partial derivatives do not automatically establish full differentiability

This is a distinction many calculus introductions underemphasize.

Consider:

$$
f(x,y)= \begin{cases} \displaystyle\frac{xy}{x^2+y^2}, &(x,y)\ne(0,0),\\ 0,&(x,y)=(0,0). \end{cases}
$$

Along the $x$-axis, where $y=0$, the function equals zero. Along the $y$-axis, where $x=0$, it also equals zero.

Consequently, both partial derivatives at the origin exist and equal zero.

But along the diagonal $y=x$:

$$
f(x,x)=\frac{x^2}{2x^2}=\frac12
$$

for every nonzero $x$.

Thus, approaching the origin along that diagonal produces a limiting value of $1/2$, while approaching along an axis produces zero.

The function is not continuous at the origin and therefore cannot be differentiable there.

The lesson: Having valid derivatives along coordinate axes does not guarantee that a function behaves linearly under arbitrary multidimensional perturbations.

Even continuity plus the existence of directional derivatives in every direction is not, by itself, sufficient for total differentiability.

A useful sufficient condition is that all first partial derivatives exist in a neighborhood and are continuous there. This guarantees differentiability at the point, although it is stronger than strictly necessary.

#### The gradient: collecting every first-order sensitivity

For a differentiable scalar-valued function:

$$
f:\mathbb R^n\rightarrow\mathbb R,
$$

we collect its partial derivatives into a vector:

$$
\boxed{ \nabla f(\mathbf x)= \begin{bmatrix} \frac{\partial f}{\partial x_1}\\ \frac{\partial f}{\partial x_2}\\ \vdots\\ \frac{\partial f}{\partial x_n} \end{bmatrix} }.
$$

The symbol $\nabla$, pronounced nabla or del, is the gradient operator in this context.

For the earlier example:

$$
f(x,y)=x^2y+3y^2,
$$

we obtain:

$$
\nabla f(x,y)= \begin{bmatrix} 2xy\\ x^2+6y \end{bmatrix}.
$$

At $(2,1)$:

$$
\nabla f(2,1)= \begin{bmatrix}4\\10\end{bmatrix}.
$$

This one vector describes the first-order sensitivity of the scalar output to changes in all input coordinates.

##### Why is the gradient a vector?

Imagine a very small input displacement:

$$
\Delta\mathbf x= \begin{bmatrix} \Delta x\\ \Delta y \end{bmatrix}.
$$

If the function is differentiable, the first-order output change is:

$$
\Delta f \approx f_x\Delta x+f_y\Delta y.
$$

This is a dot product:

$$
\boxed{ \Delta f\approx\nabla f(\mathbf x)^T\Delta\mathbf x }.
$$

For our point $(2,1)$, suppose:

$$
\Delta\mathbf x= \begin{bmatrix} 0.01\\ -0.02 \end{bmatrix}.
$$

Then:

$$
\begin{aligned} \Delta f &\approx4(0.01)+10(-0.02)\\ &=0.04-0.20\\ &=-0.16. \end{aligned}
$$

The function's value should decrease by approximately $0.16$.

We can check this directly.

Initially:

$$
f(2,1)=2^2(1)+3(1)^2=7.
$$

At the new point:

$$
\begin{aligned} f(2.01,0.98) &=(2.01)^2(0.98)+3(0.98)^2\\ &=3.959298+2.8812\\ &=6.840498. \end{aligned}
$$

So the exact change is:

$$
\Delta f=-0.159502.
$$

The gradient prediction was $-0.16$, giving an error of $0.000498$.

That small error is the result of neglected nonlinear curvature.

##### Gradient versus derivative

We must distinguish two closely related mathematical objects.

The derivative of a scalar function of several variables is formally a linear functional: it takes a small displacement and produces a first-order scalar change.

For Euclidean coordinates, this linear functional can be represented as a row vector:

$$
Df(\mathbf x)= \begin{bmatrix} f\_{x_1}&f\_{x_2}&\cdots&f\_{x_n} \end{bmatrix}.
$$

The gradient is conventionally written as the corresponding column vector:

$$
\nabla f=(Df)^T.
$$

We identify these through the Euclidean inner product:

$$
Df[\mathbf h]=\nabla f\cdot\mathbf h.
$$

This distinction becomes significant in differential geometry, where a differential is naturally a covector, and converting it to a gradient vector requires a metric.

##### What if variables use different physical units?

Suppose an objective depends on camera translation in metres and rotation in radians:

$$
L=L(t_x,t_y,t_z,\theta_x,\theta_y,\theta_z).
$$

Then the partial derivatives have different units. A derivative with respect to translation has loss units per metre, while one with respect to angle has loss units per radian.

Comparing their raw magnitudes is not automatically meaningful.

Later optimization methods must take account of parameter scaling, physical interpretation, and possibly a metric or preconditioner. A naive Euclidean gradient over mixed units may favor changes simply because of how parameters were numerically encoded.

This connects calculus directly to the conditioning concepts you studied in Phase 8.

#### Directional derivatives

Partial derivatives tell us what happens when moving along individual coordinate axes.

But what happens when we move in an arbitrary direction?

Let $\mathbf u$ be a unit direction vector. The [[Chapter Edition/01 - Computing Foundations/21 - Multivariable Derivatives and Sensitivity#Multivariable derivatives and sensitivity|directional derivative]] is:

$$
D\_{\mathbf u}f(\mathbf x) = \lim\_{h\to0} \frac{f(\mathbf x+h\mathbf u)-f(\mathbf x)}h.
$$

When $f$ is differentiable:

$$
\boxed{ D\_{\mathbf u}f(\mathbf x) = \nabla f(\mathbf x)\cdot\mathbf u }.
$$

This follows by substituting $\Delta\mathbf x=h\mathbf u$ into the local linear approximation:

$$
f(\mathbf x+h\mathbf u) = f(\mathbf x)+h\nabla f\cdot\mathbf u+o(|h|).
$$

Subtract $f(\mathbf x)$, divide by $h$, and take the limit.

##### Worked example: directional derivative at $(2,1)$

At $(2,1)$, our gradient is:

$$
\nabla f= \begin{bmatrix}4\\10\end{bmatrix}.
$$

Suppose we move along:

$$
\mathbf d= \begin{bmatrix}3\\4\end{bmatrix}.
$$

Its length is $5$, so the corresponding unit direction is:

$$
\mathbf u= \begin{bmatrix}3/5\\4/5\end{bmatrix}.
$$

Then:

$$
\begin{aligned} D\_{\mathbf u}f &=4\left(\frac35\right)+10\left(\frac45\right)\\ &=\frac{12}{5}+\frac{40}{5}\\ &=\boxed{10.4}. \end{aligned}
$$

The output increases at a local rate of $10.4$ per unit distance traveled in that direction.

Had we used $\mathbf d=(3,4)$ without normalization, the directional parameter would correspond to movement of five spatial units per unit parameter change. Its derivative would be $52$, not $10.4$.

Both calculations are mathematically meaningful, but they answer different questions.

##### Connection to contour lines

A level set of a scalar function is defined by:

$$
f(\mathbf x)=c
$$

for a constant $c$.

Imagine moving along a contour where the output value does not change. If the path is $\mathbf x(t)$, then:

$$
\frac d{dt}f(\mathbf x(t)) = \nabla f\cdot\mathbf x'(t)=0.
$$

Therefore, the gradient is perpendicular to the tangent direction of the contour whenever the gradient is nonzero.

[3.3. Optimality conditions — MMiDS Textbook](https://images.openai.com/static-rsc-4/v46Mm5YWBYnLwLfo1X8evI0zsZ6KQblMrBzNblf8TQLA-sQyDBWZ0mUgOBYe8xGNst9diAopOVwwDJp922HzIS0Wu6TtpLFzxPZjmFnZNX6_cmlh-XAHYLSntx-FivIjSE9LquhxX5wmwLGMb6B8x-zvsRNbuOiNaWWc-qQe2bE?purpose=inline)

[mmids-textbook.github.io](https://mmids-textbook.github.io/chap03_opt/03_optimality/roch-mmids-opt-optimality.html?utm_source=chatgpt.com)

In 3D, this means the gradient of a differentiable scalar field is normal to its level surface.

For a signed-distance field:

$$
d(\mathbf p)=0
$$

defines the surface itself. When the gradient exists and is nonzero, the surface normal is:

$$
\mathbf n= \frac{\nabla d}{\|\nabla d\|}.
$$

For a true signed-distance function, $\|\nabla d\|=1$ almost everywhere in regions where the nearest surface point is unique and the function is differentiable.

At corners, medial axes, or locations with multiple equally close surface points, differentiability can fail.

This explains why SDF normals become ambiguous at certain geometric features.

#### Why the gradient is the steepest-ascent direction

People often memorize that the gradient points uphill. We should prove it and understand its assumptions.

Suppose we want the direction $\mathbf u$ that maximizes the rate of increase of $f$, subject to:

$$
\|\mathbf u\|\_2=1.
$$

The directional derivative is:

$$
D\_{\mathbf u}f=\nabla f\cdot\mathbf u.
$$

Using the geometric dot-product definition:

$$
\nabla f\cdot\mathbf u = \|\nabla f\|\|\mathbf u\|\cos\theta.
$$

Since $\|\mathbf u\|=1$:

$$
D\_{\mathbf u}f = \|\nabla f\|\cos\theta.
$$

Its maximum occurs when:

$$
\cos\theta=1,
$$

meaning $\mathbf u$ points in the same direction as the gradient.

Thus, if $\nabla f\ne0$:

$$
\boxed{ \mathbf u\_{\max}= \frac{\nabla f}{\|\nabla f\|} }.
$$

The steepest-descent direction is the negative:

$$
\boxed{ \mathbf u\_{\min}= -\frac{\nabla f}{\|\nabla f\|} }.
$$

This is a direct consequence of the Cauchy–Schwarz inequality.

If the gradient is zero, all directional derivatives predicted by the first-order model are zero. That does not necessarily imply a minimum or maximum; the point could be a saddle.

##### A subtle limitation: steepest according to which distance?

The argument assumes Euclidean unit length.

Suppose distance is instead measured by a positive-definite metric matrix $G$:

$$
\|\mathbf u\|\_G^2=\mathbf u^TG\mathbf u.
$$

The steepest-ascent direction under this metric is proportional to:

$$
G^{-1}\nabla f.
$$

After normalization in the $G$-metric:

$$
\mathbf u\_{\max} = \frac{G^{-1}\nabla f} {\sqrt{\nabla f^TG^{-1}\nabla f}}.
$$

To see why, substitute $\mathbf v=G^{1/2}\mathbf u$ into the maximization problem. The constraint becomes $\|\mathbf v\|\_2=1$, while the objective becomes $(G^{-1/2}\nabla f)^T\mathbf v$. Ordinary Cauchy–Schwarz then gives the formula.

This generalized view is the mathematical foundation behind preconditioning, natural gradients, and optimization on curved spaces.

It will become important when we optimize rotations, camera poses, Gaussian covariances, or parameters with very different scales.

**The gradient is normal to a level set.** Let a differentiable curve $\mathbf r(s)$ remain on $f=c$. Differentiating the constant value gives
$$
\frac{d}{ds}f(\mathbf r(s))=\nabla f(\mathbf r(s))^T\mathbf r'(s)=0.
$$
Since $\mathbf r'(s)$ is tangent to the level set, the gradient is perpendicular to it. Where $\nabla f\ne0$, it gives the local normal direction. For a unit direction $\mathbf u$,
$$
D_{\mathbf u}f=\nabla f\cdot\mathbf u=\|\nabla f\|\cos\theta.
$$
The largest and smallest values are $\|\nabla f\|$ along the gradient and $-\|\nabla f\|$ opposite it. If the gradient is zero, every first-order directional derivative is zero and there is no unique steepest direction. This chain-rule proof appears in MIT 18.02SC Session 35; the selected captions were reviewed, but board visuals were not independently inspected. [Official Session 35 notes](https://ocw.mit.edu/courses/18-02sc-multivariable-calculus-fall-2010/6d4f46e89733b7291f86a22882aa2311_MIT18_02SC_notes_18.pdf), [selected caption clip](https://www.youtube.com/watch?v=2XraaWefBd8)

#### The total derivative

Partial derivatives examine one input while other independent coordinates remain fixed.

A total derivative accounts for the fact that several inputs may change together, possibly because they depend on another variable.

Suppose:

$$
z=f(x,y)
$$

and both $x$ and $y$ depend on time:

$$
x=x(t),\qquad y=y(t).
$$

Then:

$$
z(t)=f(x(t),y(t)).
$$

Applying the chain rule:

$$
\boxed{ \frac{dz}{dt} = \frac{\partial f}{\partial x}\frac{dx}{dt} + \frac{\partial f}{\partial y}\frac{dy}{dt} }.
$$

If $f$ also depends explicitly on time:

$$
f=f(x,y,t),
$$

the formula becomes:

$$
\boxed{ \frac{df}{dt} = f_x\frac{dx}{dt} + f_y\frac{dy}{dt} + f_t }.
$$

##### Numerical example: total derivative of a moving point

Suppose a scalar field is:

$$
T(x,y,t)=x^2+3y+t^2x.
$$

A moving particle follows:

$$
x(t)=t,\qquad y(t)=t^2.
$$

First substitute the trajectory into the field:

$$
T(t,t^2,t)=t^2+3t^2+t^3=4t^2+t^3.
$$

Differentiate directly:

$$
\boxed{\frac{dT}{dt}=8t+3t^2}.
$$

Now derive the same result using partial derivatives.

We have:

$$
T_x=2x+t^2,\qquad T_y=3,\qquad T_t=2tx.
$$

The trajectory derivatives are:

$$
x'(t)=1,\qquad y'(t)=2t.
$$

Thus:

$$
\begin{aligned} \frac{dT}{dt} &=(2x+t^2)(1)+3(2t)+2tx\\ &=2t+t^2+6t+2t^2\\ &=\boxed{8t+3t^2}. \end{aligned}
$$

Both approaches agree.

##### The material derivative in physics

Suppose $T(\mathbf x,t)$ represents temperature at a spatial location $\mathbf x$ and time $t$.

A stationary sensor measures the explicit time variation:

$$
\frac{\partial T}{\partial t}.
$$

But a particle moving with velocity $\mathbf v$ experiences an additional change because it travels into regions with different temperatures.

Its total rate of change is:

$$
\boxed{ \frac{DT}{Dt} = \frac{\partial T}{\partial t} + \mathbf v\cdot\nabla T }.
$$

The term $\mathbf v\cdot\nabla T$ is the advective contribution.

This is the same chain rule we just derived, expressed in a physical field.

The concept is fundamental in fluid mechanics, particle transport, heat transfer, and continuum simulation.


<!-- end V09.02 -->

<!-- begin V09.03 -->
### A triangle turns a differential into a concrete field calculation

Suppose a scalar value is stored at every vertex of the panel mesh: temperature, displacement error, or a texture coordinate. A **scalar field** assigns one number to each location; a **vector field** assigns a vector. Inside one nondegenerate triangle, linearly interpolate the three scalar values with barycentric basis functions $\lambda_i$:

$$
f(x)=\sum_{i=0}^2 f_i\lambda_i(x),\qquad
\sum_i\lambda_i(x)=1.
$$

Each basis function is one at its own vertex and zero at the others. Its gradient is constant on a flat triangle and perpendicular to the opposite edge. This gives an actual local derivative instead of treating a mesh as an arbitrary collection of unrelated numbers.

For $A=(0,0)$, $B=(2,0)$, $C=(0,1)$, the basis functions are $\lambda_A=1-x/2-y$, $\lambda_B=x/2$ and $\lambda_C=y$. If values are $(0,2,3)$, the interpolated field is $f(x,y)=x+3y$ and its gradient is $(1,3)$. Adding the same constant to all three values changes $f$ but leaves its gradient unchanged because $\sum_i\nabla\lambda_i=0$.

For a three-dimensional triangle, this gradient lies in its tangent plane. Only tangential changes are determined by the surface samples. A normal-direction derivative cannot be recovered from one surface field without an extension model. This is a geometric limitation, not a missing line of code.

Define the **Dirichlet energy** as the integrated squared change,

$$
E(f)=\frac12\int_{\text{mesh}}\|\nabla f\|^2\,dA.
$$

The integral sums local change over surface area. On our triangle, area is one and $\|\nabla f\|^2=10$, so the energy is five. The factor $1/2$ is a declared convention that makes the derivative of a quadratic expression cleaner.

The same energy can be assembled into a **stiffness matrix** $S$:

$$
E(f)=\frac12 f^{\mathsf T}Sf,\qquad
S_{ij}=\int\nabla\lambda_i\cdot\nabla\lambda_j\,dA.
$$

Here $f$ now denotes the vector of vertex coefficients. The continuous field and its coefficient vector are related but different representations. Each triangle contributes a small matrix; adding contributions at shared vertices assembles the global operator.

For an edge $ij$, the usual cotangent weight is $w_{ij}=(\cot\alpha+\cot\beta)/2$, where the angles lie opposite that edge in its incident triangles. At a boundary only one triangle contributes. Set $S_{ij}=-w_{ij}$ off the diagonal and $S_{ii}=\sum_jw_{ij}$. For our single triangle, the edge weights on $AB$, $AC$, $BC$ are $1/4$, $1$, $0$. Therefore

$$
S=\begin{pmatrix}5/4&-1/4&-1\\-1/4&1/4&0\\-1&0&1\end{pmatrix},
\qquad \frac12(0,2,3)S(0,2,3)^{\mathsf T}=5.
$$

This agrees with the direct gradient integral. It is a useful check on signs and factors of two. The matrix is symmetric positive semidefinite, and constants lie in its null space when the assembled boundary treatment permits them. A negative individual cotangent weight can occur on obtuse configurations; it does not alone prove that the assembled finite-element energy is negative. It does, however, matter to positivity and maximum-principle properties of algorithms that interpret the weights as local averaging weights.

### Storage coefficients are not yet a continuous inner product

A second matrix, the **mass matrix** $M$, represents the field inner product. For piecewise-linear basis functions, $M_{ij}=\int\lambda_i\lambda_j\,dA$. A common diagonal approximation, called **lumped mass**, allocates one third of each triangle's area to each incident vertex. On our unit-area triangle, $M=\operatorname{diag}(1/3,1/3,1/3)$ under that approximation. State which mass convention is used; different conventions change the discrete operator.

With the nonnegative stiffness convention above, a discrete Laplace operator approximating $\Delta$ is $-M^{-1}S$. Heat diffusion with coefficient $\kappa>0$ becomes

$$
M\dot f=-\kappa Sf.
$$

The sign makes energy decay. In fact,

$$
\frac{d}{dt}\frac12 f^{\mathsf T}Mf
=-\kappa f^{\mathsf T}Sf\le0
$$

when the matrices are fixed and the boundary treatment supplies no external input. A reversed sign produces growth instead of smoothing. Multiplying by $M^{-1}$ also shows why $M^{-1}S$ need not be symmetric under an ordinary coefficient dot product even though it is self-adjoint under $\langle u,v\rangle_M=u^{\mathsf T}Mv$. “Self-adjoint” means that moving the operator from one inner-product argument to the other leaves the measurement unchanged. This is the earlier metric lesson applied to fields.

Boundary conditions remain part of the problem: fixing boundary values, prescribing flux, and leaving a boundary natural are different models. A cotangent formula alone does not choose among them. The corresponding graphics chapter develops mesh processing; the mathematics needed to interpret its coefficients and energy is now present here.

### Positive energy and staying within a range are different guarantees

A harmonic interior value solves $\sum_jw_{ij}(f_i-f_j)=0$. If the weights are nonnegative and their sum is positive, rearrange it as $f_i=\sum_j[w_{ij}/\sum_kw_{ik}]f_j$. The coefficients are then nonnegative and sum to one, so the value lies between its neighbors' minimum and maximum. This local averaging property helps establish a discrete maximum principle: with suitable connectedness and fixed boundary data, an interior solution cannot create a new extreme. A positive-semidefinite energy matrix alone does not prove that property when individual weights can be negative.

This distinction motivates research beyond a textbook cotangent formula. Sharp and Crane's *A Laplacian for Nonmanifold Triangle Meshes* constructs an operator with nonnegative weights, including at boundaries, using a tufted cover and intrinsic Delaunay triangulation. A cover makes local sheets available for the construction; intrinsic triangulation uses the surface's metric rather than requiring every edge to be a straight segment in ambient space. The authors report improved behavior for several geometry-processing tasks. That is a specific operator and evidence set, not a universal guarantee for every mesh solver. [Authors' project and paper](https://www.cs.cmu.edu/~kmcrane/Projects/NonmanifoldLaplace/index.html)

The gradient and cotangent derivation was directly checked against *Polygon Mesh Processing*, physical PDF pages 54–58, printed pages 42–46. Its text contains embedded form-feed glyphs, so cached form-feed segments are not reliable physical page numbers; these citations were verified against the original PDF page objects.

## Jacobians, curvature, and local approximation
### The Jacobian: the matrix of local change

We now reach one of the central ideas of the entire graphics curriculum.

For:

$$
F:\mathbb R^n\rightarrow\mathbb R^m,
$$

write the output components as:

$$
F(\mathbf x)= \begin{bmatrix} f_1(x_1,\ldots,x_n)\\ f_2(x_1,\ldots,x_n)\\ \vdots\\ f_m(x_1,\ldots,x_n) \end{bmatrix}.
$$

The [[Chapter Edition/01 - Computing Foundations/21 - Multivariable Derivatives and Sensitivity#Jacobians, curvature, and local approximation|Jacobian matrix]] is:

$$
\boxed{ J_F(\mathbf x)= \begin{bmatrix} \frac{\partial f_1}{\partial x_1}&\cdots& \frac{\partial f_1}{\partial x_n}\\ \vdots&\ddots&\vdots\\ \frac{\partial f_m}{\partial x_1}&\cdots& \frac{\partial f_m}{\partial x_n} \end{bmatrix} }.
$$

The shape is always:

$$
\boxed{m\times n}
$$

with the output dimensions as rows and input dimensions as columns.

Each entry:

$$
J\_{ij}=\frac{\partial f_i}{\partial x_j}
$$

answers: How sensitive is output component $i$ to input component $j$?

This is not a storage convention. It is a mathematical shape convention for column-vector inputs. A C++ implementation may choose any suitable physical memory layout while still respecting these mathematical indices.

#### Worked example: a nonlinear 2D transformation

Let:

$$
F(x,y)= \begin{bmatrix} x^2+y\\ xy \end{bmatrix}.
$$

The first output is:

$$
f_1(x,y)=x^2+y.
$$

The second output is:

$$
f_2(x,y)=xy.
$$

Differentiate each output with respect to each input:

$$
\frac{\partial f_1}{\partial x}=2x,\qquad \frac{\partial f_1}{\partial y}=1,
$$

$$
\frac{\partial f_2}{\partial x}=y,\qquad \frac{\partial f_2}{\partial y}=x.
$$

Therefore:

$$
\boxed{ J_F(x,y)= \begin{bmatrix} 2x&1\\ y&x \end{bmatrix} }.
$$

At $(2,3)$:

$$
J_F(2,3)= \begin{bmatrix} 4&1\\ 3&2 \end{bmatrix}.
$$

The original function produces:

$$
F(2,3)= \begin{bmatrix}7\\6\end{bmatrix}.
$$

Now perturb the input by:

$$
\Delta\mathbf x= \begin{bmatrix} 0.01\\-0.02 \end{bmatrix}.
$$

The Jacobian predicts:

$$
\Delta F\approx \begin{bmatrix} 4&1\\ 3&2 \end{bmatrix} \begin{bmatrix} 0.01\\-0.02 \end{bmatrix} = \begin{bmatrix} 0.02\\-0.01 \end{bmatrix}.
$$

Thus the predicted new output is:

$$
F(2.01,2.98)\approx \begin{bmatrix} 7.02\\5.99 \end{bmatrix}.
$$

Compute the exact values:

$$
F(2.01,2.98)= \begin{bmatrix} (2.01)^2+2.98\\ (2.01)(2.98) \end{bmatrix} = \begin{bmatrix} 7.0201\\ 5.9898 \end{bmatrix}.
$$

The Jacobian correctly predicts the leading changes. The remaining differences arise from second-order terms.

#### The Jacobian is not a global transformation matrix

The Jacobian depends on the point at which it is evaluated.

For the example:

$$
J_F(x,y)= \begin{bmatrix}2x&1\\y&x\end{bmatrix},
$$

changing the input changes the Jacobian.

A fixed linear transformation $F(\mathbf x)=A\mathbf x$ has:

$$
J_F(\mathbf x)=A
$$

everywhere.

An affine transformation $F(\mathbf x)=A\mathbf x+\mathbf b$ also has Jacobian $A$, because translation does not affect local displacement.

A nonlinear transformation generally has a different Jacobian at every point.

So you can think of the Jacobian as the best first-order linear transformation at a specific location.

The rigorous statement is:

$$
F(\mathbf x+\mathbf h) = F(\mathbf x)+J_F(\mathbf x)\mathbf h+\mathbf r(\mathbf h)
$$

where:

$$
\lim\_{\|\mathbf h\|\to0} \frac{\|\mathbf r(\mathbf h)\|}{\|\mathbf h\|}=0.
$$

This definition is known as Fréchet differentiability in Euclidean spaces.

It is stronger than merely requiring directional derivatives.

#### How the Jacobian transforms geometry

Imagine drawing a tiny circle around some input point in 2D.

A nonlinear transformation may distort this circle into a complicated curved shape. But as we shrink the circle, the first-order output shape approaches the image of that circle under the Jacobian.

A 2D linear map generally transforms circles into ellipses.

[Visualizing the Singular Value Decomposition (SVD) · Árni Sturluson](https://images.openai.com/static-rsc-4/snh-e19GotRQocePDuoqQRldKi909Wv3Ghn6oy7qF4vh-DmEyMU0aOYOrfy6z9hhcRviS00W0fgYnoU6OnDjUJDqI_E0F7jsn5R4hOfc-E6wNdPAnQTW2qd7NnlpFlxmOrTHzMeuPAv3DS5XmKmuNKw19bkuYGpa9H3DCFJSaeA?purpose=inline)

[surluson.github.io](https://surluson.github.io/blog/2021/01/11/visualizing-svd/?utm_source=chatgpt.com)

If:

$$
J=U\Sigma V^T
$$

is the singular-value decomposition of the Jacobian, then the right singular vectors in $V$ give input directions, the singular values give local stretch factors, and the left singular vectors in $U$ give the corresponding output directions.

This connects Phase 8's SVD directly to calculus.

For example, a $2\times3$ camera projection Jacobian maps small 3D displacements into a 2D image. Its rank is at most two, so at least one 3D direction lies in its nullspace.

For a pinhole camera, that null direction corresponds locally to motion along the ray passing through the camera centre and the 3D point. Moving the point along that ray changes depth but not image coordinates.

The Jacobian therefore reveals not only sensitivity but also which changes are invisible to a measurement.

This becomes important in calibration, reconstruction, bundle adjustment, and observability analysis.

#### The determinant also changes area

For a differentiable coordinate map $\Phi(u,v)=(x(u,v),y(u,v))$, a tiny rectangle is locally mapped to a parallelogram whose area is multiplied by $|\det D\Phi|$. The determinant sign records orientation; ordinary area uses its absolute value. The corresponding change-of-variables formula is
$$
\iint_D g(x,y)\,dx\,dy
=
\iint_{\Phi^{-1}(D)}g(\Phi(u,v))\,|\det D\Phi(u,v)|\,du\,dv.
$$
For $\Phi(u,v)=(2u+v,u-v)$, $\det D\Phi=-3$: area expands by three and orientation reverses; the inverse map contributes $1/3$. State the mapping direction before choosing the determinant or its reciprocal. MIT 18.02SC Session 101 Clip 1 is a selected 15:45 official-transcript excerpt from Lecture 35, not the full lecture; it reviews polar and general substitutions. [MIT 18.02SC Session 101, Clip 1](https://ocw.mit.edu/courses/18-02sc-multivariable-calculus-fall-2010/resources/clip-1-double-integrals-jacobian-triple-integrals/)

### Deriving the perspective-projection Jacobian

We will now derive the result used throughout camera geometry, 3D Gaussian splatting, visual reconstruction, and many inverse-rendering systems.

#### Establish the coordinate convention

For this mathematical derivation, use a camera-space coordinate system in which:

- $X$ increases to the right.
- $Y$ increases downward.
- $Z>0$ represents depth in front of the camera.
- $u,v$ are image coordinates in pixels.
- $f_x,f_y$ are focal lengths measured in pixels.
- $c_x,c_y$ locate the principal point in pixels.

This positive-$Z$, positive-downward-$Y$ convention is convenient for image geometry. It is not the same as every rendering API's view-space convention.

The ideal pinhole projection equations are:

$$
\boxed{ u=f_x\frac XZ+c_x }
$$

and:

$$
\boxed{ v=f_y\frac YZ+c_y }.
$$

The terms $X/Z$ and $Y/Z$ arise from similar triangles.

They explain perspective foreshortening: doubling a point's depth while keeping its lateral displacement unchanged halves its displacement from the image centre.

The mapping is:

$$
\pi:\mathbb R^3\rightarrow\mathbb R^2.
$$

Its Jacobian must therefore be a $2\times3$ matrix.

#### Differentiate the horizontal coordinate

Start with:

$$
u(X,Y,Z)=f_x\frac XZ+c_x.
$$

Differentiate with respect to $X$, holding $Y,Z$ fixed:

$$
\boxed{\frac{\partial u}{\partial X}=\frac{f_x}{Z}}.
$$

Differentiate with respect to $Y$:

$$
\boxed{\frac{\partial u}{\partial Y}=0}.
$$

This occurs because horizontal pixel position does not depend directly on vertical coordinate $Y$ in this ideal pinhole model without skew or distortion.

Differentiate with respect to $Z$:

$$
u=f_xXZ^{-1}+c_x.
$$

Therefore:

$$
\boxed{ \frac{\partial u}{\partial Z} =-\frac{f_xX}{Z^2} }.
$$

The negative sign is physically meaningful. Increasing depth moves the projection toward the principal point when the point is laterally offset.

#### Differentiate the vertical coordinate

Similarly:

$$
v(X,Y,Z)=f_y\frac YZ+c_y.
$$

The derivatives are:

$$
\frac{\partial v}{\partial X}=0,
$$

$$
\frac{\partial v}{\partial Y}=\frac{f_y}{Z},
$$

$$
\frac{\partial v}{\partial Z}=-\frac{f_yY}{Z^2}.
$$

Assemble the Jacobian, using one output coordinate per row:

$$
\boxed{ J\_\pi(X,Y,Z)= \begin{bmatrix} \frac{f_x}{Z}&0&-\frac{f_xX}{Z^2}\\[4pt] 0&\frac{f_y}{Z}&-\frac{f_yY}{Z^2} \end{bmatrix} }.
$$

Every term now has a clear interpretation.

The $f_x/Z$ and $f_y/Z$ terms convert lateral 3D movement into pixel movement.

The terms containing $X/Z^2$ and $Y/Z^2$ describe how changes in depth alter the apparent lateral position.

The zeros arise from the ideal projection's independence between horizontal and vertical image coordinates.

All entries have units of pixels per metre when $X,Y,Z$ are measured in metres.

#### A fully worked numerical example

Let the camera intrinsics and point be:

$$
f_x=800,\quad f_y=820, \quad c_x=640,\quad c_y=360,
$$

$$
\mathbf p= \begin{bmatrix} 0.2\\0.1\\2.0 \end{bmatrix}\mathrm{m}.
$$

The projected image coordinates are:

$$
u=800\left(\frac{0.2}{2}\right)+640=720,
$$

$$
v=820\left(\frac{0.1}{2}\right)+360=401.
$$

Therefore:

$$
\boxed{\pi(\mathbf p)=(720,401)\text{ pixels}}.
$$

Now evaluate the Jacobian:

$$
\frac{f_x}{Z}=\frac{800}{2}=400,
$$

$$
-\frac{f_xX}{Z^2} =-\frac{800(0.2)}{4}=-40,
$$

$$
\frac{f_y}{Z}=\frac{820}{2}=410,
$$

$$
-\frac{f_yY}{Z^2} =-\frac{820(0.1)}4=-20.5.
$$

Thus:

$$
\boxed{ J\_\pi= \begin{bmatrix} 400&0&-40\\ 0&410&-20.5 \end{bmatrix} }.
$$

Suppose the point moves by:

$$
\Delta\mathbf p= \begin{bmatrix} 0.001\\ -0.002\\ 0.01 \end{bmatrix}\mathrm m.
$$

The predicted pixel displacement is:

$$
\Delta\mathbf s \approx J\_\pi\Delta\mathbf p.
$$

Multiplying:

$$
\Delta u =400(0.001)-40(0.01)=0,
$$

$$
\Delta v =410(-0.002)-20.5(0.01)=-1.025.
$$

Therefore:

$$
\boxed{ \pi(\mathbf p+\Delta\mathbf p) \approx(720,399.975) }.
$$

We can calculate the actual projection at the new point:

$$
\mathbf p'=(0.201,0.098,2.01).
$$

Then:

$$
u'=800\frac{0.201}{2.01}+640=720,
$$

$$
v'=820\frac{0.098}{2.01}+360 \approx399.9800995.
$$

Our first-order prediction missed the vertical coordinate by only:

$$
\boxed{0.0050995\text{ pixels}}.
$$

The original function is nonlinear, but for this small perturbation, the Jacobian provides a close approximation.

#### Why the approximation breaks for large depth changes

The exact horizontal displacement is:

$$
\Delta u\_{\text{exact}} = f_x\left( \frac{X+\Delta X}{Z+\Delta Z} -\frac XZ \right).
$$

Combine the fractions:

$$
\Delta u\_{\text{exact}} = f_x\frac{Z\Delta X-X\Delta Z}{Z(Z+\Delta Z)}.
$$

The Jacobian prediction is:

$$
\Delta u\_{\text{linear}} = f_x\frac{Z\Delta X-X\Delta Z}{Z^2}.
$$

Subtract:

$$
\boxed{ \Delta u\_{\text{exact}}-\Delta u\_{\text{linear}} = -\frac{f_x(Z\Delta X-X\Delta Z)\Delta Z} {Z^2(Z+\Delta Z)} }.
$$

This is an exact expression for the horizontal [[Chapter Edition/01 - Computing Foundations/21 - Multivariable Derivatives and Sensitivity#Jacobians, curvature, and local approximation|linearization error]], provided both projections are defined.

It reveals several important things.

If $\Delta Z=0$, then the projection is exactly linear in lateral displacement, so the error is zero.

If the perturbation follows the original viewing ray, meaning $Z\Delta X=X\Delta Z$, then the projected horizontal coordinate does not change at all. The Jacobian also predicts zero change.

If $|\Delta Z|/Z$ is small, the linearization error tends to be small relative to the first-order displacement.

But as $Z+\Delta Z\to0$, the exact projection becomes singular and the local approximation can fail catastrophically.

This is why a screen-space footprint cannot always be trusted for Gaussians or geometry extending near or through the camera plane.

### The chain rule for Jacobians

The scalar chain rule stated:

$$
(f\circ g)'=f'(g)g'.
$$

For multivariable functions, scalar multiplication becomes matrix multiplication.

Let:

$$
F:\mathbb R^n\rightarrow\mathbb R^m,
$$

$$
G:\mathbb R^m\rightarrow\mathbb R^k.
$$

The composition is:

$$
H=G\circ F:\mathbb R^n\rightarrow\mathbb R^k.
$$

The Jacobian chain rule is:

$$
\boxed{ J_H(\mathbf x) = J_G(F(\mathbf x))J_F(\mathbf x) }.
$$

Notice the multiplication order. The inner Jacobian acts first on the input perturbation; the outer Jacobian acts on the resulting intermediate perturbation.

##### Why matrix multiplication appears

For a small input displacement $\Delta\mathbf x$, the inner function changes approximately by:

$$
\Delta\mathbf y\approx J_F\Delta\mathbf x.
$$

The outer function responds with:

$$
\Delta\mathbf z\approx J_G\Delta\mathbf y.
$$

Substitute:

$$
\Delta\mathbf z \approx J_G(J_F\Delta\mathbf x).
$$

By associativity:

$$
\boxed{ \Delta\mathbf z \approx(J_GJ_F)\Delta\mathbf x }.
$$

The resulting composite Jacobian must therefore be $J_GJ_F$.

Check its dimensions:

$$
J_G\in\mathbb R^{k\times m}, \qquad J_F\in\mathbb R^{m\times n}.
$$

Their product is $k\times n$, exactly what is required for a mapping from $n$ inputs to $k$ outputs.

Dimension checking is one of the easiest ways to catch a wrongly ordered derivative expression.

#### Camera transformation chain

Suppose a world-space point is transformed to camera space using:

$$
\mathbf p_c=R\mathbf p_w+\mathbf t,
$$

where $R$ is a $3\times3$ rotation matrix and $\mathbf t$ is a translation vector.

Because this mapping is affine:

$$
\frac{\partial\mathbf p_c}{\partial\mathbf p_w}=R.
$$

Then the point is projected:

$$
\mathbf s=\pi(\mathbf p_c).
$$

By the chain rule:

$$
\boxed{ \frac{\partial\mathbf s}{\partial\mathbf p_w} = J\_\pi(\mathbf p_c)R }.
$$

This matrix is $2\times3$.

Its entries describe how a world-space motion affects image-space position after accounting for camera orientation.

Translation affects the camera-space point at which $J\_\pi$ is evaluated, but the derivative of the affine world-to-camera mapping with respect to point position is still $R$.

##### Including object transforms

Suppose:

$$
\mathbf p_w=A\mathbf p_o+\mathbf b,
$$

where $A$ is the linear part of an object transform.

Then:

$$
\boxed{ \frac{\partial\mathbf s}{\partial\mathbf p_o} = J\_\pi(\mathbf p_c)RA }.
$$

Each matrix acts on perturbations in its own coordinate space.

This is precisely why tracking coordinate spaces matters. Multiplying otherwise valid Jacobians in the wrong order, or using a Jacobian evaluated in the wrong frame, produces incorrect derivatives.

##### Extending the chain to camera-pose optimization

Suppose we perturb a camera-space point using a small translation $\delta\mathbf t$ and a small rotation vector $\delta\boldsymbol\omega$.

For a left-applied perturbation to the camera-space point:

$$
\mathbf p_c' = \exp([\delta\boldsymbol\omega]\_\times)\mathbf p_c +\delta\mathbf t.
$$

Here $[\mathbf a]\_\times$ is the skew-symmetric matrix satisfying:

$$
[\mathbf a]\_\times\mathbf b=\mathbf a\times\mathbf b.
$$

For small rotation:

$$
\exp([\delta\boldsymbol\omega]\_\times) \approx I+[\delta\boldsymbol\omega]\_\times.
$$

Therefore:

$$
\begin{aligned} \delta\mathbf p_c &\approx\delta\boldsymbol\omega\times\mathbf p_c +\delta\mathbf t\\ &=-[\mathbf p_c]\_\times\delta\boldsymbol\omega +\delta\mathbf t. \end{aligned}
$$

If we parameterize the perturbation as:

$$
\delta\boldsymbol\xi= \begin{bmatrix} \delta\boldsymbol\omega\\ \delta\mathbf t \end{bmatrix},
$$

then:

$$
\frac{\partial\mathbf p_c} {\partial\delta\boldsymbol\xi} = \begin{bmatrix} -[\mathbf p_c]\_\times&I_3 \end{bmatrix}.
$$

The projected-point Jacobian with respect to these six perturbation parameters becomes:

$$
\boxed{ J\_{\text{pose}} = J\_\pi \begin{bmatrix} -[\mathbf p_c]\_\times&I_3 \end{bmatrix} }.
$$

This is a $2\times6$ Jacobian.

It is directly relevant to visual odometry, SLAM, bundle adjustment, and camera tracking.

The formula depends on the chosen perturbation convention. Right-applied perturbations, camera-to-world parameterizations, and different twist ordering produce different expressions. One must derive the Jacobian for the exact pose convention rather than transplanting a formula blindly.

### Hessians and second derivatives

First derivatives describe local slope. Second derivatives describe how that slope changes.

For a scalar function $f:\mathbb R^n\to\mathbb R$, the Hessian matrix is:

$$
\boxed{ H_f(\mathbf x)= \begin{bmatrix} \frac{\partial^2f}{\partial x_1^2} & \cdots & \frac{\partial^2f}{\partial x_1\partial x_n} \\ \vdots&\ddots&\vdots\\ \frac{\partial^2f}{\partial x_n\partial x_1} & \cdots & \frac{\partial^2f}{\partial x_n^2} \end{bmatrix} }.
$$

It is the Jacobian of the gradient, using the ordinary Euclidean coordinate convention.

Its shape is:

$$
n\times n.
$$

#### A complete numerical example

Consider:

$$
f(x,y)=x^2+2xy+3y^2.
$$

The gradient is:

$$
\nabla f= \begin{bmatrix} 2x+2y\\ 2x+6y \end{bmatrix}.
$$

Differentiate each gradient component again:

$$
\frac{\partial^2f}{\partial x^2}=2, \qquad \frac{\partial^2f}{\partial x\partial y}=2,
$$

$$
\frac{\partial^2f}{\partial y\partial x}=2, \qquad \frac{\partial^2f}{\partial y^2}=6.
$$

Thus:

$$
\boxed{ H_f= \begin{bmatrix} 2&2\\ 2&6 \end{bmatrix} }.
$$

Because this function is quadratic, its Hessian is constant everywhere.

##### Why the Hessian is often symmetric

Under the usual sufficient assumption that the second partial derivatives are continuous near a point, mixed partials commute:

$$
\frac{\partial^2f}{\partial x_i\partial x_j} = \frac{\partial^2f}{\partial x_j\partial x_i}.
$$

This is commonly called Clairaut's or Schwarz's theorem.

Therefore:

$$
H_f=H_f^T.
$$

Symmetry is not an unconditional rule for every pathological function possessing some second partial derivatives. The required regularity assumptions matter.

A sufficiently smooth real-valued function has a symmetric Hessian. Small floating-point asymmetries in a computed Hessian may arise from numerical approximation, but large asymmetries often indicate an implementation error or inconsistent derivative calculations.

#### Directional curvature

For a unit vector $\mathbf u$, consider the one-dimensional function:

$$
g(t)=f(\mathbf x+t\mathbf u).
$$

Its first derivative is:

$$
g'(t)=\nabla f(\mathbf x+t\mathbf u)^T\mathbf u.
$$

Differentiating again:

$$
g''(t)=\mathbf u^TH_f(\mathbf x+t\mathbf u)\mathbf u.
$$

At $t=0$:

$$
\boxed{ g''(0)=\mathbf u^TH_f(\mathbf x)\mathbf u }.
$$

This quantity is the second directional derivative.

It measures how quickly the slope changes when moving along direction $\mathbf u$.

The Hessian therefore generalizes scalar curvature information to all possible directions.

##### Eigenvalues of the Hessian

For a symmetric Hessian, eigenvectors identify directions in which the quadratic curvature model acts without coupling to other eigenvector directions.

If:

$$
H\mathbf v_i=\lambda_i\mathbf v_i,
$$

then the second directional derivative along a unit eigenvector $\mathbf v_i$ is:

$$
\mathbf v_i^TH\mathbf v_i=\lambda_i.
$$

A positive eigenvalue means locally upward-bending quadratic behavior in that direction. A negative eigenvalue means downward-bending behavior.

At a stationary point, where $\nabla f=0$, the Hessian provides a familiar classification when it is definite:

| Hessian at a stationary point                     | Local interpretation                  |
| ------------------------------------------------- | ------------------------------------- |
| All eigenvalues positive                          | Strict local minimum                  |
| All eigenvalues negative                          | Strict local maximum                  |
| Both positive and negative eigenvalues            | Saddle point                          |
| One or more zero eigenvalues, without mixed signs | Second-order test may be inconclusive |

For our example:

$$
H= \begin{bmatrix}2&2\\2&6\end{bmatrix},
$$

the characteristic polynomial is:

$$
\det(H-\lambda I) =(2-\lambda)(6-\lambda)-4 =\lambda^2-8\lambda+8.
$$

Solving:

$$
\lambda=4\pm2\sqrt2.
$$

Both eigenvalues are positive, approximately $1.1716$ and $6.8284$. The quadratic function bends upward in every nonzero direction.

The ratio of these eigenvalues indicates how anisotropic the curvature is. This contributes to optimization conditioning: an algorithm using one uniform step size may make progress very differently along steep and shallow directions.

#### Hessian versus geometric surface curvature

The Hessian describes second derivatives in the chosen coordinate system. It does not automatically equal every differential-geometric notion of curvature.

For a surface written as $z=f(x,y)$, physical surface curvature also depends on the local surface metric and gradient.

For a signed-distance function, the Hessian helps describe how normals change, and its tangential restriction relates to surface curvature under appropriate assumptions.

For a nonlinear change of coordinates, the ordinary Hessian does not transform merely by a simple basis-change formula; additional terms from second derivatives of the coordinate map appear.

This is one reason differential geometry distinguishes ordinary coordinate Hessians from covariant Hessians on manifolds.

### Taylor series in one variable

We previously derived:

$$
f(a+h)=f(a)+f'(a)h+o(|h|).
$$

But a nonlinear function may need more than a straight-line approximation.

The next term accounts for curvature:

$$
f(a+h)\approx f(a)+f'(a)h+\frac12f''(a)h^2.
$$

Why the factor $1/2$?

Imagine a quadratic approximation:

$$
P_2(h)=c_0+c_1h+c_2h^2.
$$

We want the polynomial and the original function to have the same value, first derivative, and second derivative at $h=0$.

Matching values:

$$
P_2(0)=c_0=f(a).
$$

Matching first derivatives:

$$
P_2'(0)=c_1=f'(a).
$$

Matching second derivatives:

$$
P_2''(0)=2c_2=f''(a).
$$

Therefore:

$$
c_2=\frac{f''(a)}2.
$$

For a cubic coefficient, the third derivative of $c_3h^3$ at zero is $3!c_3$. This gives:

$$
c_3=\frac{f'''(a)}{3!}.
$$

The general degree-$n$ Taylor polynomial is:

$$
\boxed{ T_n(a+h)= \sum\_{k=0}^{n} \frac{f^{(k)}(a)}{k!}h^k }.
$$

The factorial appears because differentiating $h^k$ exactly $k$ times produces $k!$.

#### Maclaurin series

A Taylor expansion centered at zero is called a Maclaurin expansion.

Important examples are:

$$
e^x= 1+x+\frac{x^2}{2!}+\frac{x^3}{3!}+\cdots,
$$

$$
\sin x= x-\frac{x^3}{3!}+\frac{x^5}{5!}-\cdots,
$$

$$
\cos x= 1-\frac{x^2}{2!}+\frac{x^4}{4!}-\cdots.
$$

For $|x|<1$:

$$
\frac1{1-x} = 1+x+x^2+x^3+\cdots.
$$

The last expression is a geometric series and provides a useful example of a finite radius of convergence.

These formulas are not merely theoretical. Polynomial approximations are used in fast numerical functions, small-angle calculations, embedded systems, shader approximations, and numerically stable formulations near special values.

For instance:

$$
\sin x\approx x-\frac{x^3}{6}
$$

is often much more accurate near zero than the crude approximation $\sin x\approx x$.

##### A Taylor series is not always equal to its function

If a function possesses derivatives of every order, one can formally construct its Taylor coefficients. That does not guarantee that the resulting infinite Taylor series equals the function near the expansion point.

A classical example is:

$$
f(x)= \begin{cases} e^{-1/x^2},&x\ne0,\\ 0,&x=0. \end{cases}
$$

This function is infinitely differentiable, and all derivatives at zero vanish. Its Taylor series at zero is identically zero, yet the function is positive at every nonzero real $x$.

Such functions are smooth but not analytic at the point.

For numerical engineering, always distinguish a finite local approximation, convergence of an infinite series, and equality of the converged series to the target function.

### Multivariable Taylor approximation

The one-variable second-order model is:

$$
f(a+h) \approx f(a)+f'(a)h+\frac12f''(a)h^2.
$$

For a function of multiple variables, replace the first derivative with the gradient and the second derivative with the Hessian:

$$
\boxed{ f(\mathbf x+\mathbf h) \approx f(\mathbf x) + \nabla f(\mathbf x)^T\mathbf h + \frac12\mathbf h^TH_f(\mathbf x)\mathbf h }.
$$

The three pieces have distinct roles.

The first term is the current function value.

The second term predicts the first-order change.

The third term corrects that prediction using the local curvature.

#### Deriving the quadratic form

Suppose $f=f(x,y)$, and let:

$$
\mathbf h= \begin{bmatrix}h_x\\h_y\end{bmatrix}.
$$

The second-order part is:

$$
\frac12 \begin{bmatrix}h_x&h_y\end{bmatrix} \begin{bmatrix} f\_{xx}&f\_{xy}\\ f\_{yx}&f\_{yy} \end{bmatrix} \begin{bmatrix}h_x\\h_y\end{bmatrix}.
$$

Multiplying gives:

$$
\frac12\left( f\_{xx}h_x^2+ f\_{xy}h_xh_y+ f\_{yx}h_yh_x+ f\_{yy}h_y^2 \right).
$$

When mixed derivatives agree:

$$
\boxed{ \frac12\left( f\_{xx}h_x^2+ 2f\_{xy}h_xh_y+ f\_{yy}h_y^2 \right) }.
$$

The cross term $2f\_{xy}h_xh_y$ describes coupling between the two perturbation directions.

This is the second-order equivalent of how an off-diagonal matrix element couples coordinates in linear algebra.

#### Complete numerical trace

Use:

$$
f(x,y)=x^2+2xy+3y^2.
$$

Choose:

$$
\mathbf x= \begin{bmatrix} 1\\-1 \end{bmatrix}, \qquad \mathbf h= \begin{bmatrix} 0.1\\0.2 \end{bmatrix}.
$$

First compute the original value:

$$
f(1,-1)=1-2+3=2.
$$

The gradient is:

$$
\nabla f= \begin{bmatrix} 2x+2y\\ 2x+6y \end{bmatrix} = \begin{bmatrix} 0\\-4 \end{bmatrix}.
$$

The first-order correction is:

$$
\nabla f^T\mathbf h =0(0.1)-4(0.2)=-0.8.
$$

The Hessian is:

$$
H= \begin{bmatrix} 2&2\\2&6 \end{bmatrix}.
$$

Compute:

$$
H\mathbf h= \begin{bmatrix} 2(0.1)+2(0.2)\\ 2(0.1)+6(0.2) \end{bmatrix} = \begin{bmatrix} 0.6\\1.4 \end{bmatrix}.
$$

Then:

$$
\mathbf h^TH\mathbf h = 0.1(0.6)+0.2(1.4) =0.34.
$$

So the second-order correction is:

$$
\frac12\mathbf h^TH\mathbf h=0.17.
$$

The Taylor model predicts:

$$
f(\mathbf x+\mathbf h) \approx2-0.8+0.17=\boxed{1.37}.
$$

Compute the exact value at $(1.1,-0.8)$:

$$
f(1.1,-0.8) =1.21-1.76+1.92 =\boxed{1.37}.
$$

The second-order model is exactly correct here because the original function is a degree-two polynomial. There are no third- or higher-order terms to neglect.

##### Why this matters to optimization

Gradient descent uses first-order information to select a direction.

Newton's method uses Hessian information to account for curvature.

The local quadratic optimization model is:

$$
m(\mathbf h)= f(\mathbf x)+ \nabla f^T\mathbf h+ \frac12\mathbf h^TH\mathbf h.
$$

Differentiate with respect to $\mathbf h$, assuming symmetric $H$:

$$
\nabla\_{\mathbf h}m=\nabla f+H\mathbf h.
$$

Setting this equal to zero gives:

$$
\boxed{H\mathbf h=-\nabla f}.
$$

If $H$ is positive definite, this produces the unique minimizer of the quadratic model.

This is the origin of the Newton step. We will examine convergence, globalization, indefinite Hessians, and alternative methods fully in M053.

### First-order linearization

Linearization means replacing a nonlinear system near an operating point with its first-order affine approximation.

For:

$$
F:\mathbb R^n\rightarrow\mathbb R^m,
$$

around $\mathbf x_0$:

$$
\boxed{ F(\mathbf x)\approx F(\mathbf x_0)+ J_F(\mathbf x_0)(\mathbf x-\mathbf x_0) }.
$$

This expression is affine in $\mathbf x$.

An important distinction: the derivative $J_F$ is linear, but the tangent approximation of the function includes the offset $F(\mathbf x_0)$, so it is generally affine.

This distinction matters when discussing coordinate origins and transformed tangent spaces.

#### Local linear models in dynamic systems

Suppose a nonlinear continuous-time system obeys:

$$
\dot{\mathbf x}=F(\mathbf x,\mathbf u),
$$

where $\mathbf x$ is the system state and $\mathbf u$ is a control input.

Around an operating point $(\mathbf x_0,\mathbf u_0)$:

$$
F(\mathbf x,\mathbf u) \approx F(\mathbf x_0,\mathbf u_0) +A\delta\mathbf x+B\delta\mathbf u,
$$

where:

$$
A=\left.\frac{\partial F}{\partial\mathbf x}\right|\_0, \qquad B=\left.\frac{\partial F}{\partial\mathbf u}\right|\_0.
$$

If the operating point is an equilibrium satisfying $F(\mathbf x_0,\mathbf u_0)=0$, the perturbation dynamics become:

$$
\delta\dot{\mathbf x}\approx A\delta\mathbf x+B\delta\mathbf u.
$$

This connects nonlinear simulation to linear systems, eigenvalue stability analysis, control theory, and state estimation.

##### The Jacobian can expose instability

For a one-dimensional nonlinear dynamic system:

$$
\dot x=f(x),
$$

let $x^\ast$ be an equilibrium with $f(x^\ast)=0$.

For a small perturbation $\delta x$:

$$
\delta\dot x\approx f'(x^\ast)\delta x.
$$

The linearized solution is:

$$
\delta x(t)\approx\delta x(0)e^{f'(x^\ast)t}.
$$

If $f'(x^\ast)<0$, the perturbation decays locally.

If $f'(x^\ast)>0$, it grows locally.

If the derivative is zero, first-order linearization alone cannot decide stability.

In higher dimensions, the eigenvalues of the state Jacobian play a similar role. Eigenvalues with negative real parts indicate decay in the linearized continuous-time system; positive real parts indicate instability.

This is another direct connection between Phases 8 and 9.

### Error terms and the limits of local approximations

The usefulness of a local approximation depends on whether we can control its error.

Suppose:

$$
f(x+h)=f(x)+f'(x)h+r(h).
$$

Differentiability guarantees:

$$
r(h)=o(|h|).
$$

This means the error divided by $|h|$ tends to zero.

It does not automatically specify a useful numerical error bound at a particular finite $h$.

To obtain stronger statements, we need stronger smoothness assumptions.

#### Big-O and little-o are not interchangeable

We write:

$$
r(h)=O(h^2)
$$

when there exist constants $C>0$ and $\delta>0$ such that:

$$
|r(h)|\le C|h|^2
$$

whenever $0<|h|<\delta$.

We write:

$$
r(h)=o(h)
$$

when:

$$
\lim\_{h\to0}\frac{r(h)}{|h|}=0.
$$

A function can satisfy $o(h)$ without an $O(h^2)$ bound.

For example, $|h|^{3/2}=o(|h|)$, but it is not $O(h^2)$ near zero.

So differentiability alone does not guarantee quadratic first-order approximation error. Such a bound requires additional regularity, such as a locally Lipschitz-continuous gradient.

#### Error bound using a Hessian

For a twice continuously differentiable scalar function, if the Hessian operator norm satisfies:

$$
\|H_f(\mathbf z)\|\_2\le M
$$

throughout the line segment joining $\mathbf x$ and $\mathbf x+\mathbf h$, then the first-order Taylor error satisfies:

$$
\boxed{ \left| f(\mathbf x+\mathbf h) -f(\mathbf x) -\nabla f(\mathbf x)^T\mathbf h \right| \le \frac M2\|\mathbf h\|\_2^2 }.
$$

The constant $M$ bounds the maximum local curvature along the segment.

If a suitable third-derivative bound $K$ exists, the second-order approximation error can be bounded by a term of order:

$$
\frac K6\|\mathbf h\|^3,
$$

with the tensor norm and assumptions defined appropriately.

The key principle is that approximation validity depends on both perturbation size and local curvature.

A large perturbation in a nearly linear region may be acceptable. A tiny perturbation near a singularity may still be problematic.

#### Relative error versus absolute error

Absolute error is:

$$
E\_{\text{abs}}=|\hat y-y|.
$$

Relative error is:

$$
E\_{\text{rel}}=\frac{|\hat y-y|}{|y|},
$$

when $y\ne0$.

The difference matters whenever outputs vary greatly in magnitude.

An error of $0.01$ pixels may be negligible for some rendering tasks. An error of $0.01$ metres can be disastrous for a precision-measurement system.

Likewise, a tiny absolute error near a true value of zero can correspond to an enormous or undefined relative error.

For vectors, we often use norm-based errors:

$$
E=\|\hat{\mathbf y}-\mathbf y\|\_2.
$$

For multiple samples, additional metrics such as maximum error, RMS error, and percentile errors reveal different kinds of failure.

#### Conditioning versus approximation error

A local approximation may be mathematically accurate but still produce a sensitive output.

For a differentiable mapping:

$$
\Delta\mathbf y\approx J\Delta\mathbf x.
$$

A bound follows:

$$
\|\Delta\mathbf y\| \lesssim \|J\|\|\Delta\mathbf x\|.
$$

The operator norm $\|J\|$ measures the largest local amplification of input perturbations under the chosen norms.

If this norm is enormous, small input errors can produce large output changes.

Near a pinhole camera's $Z=0$ plane, the terms $f_x/Z$ and $f_xX/Z^2$ can grow without bound. The mapping becomes highly sensitive.

This is conditioning, a property of the mathematical problem, rather than necessarily a bug in the algorithm used to evaluate it.

Numerical stability is a separate issue: whether the computation introduces error beyond what the problem's sensitivity requires.

For engineering work, always distinguish mathematical approximation error, problem conditioning, and floating-point implementation error.

### Production C++23 implementation: a camera projection differential oracle

We now turn the mathematical framework into executable code.

The implementation below uses a standalone positive-forward camera convention and verifies both the analytical projection Jacobian and a second-order Taylor model.

It deliberately uses `double` for CPU-side numerical validation. A production renderer may use `float`, but a higher-precision reference implementation is useful for detecting numerical discrepancies.

The supplied source claims this program was compiled and executed as C++23 with optimization and strict warnings. That run has not been independently reproduced in this pass.

`phase9_m049_m050_demo.cpp`

Complete runnable reference

```cpp

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <stdexcept>

// ---------------------------------------------------------------------
// CANONICAL MATH TYPES
// ---------------------------------------------------------------------
// These are simplified teaching types, not replacements for KairoMath.
// The purpose is to make storage, indexing and arithmetic explicit.

struct Vec2 {
    double x{}, y{};
};

struct Vec3 {
    double x{}, y{}, z{};
};

// A 2x3 Jacobian has:
//   2 rows = output components (u, v)
//   3 cols = input components (X, Y, Z)
//
// Storage: std::array of contiguous row arrays.
// J[outputIndex][inputIndex] is the mathematical Jacobian entry.

using Matrix2x3 = std::array<std::array<double, 3>, 2>;

// ---------------------------------------------------------------------
// IDEAL PINHOLE CAMERA
// ---------------------------------------------------------------------

struct PinholeCamera {
    double fx{}, fy{};  // Focal lengths, in pixels.
    double cx{}, cy{};  // Principal point, in pixels.

    // Convention:
    //   X = right, Y = down, Z = forward.
    //   Z must be strictly positive.
    //
    // This camera operates on CAMERA-space positions, not world-space.
    // Lens distortion and clipping are intentionally excluded.

    [[nodiscard]]
    Vec2 project(const Vec3& p) const {
        if (!std::isfinite(p.x) ||
            !std::isfinite(p.y) ||
            !std::isfinite(p.z) ||
            !(p.z > 0.0))
            throw std::domain_error(
                "Projection requires finite point with Z > 0"
            );

        // Similar-triangle perspective projection.
        return {
            fx * (p.x / p.z) + cx,
            fy * (p.y / p.z) + cy
        };
    }

    [[nodiscard]]
    Matrix2x3 jacobian(const Vec3& p) const {
        if (!std::isfinite(p.x) ||
            !std::isfinite(p.y) ||
            !std::isfinite(p.z) ||
            !(p.z > 0.0))
            throw std::domain_error(
                "Jacobian requires finite point with Z > 0"
            );

        // Avoid repeated division by Z.
        const double invZ = 1.0 / p.z;
        const double invZ2 = invZ * invZ;

        // Row 0: derivatives of horizontal pixel coordinate u.
        // Row 1: derivatives of vertical pixel coordinate v.
        //
        // du/dX = fx/Z       du/dY = 0    du/dZ = -fx*X/Z^2
        // dv/dX = 0          dv/dY = fy/Z dv/dZ = -fy*Y/Z^2

        return {{
            {fx * invZ, 0.0, -fx * p.x * invZ2},
            {0.0, fy * invZ, -fy * p.y * invZ2}
        }};
    }
};

// ---------------------------------------------------------------------
// NUMERICAL JACOBIAN VALIDATION
// ---------------------------------------------------------------------

[[nodiscard]]
Vec3 shifted(Vec3 p, int axis, double displacement) {
    if (axis == 0)
        p.x += displacement;
    else if (axis == 1)
        p.y += displacement;
    else
        p.z += displacement;

    return p;
}

[[nodiscard]]
Matrix2x3 numericJacobian(
    const PinholeCamera& camera,
    const Vec3& p,
    double h
) {
    Matrix2x3 result{};

    // Compute each Jacobian column independently by perturbing
    // exactly one input coordinate at a time.
    //
    // This is central finite differentiation:
    //
    // dF/dx_i ~= [F(x + h*e_i) - F(x - h*e_i)] / (2*h)
    //
    // It is a verification oracle, not a universal differentiation
    // implementation. The chosen h must respect scale and domain.

    for (int axis = 0; axis < 3; ++axis) {
        const Vec2 plus =
            camera.project(shifted(p, axis, +h));

        const Vec2 minus =
            camera.project(shifted(p, axis, -h));

        result[0][axis] =
            (plus.x - minus.x) / (2.0 * h);

        result[1][axis] =
            (plus.y - minus.y) / (2.0 * h);
    }

    return result;
}

// ---------------------------------------------------------------------
// APPLY LOCAL LINEAR MAP
// ---------------------------------------------------------------------

[[nodiscard]]
Vec2 multiply(const Matrix2x3& j, const Vec3& d) {
    // This performs:
    //
    //   deltaPixel = J * deltaPosition
    //
    // The matrix is dimensionally 2x3 and the input vector is 3x1.
    // The resulting displacement is a 2x1 pixel-space vector.

    return {
        j[0][0]*d.x + j[0][1]*d.y + j[0][2]*d.z,
        j[1][0]*d.x + j[1][1]*d.y + j[1][2]*d.z
    };
}

// ---------------------------------------------------------------------
// QUADRATIC FUNCTION WITH KNOWN GRADIENT AND HESSIAN
// ---------------------------------------------------------------------

[[nodiscard]]
double quadratic(Vec2 x) {
    return x.x*x.x + 2.0*x.x*x.y + 3.0*x.y*x.y;
}

[[nodiscard]]
Vec2 quadraticGradient(Vec2 x) {
    return {
        2.0*x.x + 2.0*x.y,
        2.0*x.x + 6.0*x.y
    };
}

// ---------------------------------------------------------------------
// TEST AND TRACE
// ---------------------------------------------------------------------

int main() {
    const PinholeCamera camera{
        800.0, 820.0, 640.0, 360.0
    };

    const Vec3 point{0.2, 0.1, 2.0};
    const Vec3 delta{0.001, -0.002, 0.01};

    // 1. Compute the exact original projection.
    const Vec2 uv = camera.project(point);

    // 2. Compute its analytically derived Jacobian.
    const auto analytic = camera.jacobian(point);

    // 3. Independently estimate the Jacobian by finite differences.
    const auto numeric =
        numericJacobian(camera, point, 1e-5);

    double maximumDifference = 0.0;

    for (int row = 0; row < 2; ++row) {
        for (int col = 0; col < 3; ++col) {
            maximumDifference = std::max(
                maximumDifference,
                std::abs(
                    analytic[row][col] - numeric[row][col]
                )
            );
        }
    }

    // 4. Predict the pixel displacement from the Jacobian.
    const Vec2 predictedChange = multiply(analytic, delta);

    const Vec2 predictedNext{
        uv.x + predictedChange.x,
        uv.y + predictedChange.y
    };

    // 5. Calculate the exact projected point after displacement.
    const Vec2 exactNext = camera.project({
        point.x + delta.x,
        point.y + delta.y,
        point.z + delta.z
    });

    // 6. Verify a second-order Taylor model for an exact quadratic.
    const Vec2 x{1.0, -1.0};
    const Vec2 displacement{0.1, 0.2};

    const Vec2 grad = quadraticGradient(x);

    // Hessian:
    // H = [2 2]
    //     [2 6]
    //
    // Second-order contribution = 0.5 * h^T * H * h.

    const double curvature =
        0.5 * (
            2.0*displacement.x*displacement.x +
            4.0*displacement.x*displacement.y +
            6.0*displacement.y*displacement.y
        );

    const double quadraticPrediction =
        quadratic(x)
        + grad.x*displacement.x
        + grad.y*displacement.y
        + curvature;

    const double quadraticExact = quadratic({
        x.x + displacement.x,
        x.y + displacement.y
    });

    std::cout << std::fixed << std::setprecision(9)
              << "Projected pixel: ("
              << uv.x << ", " << uv.y << ")\n"
              << "Analytic J row 0: ("
              << analytic[0][0] << ", "
              << analytic[0][1] << ", "
              << analytic[0][2] << ")\n"
              << "Analytic J row 1: ("
              << analytic[1][0] << ", "
              << analytic[1][1] << ", "
              << analytic[1][2] << ")\n"
              << "Max analytic/numeric J error: "
              << maximumDifference << "\n"
              << "Predicted pixel: ("
              << predictedNext.x << ", "
              << predictedNext.y << ")\n"
              << "Exact pixel: ("
              << exactNext.x << ", "
              << exactNext.y << ")\n"
              << "Quadratic predicted/exact: "
              << quadraticPrediction << " / "
              << quadraticExact << "\n";

    // Validation thresholds here are calibrated for this fixed
    // deterministic test case, not universal application tolerances.
    assert(maximumDifference < 1e-6);
    assert(std::abs(quadraticPrediction - quadraticExact) < 1e-12);
}

```

The supplied source reports a C++23 optimized build with strict warnings and passing assertions; this result has not been reproduced here. The following numbers are source-reported output, not independent verification.

Projected pixel: (720.000000000, 401.000000000)\
Analytic J row 0: (400.000000000, 0.000000000, -40.000000000)\
Analytic J row 1: (0.000000000, 410.000000000, -20.500000000)\
Max analytic/numeric J error: 0.000000002\
Predicted pixel: (720.000000000, 399.975000000)\
Exact pixel: (720.000000000, 399.980099502)\
Quadratic predicted/exact: 1.370000000 / 1.370000000

Download the complete C++23 source

#### Understanding the memory layout

The type:

```cpp
using Matrix2x3 = std::array<std::array<double, 3>, 2>;
```

contains six `double` elements.

On a common platform with 8-byte `double` and no additional array padding, the six values occupy 48 contiguous bytes.

The logical arrangement is:

```text
Matrix2x3
  ├─ row[0]
  │    ├─ J[0][0] = du/dX
  │    ├─ J[0][1] = du/dY
  │    └─ J[0][2] = du/dZ
  │
  └─ row[1]
       ├─ J[1][0] = dv/dX
       ├─ J[1][1] = dv/dY
       └─ J[1][2] = dv/dZ
```

For the numerical example, the six values in row-major order are:

```text
400.0  0.0  -40.0  0.0  410.0  -20.5
```

The analytical Jacobian uses a fixed number of arithmetic operations and requires no dynamic allocation.

The numerical Jacobian requires six camera projection evaluations: two evaluations for each of three input axes.

For a more complex function whose evaluation is expensive, computing a full numerical Jacobian can become costly. This is why automatic differentiation and custom analytic Jacobians matter in large optimization systems.

#### The two independent validations

The first validation compares the analytically derived Jacobian against central finite differences.

This catches errors such as a missing negative sign, incorrect denominator power, or transposed Jacobian storage.

The second validation checks a second-order Taylor expansion against an exact quadratic function.

Because the test function is quadratic, the second-order Taylor model should reproduce it to floating-point precision.

These tests verify different mathematical mechanisms. Passing one does not imply the other is correct.

A fuller production test suite would also include randomized valid points, scaled scenes, points near clipping boundaries, varying focal lengths, and failure cases. It would compare both componentwise errors and directional derivatives, rather than relying only on one maximum absolute-error threshold.

### Repository-grounded analysis: KairoMath and KairoRenderer — source claims pending independent repository verification

The supplied lesson identifies `KairoMath/Optimization.cppm` and `KairoMath/TensorAutograd.cppm` as repository anchors and says it inspected default-branch files. The following repository details are source claims only; no KairoMath or KairoRenderer checkout was inspected for this temporary draft.

#### KairoMath's actual matrix convention

In [KairoMath/Matrix.cppm](https://github.com/swayam8624/KairoMath/blob/master/Matrix.cppm?utm_source=chatgpt.com), lines 17–23, the module declares a row-major memory layout with column-vector multiplication.

This distinction matters for calculus because the mathematical Jacobian multiplication order and the CPU representation must agree semantically, even though storage layout and mathematical vector convention are independent choices.

Your camera Jacobian should therefore be interpreted as a mapping from column-vector position perturbations to column-vector pixel perturbations.

#### KairoRenderer's actual camera projection

In [KairoRenderer/Camera.cppm](https://github.com/swayam8624/KairoRenderer/blob/main/Camera.cppm?utm_source=chatgpt.com), the current `CameraPose` uses a right-handed coordinate convention, and `ShowcaseCamera` obtains view and projection matrices using KairoMath.

The implementation also applies a Y-axis sign flip once in its projection matrix for the selected viewport convention.

Meanwhile, [KairoMath/Matrix.cppm](https://github.com/swayam8624/KairoMath/blob/master/Matrix.cppm?utm_source=chatgpt.com), around lines 1758–1781, constructs a right-handed perspective matrix with a zero-to-one depth range and:

$$
w\_{\text{clip}}=-z\_{\text{view}}.
$$

This differs in representation from the positive-forward camera-space convention used in our canonical projection derivation.

For an ideal centered projection, we can convert between these conventions by using:

$$
X=X\_{\text{view}},\quad Y=-Y\_{\text{view}},\quad Z=-Z\_{\text{view}}
$$

when converting to a positive-forward, positive-downward image-camera frame, with the relevant viewport mapping applied.

Consequently, you should not paste the canonical $J\_\pi$ directly into an engine function without checking what coordinate space its input occupies and whether viewport or projection sign conventions have already been applied.

A robust engine API should either state the expected coordinate frame explicitly or expose a transformation that makes the conversion unambiguous.

#### How the Jacobian already appears in your optimization implementation

In [KairoMath/Optimization.cppm](https://github.com/swayam8624/KairoMath/blob/master/Optimization.cppm?utm_source=chatgpt.com), the internal helper `NormalEquationsGradient` computes the product:

```cpp
return jacobian.Transpose() * residual;
```

Why is that the correct gradient for a standard least-squares objective?

Suppose:

$$
E(\mathbf x)=\frac12\|\mathbf r(\mathbf x)\|^2 =\frac12\sum\_{i=1}^m r_i(\mathbf x)^2.
$$

Here $\mathbf r$ is a residual vector, such as the difference between predicted and observed image coordinates.

Differentiate with respect to parameter $x_j$:

$$
\frac{\partial E}{\partial x_j} = \sum\_{i=1}^m r_i \frac{\partial r_i}{\partial x_j}.
$$

The residual Jacobian has entries:

$$
J\_{ij}=\frac{\partial r_i}{\partial x_j}.
$$

Collecting all parameter derivatives produces:

$$
\boxed{ \nabla E=J^T\mathbf r }.
$$

That is precisely the mathematical operation expressed by the repository code.

Its transpose is not arbitrary. The Jacobian maps parameter changes into residual changes, while $J^T$ maps residual-space sensitivities back into parameter-space sensitivities.

##### The exact Hessian of least squares

This provides a useful bridge to future optimization chunks.

Since:

$$
\nabla E=J^T\mathbf r,
$$

differentiate again:

$$
\boxed{ \nabla^2E = J^TJ+ \sum\_{i=1}^m r_i\nabla^2r_i }.
$$

The term $J^TJ$ is positive semidefinite and describes the first-order residual sensitivity.

The remaining term depends on the residual values and second derivatives of the residual functions.

Gauss–Newton approximates the Hessian by:

$$
H\_{\mathrm{GN}}=J^TJ.
$$

This approximation is particularly attractive when the residuals are small near a solution.

We will derive the algorithm, solve strategies, and failure cases in M053.

#### KairoMath's automatic differentiation representation

In [KairoMath/TensorAutograd.cppm](https://github.com/swayam8624/KairoMath/blob/master/TensorAutograd.cppm?utm_source=chatgpt.com), the internal `Node` stores a tensor value, accumulated gradient, parent-node references, and a backward callback.

The source-described `Variable::Backward` implementation builds a graph traversal order and then processes it in reverse.

Your `AutogradMatMul` implements derivative accumulation corresponding to:

$$
C=AB.
$$

For an upstream sensitivity matrix $G=\partial L/\partial C$, the gradients are:

$$
\boxed{ \frac{\partial L}{\partial A}=GB^T, \qquad \frac{\partial L}{\partial B}=A^TG }.
$$

The actual implementation computes these products using nested loops, accumulating contributions into the participating nodes.

This is the same chain rule developed in this lesson, applied to a matrix-valued operation.

The supplied source claims its ReLU backward implementation sends zero gradient when an input value is nonpositive, including at zero; this repository behavior is unverified here. Mathematically, ReLU is not differentiable at zero, so any selected branch value is a convention rather than a uniquely determined classical derivative.

Automatic differentiation, computational graphs, gradient accumulation, and their implementation trade-offs are covered fully in M052.

### Research bridge: from Jacobians to 3D Gaussian splatting

This is where the current mathematics becomes directly applicable to your reconstruction and Gaussian-rendering work.

A three-dimensional Gaussian can be described using a mean:

$$
\boldsymbol\mu\in\mathbb R^3
$$

and a covariance matrix:

$$
\Sigma\in\mathbb R^{3\times3}.
$$

The mean describes its centre, while the covariance describes its spatial spread and correlations.

Suppose the Gaussian is already expressed in camera space. We want to approximate its two-dimensional footprint in image space.

A perspective projection is nonlinear, so a 3D Gaussian does not generally transform into an exact 2D Gaussian simply by applying the projection to its mean and multiplying its covariance by a fixed camera matrix.

However, we can linearize the projection around the mean:

$$
\pi(\mathbf p) \approx \pi(\boldsymbol\mu) + J\_\pi(\boldsymbol\mu) (\mathbf p-\boldsymbol\mu).
$$

Define the random perturbation:

$$
\boldsymbol\epsilon=\mathbf p-\boldsymbol\mu.
$$

Assume:

$$
\mathbb E[\boldsymbol\epsilon]=0, \qquad \operatorname{Cov}(\boldsymbol\epsilon)=\Sigma.
$$

Then the linearized image perturbation is:

$$
\delta\mathbf s=J\_\pi\boldsymbol\epsilon.
$$

Its covariance is:

$$
\begin{aligned} \operatorname{Cov}(\delta\mathbf s) &=\mathbb E[ (J\_\pi\boldsymbol\epsilon) (J\_\pi\boldsymbol\epsilon)^T ]\\ &=J\_\pi\\, \mathbb E[\boldsymbol\epsilon\boldsymbol\epsilon^T]\\, J\_\pi^T. \end{aligned}
$$

Therefore:

$$
\boxed{ \Sigma\_{\text{image}} \approx J\_\pi\Sigma J\_\pi^T }.
$$

If the original Gaussian covariance is defined in world space, and $R$ is the linear rotational part of the world-to-camera transformation:

$$
\boxed{ \Sigma\_{\text{image}} \approx J\_\pi R\Sigma\_{\text{world}}R^TJ\_\pi^T }.
$$

This is the first-order covariance projection used in 3D Gaussian splatting formulations. A survey of the field expresses this same Jacobian-based transformation and explains its role in approximating projective geometry.&#x20;

[image](https://www.google.com/s2/favicons?domain=https://doi.org\&sz=32)

ACM Computing Surveys



#### Numerical covariance example

Use our previously derived Jacobian:

$$
J\_\pi= \begin{bmatrix} 400&0&-40\\ 0&410&-20.5 \end{bmatrix}.
$$

Suppose the camera-space Gaussian has independent coordinate uncertainties with standard deviations:

$$
\sigma_X=0.01\ \mathrm m,\quad \sigma_Y=0.02\ \mathrm m,\quad \sigma_Z=0.05\ \mathrm m.
$$

Then its covariance is diagonal:

$$
\Sigma= \begin{bmatrix} 0.0001&0&0\\ 0&0.0004&0\\ 0&0&0.0025 \end{bmatrix}.
$$

Compute:

$$
\Sigma\_{\text{image}}=J\_\pi\Sigma J\_\pi^T.
$$

The top-left entry is:

$$
\begin{aligned} \sigma_u^2 &=400^2(0.0001)+(-40)^2(0.0025)\\ &=16+4\\ &=\boxed{20}. \end{aligned}
$$

The bottom-right entry is:

$$
\begin{aligned} \sigma_v^2 &=410^2(0.0004)+(-20.5)^2(0.0025)\\ &=67.24+1.050625\\ &=\boxed{68.290625}. \end{aligned}
$$

The off-diagonal term is:

$$
\begin{aligned} \operatorname{Cov}(u,v) &=(-40)(-20.5)(0.0025)\\ &=\boxed{2.05}. \end{aligned}
$$

Thus:

$$
\boxed{ \Sigma\_{\text{image}}= \begin{bmatrix} 20&2.05\\ 2.05&68.290625 \end{bmatrix} \text{ pixels}^2 }.
$$

Even though the original 3D covariance was diagonal, its image-space covariance contains an off-diagonal component.

This happens because uncertainty in depth simultaneously affects both horizontal and vertical projected positions.

That creates correlation between image-space coordinates.

The approximate marginal standard deviations are:

$$
\sigma_u=\sqrt{20}\approx4.472\text{ pixels},
$$

$$
\sigma_v=\sqrt{68.290625}\approx8.264\text{ pixels}.
$$

The image footprint is therefore more extended vertically than horizontally.

The eigenvectors of the projected covariance determine its principal orientations, and the eigenvalues determine the squared standard deviations along those principal axes.

This connects Jacobians, covariance, eigendecomposition, SVD, and Gaussian rendering in one pipeline.

#### Important limitation of the first-order covariance approximation

Perspective projection can become strongly nonlinear across a Gaussian with significant depth extent.

If the Gaussian is broad relative to its distance from the camera, the Jacobian at the mean may no longer adequately describe the entire footprint.

Furthermore, a mathematical Gaussian has unbounded support. If its depth distribution extends across the camera plane, the ideal unconditioned perspective division can produce problematic or undefined moments.

Real renderers therefore need practical handling for clipping, covariance conditioning, footprint bounds, and degenerate projections.

A useful research question is:

How does the error of Jacobian-based covariance projection depend on the ratio of a Gaussian's depth uncertainty to its mean depth?

One could compare first-order covariance projection against clipped high-quality numerical or Monte Carlo references over a controlled sweep of depth variance, viewing angle, covariance anisotropy, and mean depth.

Measurements could include footprint orientation error, covariance error, rendered-image discrepancy, and temporal instability.

This provides a falsifiable experiment directly tied to reconstruction and splatting accuracy rather than a vague claim about improving Gaussian rendering.

#### Beyond first-order: curvature-induced projection bias

The Jacobian predicts the mean projection as approximately:

$$
\mathbb E[\pi(\mathbf p)] \approx\pi(\boldsymbol\mu).
$$

But nonlinear projection introduces a second-order correction.

For a scalar component $f$ of the projection, Taylor expansion gives:

$$
f(\boldsymbol\mu+\boldsymbol\epsilon) \approx f(\boldsymbol\mu) + \nabla f^T\boldsymbol\epsilon + \frac12\boldsymbol\epsilon^TH_f\boldsymbol\epsilon.
$$

Taking expectations and using $\mathbb E[\boldsymbol\epsilon]=0$:

$$
\mathbb E[f(\mathbf p)] \approx f(\boldsymbol\mu) + \frac12\mathbb E[ \boldsymbol\epsilon^TH_f\boldsymbol\epsilon ].
$$

The quadratic expectation satisfies:

$$
\mathbb E[ \boldsymbol\epsilon^TH_f\boldsymbol\epsilon ] = \operatorname{tr}(H_f\Sigma).
$$

Therefore:

$$
\boxed{ \mathbb E[f(\mathbf p)] \approx f(\boldsymbol\mu)+ \frac12\operatorname{tr}(H_f\Sigma) }.
$$

This reveals something the first-order approximation misses: a nonlinear transformation can shift the expected output even when the input noise has zero mean.

For the horizontal projection $u=f_xX/Z+c_x$, the nonzero second derivatives include:

$$
u\_{XZ}=u\_{ZX}=-\frac{f_x}{Z^2}, \qquad u\_{ZZ}=\frac{2f_xX}{Z^3}.
$$

The second-order mean correction is:

$$
\boxed{ \Delta\bar u \approx -\frac{f_x}{Z^2}\operatorname{Cov}(X,Z) + \frac{f_xX}{Z^3}\operatorname{Var}(Z) }.
$$

For independent $X$ and $Z$, the covariance term vanishes, leaving the depth-variance contribution.

This does not mean second-order projection should automatically replace first-order projection in a renderer. It introduces additional computation and still has its own approximation regime.

But it gives a mathematically precise mechanism worth testing when depth uncertainty is large.

### Additional essential connections

Before closing, we should connect the first two chunks to several advanced concepts that otherwise tend to appear suddenly in later research papers.

#### The Hessian of a composition

The first derivative of $g(F(\mathbf x))$ is governed by the Jacobian chain rule.

For a scalar outer function $g:\mathbb R^m\to\mathbb R$ and a vector-valued inner function $F:\mathbb R^n\to\mathbb R^m$:

$$
\nabla(g\circ F)=J_F^T\nabla g.
$$

Differentiating again:

$$
\boxed{ H\_{g\circ F} = J_F^TH_gJ_F + \sum\_{k=1}^{m} \frac{\partial g}{\partial y_k}H\_{F_k} }.
$$

The first term transports the outer function's curvature through the inner Jacobian.

The second term accounts for curvature of the inner mapping itself.

This is the general structure behind the exact least-squares Hessian and many second-order differentiation formulas.

It also explains why dropping certain second-derivative terms is an approximation, not an identity.

#### The inverse Jacobian

Suppose:

$$
F:\mathbb R^n\to\mathbb R^n
$$

has an invertible Jacobian at a point and satisfies the assumptions of the inverse function theorem.

Then the inverse mapping exists locally, and:

$$
\boxed{ J\_{F^{-1}}(F(\mathbf x))= [J_F(\mathbf x)]^{-1} }.
$$

This is the multidimensional counterpart of:

$$
(f^{-1})'=\frac1{f'}.
$$

It matters when converting between parameterizations, inverting coordinate mappings, or recovering local motion from observed displacement.

If the Jacobian is singular, a differentiable local inverse generally cannot be constructed by this rule.

A rectangular Jacobian cannot be inverted ordinarily. One may instead use a pseudoinverse, but that solves a related least-squares or minimum-norm problem, not necessarily a true inverse mapping.

#### Differentiation on rotation manifolds

A rotation matrix $R\in SO(3)$ has nine stored entries but only three independent rotational degrees of freedom.

Treating its nine entries as independent optimization variables ignores the orthogonality and determinant constraints.

A more appropriate local update uses a three-dimensional tangent-space perturbation:

$$
R\_{\text{new}}=\exp([\delta\boldsymbol\omega]\_\times)R.
$$

The gradient and Jacobian are then expressed with respect to the local perturbation coordinates, rather than nine unconstrained matrix entries.

This is why derivatives with respect to quaternions, Euler angles, rotation matrices, and Lie-algebra coordinates require careful parameterization.

We will encounter these distinctions repeatedly in camera-pose optimization, rigid-body simulation, and differentiable transformations.

#### Coordinate changes and derivative transport

For a linear change of input coordinates:

$$
\mathbf x=A\mathbf z,
$$

the scalar function becomes:

$$
\tilde f(\mathbf z)=f(A\mathbf z).
$$

By the chain rule:

$$
\boxed{ \nabla\_{\mathbf z}\tilde f = A^T\nabla\_{\mathbf x}f }.
$$

If $A$ is constant, the Hessian transforms as:

$$
\boxed{ H\_{\mathbf z}=A^TH\_{\mathbf x}A }.
$$

These formulas show how gradients and Hessians depend on parameterization.

If $A$ is not orthogonal, the Euclidean steepest-descent direction in $\mathbf z$-coordinates generally maps to a different physical direction than Euclidean steepest descent in $\mathbf x$-coordinates.

This is not a contradiction. Steepest descent depends on the metric used to measure the size of a step.

### Debugging and failure-mode laboratory

Knowing the correct derivatives is only half the engineering problem. We must also recognize when an implementation violates their assumptions.

| Failure                                                          | Mathematical cause                                     | Diagnostic                                                                  |
| ---------------------------------------------------------------- | ------------------------------------------------------ | --------------------------------------------------------------------------- |
| Camera movement changes pixels in the wrong direction            | Incorrect depth or coordinate sign                     | Compare analytical Jacobian columns against controlled finite perturbations |
| Jacobian looks transposed                                        | Confused input/output dimensions                       | Assert shape $m\times n$, trace one column                              |
| Gradient descent moves uphill                                    | Incorrect sign or gradient                             | Evaluate directional derivative against actual objective change             |
| Derivatives explode near zero depth                              | Perspective singularity                                | Monitor $Z$, clipping, and Jacobian norms                               |
| Normal derivatives become unstable                               | Division by near-zero vector length                    | Test zero and near-zero vectors                                             |
| Hessian unexpectedly asymmetric                                  | Incorrect mixed derivatives or numerical approximation | Compare $H\_{ij}$ and $H\_{ji}$, inspect regularity                 |
| Finite differences disagree at tiny step sizes                   | Floating-point cancellation                            | Sweep step size logarithmically                                             |
| Linearization accurate nearby but fails farther away             | Neglected nonlinear terms                              | Plot error versus perturbation magnitude                                    |
| Gradient is zero but result is not a minimum                     | Stationary saddle or maximum                           | Inspect Hessian eigenvalues or directional behavior                         |
| Automatic differentiation produces a chosen gradient at a corner | Nonsmooth operation or branch convention               | Identify active branch and selected subgradient                             |

A particularly useful testing strategy is the directional derivative test.

For a function $F$, analytical Jacobian $J$, and direction $\mathbf d$, compare:

$$
J(\mathbf x)\mathbf d
$$

against:

$$
\frac{ F(\mathbf x+h\mathbf d) - F(\mathbf x-h\mathbf d) }{2h}.
$$

This checks the Jacobian's action without constructing the entire numerical matrix.

For a scalar objective, the corresponding test is:

$$
\nabla f(\mathbf x)^T\mathbf d \approx \frac{ f(\mathbf x+h\mathbf d) - f(\mathbf x-h\mathbf d) }{2h}.
$$

This technique is frequently valuable in optimization libraries. Ceres Solver, for instance, supports analytic, numerical, and automatic derivatives, and its documentation emphasizes the practical importance of correct Jacobian evaluation.&#x20;

[image](https://www.google.com/s2/favicons?domain=https://ceres-solver.readthedocs.io\&sz=32)

Ceres Solver+1



### Reconstructing the conceptual system

The complete progression from Parts 801–831 can be expressed as one chain:

The following formulas are the most important reconstruction anchors:

$$
\boxed{ \begin{aligned} f'(x)&=\lim\_{h\to0}\frac{f(x+h)-f(x)}h\\[3pt] Df(\mathbf x)[\mathbf h]&=\nabla f(\mathbf x)^T\mathbf h\\[3pt] F(\mathbf x+\mathbf h)&\approx F(\mathbf x)+J_F(\mathbf x)\mathbf h\\[3pt] J\_{G\circ F}&=J_GJ_F\\[3pt] f(\mathbf x+\mathbf h)&\approx f(\mathbf x)+\nabla f^T\mathbf h+ \frac12\mathbf h^TH_f\mathbf h\\[3pt] \Sigma\_{\text{image}}&\approx J\_\pi\Sigma J\_\pi^T \end{aligned} }
$$

Every formula is a different expression of how local changes propagate through a system.

#### Mastery challenge

Do these without consulting the explanations first.

1. Starting from the limit definition, derive the derivative of $f(x)=1/x$. Then explain why that derivative becomes sensitive near zero and distinguish sensitivity of the function from floating-point cancellation.
2. Derive the gradient and Hessian of $f(x,y)=x^2+3xy+4y^2$. Determine whether its stationary point at the origin is a strict minimum, maximum, or saddle.
3. Given $F(x,y)=(x^2+y,\sin(xy))$, derive its Jacobian, evaluate it at $(1,2)$, and predict the output change under $\Delta\mathbf x=(0.01,-0.02)$.
4. Starting from similar triangles, derive the $2\times3$ ideal perspective-projection Jacobian. Explain why it has a nontrivial nullspace and what that means physically.
5. Derive the covariance propagation formula $J\Sigma J^T$ and explain why it can become inaccurate for a Gaussian with large uncertainty in depth.

For implementation mastery, reconstruct the `PinholeCamera::jacobian()` function, write a central-difference checker without copying, and demonstrate empirically how the linearization error changes when you scale a perturbation by factors of $1/2$, $1/4$, and $1/8$. For generic smooth functions away from degeneracies, first-order error should show approximately quadratic scaling while truncation dominates.

**Concept link:** [[Chapter Edition/01 - Computing Foundations/21 - Multivariable Derivatives and Sensitivity#Jacobians, curvature, and local approximation|second derivative matrix]] — The Hessian collects second partial derivatives of a scalar….



<!-- end V09.03 -->


### Immediate footing — ordinary differential equation

An ordinary differential equation (ODE) relates a changing state to its derivatives with respect to one independent variable, usually time. The initial-value problem $x^{\prime}(t)=2$ with $x(0)=1$ has solution $x(t)=1+2t$. A general system $\mathbf x^{\prime}=f(t,\mathbf x)$ usually needs a numerical method that advances the state in finite steps. The initial condition selects a trajectory; it is part of the problem. Existence, uniqueness, stiffness, stability and approximation error must be distinguished. A small step alone does not guarantee a correct simulation.

[[Chapter Edition/01 - Computing Foundations/25 - Continuous Models and Numerical Error#ODEs and numerical simulation|Full explanation and worked examples in this book]]

<!-- begin V09.04 -->
<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/01 - Computing Foundations/00 - Reading Map|Reading map]] · [[Chapter Edition/01 - Computing Foundations/20 - Choose Matrix Factorizations from Structure|Previous chapter]] · [[Chapter Edition/01 - Computing Foundations/22 - Accumulation, Quadrature, and the Bridge to Probability|Next chapter]]
