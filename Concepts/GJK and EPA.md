# GJK and EPA

GJK answers geometric questions about two convex shapes by working with their Minkowski difference

$$
A-B=\{a-b\mid a\in A,b\in B\}.
$$

The shapes intersect exactly when the origin lies inside $A-B$. The algorithm does not construct that difference explicitly. It asks each shape for an extreme support point. For search direction $d$,

$$
support_{A-B}(d)=support_A(d)-support_B(-d).
$$

Each support query adds one point, and GJK maintains a simplex containing at most four support points in 3D.

## Simplex evolution

A simplex may be a point, segment, triangle or tetrahedron. The update step determines which Voronoi region of the simplex can still contain the closest point to the origin, discards vertices that cannot matter, and produces a new search direction.

This geometric classification is the heart of GJK. Memorizing a chain of cross products without the region argument makes degeneracy bugs almost impossible to diagnose.

A production implementation must define behavior for repeated support points, nearly collinear segments, nearly coplanar triangles, tiny search directions and finite iteration limits.

Intersection-GJK and distance-GJK use related simplexes but do not have identical termination rules. A condition sufficient to say “origin cannot be passed in this direction” is not automatically a high-quality distance convergence test.

## Termination and finite precision

Exact zero comparisons are brittle. Useful termination signals include lack of support progress, lack of distance improvement, repeated support points, simplex containment within tolerance, and an explicit maximum iteration count.

Every exit should identify the reason. An iteration cap is “uncertain/no convergence” unless the algorithm has another conservative proof of separation or intersection.

## EPA after intersection

When GJK produces an appropriate simplex/polytope enclosing the origin, EPA estimates penetration by refining the boundary of that Minkowski polytope.

The core loop is:

1. find the face closest to the origin;
2. query a support point in the outward face normal;
3. compare the new support distance with the current face distance;
4. if their gap is within tolerance, return the face normal/depth approximation;
5. otherwise remove faces visible from the new support point;
6. extract the horizon boundary;
7. connect the new point to the horizon and continue.

EPA therefore owns topology as well as geometry. Incorrect face orientation or horizon ordering can create an inside-out polytope even when GJK was correct.

Finite support-gap tolerance gives an approximation. EPA should not be described as exact penetration recovery for arbitrary floating-point convex shapes.

## Contact generation is later

A penetration normal and depth are not a persistent contact manifold. The physics engine still needs witness/contact points, feature identity, clipping or support-feature reconstruction, warm-start keys, friction basis and lifetime policy.

Read the connected explanation in [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#From a Scene to a Collision Answer: Geometry as a Chain of Contracts]].

Related: [[Continuous collision detection]], [[Minkowski sums and support mappings]], [[Continuous Notes/Game Engine - From First Frame to Living Worlds#Complete persistent-contact implementation and cache policy|Persistent contact manifolds]].
