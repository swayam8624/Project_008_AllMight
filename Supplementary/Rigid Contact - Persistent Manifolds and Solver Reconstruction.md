# Rigid contacts: from point velocity to a persistent contact manifold

> Scope: original, independently reconstructed teaching for the existing engine collision-response narrative. The executable witnesses are [rigid_contact_lab.cpp](../tools/labs/rigid_contact_lab.cpp) and [persistent_contact_lab.cpp](../tools/labs/persistent_contact_lab.cpp). These are **2D velocity-level CPU witnesses**; they do not claim to be Kairo's engine implementation, 3D production manifold generation, or continuous collision detection. Symbols and sign conventions below follow those two witnesses.

## Why a contact is a constraint rather than a force chosen by eye

Imagine a box descending onto a static floor. The collision detector reports that the geometry is touching or overlapping, but it does not say which velocities are physically admissible. With body A as the floor and body B as the box, define the contact normal **n** to point from A toward B. At contact position **p**, the lever arms measured from each body's centre of mass are **r_A = p − x_A** and **r_B = p − x_B**. For a planar rigid body, its point velocity is **v(p) = v_com + ω × r**, where planar scalar angular speed ω produces **ω × (r_x,r_y) = (−ω r_y, ω r_x)**. This matters because an object rotating toward the floor can be closing at a corner even when the centre of mass is moving away.

The relative velocity of the two material points is **u = (v_B + ω_B × r_B) − (v_A + ω_A × r_A)**. The normal component **u_n = u · n** tells us whether those two points are closing (**u_n < 0**) or separating (**u_n > 0**). For a non-penetrating unilateral contact without restitution, the desired post-solve condition is **u_n ≥ 0**, with a nonnegative accumulated contact impulse **λ_n ≥ 0**. Their complementarity condition is **λ_n u_n = 0** at an exact solution: either the contact carries a positive separating-prevention impulse and reaches zero normal speed, or the bodies separate freely and the contact carries no impulse. Finite-iteration solvers only approximate this coupled condition.

## Deriving effective inverse mass, rather than memorizing its denominator

The impulse on B is **+λ n** and the reaction on A is **−λ n**. Newton's linear momentum law gives **Δv_B = +λ m_B⁻¹ n** and **Δv_A = −λ m_A⁻¹ n**. Angular impulse about the centre of mass gives **Δω_B = +λ I_B⁻¹ (r_B × n)** and **Δω_A = −λ I_A⁻¹ (r_A × n)**, where the planar cross product of vectors is the scalar **r_x n_y − r_y n_x**. A rotation changes point velocity through **Δω × r**, and its projection onto n adds an angular term proportional to the squared lever arm.

Consequently the normal relative velocity changes by **Δu_n = k_n λ**, where

$$
k_n = m_A^{-1} + m_B^{-1}
    + I_A^{-1}(r_A \times n)^2
    + I_B^{-1}(r_B \times n)^2.
$$

Here **k_n** is *inverse effective mass* in the contact direction, not mass itself. The same physical impulse changes normal speed less when the effective mass is large. The equation follows from the full point-velocity update; it is not valid to omit rotational terms whenever the contact is off-centre. A static body uses zero inverse mass and inertia, but two completely unresponsive bodies yield **k_n = 0** and must not be divided by. The corresponding tangential value **k_t** is obtained by substituting the perpendicular unit tangent **t = (−n_y, n_x)** for n.

For a desired post-impact normal speed **b**, one unconstrained correction is **Δλ = (b − u_n)/k_n**. A real contact cannot pull the bodies together, so the accumulated normal multiplier is projected:

$$
\lambda_n^{new}=\max\left(0,\lambda_n^{old}
 + \frac{b-u_n}{k_n}\right).
$$

**Apply only the difference**, **Δλ_n = λ_n^{new} − λ_n^{old}**, to body velocities. Applying the whole accumulated value every iteration would add the same historical impulse repeatedly, making motion energy grow artificially. Conversely, resetting the accumulator on every sweep defeats the projected Gauss–Seidel interpretation and prevents reliable friction bounds.

## Restitution is a contact-creation decision, not a new bounce each iteration

For an impact with pre-solve normal speed **u_n^before < −v_threshold**, the outgoing speed target in this simplified solver is **b = −e u_n^before**, with restitution coefficient **e ∈ [0,1]**. This target must be sampled **before** contact impulses change the velocity and then held fixed during the iteration. If a solver recomputes **b** from the current velocity at every visit, a persistent resting point can acquire artificial new bounce energy. The witness distinguishes a new impact from a persistent contact through an explicit `newImpact` argument; a production engine requires an actual contact-state machine and impact detection so the decision is not left to a careless caller.

