# GJK and EPA

GJK searches the Minkowski difference of two convex shapes using support queries and small simplices. Its intersection and distance forms need explicit simplex, degeneracy and termination contracts. EPA starts from an appropriate enclosing simplex/polytope and refines a penetration estimate; finite support-gap convergence gives an approximation, not exact recovery for every shape.

Read the connected explanation in [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#From a Scene to a Collision Answer: Geometry as a Chain of Contracts]].

## Mastery expansion — support mappings turn convex collision into origin reasoning

For convex shapes (A) and (B), the Minkowski difference (A-B) contains the origin exactly when they intersect. A support mapping avoids constructing that difference:

$$
\operatorname{support}_{A-B}(d)=\operatorname{support}_A(d)-\operatorname{support}_B(-d).
$$

GJK iteratively builds a simplex of support points and asks whether the simplex can contain the origin. A simplex update must return both the retained simplex and a new search direction. Degenerate directions, duplicate support points and insufficient progress need explicit termination policy.

Distance-GJK variants track the closest simplex for separated shapes. For intersecting shapes, EPA expands a polytope around the origin and searches for the closest boundary face, producing an approximate penetration normal/depth when it converges.

Store witness points from the original shapes alongside each Minkowski support point; otherwise a final simplex/polytope cannot reconstruct meaningful contact features.

**Required next witness:** deterministic point/segment/triangle/tetrahedron simplex traces, duplicate-support termination, near-touching cases and EPA convergence/failure fixtures.

