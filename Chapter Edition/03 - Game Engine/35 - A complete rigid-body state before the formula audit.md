# 35 — A complete rigid-body state before the formula audit

[[Chapter Edition/03 - Game Engine/00 - Reading Map|Reading map]] · [[Chapter Edition/03 - Game Engine/34 - Rigid-body motion from forces to contact|Previous chapter]] · [[Chapter Edition/03 - Game Engine/36 - Auditing capsule mass properties and collision formulas|Next chapter]]

A simulator becomes inspectable when position, orientation, linear and angular velocity, mass properties, and force accumulators have explicit owners and update rules. This chapter consolidates the engine state reached so far, then uses it as the reference for checking individual formulas.

**By the end:** Describe the rigid-body state and its update responsibilities, including body-local force points, world transforms, and the distinction between stored state and derived values.

**Recall question:** Which values are authoritative state, which are derived, and how does a body-local force point become a world-space torque each step?

<!-- Derived from Continuous Notes/Game Engine - From First Frame to Living Worlds.md; source lines 82586–85119; source-span SHA256 1a512801cc0b42eaec79556488ed5caa8a7c5d3e1d0676698885d0e965949cf4. -->

<!-- CHAPTER-SOURCE-BEGIN -->
### THE COMPLETE KAIRO STATE SO FAR

Current `RigidBody` contains essentially:

```cpp
struct RigidBody
{
    BodyID ID;

    BodyType Type;

    MotionState State;

    MassProperties Mass;

    ForceAccumulation Forces;

    bool EnableGravity;

    float GravityScale;

    float LinearDamping;
    float AngularDamping;

    ...
};
```

That is already enough to express:

$$
\boxed{
\text{translational Newtonian dynamics}
+
\text{rotational dynamics}.
}
$$

Now M098 explains the rotational half.

---

### TORQUE AND INERTIA FROM FIRST PRINCIPLES

### Part 1621 — Torque from an off-center force

Suppose force:

$$
\mathbf{F}
$$

is applied at point:

$$
\mathbf{p}.
$$

Let COM be:

$$
\mathbf{c}.
$$

Define lever arm:

$$
\boxed{
\mathbf{r}
=
\mathbf{p}-\mathbf{c}.
}
$$

Torque:

$$
\boxed{
\boldsymbol{\tau}
=
\mathbf{r}\times\mathbf{F}.
}
$$

This is the rotational effect of force.

---

**Magnitude of torque**

Cross-product magnitude:

$$
\|\tau\|
=
\|r\|\|F\|\sin\theta.
$$

Therefore torque increases when:

$$
r
$$

is longer,

or:

$$
F
$$

is larger,

or force is more perpendicular to lever arm.

Maximum:

$$
\theta=90^\circ.
$$

Zero:

$$
\theta=0^\circ
$$

or:

$$
180^\circ.
$$

---

**Force through COM**

If application point is COM:

$$
r=0.
$$

Therefore:

$$
\tau=0.
$$

The body gets linear acceleration but no rotation.

Likewise if F's line of action passes through COM:

$$
r\times F=0.
$$

Again no rotational effect.

---

**Kairo's implementation**

```cpp
accumulation.Force += force;

accumulation.Torque +=
    Cross(
        worldPoint -
        worldCenterOfMass,
        force);
```

Perfect form.

The subtle question is:

> What exactly is `worldCenterOfMass`?

We will return to this in the Kairo audit because that convention currently deserves tightening.

---

### Part 1622 — Lever arm vectors

Lever arm is not merely "distance from COM."

It is the full vector:

$$
\mathbf{r}.
$$

Direction matters because torque is a cross product.

Example:

$$
r=(1,0,0)
$$

and:

$$
F=(0,10,0).
$$

Then:

$$
\tau
=
(1,0,0)
\times
(0,10,0)
=
(0,0,10).
$$

Kairo's test explicitly verifies this case.

---

**Units of torque**

$$
[Nm]
=
kg\cdot m^2/s^2.
$$

Torque and energy both have dimensional units of Newton-metres, but they are not the same physical quantity.

Torque is a pseudovector.

Energy is scalar.

Do not conflate them because units happen to match.

---

### Part 1623 — Angular velocity

Angular velocity:

$$
\boldsymbol{\omega}.
$$

Magnitude:

$$
\|\omega\|
$$

is angular rate in:

$$
rad/s.
$$

Direction follows right-hand rule and gives instantaneous rotation axis.

For a body point:

$$
\boxed{
\mathbf{v}_{rot}
=
\boldsymbol{\omega}\times\mathbf{r}.
}
$$

