# Metric gradients and differential measurements

The differential is a linear scalar measurement of displacement. The metric gradient is the vector representing that measurement under the chosen inner product: solve G times gradient equals derivative coefficients.

[[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#A derivative measures change; a gradient needs a ruler|Full definitions, derivation and worked trace]].

## Mastery expansion — the gradient depends on the metric used to identify covectors with vectors

The differential (df) is fundamentally a linear functional: it maps a perturbation (v) to the first-order change (df(v)). A gradient vector is obtained only after choosing an inner product/metric (g) such that (df(v)=g(\nabla f,v)).

In Euclidean orthonormal coordinates, this identification is invisible and the gradient components are just partial derivatives. In a non-orthonormal coordinate basis with metric matrix (G), the coordinate vector of the gradient satisfies (G\,\nabla f = \partial f), hence (\nabla f=G^{-1}\partial f).

This distinction matters in curvilinear coordinates, optimization with non-Euclidean metrics, surface geometry and when interpreting covectors such as normals.

**Required next witness:** evaluate one scalar field in a skew basis and show that raw partial-derivative coordinates and the metric-correct gradient produce different vectors but the same directional derivative.

