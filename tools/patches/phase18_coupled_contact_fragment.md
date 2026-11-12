That closes the original rigid-body mechanics reconstruction **through Part 1700**. It does **not** close Parts 1701–1800: the supplied lesson-body provenance for that range is still missing, so the following contact/constraint material is an explicit reconstruction and deepening rather than a claim that the original source text was preserved.

## From one collision impulse to a coupled contact problem — reconstructed Phase 18 bridge

A single isolated impact can be solved with one scalar impulse. A resting crate immediately breaks that mental model. Put two contact points under the same rigid body. An impulse at the left point changes the center-of-mass velocity and the angular velocity, so the right point's relative velocity changes too. The two contacts are therefore coupled.

For contact (i), let the world point be (mathbf p_i), let the normal (mathbf n_i) point from body (A) toward body (B), and define the lever arms

$$
mathbf r_{Ai}=mathbf p_i-mathbf x_A,
qquad
mathbf r_{Bi}=mathbf p_i-mathbf x_B.
$$

The point velocities are

$$
mathbf v_{Ai}
=
mathbf v_A+oldsymbolomega_A	imesmathbf r_{Ai},
qquad
mathbf v_{Bi}
=
mathbf v_B+oldsymbolomega_B	imesmathbf r_{Bi},
$$

so the signed normal relative speed is

$$
v_{n,i}
=
mathbf n_i^mathsf T(mathbf v_{Bi}-mathbf v_{Ai}).
$$

With this convention, (v_{n,i}<0) means the points are closing. A normal impulse magnitude (lambda_ige0) applies (lambda_imathbf n_i) to (B) and the equal-and-opposite impulse to (A). The non-negativity is physical: ordinary contact may push, but it may not attract.

Stack the contact impulses into (oldsymbollambda). After applying all of them, the vector of contact normal speeds has the form

$$
oxed{
mathbf w
=
mathbf v_n^-+
Koldsymbollambda
}
$$

where (mathbf v_n^-) contains the pre-solve contact speeds and (K) is the contact-space inverse effective-mass matrix. In Jacobian form,

$$
oxed{
K=JM^{-1}J^mathsf T.
}
$$

This expression is not decorative notation. (J^mathsf Toldsymbollambda) maps contact impulses into generalized body impulses; (M^{-1}) turns those impulses into generalized velocity changes; (J) projects those changed body velocities back into contact-space velocities. The off-diagonal entry (K_{ij}) therefore measures how impulse (j) changes contact speed (i).

For a planar two-body problem with a common normal direction, the matrix entry reduces to

$$
K_{ij}
=
m_A^{-1}+m_B^{-1}
+
I_A^{-1}
(mathbf r_{Ai}	imesmathbf n)
(mathbf r_{Aj}	imesmathbf n)
+
I_B^{-1}
(mathbf r_{Bi}	imesmathbf n)
(mathbf r_{Bj}	imesmathbf n).
$$

If all off-diagonal entries vanished, the contacts would be independent. A rigid body generally makes them nonzero.

### Contact is a complementarity condition

A resting contact does not mean “always force the normal velocity to zero.” If the bodies are separating, the contact should apply no normal impulse. For the frictionless, zero-restitution velocity problem, the unilateral conditions are

$$
oxed{
oldsymbollambdage0,
qquad
mathbf wge0,
qquad
lambda_i w_i=0.
}
$$

The first condition forbids pulling impulses. The second forbids residual closing velocity. The third states that either a contact carries positive impulse and sits on the active boundary (w_i=0), or it is inactive and may separate with (lambda_i=0).

### A symmetric two-point trace

Take a unit-mass planar crate with unit inertia falling at (2,mathrm{m/s}) onto a static floor. Put contacts one metre to the left and right of the center of mass. The matrix is

$$
K=
egin{bmatrix}
2&0\
0&2
end{bmatrix},
qquad
mathbf v_n^-=
egin{bmatrix}
-2\
-2
end{bmatrix}.
$$

Assuming both contacts are active, solve (mathbf w=0):

$$
0=
egin{bmatrix}
-2\
-2
end{bmatrix}
+
egin{bmatrix}
2&0\
0&2
end{bmatrix}
egin{bmatrix}
lambda_1\
lambda_2
end{bmatrix},
$$

