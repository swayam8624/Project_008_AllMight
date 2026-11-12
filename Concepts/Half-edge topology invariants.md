# Half-edge topology invariants

A triangle index buffer tells the renderer which three vertices form each face, but many geometry-processing algorithms need explicit adjacency: which face lies across an edge, which edges surround a vertex, and where the mesh boundary is.

A half-edge representation splits every undirected edge into directed records attached to faces. For triangle face $(v_0,v_1,v_2)$, create directed half-edges

$$
v_0\rightarrow v_1,\quad
v_1\rightarrow v_2,\quad
v_2\rightarrow v_0.
$$

For a consistently oriented two-manifold interior edge, exactly two half-edges share the same undirected endpoints and they run in opposite directions. They become twins.

## Invariants

A useful topology builder should establish:

- every face vertex index is valid;
- a face does not repeat one of its own vertices;
- each directed edge appears at most once for an oriented manifold triangle mesh;
- an undirected edge with one incident face is a boundary edge;
- an undirected edge with two incident faces is manifold only when directions oppose;
- an undirected edge with more than two incident faces is non-manifold;
- twin links are symmetric;
- face-cycle next links return to the original half-edge.

Vertex-manifoldness needs an additional fan test. Merely checking that every edge has at most two incident faces does not reject a “bow-tie” vertex whose incident triangles form separate fans touching only at that vertex.

## Euler characteristic as a diagnostic

For a finite mesh complex,

$$
\chi = V-E+F.
$$

A topological disk has $\chi=1$. A closed sphere-like tetrahedron has $\chi=2$. Euler characteristic does not fully classify arbitrary meshes, but it is a powerful cross-check against adjacency construction.

## Why topology must be separated from geometry

Two faces can be topologically adjacent even if their coordinates are nearly coincident or wildly deformed. Conversely, two spatially overlapping triangles need not share topology. Build connectivity from explicit vertex/edge identity unless the task is specifically to weld geometric duplicates under a tolerance.

## Executable witness

tools/labs/mesh_topology_lab.cpp builds directed/undirected edge maps, pairs twins, counts boundaries and non-manifold edges, checks orientation, and validates Euler characteristic on a two-triangle disk and a closed tetrahedron. A third case deliberately creates a three-face non-manifold edge.

The witness does not yet prove vertex-fan manifoldness, polygonal faces, robust welding, edge collapse, remeshing or production importer parity.

Related: [[Half-edge mesh connectivity]], [[Mesh topology and homology]], [[QEM edge collapse and validity]].
