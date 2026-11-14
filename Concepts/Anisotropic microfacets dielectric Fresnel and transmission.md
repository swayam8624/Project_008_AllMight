# Anisotropic microfacets, dielectric Fresnel, and transmission

A physically based surface model must distinguish roughness orientation, interface reflection, and refraction. Treating all three as one "specular amount" hides different conservation and sampling rules.

## Anisotropic GGX

Let $\mathbf m=(m_x,m_y,m_z)$ be a unit microfacet normal in the surface tangent frame, with $m_z>0$, and let the macrosurface normal be $\mathbf n=(0,0,1)$. The GGX slope parameters $\alpha_x,\alpha_y$ are positive and dimensionless. The anisotropic GGX normal distribution is

$$
D(\mathbf m)=
\frac{1}
{\pi\alpha_x\alpha_y
\left(
 m_x^2/\alpha_x^2+
 m_y^2/\alpha_y^2+
 m_z^2
\right)^2},
\qquad m_z>0.
$$

Its projected-area normalization is

$$
\int_{\Omega^+}D(\mathbf m)(\mathbf n\cdot\mathbf m)\,d\omega=1.
$$

Anisotropy therefore changes lobe shape, not the amount of projected microfacet area available.

## Exact dielectric Fresnel

Assume lossless, nonmagnetic dielectrics with positive refractive indices $\eta_i,\eta_t$; both angles are measured from the interface normal. At normal incidence the reflectance is

$$
F_0=
\left(
\frac{\eta_i-\eta_t}{\eta_i+\eta_t}
\right)^2.
$$

For unequal refractive indices, Fresnel reflection approaches one at grazing incidence. An index-matched interface ($\eta_i=\eta_t$) has zero reflectance instead. Schlick is a useful approximation for many cases, but exact dielectric Fresnel is the right oracle when validating interface math or transmission.

At oblique incidence, resolve the field into perpendicular ($s$) and parallel ($p$) polarizations. The power reflectances are

$$
F_s=
\left(
\frac{\eta_i\cos\theta_i-\eta_t\cos\theta_t}
{\eta_i\cos\theta_i+\eta_t\cos\theta_t}
\right)^2,
\qquad
F_p=
\left(
\frac{\eta_t\cos\theta_i-\eta_i\cos\theta_t}
{\eta_t\cos\theta_i+\eta_i\cos\theta_t}
\right)^2.
$$

For unpolarized incident light, $F=(F_s+F_p)/2$.

Snell's law supplies the transmitted angle:

$$
\eta_i\sin\theta_i=\eta_t\sin\theta_t.
$$

When light travels from the denser to the rarer medium and Snell's law would require $\sin\theta_t>1$, no real transmitted direction exists: total internal reflection occurs and, for a lossless dielectric, Fresnel reflectance is one.

## Furnace reasoning

A white furnace test surrounds a material with uniform unit radiance and checks whether the material returns more energy than arrives. Lambertian reflection integrates to its albedo exactly. A complete microfacet material requires reflection, transmission, masking-shadowing and often multiple-scattering compensation to satisfy its intended energy model; testing only a highlight image is insufficient.

## Executable reference

`tools/labs/pbr_extended_reference_lab.cpp` numerically verifies anisotropic GGX projected normalization, exact dielectric Fresnel at normal and grazing limits, Snell refraction and total internal reflection, and a Lambertian white-furnace integral. The earlier `pbr_reference_lab.cpp` separately verifies isotropic GGX normalization, Smith-based reciprocity/finiteness and Schlick endpoints.

Related: [[Microfacet normal distributions]], [[Surface tangents normals and bitangents]], [[Light spectra and radiometry]].
