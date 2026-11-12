# Anisotropic microfacets, dielectric Fresnel, and transmission

A physically based surface model must distinguish roughness orientation, interface reflection, and refraction. Treating all three as one "specular amount" hides different conservation and sampling rules.

## Anisotropic GGX

For tangent-space microfacet normal $m=(m_x,m_y,m_z)$ and directional roughnesses $alpha_x,alpha_y$, an anisotropic GGX normal distribution can be written

$$
D(m)=
rac{1}
{pialpha_xalpha_y
left(
m_x^2/alpha_x^2+
m_y^2/alpha_y^2+
m_z^2
ight)^2},
qquad m_z>0.
$$

Its projected-area normalization remains

$$
int_{Omega^+}D(m)(ncdot m),domega=1.
$$

Anisotropy therefore changes lobe shape, not the amount of projected microfacet area available.

## Exact dielectric Fresnel

At normal incidence between indices $eta_i$ and $eta_t$,

$$
F_0=
left(
rac{eta_i-eta_t}{eta_i+eta_t}
ight)^2.
$$

At grazing incidence Fresnel reflection approaches one. Schlick is a useful approximation for many cases, but exact dielectric Fresnel is the right oracle when validating interface math or transmission.

Snell's law supplies the transmitted angle:

$$
eta_isin	heta_i=eta_tsin	heta_t.
$$

When light travels from denser to rarer medium and the computed $sin	heta_t>1$, no real transmitted direction exists: total internal reflection occurs and Fresnel reflectance is one.

## Furnace reasoning

A white furnace test surrounds a material with uniform unit radiance and checks whether the material returns more energy than arrives. Lambertian reflection integrates to its albedo exactly. A complete microfacet material requires reflection, transmission, masking-shadowing and often multiple-scattering compensation to satisfy its intended energy model; testing only a highlight image is insufficient.

## Executable reference

`tools/labs/pbr_extended_reference_lab.cpp` numerically verifies anisotropic GGX projected normalization, exact dielectric Fresnel at normal and grazing limits, Snell refraction and total internal reflection, and a Lambertian white-furnace integral. The earlier `pbr_reference_lab.cpp` separately verifies isotropic GGX normalization, Smith-based reciprocity/finiteness and Schlick endpoints.

Related: [[Microfacet normal distributions]], [[Surface tangents normals and bitangents]], [[Light spectra and radiometry]].