This equation is more fundamental than thinking of $\omega$ as "Euler rotation speed."

---

**Angular velocity is not Euler-angle derivative**

Suppose orientation is stored as Euler:

```text
yaw
pitch
roll
```

It is tempting to assume:

$$
\omega
=
(\dot{roll},\dot{pitch},\dot{yaw}).
$$

Generally false.

Euler-angle derivatives depend on current orientation and chosen rotation order.

This is one reason physics engines store:

$$
\boldsymbol{\omega}\in\mathbb{R}^3
$$

directly.

Orientation integration then updates quaternion from $\omega$.

That arrives in M100.

---

### Part 1624 — Angular acceleration

Angular acceleration:

$$
\boxed{
\boldsymbol{\alpha}
=
\frac{d\boldsymbol{\omega}}{dt}.
}
$$

For a simple rotation about one fixed principal axis:

$$
\tau=I\alpha.
$$

So:

$$
\alpha
=
\frac{\tau}{I}.
$$

But in general 3D:

$$
I
$$

is a tensor.

And there is an additional gyroscopic term for asymmetric rotating bodies.

We'll derive that shortly.

---

### Part 1625 — Angular momentum

For one point mass relative to origin:

$$
\boxed{
\mathbf{L}
=
\mathbf{r}\times\mathbf{p}.
}
$$

Since:

$$
\mathbf{p}=m\mathbf{v},
$$

then:

$$
\mathbf{L}
=
\mathbf{r}\times
m\mathbf{v}.
$$

For a rigid body rotating around its COM:

$$
\boxed{
\mathbf{L}
=
I\boldsymbol{\omega}.
}
$$

But note:

$$
\mathbf{L}
$$

need not point in the same direction as:

$$
\boldsymbol{\omega}.
$$

That is exactly why $I$ is a matrix.

---

**Rotational Newton's second law**

The fundamental rotational law is:

$$
\boxed{
\boldsymbol{\tau}
=
\frac{d\mathbf{L}}{dt}.
}
$$

This is the rotational analogue of:

$$
\mathbf{F}
=
\frac{d\mathbf{p}}{dt}.
$$

Now because:

$$
L=I\omega,
$$

things become subtle because the body's world-space inertia changes as it rotates.

---

### Part 1626 — Moment of inertia as rotational mass

For rotation around a fixed scalar axis:

$$
\boxed{
I
=
\sum_i
m_ir_i^2.
}
$$

For continuous body:

$$
\boxed{
I
=
\int
r_\perp^2\,dm.
}
$$

Here:

$$
r_\perp
$$

is perpendicular distance from the chosen axis.

This is why mass farther from axis contributes dramatically more.

Distance appears squared.

---

**Compare two masses**

Two objects both have:

$$
m=1kg.
$$

One mass is:

$$
r=1m
$$

from axis:

$$
I_1=1.
$$

The other:

$$
r=2m:
$$

$$
I_2=4.
$$

Same mass.

Four times the rotational inertia.

That is why a dumbbell is harder to rotate when weights are farther apart.

---

**Rotational kinetic energy**

For scalar axis:

$$
E_r
=
\frac12I\omega^2.
$$

For full 3D:

$$
\boxed{
E_r
=
\frac12
\boldsymbol{\omega}^T
I
\boldsymbol{\omega}.
}
$$

Kairo's:

```cpp
0.5f *
Dot(
    angularVelocity,
    worldInertia *
    angularVelocity);
```

is exactly this.

---

### Part 1627 — Why an inertia tensor exists

Suppose an irregular object.

Apply torque along x.

Its angular acceleration may not be purely along x.

A scalar inertia cannot express this coupling.

Need a linear map:

$$
\boxed{
\mathbf{L}
=
I\boldsymbol{\omega}.
}
$$

where:

$$
I=
\begin{bmatrix}
I_{xx}&I_{xy}&I_{xz}\\
I_{yx}&I_{yy}&I_{yz}\\
I_{zx}&I_{zy}&I_{zz}
\end{bmatrix}.
$$

Diagonal entries represent resistance around coordinate axes.

Off-diagonal entries are **products of inertia** describing rotational coupling.

---

### Part 1628 — Inertia tensor representation

For a mass distribution relative to COM:

$$
\boxed{
I
=
\int
\left(
\|\mathbf{r}\|^2I_3
-
\mathbf{r}\mathbf{r}^T
\right)
dm.
}
$$

This equation is worth understanding rather than memorizing.

Let:

$$
\mathbf{r}
=
\begin{bmatrix}
x\\y\\z
\end{bmatrix}.
$$

