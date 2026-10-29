# Camera projection: shortest useful recall

```mermaid
flowchart LR
 L["Local point"] --> M["Model to world"] --> V["Inverse camera pose"] --> P["Projection to clip"] --> C["Clip inequalities"] --> D["Divide by valid w"] --> R["Viewport to raster"]
 class L,M,V,P blue; class C,D amber; class R green;
classDef blue fill:#e5eff9,stroke:#2463a6,color:#18354c;
classDef green fill:#e3f3ed,stroke:#16806a,color:#174b3c;
classDef amber fill:#fff0dc,stroke:#c57820,color:#64400d;
```


## Frames and transforms

- A coordinate needs a frame: $p=o+Bp_F$. Camera world-to-view is $p_V=R^T(p_W-c)$; points use homogeneous $w=1$, directions use $w=0$, and normals use $A^{-T}$.
- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#A coordinate is a claim about a frame|Frames and coordinate meaning]] · [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#A camera is an inverse pose|Camera pose and view transform]]

## Projection, depth, raster, and culling

- For RH view space looking down $-Z$, axial depth is $d=-z_v$ and perspective clip $w=d$. Projection and depth mapping are separate choices; declare NDC range, depth direction, viewport origin, and Y orientation.
- Recover positions using the rendered projection and matching pixel centre, viewport, and depth convention. A frustum plane test is conservative: sphere reject if $s<-r$; AABB reject if $s<-\sum_i|n_i|e_i$.
- Perspective-correct attributes interpolate as $\sum_i\lambda_i(a_i/w_i)/\sum_i\lambda_i/w_i$; depth has its own projected interpolation rule.
- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Projection turns rays into clip coordinates|Projection derivation]] · [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Depth is a numerical design choice|Depth precision]] · [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#A projected point still needs a raster convention|Raster mapping]] · [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#The viewing volume is six half-spaces|Frustum culling]] · [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Raster interpolation remembers homogeneous depth|Interpolation]]

## Time and calibrated image formation

- Track jittered and unjittered projection state. Intrinsics map optical-camera rays to pixels; extrinsics map between world and optical camera. Lens distortion is a separate nonlinear map with a valid domain and inversion method.
- Thin-lens rays originate across the aperture and converge on the focus plane. CoC and depth-of-field limits depend on an explicit sensor blur criterion. Aperture and shutter affect collected light; ISO-like gain does not add photons.
- Stereo depth follows $Z=f_xB/\Delta x$ for a rectified pinhole pair. Equal image areas on a sphere do not imply equal solid angles.
- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#A camera image changes over time|Temporal camera state]] · [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Intrinsics map optical rays to pixels|Intrinsics]] · [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Extrinsics place the optical camera in the world|Extrinsics]] · [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Real lenses bend the pinhole image|Lens distortion]]

## Reconstruction, stereo, and optics

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Recover geometry from depth and construct screen-space rays|Depth reconstruction and screen rays]] · [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Build projection and stereo geometry around real camera baselines|Stereo geometry]] · [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Represent directions over the full sphere|Panorama and fisheye]] · [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Model exposure, physical optics, and depth of field|Exposure and thin lens]] · [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Make camera conventions explicit and debug the full transform chain|Camera contract and debugging]]

## Essential figures

- [Figure placeholder: camera coordinate frames through raster and pixel centre]
- [Figure placeholder: standard and reversed-Z depth, with binary32 next-float reconstruction explicitly scoped]
- [Figure placeholder: conservative frustum plane and support-radius tests]
- [Figure placeholder: affine versus perspective-correct attribute interpolation]
- [Figure placeholder: stereo disparity and uncertainty, calibrated distortion, and sphere sampling measure]
- [Figure placeholder: thin-lens CoC and full object-to-lens-to-sensor ray path]