so

$$
oxed{lambda_1=lambda_2=1,mathrm{N,s}.}
$$

Their total upward impulse cancels the crate's downward linear momentum, while their opposite moments cancel angular response.

Now move the right contact to (x=2) while keeping the left at (x=-1). Then

$$
K=
egin{bmatrix}
2&-1\
-1&5
end{bmatrix}.
$$

Solving the coupled system gives

$$
oxed{
lambda_1=rac43,
qquad
lambda_2=rac23.
}
$$

The impulses are unequal because the geometry is asymmetric. Treating the contacts as two isolated collisions would miss this coupling.

### Why “invert the matrix” is not the whole algorithm

A candidate two-contact solution can contain a negative impulse, which means the assumed active set is wrong. A complete 2×2 block solver must therefore test the possible active sets: both active, only the first active, only the second active, or neither active. For each candidate it computes

$$
mathbf w=mathbf v_n^-+Koldsymbollambda
$$

and accepts only solutions satisfying the complementarity inequalities within a declared numerical tolerance.

A singular (K) also needs an explicit policy. Duplicate contact rows can make the matrix rank deficient because two rows encode the same physical restriction. Blind inversion is numerically invalid, but unconditional rejection is also too crude because one independent row may still satisfy the contact. The executable witness below therefore tries the full two-contact solve only when its determinant is safely nonzero, then falls back to one-active-contact candidates and finally the no-impulse case.

### Sequential impulses and block solving are two views of the same coupled system

The existing persistent-contact laboratory uses projected sequential impulses. It visits one constraint row, changes the body velocity immediately, then evaluates the next row using the updated state. Repeated sweeps approximate the coupled solution. The block solve below instead solves two normal rows together.

The important invariant is not “always use one solver.” Every constraint row must expose a Jacobian, effective mass, target velocity or bias, admissible impulse set, accumulated impulse semantics and lifetime. Once those contracts are explicit, projected Gauss-Seidel, block solving, Jacobi-like variants, direct small systems and more general optimization methods can be compared honestly.

### Restitution, penetration correction and friction are separate policies

This bridge solves only the clean velocity-level normal problem. Restitution changes the target velocity for a new impact. Penetration is positional error and needs a distinct stabilization choice such as Baumgarte bias, split impulses, compliant constraints or position projection. Friction adds tangential constraints bounded by the normal impulse; in 2-D,

$$
-mulambda_n
le
lambda_t
le
mulambda_n.
$$

None of these should be silently folded into “the normal impulse formula.” Their energy behavior, timestep dependence and persistence rules differ.

### Complete two-contact executable witness

The following C++20 laboratory is intentionally narrower than a production solver. It proves the algebraic bridge from rigid-body impulse response to a two-contact unilateral block solve, including symmetric and asymmetric traces, one-active-contact behavior, already-separating contacts, a rank-deficient duplicate-contact case, immovable closing geometry and hostile normal input. It does **not** prove 3-D manifold solving, persistent friction, restitution caching, joints, island construction, warm starting across frames, CCD, position stabilization or Kairo integration.

```cpp
{{TWO_POINT_CONTACT_BLOCK_LAB}}
```

This section therefore raises one Phase-18 subproblem to **S4 reference depth**: the forcing problem is explicit, the matrix is derived, hand traces are reproducible, alternate branches and degeneracy are represented in executable code, and negative cases exist. It does not make Phase 18 globally S4 or S5.

Most importantly, Parts 1701–1800 retain the provenance limitation identified by the Bible audit. Their original supplied lesson-body source was absent from the retained source batches used for this reconstruction. The current material is therefore labelled as reconstructed canonical teaching built from the mechanics already established, inspected physics sources elsewhere in the repository, and executable reference models. No amount of new prose or passing CPU tests can retroactively prove that the missing original lesson body was preserved.

With that distinction made, the narrative can leave rigid-body mechanics honestly: Part 1700 closes the body-state and integration foundation; this bridge establishes why multiple unilateral contacts become a coupled constraint problem; the deeper Phase-18 source/provenance denominator remains open.