Then:

$$
\|\mathbf{r}\|^2I_3
=
\begin{bmatrix}
x^2+y^2+z^2 &0&0\\
0&x^2+y^2+z^2&0\\
0&0&x^2+y^2+z^2
\end{bmatrix}.
$$

And:

$$
rr^T
=
\begin{bmatrix}
x^2&xy&xz\\
xy&y^2&yz\\
xz&yz&z^2
\end{bmatrix}.
$$

Subtract:

$$
I=
\int
\begin{bmatrix}
y^2+z^2&-xy&-xz\\
-xy&x^2+z^2&-yz\\
-xz&-yz&x^2+y^2
\end{bmatrix}
dm.
$$

Therefore:

$$
I_{xx}
=
\int(y^2+z^2)dm.
$$

This exactly matches scalar moment of inertia about x-axis.

---

### Part 1629 — Principal moments

Because physical inertia tensor is real and symmetric, it has real eigenvalues and orthogonal eigenvectors.

Solve:

$$
I\mathbf{e}_i
=
\lambda_i\mathbf{e}_i.
$$

The eigenvalues:

$$
\boxed{
I_1,I_2,I_3
}
$$

are the **principal moments of inertia**.

In the principal-axis basis:

$$
I_{principal}
=
\begin{bmatrix}
I_1&0&0\\
0&I_2&0\\
0&0&I_3
\end{bmatrix}.
$$

This is why diagonal inertia representations are so powerful.

---

### Part 1630 — Principal axes

The eigenvectors:

$$
\mathbf{e}_1,\mathbf{e}_2,\mathbf{e}_3
$$

are the **principal axes**.

Because inertia tensor is symmetric, those eigenvectors can be chosen orthonormal.

Build matrix:

$$
Q=
[
\mathbf{e}_1\
\mathbf{e}_2\
\mathbf{e}_3
].
$$

Then:

$$
\boxed{
I
=
Q
\begin{bmatrix}
I_1&0&0\\
0&I_2&0\\
0&0&I_3
\end{bmatrix}
Q^T.
}
$$

For highly symmetric shapes, obvious geometric axes are principal axes.

Sphere:

every axis is principal.

Centered box:

local x/y/z.

Cylinder:

central symmetry axis and any orthogonal radial pair.

---

### Part 1631 — Point-mass inertia tensor

For one point mass:

$$
m
$$

at:

$$
r=(x,y,z),
$$

tensor:

$$
\boxed{
I
=
m
(
\|r\|^2I-r r^T
).
}
$$

Explicitly:

$$
I
=
m
\begin{bmatrix}
y^2+z^2&-xy&-xz\\
-xy&x^2+z^2&-yz\\
-xz&-yz&x^2+y^2
\end{bmatrix}.
$$

This formula is the building block from which continuous formulas arise through integration.

---

### Part 1632 — Solid sphere inertia

Uniform solid sphere of radius:

$$
r
$$

about center:

$$
\boxed{
I
=
\frac25mr^2I_3.
}
$$

Thus:

$$
I_{xx}
=
I_{yy}
=
I_{zz}
=
\frac25mr^2.
$$

Kairo uses:

```cpp
const float value =
    0.4f *
    mass *
    radius *
    radius;
```

Correct.

Because all three moments are identical, orientation does not change the tensor:

$$
RI R^T=I.
$$

That is rotational isotropy.

---

### Part 1633 — Solid box inertia

For full dimensions:

$$
w,h,d,
$$

centered uniform box:

$$
I_{xx}
=
\frac{1}{12}m(h^2+d^2)
$$

$$
I_{yy}
=
\frac{1}{12}m(w^2+d^2)
$$

$$
I_{zz}
=
\frac{1}{12}m(w^2+h^2).
$$

Kairo uses half-extents:

$$
a=\frac w2,\quad
b=\frac h2,\quad
c=\frac d2.
$$

Then:

$$
h^2=4b^2,\qquad
d^2=4c^2.
$$

Thus:

$$
I_{xx}
=
\frac{1}{12}m(4b^2+4c^2)
=
\frac13m(b^2+c^2).
$$

Kairo:

```cpp
const float scale =
    mass / 3.0f;

return DiagonalInertia(
    scale * (y2 + z2),
    scale * (x2 + z2),
    scale * (x2 + y2));
```

Correct.

---

### Part 1634 — Cylinder inertia

For uniform solid cylinder aligned with y-axis, radius r and total cylinder height h:

Around symmetry axis:

$$
\boxed{
I_y
=
\frac12mr^2.
}
$$