## Figures and their full explanations

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Figure C11-1 — Depth mapping and floating-point spacing work together|C11-1 · Depth mapping and floating-point spacing work together]] — RH camera looks down −Z, zero-to-one depth, n=0.1 and finite conventional far=100000; reversed curve uses infinite far. Right: error after rounding depth to binary32 and moving to its next representable value, including rounding and inversion effects. This is not a complete GPU transform-error model.

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Figure C11-2 — Culling is a conservative half-space test|C11-2 · Culling is a conservative half-space test]] — This s–x cross-section has near=1, far=5 and ±45° side planes. For an inward unit plane normal, a sphere is outside only when signed centre distance < −radius. For an AABB use r = |nx|ex + |ny|ey + |nz|ez. Passing each plane does not prove exact intersection with the whole frustum.

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Figure C11-3 — Attributes follow reciprocal clip w|C11-3 · Attributes follow reciprocal clip w]] — Vertex clip w values are (1,4,4); attribute values are (0,1,1). At the screen centroid the affine value is 2/3 and the corrected value is 1/3. Use sum(λ a/w) / sum(λ/w); depth-buffer interpolation has its own projected-depth rule.

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Figure C11-4 — Stereo turns pixel uncertainty into depth uncertainty|C11-4 · Stereo turns pixel uncertainty into depth uncertainty]] — Rectified pinhole pair: focal length 800 pixels, baseline 0.064 m, disparity noise σd=0.25 pixel. Z=fB/d and σZ≈Z²σd/(fB). This local propagation assumes corresponding points, small noise and valid calibration; it is not a perception model.

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Figure C11-5 — Focus distance, focal length and aperture are different controls|C11-5 · Focus distance, focal length and aperture are different controls]] — Paraxial 50 mm lens at f/2 focused at 2 m: 1/F=1/u+1/v and c=D|v0/v−1|, with D=F/N. The 0.03 mm threshold is an explicitly chosen viewing criterion. This excludes diffraction, aberrations and sensor processing.

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Figure C11-6 — Distortion is a map with a domain, not just a coefficient list|C11-6 · Distortion is a map with a domain, not just a coefficient list]] — Normalized Brown-style radial example xd=x(1+k₁r²), yd=y(1+k₁r²). This displayed domain has positive radial derivative 1+3k₁r², so the local radial map remains monotone. Larger radii can fold; numerical undistortion requires domain and convergence checks.

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Figure C11-7 — Image coordinates need a sampling-measure conversion|C11-7 · Image coordinates need a sampling-measure conversion]] — For longitude λ and latitude φ, dω=cosφ dλ dφ. Equal-width equirectangular image rows shrink toward the poles. Ray-differential Jacobians generalize the local image-area-to-solid-angle conversion; they do not by themselves specify lens-area or shutter-time probability.

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Figure C11-8 — Residual count does not guarantee observable motion|C11-8 · Residual count does not guarantee observable motion]] — Toy Lucas–Kanade information matrices H=sum(∇I∇Iᵀ): repeated horizontal gradients give eigenvalues (0,3); diverse gradients give (1,3). Small eigenvalues expose weak directions. Camera-calibration gauge freedoms require a larger Jacobian model and explicit constraints.

## Linked camera concepts

[[Concepts/Axial depth and Euclidean range|Axial depth and Euclidean range]] · [[Concepts/Hardware depth-buffer linearization|Hardware depth-buffer linearization]] · [[Concepts/Position reconstruction from depth|Position reconstruction from depth]] · [[Concepts/Screen-space ray generation and mouse picking|Screen-space ray generation and mouse picking]] · [[Concepts/Off-axis stereo projection and disparity|Off-axis stereo projection and disparity]] · [[Concepts/Omnidirectional camera projections|Omnidirectional camera projections]]

## Deeper research retrieval

- **Robust decisions:** compare a determinant against a certified arithmetic error bound; escalate uncertain signs. Exactness refers to the represented inputs. [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#A sign can matter more than a coordinate: robust predicates|Predicate derivation]]
- **Observable parameters:** fix frame/scale gauges before interpreting weak directions of the weighted Jacobian. More residuals alone do not guarantee new information. [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Calibration needs informative observations and a fixed gauge|Calibration and gauge]]
- **Finite pixels:** differentiate the normalized camera-ray map; convert image area to solid angle with the cross-product Jacobian. [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#A camera ray carries a pixel footprint|Ray footprints]]
- **Two and three views:** recover translation direction before metric scale; test cheirality and degeneracy; pairwise consistency alone is weaker than joint multiview consistency. [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Calibrated two-view geometry: direction before distance|Multi-view reconstruction]]

**Source scope:** selected TUM playlist 17 positions: 13 fully read caption tracks and four explicitly labeled official-PDF fallbacks. The full per-position legend is in [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#External course legend and evidence boundary|the continuous volume]].

## Research concept links

[[Concepts/Camera gauge freedoms and observability|Camera gauge freedoms and observability]] · [[Concepts/Ray differentials and camera sampling measures|Ray differentials and camera sampling measures]]