For the central collision test, both bodies have mass 1 kg, A starts at 2 m/s, B at 0 m/s, and the normal is +x. Their relative closing speed is −2 m/s; with **e=1** and **k_n=2 kg⁻¹**, we need **Δλ_n = (2 − (−2))/2 = 2 kg·m/s**. The resulting velocities are 0 m/s and 2 m/s. Linear momentum remains 2 kg·m/s, and the kinetic energy remains 2 joules. This equality is specific to the elastic, frictionless, central, idealized two-body case—not a conservation promise for frictional or inelastic impacts.

## Why friction depends on the normal impulse

For the upward normal **n=(0,1)** the laboratory chooses the tangent **t=(−1,0)**. The sliding box initially moves at +3 m/s in x, so its *tangent-coordinate* velocity is **u_t = −3 m/s**. The unconstrained tangent impulse is **−u_t/k_t**; here **k_t=1 kg⁻¹**, giving +3 kg·m/s. The normal collision supplies **λ_n=2 kg·m/s**. With coefficient **μ=0.5**, the Coulomb limit is **μλ_n=1 kg·m/s**, so the stored tangent impulse is **+1**, not −1. Multiplying **+1 × (−1,0)** produces an x-directed impulse of **(−1,0)**, reducing the box's x velocity from +3 to +2 m/s. **The sign of the stored scalar is relative to the chosen tangent direction; it cannot be interpreted without that basis.**

The projected update is **λ_t^{new} = clamp(λ_t^{old} − u_t/k_t, −μλ_n, +μλ_n)**. This is a scalar tangent model for 2D; real 3D contact has a two-dimensional tangent plane and requires a disk-cone constraint or a chosen approximation. The witness applies a single coefficient and does not model static-versus-dynamic friction transition, anisotropy, rolling friction, torsional friction or compliant materials.

## Why a manifold needs persistent identity and sequential sweeps

A wide box can contact a floor at two corners. Both rows modify the **same** centre-of-mass velocity and angular velocity. Solving one row therefore changes the velocity seen by the next row. A one-pass independent pair of impulses is not equivalent to solving their coupled system. The reference witness stores two world-space contact points, evaluates each row in sequence, and repeats sweeps. The residual normal speeds and angular velocity are checked after twenty sweeps. Its iteration result is deterministic for its fixed row order but should not be advertised as an order-independent exact solution.

On the next frame, a geometry matcher must identify whether each *physical contact feature* persisted. If the same feature, body pair, contact orientation, and compatible basis survive, previously accumulated impulses are useful initial guesses. **Warm-start once** by applying the cached impulses to the newly constructed pre-solve velocities before iteration. Never warm-start twice, and never reuse a tangential scalar after rotating the tangent basis without transforming it. A changed feature ID, reversed A/B order, altered normal, teleport, or discontinuous mass/inertia change may invalidate the cache. The witness includes a feature ID as data but **does not implement feature matching**; that is a separate production requirement.

Caches may also require `dt`-aware scaling when timestep changes, and a true contact solver must decide how speculative contacts and missed or sleeping frames are handled. Warm-starting can improve convergence, but it is not a substitute for manifold quality.

## Contact identity: why matching by memory address or contact index is insufficient

The physical reaction at a point can be reused between frames only if the cache still refers to the same interacting surface features. When two faces touch, clipping may produce two manifold points whose array order changes as the bodies rotate. If we cache impulse #0 against array position #0, a later reordering can accidentally apply the left-corner impulse at the right corner, creating a torque that was never physically demanded. A robust narrow phase should assign each contact a stable feature pair, for example an incident vertex/edge identifier and reference face identifier, with deterministic canonicalization of body ordering. These identifiers are geometric facts maintained by collision generation, not sequential indices assigned by a container.

The executable [contact-cache witness](../tools/labs/contact_cache_lab.cpp) concentrates on the subsequent **policy** rather than inventing a complete polygon clipper. The `Key` holds canonical body IDs plus two feature identifiers. The map stores the previously accepted normal and tangent impulse, its local normal direction, the previous friction coefficient, timestep, and last-seen frame. The cache is valid only if the same key was seen on the immediately preceding frame, the normal has changed by no more than 15 degrees, and the new timestep is between half and twice the cached timestep. These are **documented laboratory thresholds**, not universally optimal thresholds or engine-wide facts. The code rejects reversed body order because reversing A and B changes the sign of the normal and impulse convention.

A tangent impulse is a vector in world space, although this 2D solver stores it as one scalar relative to a perpendicular tangent. When the normal rotates from **n_old** to **n_new**, the tangent axes **t_old=(-n_old.y,n_old.x)** and **t_new=(-n_new.y,n_new.x)** also rotate. The old physical friction impulse is **P_t_old=λ_t_old t_old**. The scalar along the new tangent is its dot projection:

$
\lambda_{t,new}^{seed} =
\frac{\Delta t_{new}}{\Delta t_{old}}
\left(\lambda_{t,old}\,\mathbf t_{old}\cdot\mathbf t_{new}\right).
$

The normal cache receives the same heuristic timestep ratio, and the tangent is clamped to **± μ_new λ_n_new**. With an old upward normal **(0,1)**, the old tangent is **(-1,0)**. If the normal turns 10 degrees, the tangent projection of a previous scalar impulse 0.5 is **0.5 cos(10°) ≈ 0.492404**, while the world impulse direction has changed consistently. Reusing 0.5 unchanged would preserve the scalar but **not** the intended projection onto the new contact tangent. This linear projection is a warm-start heuristic, not an exact solution of the new contact constraint; the subsequent iterative solver must still correct its residual.

An edit to the friction coefficient clears only the tangential cached impulse in this experiment, retaining the normal impulse if all other validity rules pass. A material change in restitution, velocity discontinuity, teleport, substantial mass/inertia change, sleeping transition, or topology discontinuity may also require cache rejection in a production system and is not automatically detected by this small input type. Impulse scaling by timestep ratio is similarly a common starting heuristic, not an unconditional physical identity; the test exercises ratios 0.5, 1, and 4 and rejects the last. A caller must invoke `begin_frame`, use `lookup` to seed each new manifold row, warm-start those rows **once**, run velocity iterations, `store` each final impulse exactly once, and `prune` stale entries. Pruning at the wrong moment can erase still-needed contacts; storing duplicate feature keys silently makes iteration results depend on array ordering, so the witness rejects duplicates.

The implementation has a testable boundary: it assumes a reliable feature matcher has already produced identifiers, that contact validity has been established by the geometry system, and that stable body IDs are never prematurely recycled. A generation-counted body handle would be safer than an integer object index alone. The cache does not detect 3D tangent-frame rotation, it does not calculate a physically accurate contact point, and its ordered map is a reference implementation rather than a measured real-time allocation strategy. A production engine should benchmark a preallocated flat hash map or per-pair cache with explicit frame ownership and deterministic iteration requirements.

## Velocity correction is not penetration correction

The equations above change velocities. They do not, by themselves, repair existing overlaps in positions. A production pipeline generally needs a distinct and carefully scoped policy: geometry generation and signed separation, speculative contact margins, split impulses or position-level stabilization, and consistent timestep handling. Naively adding a large position-error velocity bias can inject energy, make stacks jitter, or break restitution. This CPU witness intentionally has no position integrator, no Baumgarte term, no split impulse and no continuous collision detection. Therefore its passing tests cannot establish the correctness of CCD tunneling prevention or deep-overlap recovery.

## How to reconstruct and verify the witness

Read the `Row` fields as a prepared contact Jacobian in compressed form: **ra/rb** are the lever arms, **n/t** are the constraint axes, **kn/kt** are scalar rows of the matrix **J M⁻¹ Jᵀ**. The method `relative_velocity` computes **Jv**, `inverse_effective_mass` computes a row's inverse effective mass, `solve_one` projects impulse increments, and `warm_start` applies an already-owned cache once. `make_row` validates data and freezes the impact target. The six tests cover central momentum/energy, friction orientation, angular impulse, separating and static degeneracies, coupled two-contact iteration/next-frame cache, and nonfinite-input rejection.

Run strict compilation and execution, then run a sanitizer build. Expected output is `persistent-contact witness: 6 suites PASS`. This verifies only the tested cases on the compiling environment; the repository GitHub Actions workflow repeats strict and undefined-behavior checks, with remote status to be verified separately.

A reconstruction exercise is to add two bodies with unequal masses and an off-centre contact, compute **kn** by hand and compare every intermediate **u_n**, **Δλ_n**, **Δv** and **Δω**. A stronger follow-up exercise is to implement real feature matching, deliberately reverse normals between frames, and demonstrate that incorrectly reused cached tangential impulses generate artificial energy. Neither extension is presently claimed as implemented.

## Verified boundary and research questions

The lab does **not** verify Kairo's current renderer or physics engine, broadphase pair generation, narrowphase contact topology, 3D Jacobians, static/dynamic island management, precise multi-contact complementarity, numerical convergence across arbitrary scenes, determinism across compilers, robust CCD, or real-time performance. Before a research claim about stability or speed, define independent scene families, error metrics (maximum penetration, residual closing speed, drift, energy increase), iteration budgets, timestep families and order-randomization trials. Compare warm versus cold starts without changing geometry, feature matching or tolerances at the same time.