Around x and z:

$$
\boxed{
I_x=I_z
=
\frac{1}{12}
m(3r^2+h^2).
}
$$

So:

$$
I=
\begin{bmatrix}
I_x&0&0\\
0&I_y&0\\
0&0&I_x
\end{bmatrix}.
$$

Again local geometric axes are principal axes.

---

### Part 1635 — Capsule inertia concept

A capsule is:

$$
\text{cylinder}
+
\text{two hemispherical caps}.
$$

The accurate analytic solution requires:

1. determine cylinder mass,
2. determine each hemisphere mass,
3. determine each part's own COM,
4. compute each part's inertia about its own COM,
5. shift each to capsule COM using parallel-axis theorem,
6. sum.

That is more involved than pretending the caps are one sphere at the center.

---

**Kairo's capsule model**

Kairo explicitly labels its formula an **approximation for gameplay stability rather than CAD-grade analytic precision**.

It computes cylinder and sphere-equivalent masses based on volume fractions, then constructs:

$$
I_y
$$

and:

$$
I_x=I_z
$$

with a stable approximation.

That is completely reasonable for an early engine, provided the contract remains explicit.

Do not call it an exact capsule inertia tensor.

That would be incorrect.

---

### Part 1636 — Parallel-axis theorem

Suppose you know inertia about COM:

$$
I_C.
$$

You want inertia about a parallel axis displaced by:

$$
\mathbf{d}.
$$

Then:

$$
\boxed{
I_P
=
I_C
+
m
\left(
\|\mathbf{d}\|^2I_3
-
\mathbf{d}\mathbf{d}^T
\right).
}
$$

Kairo implements exactly this.

---

**Derivation of parallel-axis theorem**

Let point relative to COM be:

$$
\mathbf{r}.
$$

Relative to new origin:

$$
\mathbf{r}'
=
\mathbf{r}+\mathbf{d}.
$$

Tensor:

$$
I'
=
\int
[
\|r+d\|^2I
-
(r+d)(r+d)^T
]
dm.
$$

Expand:

$$
\|r+d\|^2
=
r^2+2r\cdot d+d^2.
$$

And:

$$
(r+d)(r+d)^T
=
rr^T+rd^T+dr^T+dd^T.
$$

Terms involving r integrate to zero because coordinates are measured from COM:

$$
\int r\,dm=0.
$$

Remaining:

$$
I'
=
I_C+
m(d^2I-dd^T).
$$

Done.

---

**Important off-diagonal consequence**

Let:

$$
d=(1,1,0).
$$

Then:

$$
dd^T
=
\begin{bmatrix}
1&1&0\\
1&1&0\\
0&0&0
\end{bmatrix}.
$$

And:

$$
d^2=2.
$$

Therefore:

$$
d^2I-dd^T
=
\begin{bmatrix}
1&-1&0\\
-1&1&0\\
0&0&2
\end{bmatrix}.
$$

Notice the off-diagonal:

$$
-1.
$$

This is extremely important for the Kairo implementation audit.

Parallel-axis shifting can turn an initially diagonal tensor into a **non-diagonal tensor** in the chosen aggregate coordinate frame.

---

### Part 1637 — Composite-body mass properties

Suppose children:

$$
i=1,\dots,n.
$$

Each has:

$$
m_i
$$

COM:

$$
c_i
$$

and inertia tensor around its own COM:

$$
I_i.
$$

Total mass:

$$
\boxed{
M=\sum_i m_i.
}
$$

Combined COM:

$$
\boxed{
c=
\frac{
\sum_i m_ic_i
}{
M
}.
}
$$

Let:

$$
d_i=c_i-c.
$$

Then if each child tensor is already expressed in the same aggregate orientation basis:

$$
\boxed{
I_{total}
=
\sum_i
\left[
I_i+
m_i
(
\|d_i\|^2I-d_id_i^T
)
\right].
}
$$

That is exactly what Kairo's `CombineMassProperties()` is attempting.

---

**Rotated child shapes**

If child inertia is defined in its own local axes and child has relative orientation:

$$
R_i,
$$

first rotate it into aggregate body frame:

$$
I_i^{body}
=
R_i
I_i^{child}
R_i^T.
$$

Then shift:

$$
I_i^{shifted}
=
I_i^{body}
+
m_i
(
d_i^2I-d_id_i^T
).
$$

Therefore complete compound formula:

$$
\boxed{
I_{total}
=
\sum_i
\left[
R_iI_iR_i^T
+
m_i
(
d_i^2I-d_id_i^T
)
\right].
}
$$

