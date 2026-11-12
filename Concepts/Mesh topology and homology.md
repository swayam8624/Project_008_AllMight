# Mesh topology and homology

Mesh topology describes adjacency and the existence of components, loops, boundaries and fillings independently of vertex coordinates. Homology formalizes which cycles fail to be boundaries. Euler characteristic is a useful consistency check under declared assumptions, but equal Euler characteristic does not establish equal topology in arbitrary complexes.

Read the connected explanation in [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#From a scanned triangle soup to a mesh we can trust]].

## Mastery expansion — connectivity is separate from embedding

A triangle mesh has combinatorial topology—vertices, edges, faces and incidence—and geometric embedding—positions, normals and other attributes. Moving vertices changes geometry without necessarily changing topology; splitting/collapsing edges can change both.

For a closed orientable connected surface, Euler characteristic (\chi=V-E+F) relates to genus by (\chi=2-2g). With boundaries/components, interpretation changes and should be derived from the actual complex rather than applying the closed-surface formula blindly.

A half-edge or equivalent connectivity structure should make boundary edges, twins, incident faces and vertex neighborhoods explicit. Non-manifold edges/vertices violate assumptions used by many smoothing, parameterization and subdivision algorithms.

**Required next witness:** construct adjacency from indexed triangles, classify boundaries/non-manifold cases, compute Euler characteristic and verify invariants before and after edge operations.

