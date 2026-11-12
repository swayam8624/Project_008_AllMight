# Persistent homology

Persistent homology follows topological features through an ordered family of complexes called a filtration. Birth and death values measure persistence relative to that filtration; they are not direct proof that a feature is physically real. Sampling, metric, noise scale and construction assumptions determine what the result can support.

Read the connected explanation in [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#From a scanned triangle soup to a mesh we can trust]].

## Mastery expansion — topology across a filtration

Persistent homology tracks topological features while a threshold changes. A filtration is a nested sequence of complexes (K_0\subseteq K_1\subseteq\cdots\). Homology classes are born when a feature appears and die when it becomes a boundary or merges according to the chosen dimension/pairing convention.

The output is commonly represented as intervals ([b,d)) or points ((b,d)) in a persistence diagram. Long persistence can indicate structure stable over a range of thresholds, but “long means meaningful” is an application hypothesis, not a theorem about noise.

For geometry processing, persistence can help reason about connected components, loops and voids across scale or reconstruction thresholds. Correct interpretation requires stating the filtration value, coefficient field and homology dimension.

**Required next witness:** build a tiny simplicial filtration by hand, reduce its boundary matrix over (\mathbb{Z}_2), and recover H0/H1 birth-death pairs.