Current `MassProperties` children do not carry a child rotation in `CombineMassProperties()`, so the current helper assumes all part inertia tensors are already expressed in the same local frame.

That is a legitimate restricted contract, but it should be stated explicitly.

---

### Part 1638 — Local-space inertia tensor

Why store inertia in local/body space?

Because the body's mass distribution relative to itself does not change.

A box remains a box while rotating.

Therefore:

$$
I_{body}
$$

is constant.

If you stored only world-space tensor:

$$
I_{world}
$$

you would need to recompute/transform it as orientation changes.

Better:

```text
store local inertia once
```

and each frame compute:

$$
I_{world}
=
RI_{body}R^T.
$$

This is what engines normally do.

---

### Part 1639 — Inverse inertia tensor

Solvers usually need:

$$
I^{-1}
$$

more often than:

$$
I.
$$

Analogous to inverse mass.

For diagonal tensor:

$$
I=
\begin{bmatrix}
I_x&0&0\\
0&I_y&0\\
0&0&I_z
\end{bmatrix}
$$

then:

$$
\boxed{
I^{-1}
=
\begin{bmatrix}
1/I_x&0&0\\
0&1/I_y&0\\
0&0&1/I_z
\end{bmatrix}.
}
$$

Kairo's `InverseDiagonalInertia()` performs this.

---

**Locked rotational axis**

Suppose rotation around x should be forbidden.

Interpret:

$$
I_x\rightarrow\infty.
$$

Then:

$$
I_x^{-1}\rightarrow0.
$$

So engines can encode a locked axis as:

$$
I_x^{-1}=0.
$$

Again inverse quantities elegantly represent infinite resistance.

---

**Full tensor inversion**

For a non-diagonal tensor:

$$
I=
\begin{bmatrix}
a&b&c\\
b&d&e\\
c&e&f
\end{bmatrix},
$$

you cannot invert it by taking only reciprocal diagonal entries.

The determinant is:

$$
\det(I)
=
a(df-e^2)
-
b(bf-ce)
+
c(be-cd).
$$

Inverse:

$$
I^{-1}
=
\frac1{\det I}
\begin{bmatrix}
df-e^2 & ce-bf & be-cd\\
ce-bf & af-c^2 & bc-ae\\
be-cd & bc-ae & ad-b^2
\end{bmatrix}.
$$

For a physically valid full 3D dynamic inertia tensor it should normally be symmetric positive definite.

---

**Better principal-axis approach**

Since I is symmetric:

$$
I=QDQ^T.
$$

Then:

$$
I^{-1}
=
QD^{-1}Q^T.
$$

If:

$$
D=
diag(\lambda_1,\lambda_2,\lambda_3),
$$

then:

$$
D^{-1}
=
diag(
1/\lambda_1,
1/\lambda_2,
1/\lambda_3
).
$$

This is numerically and conceptually elegant.

It also permits a pseudoinverse convention:

$$
\lambda_i^{-1}=0
$$

for intentionally locked axes.

---

### Part 1640 — Rotating inertia into world space

Let:

$$
R
$$

map body-space vectors to world-space vectors.

Body inertia:

$$
I_b.
$$

Then:

$$
\boxed{
I_w
=
RI_bR^T.
}
$$

Why?

Angular velocity transforms:

$$
\omega_w=
R\omega_b.
$$

Angular momentum:

$$
L_b=
I_b\omega_b.
$$

Transform to world:

$$
L_w=
RL_b.
$$

Thus:

$$
L_w=
RI_b\omega_b.
$$

But:

$$
\omega_b=
R^T\omega_w
$$

because rotation matrices are orthogonal:

$$
R^{-1}=R^T.
$$

Therefore:

$$
L_w=
RI_bR^T\omega_w.
$$

So:

$$
I_w=
RI_bR^T.
$$

---

### Part 1641 — World-space inverse inertia

Start:

$$
I_w
=
RI_bR^T.
$$

Take inverse:

$$
I_w^{-1}
=
(RI_bR^T)^{-1}.
$$

Using:

$$
(ABC)^{-1}
=
C^{-1}B^{-1}A^{-1},
$$

we get:

$$
I_w^{-1}
=
(R^T)^{-1}
I_b^{-1}
R^{-1}.
$$

For orthogonal R:

$$
R^{-1}=R^T.
$$

and:

$$
(R^T)^{-1}=R.
$$

Therefore:

$$
\boxed{
I_w^{-1}
=
R
I_b^{-1}
R^T.
}
$$

This is exactly Kairo's implementation:

```cpp
const Matrix3f r =
    ToMatrix3(rotation);

return
    r *
    localInverseInertia *
    Transpose(r);
```

Correct.

---

**Why transform inverse directly**

You could compute:

$$
I_w
$$

then invert it every frame.

Wasteful.

If:

$$
I_b^{-1}
$$

is precomputed once:

$$
I_w^{-1}
=
RI_b^{-1}R^T.
$$

No matrix inversion per body per frame.

Only matrix multiplications.

That is exactly the right runtime design.

---

### Part 1642 — Why inertia tensors are symmetric

From definition:

$$
I
=
\int
(
r^2I_3
-
rr^T
)
dm.
$$

$r^2I_3$ is symmetric.

And:

$$
(rr^T)^T
=
rr^T.
$$

Therefore integrand is symmetric.

An integral/sum of symmetric matrices remains symmetric.

Thus:

$$
\boxed{
I^T=I.
}
$$

Component-wise:

$$
I_{xy}=I_{yx}
$$

$$
I_{xz}=I_{zx}
$$

$$
I_{yz}=I_{zy}.
$$

---

**Symmetry is not just mathematical trivia**

A real symmetric matrix has powerful properties.

Its eigenvalues are real.

Eigenvectors belonging to distinct eigenvalues are orthogonal.

It can be diagonalized by an orthogonal matrix:

$$
I=QDQ^T.
$$

This is exactly the mathematical foundation of principal axes.

---

**Positive semidefiniteness**

For any vector:

$$
u,
$$

consider:

$$
u^TIu.
$$

Using tensor definition:

$$
u^TIu
=
\int
u^T
(r^2I-rr^T)
u\,dm.
$$

This becomes:

$$
\int
(
r^2u^2-(r\cdot u)^2
)
dm.
$$

Using vector identity:

$$
\|u\times r\|^2
=
u^2r^2-(u\cdot r)^2.
$$

Therefore:

$$
\boxed{
u^TIu
=
\int
\|u\times r\|^2dm
\ge0.
}
$$

So inertia is positive semidefinite.

For a proper 3D mass distribution it is positive definite.

That means all principal moments are positive.

---

**Axis-specific moment from tensor**

For unit axis:

$$
n,
$$

moment of inertia about n:

$$
\boxed{
I_n
=
n^TIn.
}
$$

This is enormously useful.

Tensor contains the rotational inertia for **every possible axis through the chosen origin**.

You don't need to store an inertia value for infinitely many axes.

The matrix encodes them all.

---

**Angular dynamics: the subtle part most tutorials skip**

You will often see:

$$
\alpha
=
I^{-1}\tau.
$$

That is exactly correct in certain common situations:

- rotation along fixed principal axis,
- spherical inertia,
- certain approximations,
- when using the simplified game-engine angular update.

But the full 3D rigid-body equation is:

$$
\boxed{
\tau
=
\frac{dL}{dt}.
}
$$

And:

$$
L=I_w\omega.
$$

Differentiate:

$$
\tau
=
I_w\dot\omega
+
\dot I_w\omega.
$$

Since world inertia changes as the body rotates:

$$
\dot I_w\omega
=
\omega\times(I_w\omega).
$$

Therefore:

$$
\boxed{
\tau
=
I_w\alpha
+
\omega\times(I_w\omega).
}
$$

Rearrange:

$$
\boxed{
\alpha
=
I_w^{-1}
\left[
\tau
-
\omega\times(I_w\omega)
\right].
}
$$

The extra:

$$
\omega\times L
$$

term is the **gyroscopic term**.

---

**Why gyroscopic motion exists**

Imagine a freely rotating asymmetric rigid body with:

$$
\tau=0.
$$

Naive formula:

$$
\alpha=I^{-1}\tau=0
$$

would imply angular velocity never changes.

But a torque-free asymmetric rigid body can have changing angular velocity vector in body coordinates while conserving angular momentum.

That is Euler's rigid-body dynamics.

The behavior behind:

```text
tennis-racket theorem
intermediate-axis instability
```

comes from this structure.

---

**Kairo currently uses the common simplified angular integration**

Current `PhysicsWorld` computes:

```cpp
body.State.AngularVelocity +=
    WorldInverseInertia(body) *
    body.Forces.Torque *
    dt;
```

which corresponds to:

$$
\dot\omega
\approx
I_w^{-1}\tau.
$$

The gyroscopic term:

$$
-I_w^{-1}
[
\omega\times(I_w\omega)
]
$$

is not present.

That is a common game-engine simplification, especially for stability and ordinary gameplay rigid bodies.

