# Microfacet normal distributions

A normal distribution function describes unresolved surface orientation statistics. Its projected-area normalization includes the cosine to the macroscopic normal. A sampling PDF must therefore not confuse the NDF alone with a probability density over solid angle.

Teach and reconstruct: [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Ask a surface where incoming energy can go]].

Related integration record: [[Supplementary/Manual Integration - Sources and Verification]].

## Mastery expansion — the distribution is measured over microfacet orientation

A microfacet BRDF models a rough surface as unresolved oriented facets. The normal-distribution function (D(m)) describes the density of microfacet normals around direction (m); it is not itself a probability density over solid angle unless the appropriate projected-measure factor and normalization are accounted for.

For an isotropic GGX/Trowbridge-Reitz model, one common form is

[
D(m)=\frac{\alpha^2}{\pi\left((n\cdot m)^2(\alpha^2-1)+1\right)^2}.
]

The BRDF combines the normal distribution with Fresnel (F), masking-shadowing (G), and the projected-area denominator:

[
f_r(v,l)=\frac{D(h)F(v,h)G(v,l)}
{4|n\cdot v||n\cdot l|}.
]

Every term has a geometric role. (D) says how much facet orientation is available; (F) says how much of an incident wave reflects at an interface; (G) accounts for facets hidden by other facets; the denominator converts between projected measures.

The implementation must define how roughness maps to (alpha), how zero/near-zero roughness is handled, which hemisphere conventions apply, and what happens at grazing angles. Sampling must use a PDF consistent with the chosen sampling strategy; using a correct BRDF with the wrong PDF still biases a Monte Carlo estimator.

**Required next witness:** hemispherical numerical integration over several roughness/F0 cases, finite-output edge tests, reciprocity checks where the model applies, and comparison against a trusted reference implementation.

