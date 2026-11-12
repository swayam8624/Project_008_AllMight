# QEM edge collapse and validity

Mesh simplification is not “remove the shortest edge.” A collapse changes both geometry and topology, so a useful cost must estimate geometric error while a validity gate protects the mesh from illegal connectivity or orientation changes.

## Plane quadrics

For a face plane

$$
ax+by+cz+d=0
$$

with normalized normal $(a,b,c)$, write the homogeneous plane vector

$$
p=(a,b,c,d)^{\mathsf T}.
$$

The squared distance-like penalty of homogeneous vertex $\bar v=(x,y,z,1)^{\mathsf T}$ to that plane is

$$
(p^{\mathsf T}\bar v)^2
=
\bar v^{\mathsf T}(pp^{\mathsf T})\bar v.
$$

So the face contributes quadric

$$
K_p=pp^{\mathsf T}.
$$

Sum incident face quadrics around a vertex. For candidate edge $(u,v)$, combine

$$
Q=Q_u+Q_v.
$$

Evaluate a proposed replacement position $\bar x$ using

$$
E(x)=\bar x^{\mathsf T}Q\bar x.
$$

The optimal unconstrained position comes from the upper-left linear system when it is nonsingular. If that solve is unstable or singular, candidates such as endpoints and midpoint provide robust fallbacks.

## Topology gate before cost

A low QEM cost does not make a collapse legal. Before committing, check conditions such as:

- the link condition or equivalent local-manifold test;
- boundary preservation policy;
- no face becomes degenerate by repeated vertex identity;
- no surviving triangle flips orientation beyond the chosen geometric tolerance;
- attributes/seams that the simplifier promises to preserve;
- no non-manifold edge or vertex is created.

The exact validity policy depends on whether the pipeline allows topology changes, but it must be explicit.

## Priority queue invalidation

A simplifier often keeps edge candidates in a heap. After one collapse, nearby quadrics/connectivity change, so old candidate costs become stale. Use version stamps/generations or recompute on pop. Otherwise a heap can return a numerically small cost for an edge that no longer exists in the same neighborhood.

## Verification

Compare simplification against invariants after every collapse: manifold/boundary status, valid indices, nonzero face area, finite coordinates and expected Euler behavior. Geometry error should be measured on the resulting surface rather than inferred only from the sum of local QEM costs.

The current branch now provides tools/labs/mesh_topology_lab.cpp for the topology layer and the existing cotangent-energy witness for discrete operators. A full executable QEM collapse loop remains the next implementation milestone rather than being silently claimed complete.

Related: [[Half-edge topology invariants]], [[Mesh topology and homology]].

## Executable collapse layer

`tools/labs/qem_edge_collapse_lab.cpp` now implements the first complete QEM collapse witness instead of leaving this section at topology-only prose. It constructs normalized face planes, accumulates $pp^T$ quadrics into vertices, combines the two endpoint quadrics for an edge, evaluates candidate cost in homogeneous coordinates, and runs a geometric validity gate before accepting the collapse.

The validity witness deliberately distinguishes a triangle that disappears because it contains both collapsed endpoints from a surviving triangle that becomes degenerate or flips. The former is expected topology change; the latter is rejected. This still leaves the optimal $3\times3$ solve, link-condition proof, boundary-feature weights, attribute quadrics and progressive-mesh bookkeeping as higher layers, but QEM is no longer merely named.