But if you are using the engine for **Reality Probe / physical-replication experiments**, this distinction becomes scientifically relevant.

For high-spin asymmetric bodies, the simplified model does not reproduce full torque-free rigid-body dynamics.

---

**Full angular acceleration helper**

Conceptually:

```cpp
Vec3f ComputeAngularAcceleration(
    const Matrix3f& worldInertia,
    const Matrix3f& worldInverseInertia,
    const Vec3f& angularVelocity,
    const Vec3f& torque)
{
    // Angular momentum in world coordinates.
    const Vec3f angularMomentum =
        worldInertia *
        angularVelocity;

    // Full Euler rigid-body equation:
    //
    // τ = I α + ω × (I ω)
    //
    // therefore:
    //
    // α = I^-1 [τ - ω × (Iω)]
    return worldInverseInertia *
        (
            torque -
            Cross(
                angularVelocity,
                angularMomentum)
        );
}
```

For a sphere:

$$
I=cI_3.
$$

Then:

$$
I\omega=c\omega.
$$

Therefore:

$$
\omega\times I\omega
=
c(\omega\times\omega)
=
0.
$$

So the simplified and full equations coincide.

That is why the issue often goes unnoticed with spheres.

---

### KAIRO AUDIT 1 — `MakeMassProperties()` AND NON-DIAGONAL INERTIA

This is the strongest implementation finding in this pass.

Current:

```cpp
return
{
    mass,
    InverseMassFor(mass),
    localCenterOfMass,
    localInertiaTensor,
    mass > 0.0f
        ? InverseDiagonalInertia(
            localInertiaTensor)
        : Matrix3f::Zero()
};
```

The function is named:

```text
MakeMassProperties
```

and accepts arbitrary:

```text
Matrix3f localInertiaTensor.
```

But then computes inverse assuming diagonal.

That is valid only if:

$$
I_{xy}=I_{xz}=I_{yz}=0.
$$

For sphere/centered box/capsule local principal-frame tensors:

correct.

For a generic custom tensor:

potentially wrong.

For an off-axis compound tensor:

potentially wrong.

---

**Concrete counterexample**

Suppose after a parallel-axis shift:

$$
I=
\begin{bmatrix}
2&-1&0\\
-1&2&0\\
0&0&4
\end{bmatrix}.
$$

Kairo diagonal reciprocal gives:

$$
I^{-1}_{wrong}
=
\begin{bmatrix}
0.5&0&0\\
0&0.5&0\\
0&0&0.25
\end{bmatrix}.
$$

Actual inverse of upper 2×2 block:

$$
\begin{bmatrix}
2&-1\\
-1&2
\end{bmatrix}^{-1}
=
\frac13
\begin{bmatrix}
2&1\\
1&2
\end{bmatrix}.
$$

So true inverse:

$$
I^{-1}
=
\begin{bmatrix}
0.6667&0.3333&0\\
0.3333&0.6667&0\\
0&0&0.25
\end{bmatrix}.
$$

The difference is significant.

Torque along x can generate angular acceleration with y component.

The diagonal approximation cannot express that.

---

**Correct design options**

You have three clean approaches.

**Option A — restrict the API**

Rename/constrain:

```text
MakePrincipalAxisMassProperties
```

and explicitly require diagonal inertia.

Then compound builder must diagonalize into principal axes before storing.

**Option B — implement full symmetric inverse**

Accept general symmetric positive-definite tensor and compute full inverse.

**Option C — store principal inertia + principal orientation**

Store:

```cpp
struct MassProperties
{
    float Mass;
    Vec3f LocalCenterOfMass;

    Vec3f PrincipalInertia;
    Quaternionf PrincipalRotation;
};
```

Then reconstruct:

$$
I^{-1}
=
R_p
diag(
1/I_1,
1/I_2,
1/I_3)
R_p^T.
$$

Option C is particularly elegant for rigid-body engines.

---

**A coherent full-tensor inverse helper**

For positive-definite dynamic inertia:

```cpp
Matrix3f InverseSymmetricInertia(
    const Matrix3f& I)
{
    // A physical inertia tensor is symmetric:
    //
    // [ a b c ]
    // [ b d e ]
    // [ c e f ]
    const float a = I(0, 0);
    const float b = I(0, 1);
    const float c = I(0, 2);
    const float d = I(1, 1);
    const float e = I(1, 2);
    const float f = I(2, 2);

    const float determinant =
        a * (d * f - e * e)
        - b * (b * f - c * e)
        + c * (b * e - c * d);

    // For a dynamic full-3D rigid body the inertia should
    // be positive definite, therefore determinant > 0.
    // A production engine should use a scale-aware threshold.
    if (determinant <= 1.0e-10f)
    {
        throw std::invalid_argument(
            "Dynamic inertia tensor is singular "
            "or not positive definite.");
    }

    const float invDet =
        1.0f / determinant;

    return Matrix3f
    {
        (d * f - e * e) * invDet,
        (c * e - b * f) * invDet,
        (b * e - c * d) * invDet,

        (c * e - b * f) * invDet,
        (a * f - c * c) * invDet,
        (b * c - a * e) * invDet,

        (b * e - c * d) * invDet,
        (b * c - a * e) * invDet,
        (a * d - b * b) * invDet
    };
}
```

For locked rotational axes, I would rather use principal-axis representation/pseudoinverse than attempt to overload singular general matrices with ambiguous semantics.

---

### KAIRO AUDIT 2 — CENTER-OF-MASS CONVENTION

Current `MassProperties` stores:

```cpp
Vec3f LocalCenterOfMass;
```

which implies body origin and COM can differ.

But `AddBodyForceAtPoint()` currently passes:

```cpp
record.State.Position
```

as:

```text
worldCenterOfMass.
```

At the same time collider world position is:

```cpp
body.State.Position +
Rotate(
    body.State.Rotation,
    collider.LocalCenter);
```

So current physics code effectively treats:

$$
State.Position
$$

as both:

```text
body transform origin
```

and:

```text
world center of mass.
```

Those are only the same when:

$$
LocalCenterOfMass=0.
$$

For current symmetric primitive shapes:

that is true.

For a future asymmetric compound:

not necessarily.

---

**Two valid conventions**

You should choose one explicitly.

**Convention A — simulation position is COM**

Store:

$$
State.Position
=
WorldCOM.
$$

Then geometry/collider origin must be offset from COM.

If authoring body origin has local COM:

$$
c_{local},
$$

world point from local body coordinates x becomes:

$$
x_w
=
COM_w
+
R(x_{local}-c_{local}).
$$

Collider center therefore becomes:

$$
COM_w
+
R(
ColliderLocalCenter-
LocalCOM).
$$

This is mathematically clean for dynamics.

**Convention B — simulation position is body origin**

Then world COM is:

$$
\boxed{
c_w
=
x_{origin}
+
Rc_{local}.
}
$$

Force lever arm uses:

$$
r=
p-c_w.
$$

The integration of origin becomes more complicated because COM is what naturally follows Newton's equations.

Most rigid-body solvers strongly prefer convention A internally.

---

**Recommended Kairo direction**

For physics internals:

$$
\boxed{
State.Position
=
WorldCenterOfMass.
}
$$

Then keep:

```text
authoring transform origin
```

separate through an offset.

Collider world center:

```cpp
Vec3f WorldColliderCenter(
    const RigidBody& body,
    const Collider& collider)
{
    const Vec3f colliderFromCOM =
        collider.LocalCenter -
        body.Mass.LocalCenterOfMass;

    return body.State.Position +
        Rotate(
            body.State.Rotation,
            colliderFromCOM);
}
```

Now all force, impulse and solver formulas naturally use:

```cpp
State.Position
```

as COM.

And `LocalCenterOfMass` has a real role.

---

### KAIRO AUDIT 3 — COMPOUND CHILD ROTATION

Current compound function receives:

```cpp
std::vector<MassProperties>
```

but no child orientation.

Therefore it can only correctly combine arbitrary shapes if each child tensor has already been transformed into the common body-local coordinate frame.

A more explicit part structure would be:

```cpp
struct MassPropertyPart
{
    float Mass;

    Vec3f LocalCenterOfMass;

    Quaternionf LocalRotation;

    Matrix3f InertiaAboutOwnCOM;
};
```

Then:

```cpp
Matrix3f childRotation =
    ToMatrix3(
        part.LocalRotation);

Matrix3f orientedInertia =
    childRotation *
    part.InertiaAboutOwnCOM *
    Transpose(childRotation);

Matrix3f shifted =
    ParallelAxisTheorem(
        orientedInertia,
        part.Mass,
        part.LocalCenterOfMass -
            combinedCOM);
```

Now arbitrary compound geometry works.

---

<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/03 - Game Engine/00 - Reading Map|Reading map]] · [[Chapter Edition/03 - Game Engine/34 - Rigid-body motion from forces to contact|Previous chapter]] · [[Chapter Edition/03 - Game Engine/36 - Auditing capsule mass properties and collision formulas|Next chapter]]
