# Mesh topology and homology

Mesh **geometry** concerns positions, lengths, angles and curvature. Mesh **topology** concerns connectivity: which vertices belong to edges, which edges bound faces, which faces meet, where boundaries exist, and how many connected/loop/void structures the complex contains.

Two meshes can have very different coordinates and the same topology. Two triangles can overlap geometrically without being topologically adjacent if they do not share vertex/edge identity.

## Simplicial-chain view

For a triangle mesh, vertices are 0-simplices, edges are 1-simplices and faces are 2-simplices. A chain is a formal linear combination of simplices.

Boundary operators satisfy

$$
\partial_k\partial_{k+1}=0,
$$

the algebraic statement that “the boundary of a boundary is zero.”

Cycles are chains whose boundary vanishes. Boundaries are cycles created as the boundary of a higher-dimensional chain. Homology quotients cycles by boundaries:

$$
H_k=Z_k/B_k.
$$

The Betti number $\beta_k$ counts independent homology classes over the chosen coefficient field.

For a connected orientable closed surface, $eta_0=1$; $eta_1$ captures independent loops/handles and $eta_2$ captures the enclosed oriented surface class.

## Euler characteristic

For a triangle complex,

$$
\chi=V-E+F.
$$

For a connected closed orientable surface of genus $g$,

$$
\chi=2-2g.
$$

But equal Euler characteristic alone does not prove arbitrary complexes are topologically equivalent. It is a checksum, not a complete classifier.

## Mesh diagnostics

Topology checks used by a geometry pipeline should include:

- boundary-edge count;
- non-manifold edges with more than two incident faces;
- inconsistent face orientation;
- disconnected components;
- vertex fan manifoldness;
- isolated vertices/edges;
- duplicate faces;
- Euler characteristic and, where appropriate, homology.

tools/labs/mesh_topology_lab.cpp now gives an executable layer for unique edges, half-edge twins, boundary/non-manifold classification and Euler characteristic.

Read the connected explanation in [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#From a scanned triangle soup to a mesh we can trust]].

Related: [[Half-edge topology invariants]], [[Persistent homology]], [[QEM edge collapse and validity]].
