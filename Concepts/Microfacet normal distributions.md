# Microfacet normal distributions

A microfacet model treats a macroscopically rough surface as a statistical collection of tiny planar facets. The **normal distribution function** $D(m)$ is not simply “the probability of a normal.” It measures the density of microfacet area oriented around direction $m$ relative to the macroscopic surface.

For an isotropic GGX distribution with roughness parameter $alpha$ and $c = n\cdot m$,

$$
D_{GGX}(m)=
\frac{\alpha^2}
{\pi\left[c^2(\alpha^2-1)+1\right]^2},
\qquad c>0.
$$

The important normalization is projected area:

$$
\int_{\Omega^+} D(m)(n\cdot m)\,d\omega_m = 1.
$$

That cosine factor is why $D(m)$ itself must not be substituted directly for a solid-angle sampling PDF without the transformation required by the sampling method.

## From NDF to BRDF

For reflection, the half-vector is

$$
h=\frac{\omega_i+\omega_o}{\|\omega_i+\omega_o\|}.
$$

A common Cook–Torrance microfacet BRDF is

$$
f_r(\omega_i,\omega_o)=
\frac{D(h)F(\omega_i,h)G(\omega_i,\omega_o)}
{4(n\cdot\omega_i)(n\cdot\omega_o)}.
$$

Each factor has a different job. $D$ describes facet orientation statistics. $F$ describes Fresnel reflection. $G$ accounts for masking and shadowing. The denominator converts the microfacet-area reasoning into a BRDF measured with respect to projected macroscopic area.

Confusing these roles produces common bugs: multiplying Fresnel twice, using roughness directly where $alpha=roughness^2$ was intended, or treating the NDF as the sampled-direction PDF.

## Numerical behavior

At very low roughness, the GGX lobe becomes extremely concentrated. A finite-precision implementation needs a policy for minimum roughness, denominator protection, and tangent-frame validity. Simply adding an arbitrary epsilon everywhere changes the model and can break energy behavior.

At grazing angles, both the geometry term and the denominator become sensitive. The correct response is to derive the limiting behavior and use stable forms, not to clamp intermediate terms independently until the image looks plausible.

## Executable reference

tools/labs/pbr_reference_lab.cpp numerically integrates the projected GGX normalization for several roughness values. It also checks Schlick Fresnel endpoint behavior and reciprocity/finiteness of an isotropic GGX + Smith BRDF.

Those checks establish mathematical contracts. They do not prove white-furnace energy conservation for the complete material model, anisotropic GGX, transmission, texture-space normal handling, or renderer parity.

Primary reading in the graphics volume: [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Ask a surface where incoming energy can go]].

Related: [[Multiple importance sampling]], [[Color encoding and linear-light operations]], [[Surface tangents normals and bitangents]].
