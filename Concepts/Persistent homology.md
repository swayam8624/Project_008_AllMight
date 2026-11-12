# Persistent homology

Ordinary homology asks which connected components, loops and higher-dimensional voids exist in one topological complex. Persistent homology asks how those features appear and disappear as a scale or threshold changes.

Begin with a **filtration**:

$$
K_0\subseteq K_1\subseteq\cdots\subseteq K_m.
$$

Each step adds vertices, edges, faces or higher cells according to a declared rule. A topological feature is born when it first appears and dies when it becomes a boundary or merges according to the homology pairing convention.

Its persistence is often summarized by an interval

$$
[b,d)
$$

or a point $(b,d)$ in a persistence diagram.

## What persistence means

A long-lived feature is stable across a wider range of the chosen filtration parameter. That can make it more robust to one class of perturbation, but it is not proof that the feature is physically meaningful. The conclusion depends on:

- the distance or scalar field used;
- sampling density;
- noise/outlier model;
- complex construction such as Vietoris–Rips, Čech or alpha complex;
- coefficient field;
- filtration scale.

A persistent loop in a noisy scan can reflect real geometry, a sampling hole, or the behavior of the chosen complex.

## Union-find intuition for H0

For connected components, persistence can be understood with disjoint sets. Vertices are born as components. When an edge connects two previously separate components, one component dies at that edge’s filtration value according to a consistent elder-rule convention.

Higher-dimensional persistence requires boundary-matrix reduction or more specialized algorithms.

## Mesh/reconstruction relevance

Persistent homology can help diagnose captured geometry across thresholds: which holes survive changing reconstruction confidence, neighborhood radius or density? That is a better use than quoting a Betti number from one arbitrary threshold.

A study should compare the persistence structure with controlled synthetic holes/noise so the interpretation is calibrated.

## Evidence boundary

The repository’s mathematics expansion develops complexes, chains, boundaries, Betti reasoning and persistence concepts. It does not yet claim a production-scale persistent-homology solver or a theorem that a persistent feature corresponds to a physically correct surface.

Read the connected explanation in [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#From a scanned triangle soup to a mesh we can trust]].

Related: [[Mesh topology and homology]], [[Half-edge topology invariants]].
