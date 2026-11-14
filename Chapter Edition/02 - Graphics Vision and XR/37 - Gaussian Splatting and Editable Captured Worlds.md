# 37 — Gaussian Splatting and Editable Captured Worlds

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/38 - Projecting Ellipsoids into Screen-Space Conics|Next chapter]]

Gaussian splatting represents captured scenes with oriented kernels carrying shape and appearance. This span moves from one primitive’s geometry to an editable world and its rendering implications.

**By the end:** Explain how anisotropic Gaussian primitives represent and render captured scenes.

**Recall question:** Which parameters determine a Gaussian’s spatial support and appearance?

<!-- Derived from Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography.md; source lines 82103–84849; source-span SHA256 ea06feddb05551bc324e247b4c1b72b558839cb97b983f21718f71dc5456d7e7. -->

<!-- CHAPTER-SOURCE-BEGIN -->
# From an anisotropic kernel to an editable captured world

A 3D Gaussian renderer begins with a compact description of each primitive and ends with a collection of pixel contributions. An editable captured world adds another requirement: when a chair, curtain, or wall changes, the system must still know which records represent it, which physical variables can move them, and how to prove that unrelated image regions remained stable. The useful teaching chain is therefore

$$
\text{trained parameters}
\to \text{3D covariance}
\to \text{screen conic}
\to \text{tile references}
\to \text{ordered compositing}
\to \text{verified semantic edit}.
$$

The first five arrows belong mainly to graphics. The last arrow needs engine identity, storage, physics, and publication contracts. The boundary matters: a fast picture can be an excellent view synthesizer without being a reliable mesh, collider, or editable world. The existing graphics chapter already introduces explicit Gaussian splats and first-order screen covariance; the existing engine material already treats identity, lifetime, renderer boundaries, and frame resources. This bridge connects those ideas without replacing them .

```mermaid
flowchart LR
    A[Stored mean, scale, rotation, opacity, SH] --> B[3D covariance and appearance]
    B --> C[Camera projection and 2D conic]
    C --> D[Tile counts, scan, and references]
    D --> E[Stable tile-depth order]
    E --> F[Per-pixel transmittance and color]
    F --> G[Persistent Gaussian and entity IDs]
    G --> H[Physical support and deformation]
    H --> I[Published revised render state]
```

Read left to right: each stage lowers or enriches the representation for a different question. The conic answers where a primitive contributes; tile lists answer which candidates each pixel should inspect; persistent IDs answer which world object an edit names.

## A primitive with shape and appearance

A triangle stores three corners plus connectivity to adjacent triangles. Its coverage is bounded by straight projected edges, and a collection of triangles explicitly describes a piecewise planar surface. A Gaussian splat starts from a mean $\mu\in\mathbb R^3$ and a covariance matrix $\Sigma\in\mathbb R^{3\times3}$. The mean locates the primitive. The covariance describes how its support stretches and tilts in space. Opacity and a directional color model describe how much light it contributes and how that appearance depends on the viewing direction. The 2023 3D Gaussian Splatting paper represents scenes using anisotropic Gaussians initialized from sparse reconstruction points, optimizes their position, covariance, opacity, and spherical-harmonic appearance, and renders visible elements through a tile-based splatting pipeline. Those are properties of that method and its benchmarks, not universal speed or quality guarantees ([Kerbl et al., paper](https://arxiv.org/abs/2308.04079)).

For displacement $x-\mu$, the unnormalized spatial weight is

$$
w(x)=\exp\left[-\frac12(x-\mu)^T\Sigma^{-1}(x-\mu)\right].
$$

The quadratic $q=(x-\mu)^T\Sigma^{-1}(x-\mu)$ is the squared Mahalanobis distance: it measures distance after scaling and rotating coordinates by the Gaussian shape. A Gaussian’s constant-weight sets are ellipsoids. This formula is not, by itself, a normalized probability density; the normalizing factor involving $|\Sigma|^{-1/2}$ is omitted in common rendering kernels because the primitive’s learned opacity supplies amplitude. That distinction prevents a common category error: rendered opacity is not a probability mass or a physical density.

Covariance is symmetric. If $u$ is a unit direction, its variance along that direction is $u^T\Sigma u$. A convenient representation constructs covariance from a rotation $R$ and positive axis scales $s=(s_x,s_y,s_z)$:

$$
\Sigma=R\operatorname{diag}(s_x^2,s_y^2,s_z^2)R^T.
$$

The squares appear because scale is a standard-deviation-like length while covariance stores variance. With identity rotation and $s=(2,1,0.25)$, the covariance is $\operatorname{diag}(4,1,0.0625)$. The one-standard-deviation ellipsoid has semiaxes $(2,1,0.25)$; a three-sigma support approximation has semiaxes $(6,3,0.75)$. This is a concrete way to distinguish scale from covariance before involving a camera.

The factorization also explains positivity. Let $M=R\operatorname{diag}(s)$. Then $\Sigma=MM^T$. For any vector $x$,

$$
x^T\Sigma x=x^TMM^Tx=\|M^Tx\|^2\ge 0.
$$

Thus $\Sigma$ is positive semidefinite. To invert it, it must be positive definite: no axis may collapse and $M$ must be nonsingular. A quaternion used to construct $R$ must also be normalized; otherwise its matrix formula may not describe an orthonormal rotation. In an implementation, finite inputs and sensible numeric ranges still need checks because exponentiating an extreme log scale can overflow or underflow. A mathematical positivity argument does not protect a parser from hostile values.

Three-dimensional Gaussian Splatting also stores view-dependent color. Spherical harmonics (SH) are a finite basis of smooth functions over viewing directions. A color channel is approximated as $C(d)=\sum_k c_kY_k(d)$, with coefficients $c_k$ and basis functions $Y_k$. Degree $L$ contains $(L+1)^2$ terms per channel: degree three has sixteen terms per color, and if the degree-zero coefficient is stored separately, fifteen remaining terms per channel, or forty-five higher-order values total. The direction convention is part of the data contract. Reversing the view direction changes the sign of odd-degree terms, so a model can have plausible geometry and still show wrong view-dependent color. Test degree zero first—it should not vary with camera direction—then activate one degree-one coefficient at a time.

A PLY file is a container of named scalar properties, not a universal 3DGS schema. A loader must define property names, accepted scalar types, byte order, record size, optional coefficient counts, and malformed-input behavior. Parsing by declared property name is safer than assuming that position occupies the first three columns. The distinction between stored and decoded values matters: log-scales and opacity logits are optimizer-friendly encodings; decoding them belongs at a clearly defined boundary. Likewise, a source array index is stable only while the array’s order stays fixed. It is not a persistent primitive identity when compaction or filtering reorders records.

## Local projection turns an ellipsoid into a screen ellipse

Perspective projection maps camera-space coordinates $(X,Y,Z)$ to pixel coordinates

$$
u=f_x\frac XZ+c_x,\qquad v=f_y\frac YZ+c_y,
$$

where $f_x,f_y$ are focal lengths in pixel units and $(c_x,c_y)$ is the principal point. Assume the camera looks along positive $Z$ and the point is in front of the near plane. The Jacobian is the matrix of first derivatives of $(u,v)$ with respect to $(X,Y,Z)$:

$$
J=\begin{bmatrix}
f_x/Z&0&-f_xX/Z^2\\
0&f_y/Z&-f_yY/Z^2
\end{bmatrix}.
$$

Why does this matrix appear? For a small 3D displacement $\delta$, Taylor expansion around the Gaussian mean gives $\pi(\mu+\delta)\approx\pi(\mu)+J\delta$. This is a first-order local approximation; it is not an exact projection of an entire 3D Gaussian through a nonlinear perspective map. If a random displacement has covariance $\Sigma$, and a linear map $A$ transforms it, then the transformed covariance is $A\Sigma A^T$. Apply this first to the rigid world-to-camera rotation $R_c$, then to the local projection Jacobian:

$$
\Sigma_c=R_c\Sigma_wR_c^T,
\qquad
\Sigma_{screen}=J\Sigma_cJ^T.
$$

The first relation is exact for an affine coordinate transform; the second inherits the local linearization. Translation changes the mean but not covariance. Near $Z=0$, the $1/Z$ and $1/Z^2$ entries grow without bound, which explains why near-plane handling is both geometric policy and numerical protection.

Here is a numeric witness with units. Let the camera-space mean be $(1,0.5,5)$ meters, $f_x=f_y=1000$ pixels, and principal point $(960,540)$ pixels. The screen center is $(1160,640)$ pixels. The Jacobian entries are in pixels per meter:

$$
J=\begin{bmatrix}200&0&-40\\0&200&-20\end{bmatrix}.
$$

If camera-space covariance is $0.01I\,\mathrm{m}^2$, then the unregularized screen covariance is

$$
J(0.01I)J^T=\begin{bmatrix}416&8\\8&404\end{bmatrix}\mathrm{pixel}^2.
$$

The off-diagonal $8$ records a tilt in screen-space support; it is not a color-channel correlation. A 2D covariance $S=\begin{bmatrix}a&b\\b&c\end{bmatrix}$ has determinant $D=ac-b^2$. If $S$ is positive definite, its inverse is

$$
S^{-1}=\frac1D\begin{bmatrix}c&-b\\-b&a\end{bmatrix}.
$$

This inverse is the conic used by the pixel kernel. For offset $d=(d_x,d_y)$ from projected center, the squared distance is

$$
q=d^TS^{-1}d=A d_x^2+2B d_xd_y+C d_y^2,
$$

and the pixel weight is $e^{-q/2}$. The factor of two in the mixed term follows from the symmetric off-diagonal entries. Forgetting it changes the ellipse orientation and extent.

To obtain a conservative square bound, compute the largest eigenvalue

$$
\lambda_{max}=\frac{a+c+\sqrt{(a-c)^2+4b^2}}2.
$$

A three-sigma radius is $r=3\sqrt{\lambda_{max}}$. In the numeric witness, adding a screen-space floor of $0.3I$ gives $a=416.3$, $b=8$, $c=404.3$ pixel-squared. The discriminant is $20$, so $\lambda_{max}=420.3\,\mathrm{pixel}^2$ and $r\approx61.5$ pixels. Thus an axis-aligned conservative square extends about 61.5 pixels from center $(1160,640)$. A tighter axis-aligned bound uses $3\sqrt a$ horizontally and $3\sqrt c$ vertically; a still tighter ellipse-versus-tile test costs more arithmetic.

The reference Graphdeco CUDA code uses the analytic perspective Jacobian, adds $0.3$ to two screen-covariance diagonals, and describes it as a low-pass filter that makes a Gaussian at least about a pixel wide/high. Numerically, $\sqrt{0.3}\approx0.548$ pixels is the added standard-deviation scale. This constant is a practical dilation regularizer, not a complete proof of antialiasing. [Mip-Splatting](https://openaccess.thecvf.com/content/CVPR2024/html/Yu_Mip-Splatting_Alias-free_3D_Gaussian_Splatting_CVPR_2024_paper.html) analyzes sampling-rate artifacts in original 3DGS and attributes them to missing 3D frequency constraints and the fixed 2D dilation filter; it proposes a 3D smoothing filter and a 2D Mip filter that models pixel-area box filtering. The broader lesson from the earlier EWA surface-splatting work is that a projected footprint is a filtering problem as well as a shape problem: [Zwicker et al., *Surface Splatting*](https://www.cs.princeton.edu/courses/archive/spr01/cs598b/papers/zwicker01.pdf). A covariance ellipse alone does not guarantee correct integration of high frequencies over a pixel.

## From ellipses to bounded tile work

A pixel-by-Gaussian loop costs $P\times N$ candidate tests for $P$ pixels and $N$ primitives. At 1920×1080 and five million splats, that is over ten trillion tests. Local support gives a better strategy: divide the image into tiles, and ask each projected Gaussian which tiles its conservative bound overlaps. A tile is a fixed pixel rectangle. For tile side $T$, the grid has $N_x=\lceil W/T\rceil$ columns and $N_y=\lceil H/T\rceil$ rows. With $T=16$ at 1920×1080, this is 120×68, or 8,160 tiles.

Suppose one projected bound spans tile coordinates $[x_0,x_1]\times[y_0,y_1]$, inclusive. It contributes $K=(x_1-x_0+1)(y_1-y_0+1)$ Gaussian-to-tile references. A reference is one incidence saying “this Gaussian may affect this tile.” If three Gaussians touch 4, 2, and 5 tiles, their counts are $[4,2,5]$, for a total of $11$ references. Every later offset depends on this count sum.

An exclusive prefix sum allocates disjoint output slices. For counts $K=[4,2,5,1]$, offsets are $O=[0,4,6,11]$. Gaussian $i$ writes to the half-open interval $[O_i,O_i+K_i)$, so the intervals are adjacent and non-overlapping. “Exclusive” means the prefix for an item omits its own count. A GPU implementation commonly does count → scan → scatter: count references, scan counts to reserve space, then scatter each reference into its assigned range. A multi-block scan first computes block-local prefixes, scans block totals, then adds each block’s prefix. This parallel pattern is reusable far beyond splatting.

After expansion, a flat array of Gaussian indices plus a begin/end range per tile is a compressed-sparse-row-like representation. If the flat references are grouped by tile, tile 0 might map to entries $[0,3)$, tile 1 to $[3,4)$, and tile 2 to $[4,6)$. The tile loop now traverses a contiguous interval rather than chasing many separately allocated lists. Total reference count $M=\sum_iK_i$, maximum tile occupancy, and tail depth complexity often predict cost better than Gaussian count alone. A huge Gaussian may touch tens of thousands of tiles, dominating memory even when it is only one primitive.

Within each tile, references need a deterministic order. Alpha compositing is not commutative. For foreground color $c_0$ with opacity $\alpha_0$, followed by $c_1,\alpha_1$, the visible result is $\alpha_0c_0+(1-\alpha_0)\alpha_1c_1$ plus residual background. Swapping the two changes the weights. For red at $\alpha=0.5$ over blue at $\alpha=0.5$, the foreground-first result is $(0.5,0,0.25)$; blue-first gives $(0.25,0,0.5)$. A deterministic tile/depth sort is therefore part of image semantics, even though center-depth sorting remains an approximation for volumetric, intersecting splats.

For positive finite depth values, IEEE-754 float bit patterns preserve numeric order when reinterpreted as unsigned integers. This convenient key trick is only safe after rejecting negative, non-finite, and NaN depths and declaring which sign is nearer. A composite key must group by tile and order by depth within tile. In a stable least-significant-digit radix sort, sort the secondary field first and the primary field last. Stability preserves earlier depth order when the later tile field is processed. A tie policy is needed for equal depths so repeated runs do not reorder identical keys unpredictably.

For each pixel center $p=(x+0.5,y+0.5)$, evaluate only the tile’s range. If $q=(p-\mu)^TS^{-1}(p-\mu)$ exceeds $9$, it lies outside the three-sigma ellipse and can be skipped before evaluating the exponential. Otherwise local opacity is approximately $\alpha_i=\min(0.99,\alpha_{base}e^{-q/2})$, subject to a declared small-alpha threshold. Let $T_0=1$ be remaining transmittance and $C_0=0$ accumulated color. Then

$$
w_i=T_i\alpha_i,\qquad C_{i+1}=C_i+w_ic_i,\qquad T_{i+1}=T_i(1-\alpha_i).
$$

By induction, $T_n=\prod_{j<n}(1-\alpha_j)$ and the background contributes $T_nC_{bg}$. If accumulated opacity is $A=1-T$, the same update is contribution $(1-A)\alpha$ and $A\gets A+(1-A)\alpha$. This equivalence is a useful implementation cross-check. If $T<10^{-3}$, then for colors bounded by one, the unprocessed tail can alter a channel by at most roughly $10^{-3}$ under the chosen model. That gives a concrete error budget for early termination. The exact cutoff is a policy, not an inherent property of 3DGS. In the inspected Graphdeco CUDA reference, the termination condition uses a remaining-transmittance threshold of $10^{-4}$; the supplied teaching text’s $A>0.999$ corresponds instead to $T<10^{-3}$. Keep those distinct.



## Figure ER1 - Footprint, compositing order and storage lifetime

![[Visual Recall/Explicit Rendering/footprint-order-lifetime.png]]

**Read the panels independently.** A uses the projected covariance from the worked example, with ellipses at one, two and three standard deviations. Pixel coordinates increase downward. B holds both layer opacities at one half and uses a black background: swapping red and blue swaps their contribution weights. C shows potentially non-overlapping storage lifetimes in an ordered schedule; asynchronous execution additionally needs a dependency proving completion before reuse. The plotted values are calculated CPU witnesses, not measured GPU results. The three panels distinguish spatial support, visibility order and temporal ownership.


## Validation should isolate representations before images

A CPU oracle is most useful when it shares mathematical semantics with the GPU implementation but not every implementation detail. If both paths use identical scan, key packing, radix passes, and range construction, a shared indexing bug can pass comparison. A clear CPU path can project valid splats, stable-sort by depth, partition independent rows or tiles, and preserve a fixed contributor order per pixel. Work partitions must not alter a pixel’s accumulation order. The CPU output is a reference for declared inputs and tolerances, not proof that the learned scene is physically correct.

Compare in linear light when testing the renderer’s radiometric arithmetic. For $N$ scalar channel samples, $RMSE=\sqrt{\frac1N\sum_i(G_i-R_i)^2}$. It describes distributed average error but can hide one severe pixel. Maximum error $E_{max}=\max_i|G_i-R_i|$ catches the outlier but says nothing about its area. A changed-pixel fraction requires a threshold $\epsilon$: it is the fraction of pixels where any channel differs by more than $\epsilon$. Together, these answer different questions: how large is typical error, how bad is the worst case, and how localized are changes?

PSNR is $10\log_{10}(MAX^2/MSE)$. It is meaningful only when $MAX$ is defined. For normalized LDR color, one might set $MAX=1$. For HDR color that may exceed one, this choice is wrong unless the test first defines a fixed comparison range or tone-maps by design. Object IDs are discrete evidence and generally deserve exact agreement outside raster-edge ambiguity; depth has units and its own absolute/relative tolerance. A renderer can match beauty color while writing a wrong entity ID, so semantic buffers should be compared independently. The peak-error location should retain tile, Gaussian, sort-rank, and pixel metadata so a scalar metric can guide diagnosis.

A stable numerical pipeline still does not establish antialiasing quality. A test can compare CPU and GPU perfectly while both alias at a different zoom level. Sampling tests must vary image resolution, camera distance, and focal length; verify a stable constant signal; and include high-frequency checkerboards or thin features. Reference the prior graphics material on pixel footprints and filtering, then use the distinction emphasized by Mip-Splatting: 3D bandwidth constraints and per-pixel 2D filtering address different artifacts.

## 3. Choose lighting work from the scene's workload

Rendering many lights is an assignment problem: for each visible surface sample, determine which lights might contribute, then evaluate a shading function. A forward renderer evaluates lights while drawing geometry and is simple when there are few lights or many material-specific features. A deferred renderer writes material and geometric attributes to screen-space buffers, then shades pixels using those buffers; it can decouple geometry cost from light count for opaque surfaces, but consumes bandwidth and complicates transparency and material diversity. Tiled lighting groups pixels into screen tiles. Clustered lighting adds depth slices, assigning each surface region a smaller conservative light set. Each technique moves work to a different place and has different memory and approximation costs.

A cluster is a screen tile plus a depth interval. Logarithmic depth slices allocate more resolution near the camera and progressively wider ranges far away. If near plane $n=0.1$, far plane $f=100$, and there are 16 slices, the geometric ratio is

$$
r=(f/n)^{1/16}=1000^{1/16}\approx1.54.
$$

Slice boundaries are approximately $z_k=n r^k$: 0.10, 0.154, 0.237, 0.365, 0.562, …, 100. Each interval is about 54% wider than the previous. This spends more clusters on nearby geometry where equal absolute depth errors occupy more screen space. Exact formulas depend on the chosen indexing and projection convention; the important contract is that depth-to-slice and slice-to-bound calculations agree and clamp correctly at near/far limits.

To build a light list, conservatively bound each light in screen and depth space, find overlapping clusters, then append its index to those clusters. A flattened list with one offset/count pair per cluster avoids allocating a separate dynamic array for every cell. For example, if cluster 7 has offset 22 and count 3, it reads entries 22, 23, and 24. A prefix sum can turn per-cluster counts into offsets. The GPU representation is compact and easy to consume, but it creates an overflow policy: if a cluster exceeds its assigned capacity, either allocate more, use a fallback list, truncate with a visible diagnostic, or fail loudly. Silent truncation creates light leaks that resemble shading bugs.

Conservative bounds are a deliberate approximation. A point light with range R can use a sphere; a spot light can use a cone or a conservative enclosing sphere. The loose sphere costs extra shading because false positives include lights that do not actually affect the cluster. False negatives omit contributing lights and are correctness defects. This gives a staged development path: begin with conservative bounds, visualize cluster occupancy and overflow, compare against a brute-force all-lights reference on small scenes, then add tighter cone-cluster tests only if profiling shows the extra culling is worth its compute and divergence cost.

The broader real-time renderer uses approximations at different scales. Shadow maps replace geometric visibility with depth comparisons from a light's view; cascades allocate separate maps by camera depth range, and filtering softens the sample pattern without making a hard shadow physically soft. Screen-space ambient occlusion estimates nearby occlusion from depth and normals, so it cannot see off-screen geometry. Screen-space reflections trace through the visible depth buffer, so objects outside that buffer cannot reflect. Irradiance probes represent diffuse incoming light in a sampled volume; they interpolate between nearby samples and need visibility or bias strategies to limit leakage. Contact shadows, reflection probes, and ray tracing occupy other points in the quality/performance space. These methods should be labeled by the signal they approximate and what data they cannot observe.

Adaptive algorithms choose where to spend work based on an error or importance estimate. A controller that changes quality every frame may oscillate. **Hysteresis** introduces separate thresholds or a temporal hold: quality increases only after evidence remains strong enough, and decreases at a different threshold. For example, allocate more shadow samples when projected penumbra error exceeds 1.5 pixels, but retain the higher setting until it falls below 1.0 pixel for several frames. Those values are illustrative; a production threshold must be measured for the target scene and hardware. Debug views should show both predicted work and actual cost. An adaptive budget that cannot explain why it spent a millisecond is difficult to tune.

Temporal antialiasing reuses prior-frame color. The current pixel is mapped into the previous frame using motion vectors, which encode the displacement between corresponding surface positions. Reprojection is only a guess when the surface is occluded, animated unexpectedly, disoccluded, or shaded with changed lighting. The temporal resolver therefore validates history using depth, normal, object identity, neighborhood bounds, or other signals; it reduces history weight or rejects history when correspondence is suspect. Jittering the projection moves the sample pattern across frames to cover subpixel positions. The accumulated estimate can reduce aliasing, but incorrect motion vectors or permissive history cause ghost trails. A camera cut, viewport resize, render-scale change, or incompatible scene edit must reset or deliberately transform the history.

Temporal and spatial methods need paired validation. For clustered lighting, render a tiny fixed scene with a brute-force reference and assert that cluster culling never omits a light whose influence is nonzero. For TAA, inspect static edges, high-motion thin geometry, disocclusion boundaries, and changing emissive lights; compare against a high-sample reference but also inspect frame-to-frame trails. For shadows, compare cascade transitions and test depth conventions. A screenshot of one attractive frame does not test temporal stability or worst-case list overflow.

## 4. Mesh data has both surface meaning and connectivity

A triangle mesh stores geometry plus connectivity. Positions answer where points lie. Triangle indices answer which three vertices form a face. Normals describe local orientation for shading; texture coordinates parameterize image locations over the surface; tangents define the local texture-space direction used by normal mapping. Topology describes the neighborhood relationships independent of exact coordinates. Two coincident points may be separate topological vertices, and one geometric vertex may need several render vertices where UVs, hard normals, or material boundaries split attributes.

This is why a file importer cannot deduplicate by position alone. An OBJ face corner identifies a tuple of position, texture-coordinate, and normal indices; a polygon corner's tuple becomes the render-vertex key. Negative OBJ indices are relative to the end of the corresponding list at the point they are read, not a global ID. Polygons must be triangulated using a method suited to their shape; a fan is not guaranteed to preserve a concave polygon. Tangent generation needs consistent UV orientation and should handle degenerate UV triangles explicitly. These are Graphics data rules; Engine owns the transaction that turns validated imported data into a runtime asset.

A **buffer** is a byte store. A glTF `bufferView` selects a byte range and possibly a stride inside that buffer. An **accessor** defines how to decode a typed sequence from the view: component type, element shape, count, and offsets. For a float `VEC3`, an element has 12 bytes when tightly packed. A stride can make vertex attributes interleaved; a parser must validate alignment, count, offset, stride, and range before reading. These records are not interchangeable concepts: a buffer is storage, a view is a slice/layout, and an accessor is a typed interpretation of that slice.

Consider 3 vertices with float positions and 16-bit indices. The importer resolves the accessor's bufferView, verifies a 12-byte element and 36-byte range, converts each position into a canonical mesh representation, then resolves the scalar index accessor and checks each index is less than 3. A malformed accessor must fail before allocation/publication. Normalized integer attributes require the schema's exact normalization mapping; for unsigned 8-bit values, a byte $c$ maps to $c/255$. Don't reinterpret the byte as an integer color unless that is the intended shader contract. These details are specified by the Khronos glTF 2.0.1 specification's accessor sections.

Mesh processing then operates on a chosen representation. Edge collapse simplifies a mesh by replacing two vertices with one; quadric error metrics estimate how much the replacement deviates from nearby face planes. Topology constraints prevent invalid collapses such as breaking a boundary or creating a non-manifold neighborhood. Subdivision adds vertices and faces according to a rule, often smoothing the surface. Laplacian smoothing moves a vertex toward a weighted average of neighbors; cotangent weights derive from triangle angles and better reflect local geometry, but irregular or degenerate triangles can produce numerical problems. Parameterization maps surface points to a 2D domain; seams duplicate coordinates where a continuous surface cannot be represented as one distortion-free planar chart. The details belong in Graphics, while the engine should keep derived meshes tied to source asset identity and processing settings.

Compression and optimization change different costs. Quantization reduces attribute precision and storage; vertex/index ordering can improve cache locality or reduce overdraw; mesh compression encodes connectivity and attributes compactly. A reported smaller file does not guarantee faster runtime rendering: decode cost, GPU vertex fetch, index cache, overdraw, and upload bandwidth all matter. The meshoptimizer documentation, for example, describes separate cache, overdraw, fetch, and compression operations and their ordering constraints; treat each as a measurable stage, not one undifferentiated “optimize mesh” switch.

## 5. Build a pose first, then deform the surface

A skeleton is a hierarchy of joints. Each node has a local transform; multiplying parent world transform by child local transform gives the child world transform. A pose is the set of those local transforms at a given time. An animation channel selects a target property, such as a node's translation or rotation, and a sampler supplies key times and values. Linear interpolation is appropriate for translation and scale between compatible values. Quaternion rotation interpolation needs normalized quaternions and a shortest-path choice because $q$ and $-q$ encode the same orientation. Cubic spline data uses incoming tangent, value, and outgoing tangent per key; Hermite evaluation multiplies tangents by the interval duration. A step sampler holds a value until the next key.

A skin associates a mesh with joints and inverse bind matrices. The **bind pose** is the pose used when the mesh and skeleton were authored together. An inverse bind matrix maps a bind-space vertex into a joint's local frame. The current joint transform maps it from that frame into the current pose. For joint j, in a conventional column-vector notation:

$$
J_j(t)=G_j(t)\,IBM_j,
$$

where $G_j(t)$ is the joint's current global transform and $IBM_j$ is its inverse bind matrix. For the identity-bind-shape fixture used below, choose $IBM_j=G_j(0)^{-1}$ so $G_j(0)IBM_j=I$. More general bind/output-space conventions must state the expected bind mapping rather than assume every palette matrix is the identity. glTF specifies the ordered relationship between `skin.joints` and inverse-bind matrices. The node carrying the skinned mesh has special normative handling: its transform is not simply multiplied into the joint palette as if the mesh were an ordinary attached node. Engine implementations must follow the specification's coordinate-space rule consistently; do not casually add or remove the mesh-node transform because a sample formula from another engine uses a different output-space convention.

For a numerical witness, take a bone whose bind global transform is translation by one unit along x: $B=T(1,0)$. Let a rest vertex be $p=(2,0,1)^T$. Its bind-local point is $B^{-1}p=(1,0,1)^T$. If the current joint rotates 90 degrees about its own origin and remains translated by one, then

$$
G(t)B^{-1}p=T(1,0)R_{90}(1,0,1)^T=(1,1,1)^T.
$$

The point rotates around the bone at $x=1$, not around the character origin. At the bind pose, substitute $G=B$: $BB^{-1}p=p$. This identity catches inverse/order mistakes before they become hard-to-see deformations.

Linear blend skinning (LBS) gives a vertex several joint influences. With weights $w_j\ge0$ that sum to one,

$$
p'(t)=\sum_jw_jJ_j(t)p.
$$

Suppose two joints send the same point to $(0,1)$ and $(0,-1)$; equal weights yield $(0,0)$. Each rigid transform is valid, but their matrix average collapses the point. This is the candy-wrapper or volume-loss artifact. Dual-quaternion blending can preserve rigid motion better, but it introduces its own normalization/sign-handling details, and it does not replace artist-authored corrective shapes for all deformations. The glTF core skin model specifies linear blend skinning; alternatives are runtime or extension choices and should be documented as such.

Animation evaluation has explicit stages: sample channels at time t; combine clips according to override/additive policy; update local transforms; resolve global transforms in hierarchy order; build the skin palette; apply morph targets if present; deform positions and normals; calculate current and previous clip-space positions for motion vectors; then rasterize. A morph target stores deltas from a base mesh. With several active targets, position is base plus weighted deltas. For glTF, morph targets are applied before skinning and node transforms. That order is a format contract, not an arbitrary importer choice. Test a simple target that changes one vertex by a known vector before a known joint rotation. Animated bounds must include the deformed or conservatively expanded surface, or culling can remove a character that is visibly in frame.

The animation trace continues to the pixel. For an indexed triangle, the vertex shader fetches its position, joints, weights, and normal. It evaluates the current skin matrix and then applies view-projection in the same declared output space. With the global-joint palette above the result is already in world space: do not multiply a mesh-node world transform a second time. A renderer using a mesh-local palette must include the corresponding inverse mesh transform in that palette before its final world transform. The rasterizer produces covered fragments and interpolated values. The fragment shader samples material textures and clustered lights. The temporal path also evaluates the previous pose/camera transform to produce a motion vector. That vector reprojects the previous color history; depth and neighborhood checks decide whether to retain it. A bug in any earlier transform can therefore appear as bad shading, bad culling, or a TAA ghost. Keep intermediate witnesses: bind-pose identity, known single-joint rotation, known morph delta, current/previous clip positions, and a no-temporal reference render.

## Primary references used for corrections and terminology

- [Khronos glTF 2.0.1 specification](https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html): accessor component types, offsets, stride, alignment and bounds; node TRS composition and animation target constraints; skin joint ordering and inverse-bind accessor; supported animation interpolation and its tangent layout; normalized integer decode rules. Checked as the normative format source.
- [Khronos glTF Tutorials: Skins](https://github.com/KhronosGroup/glTF-Tutorials/blob/main/gltfTutorial/gltfTutorial_020_Skins.md): explanatory derivation of joint-global transform multiplied by inverse bind matrix and the coordinate-space purpose of the inverse bind.
- [meshoptimizer documentation](https://meshoptimizer.org/): separate stages for vertex cache, overdraw, vertex fetch and compression. Used to avoid conflating file-size compression with runtime ordering optimizations.
- [A Survey of Temporal Antialiasing Techniques](https://diglib.eg.org/items/cfa6a05c-53b3-4ee1-979d-48abc6225f6b): frames TAA as sample accumulation plus history validation and catalogs implementation choices. The bridge's validation recommendations are teaching synthesis, not a claim that one validation scheme is mandatory.

The identity and physical-publication side continues in [[Chapter Edition/03 - Game Engine/42 - From cartridge bytes to a reproducible machine#Rendering identity becomes world identity only through an explicit bridge|editable world identity]]. Native ownership and GPU completion are explained in [[Chapter Edition/01 - Computing Foundations/11 - C++ Const, Borrowing, and CPU–GPU Buffering#Retire a resource after every user, not after one convenient clock|multi-queue retirement]].


## Represent a captured scene with anisotropic kernels

### + M147

One boundary correction before we begin: the authoritative curriculum puts **screen tiling, prefix-sum reference expansion, depth keys, radix sorting and front-to-back compositing in M148 starting at Part 2453**, so those belong to **Pass 16**. We will use Maveb's later GPU pipeline only when it helps explain what today's projected representation is preparing for, but Pass 15 itself stops exactly at **degeneracy and opacity culling, Part 2452**. The exact curriculum sequence is point-vs-triangle rendering → Gaussian/covariance representation → SH/PLY → camera projection/Jacobian/conic/eigen bounds/culling.

The lineage matters. Modern 3D Gaussian Splatting uses explicit anisotropic 3D Gaussian primitives and a visibility-aware rasterization pipeline rather than evaluating a neural field continuously through empty space. The original 2023 3DGS work optimizes Gaussian position, anisotropic covariance, opacity and view-dependent appearance, while its projection machinery inherits the much older mathematical idea of locally projecting Gaussian kernels into screen-space ellipses from EWA/surface splatting. [arXiv](https://arxiv.org/abs/2308.04079?utm_source=chatgpt.com)

For Maveb specifically, I verified the current canonical representation, PLY loader, deterministic CPU oracle, Metal Gaussian projection kernel and Gaussian pipeline. The CPU oracle and Metal kernel implement essentially the same chain:

$$
\boxed{
\text{stored Gaussian}
\rightarrow
\Sigma_{world}
\rightarrow
\Sigma_{camera}
\rightarrow
J\Sigma J^T
\rightarrow
\text{conic}
\rightarrow
\text{screen bound}
}
$$

That chain is the heart of this pass.

---

### THE 3D GAUSSIAN AS A GEOMETRIC PRIMITIVE


A triangle renderer represents a surface through connectivity.

A triangle gives three vertices:

$$
v_0,v_1,v_2
$$

and the rasterizer determines which pixels are covered by the projected triangle.

The triangle is fundamentally a **piecewise planar surface primitive**.

A Gaussian renderer instead starts from primitives of the form:

$$
G_i=
\{
\mu_i,
\Sigma_i,
\alpha_i,
c_i(\mathbf d)
\}.
$$

Here:

$$
\mu_i\in\mathbb R^3
$$

is the center,

$$
\Sigma_i\in\mathbb R^{3\times3}
$$

describes anisotropic spatial extent,

$$
\alpha_i
$$

controls opacity,

and

$$
c_i(\mathbf d)
$$

represents view-dependent color.

There is no triangle connectivity requirement.

The renderer asks:

> What elliptical footprint does this volumetric/spatial basis function create on the image?

rather than:

> Which side of three projected edges contains this pixel?

That is an enormous conceptual change.

---

**A Gaussian is not just a “point”**

Calling 3DGS a point renderer can be misleading.

A classical point:

$$
p=(x,y,z)
$$

has zero spatial extent.

A 3D Gaussian has an entire support function around:

$$
\mu.
$$

Its covariance can make it:

```text
sphere-like
needle-like
disc-like
elongated
rotated
```

although mathematically the Gaussian has infinite support.

So a better statement is:

$$
\boxed{
\text{3DGS uses oriented anisotropic volumetric basis functions centered at points.}
}
$$

The point stores the center.

The covariance stores the geometry-like footprint.

---

**Triangles encode surfaces explicitly; Gaussians encode appearance support**

For a triangle mesh, geometry exists as:

$$
vertices + connectivity.
$$

For 3DGS, a surface often emerges from many overlapping anisotropic kernels.

You can have:

$$
G_1+G_2+\cdots+G_N
$$

forming the appearance of a wall even though no triangle says:

> these three samples define a wall.

This is why Gaussian scenes are powerful representations of captured appearance but are not automatically equivalent to watertight physical geometry.

That distinction will become central in M150.

---

**Gaussian splatting avoids a lot of empty-space work**

A neural volumetric field may require evaluating points repeatedly along rays.

3DGS explicitly stores spatial primitives and projects only relevant splats.

This was a major motivation in the original work: explicit Gaussians preserve desirable continuous radiance-field properties while avoiding unnecessary evaluation in empty space, and a visibility-aware splatting renderer enables real-time rendering. [arXiv](https://arxiv.org/abs/2308.04079?utm_source=chatgpt.com)

That does not mean Gaussians are free.

A large scene may contain millions of splats.

The performance problem becomes:

$$
\boxed{
\text{project}
+
\text{cull}
+
\text{schedule}
+
\text{sort}
+
\text{composite}
}
$$

rather than triangle assembly/rasterization.

Pass 16 handles the scheduling/sorting half.

---


Start with:

$$
g(x)
=
\exp
\left(
-\frac{(x-\mu)^2}
{2\sigma^2}
\right).
$$

This is the shape-only Gaussian.

If we want a normalized probability density:

$$
\boxed{
p(x)
=
\frac{1}
{\sqrt{2\pi}\sigma}
\exp
\left(
-\frac{(x-\mu)^2}
{2\sigma^2}
\right)
}
$$

where:

$$
\mu=\text{mean},
$$

$$
\sigma=\text{standard deviation},
$$

$$
\sigma^2=\text{variance}.
$$

---

**What variance means geometrically**

Take:

$$
x=\mu+\sigma.
$$

Then:

$$
\frac{(x-\mu)^2}{2\sigma^2}
=
\frac12.
$$

Thus:

$$
g(\mu+\sigma)
=
e^{-1/2}
\approx0.6065.
$$

At:

$$
2\sigma:
$$

$$
g=e^{-2}
\approx0.1353.
$$

At:

$$
3\sigma:
$$

$$
g=e^{-4.5}
\approx0.0111.
$$

This explains why Gaussian rasterizers often truncate around:

$$
3\sigma.
$$

The Gaussian mathematically continues forever, but beyond three standard deviations the kernel becomes small enough to ignore for practical rendering.

---

**Rendering Gaussians are usually not normalized probability densities**

This matters.

A probability Gaussian contains:

$$
\frac{1}{\sqrt{2\pi}\sigma}.
$$

A splatting kernel commonly evaluates only:

$$
\boxed{
w(x)=
e^{-\frac12 q(x)}
}
$$

and then combines it with learned opacity:

$$
\alpha(x)
=
\alpha_0w(x).
$$

Why omit the normalization?

Because we're not asking:

> What probability density integrates to one?

We're asking:

> What spatial weight should this splat contribute at this pixel?

The learned opacity/amplitude carries intensity significance.

So covariance still determines **shape**, but the primitive is not being interpreted as a unit-probability distribution during rasterization.

---


For:

$$
\mathbf x\in\mathbb R^d
$$

the normalized Gaussian is:

$$
\boxed{
p(\mathbf x)
=
\frac{
1
}{
(2\pi)^{d/2}
|\Sigma|^{1/2}
}
\exp
\left[
-\frac12
(\mathbf x-\boldsymbol\mu)^T
\Sigma^{-1}
(\mathbf x-\boldsymbol\mu)
\right]
}
$$

where:

$$
\boldsymbol\mu\in\mathbb R^d
$$

and:

$$
\Sigma\in\mathbb R^{d\times d}.
$$

The central expression is:

$$
\boxed{
q(\mathbf x)
=
(\mathbf x-\boldsymbol\mu)^T
\Sigma^{-1}
(\mathbf x-\boldsymbol\mu)
}
$$

called the squared Mahalanobis distance.

Gaussian weight:

$$
w=e^{-q/2}.
$$

---

**Why covariance appears inverted**

Suppose 1D:

$$
\Sigma=\sigma^2.
$$

Then:

$$
\Sigma^{-1}
=
\frac1{\sigma^2}.
$$

Therefore:

$$
q=
\frac{(x-\mu)^2}{\sigma^2},
$$

recovering the 1D formula.

Large variance should make the Gaussian wider.

Indeed, if:

$$
\sigma^2\uparrow
$$

then:

$$
\Sigma^{-1}\downarrow,
$$

so the exponent decreases more slowly.

Correct.

---


In 3D:

$$
\boxed{
\boldsymbol\mu=
\begin{bmatrix}
\mu_x\\
\mu_y\\
\mu_z
\end{bmatrix}.
}
$$

For a 3DGS primitive this is simply its center in world coordinates.

Maveb stores:

```cpp
std::array<float, 3> position;
```

which corresponds directly to:

$$
\boldsymbol\mu.
$$

This is the value later transformed by:

$$
worldToCamera.
$$

---


For a random vector:

$$
X=
\begin{bmatrix}
X_1\\
X_2\\
X_3
\end{bmatrix},
$$

covariance is:

$$
\boxed{
\Sigma
=
E[
(X-\mu)
(X-\mu)^T
].
}
$$

In 3D:

$$
\Sigma=
\begin{bmatrix}
\sigma_x^2 & \sigma_{xy} & \sigma_{xz}\\
\sigma_{xy} & \sigma_y^2 & \sigma_{yz}\\
\sigma_{xz} & \sigma_{yz} & \sigma_z^2
\end{bmatrix}.
$$

Because covariance is symmetric:

$$
\Sigma^T=\Sigma.
$$

So a general symmetric 3×3 covariance has six independent values.

---


This is one of the most useful interpretations.

Take unit direction:

$$
\mathbf u.
$$

Project random displacement onto that direction:

$$
Y=
\mathbf u^T(X-\mu).
$$

Then:

$$
Var(Y)
=
E[
(\mathbf u^T(X-\mu))^2
].
$$

Rewrite:

$$
=
E[
\mathbf u^T
(X-\mu)(X-\mu)^T
\mathbf u
].
$$

Therefore:

$$
\boxed{
Var_{\mathbf u}
=
\mathbf u^T
\Sigma
\mathbf u.
}
$$

This equation tells you the variance of the Gaussian along **any direction**.

If:

$$
\mathbf u=
(1,0,0)^T,
$$

then:

$$
Var_{\mathbf u}
=
\Sigma_{00}.
$$

For a rotated direction, off-diagonal covariance terms participate.

---


A constant Gaussian-density contour satisfies:

$$
(\mathbf x-\mu)^T
\Sigma^{-1}
(\mathbf x-\mu)
=
k^2.
$$

This is an ellipsoid.

For:

$$
k=1
$$

we obtain the one-sigma ellipsoid.

For:

$$
k=3
$$

the three-sigma ellipsoid.

This is the geometric object you should imagine for each 3DGS primitive.

---


Because $\Sigma$ is real and symmetric:

$$
\Sigma
=
Q\Lambda Q^T
$$

where columns of $Q$ are orthonormal eigenvectors and:

$$
\Lambda=
diag(
\lambda_1,
\lambda_2,
\lambda_3
).
$$

Each eigenvector:

$$
\mathbf e_i
$$

is a principal direction of the ellipsoid.

Along:

$$
\mathbf e_i,
$$

directional variance is:

$$
\mathbf e_i^T
\Sigma
\mathbf e_i
=
\lambda_i.
$$

---


The standard-deviation radius along principal axis $i$ is:

$$
\boxed{
\sigma_i=
\sqrt{\lambda_i}.
}
$$

The $k\sigma$ ellipsoid has semi-axis:

$$
\boxed{
r_i=
k\sqrt{\lambda_i}.
}
$$

So eigenvectors tell orientation.

Eigenvalues tell squared extent.

This becomes exactly the same computation after we project into 2D.

---


For a valid nondegenerate Gaussian covariance we want:

$$
\boxed{
\mathbf x^T
\Sigma
\mathbf x
>
0
\quad
\forall\mathbf x\ne0.
}
$$

That means:

$$
\Sigma\succ0.
$$

Equivalent for symmetric $\Sigma$:

$$
\lambda_i>0
\quad
\forall i.
$$

Then:

$$
det(\Sigma)>0
$$

and:

$$
\Sigma^{-1}
$$

exists.

---

**Positive semidefinite versus definite**

If one axis scale becomes zero:

$$
\lambda_3=0,
$$

then:

$$
\Sigma\succeq0
$$

but not:

$$
\Sigma\succ0.
$$

It represents a collapsed distribution, perhaps an exact mathematical disc.

But:

$$
\Sigma^{-1}
$$

does not exist.

Rasterization eventually needs an inverse screen covariance, so numerical implementations avoid exact degeneracy.

This is one reason positive scale parameterizations are valuable.

---


Storing arbitrary six covariance values directly has problems.

Optimization could produce:

$$
\Sigma
$$

that is not positive semidefinite.

Instead define:

$$
S=
\begin{bmatrix}
s_x&0&0\\
0&s_y&0\\
0&0&s_z
\end{bmatrix},
$$

and rotation:

$$
R\in SO(3).
$$

Construct:

$$
M=RS.
$$

Then:

$$
\boxed{
\Sigma
=
MM^T
=
RS^2R^T.
}
$$

Because:

$$
S^2=
diag(
s_x^2,
s_y^2,
s_z^2
),
$$

all positive scales imply positive eigenvalues.

---

**Why this guarantees positive semidefiniteness**

For arbitrary vector:

$$
\mathbf x,
$$

$$
\mathbf x^T
\Sigma
\mathbf x
=
\mathbf x^T
MM^T
\mathbf x.
$$

Let:

$$
\mathbf y=M^T\mathbf x.
$$

Then:

$$
\mathbf x^T
\Sigma
\mathbf x
=
\mathbf y^T\mathbf y
=
\|\mathbf y\|^2
\ge0.
$$

So:

$$
MM^T
$$

is always PSD.

If $M$ is nonsingular, then for every nonzero $\mathbf x$:

$$
\|\mathbf y\|^2>0,
$$

hence SPD.

---

**Maveb implementation**

Current CPU oracle computes:

```cpp
const float scale =
    std::exp(
        gaussian.logScale[index]);

variance[index] =
    scale * scale;
```

then:

```cpp
result[row][column] +=
    rotation[row][axis] *
    variance[axis] *
    rotation[column][axis];
```

which is exactly:

$$
\boxed{
R
diag(s_x^2,s_y^2,s_z^2)
R^T.
}
$$

The Metal kernel performs the same computation:

```metal
const float3 scales =
    exp(
        gaussian.logScaleRestCount.xyz);

const float3 variances =
    scales * scales;

const float3x3 worldCovariance =
    rotation *
    diag(variances) *
    transpose(rotation);
```

CPU and GPU representation agree here.

---


Maveb's canonical Gaussian does not store:

$$
s_x,s_y,s_z.
$$

It stores:

```cpp
logScale
```

so:

$$
\boxed{
s_i=e^{\ell_i}.
}
$$

where:

$$
\ell_i
$$

is the learned/stored log scale.

---

**Why log scale is useful**

First:

$$
e^\ell>0
$$

for all finite $\ell$.

Thus negative physical scale cannot occur.

Second, additive log changes correspond to multiplicative physical changes:

$$
e^{\ell+\Delta}
=
e^\ell e^\Delta.
$$

For small $\Delta$:

$$
e^\Delta
\approx
1+\Delta.
$$

So optimization naturally makes relative scale changes.

Third:

$$
\frac{ds}{d\ell}
=
e^\ell
=
s.
$$

Gradient magnitude scales with physical size.

---

**Log scale does not automatically prevent numerical problems**

If:

$$
\ell=1000,
$$

then:

$$
e^\ell
$$

overflows.

If:

$$
\ell=-1000,
$$

it underflows near zero.

Maveb therefore validates:

$$
|\ell_i|
\le
\text{configured safe bound}
$$

during PLY loading.

This is good hostile-input engineering.

---


Maveb stores quaternion:

$$
q=(w,x,y,z).
$$

Current canonical structure:

```cpp
std::array<float, 4>
rotation
{
    1,0,0,0
};
```

Its PLY documentation explicitly preserves GraphDECO's:

$$
(w,x,y,z)
$$

property order.

---

**Normalize the quaternion**

Rotation quaternion should satisfy:

$$
\|q\|=1.
$$

So:

$$
q'
=
\frac{q}{\|q\|}.
$$

Maveb's loader rejects a near-zero quaternion:

$$
\|q\|^2<10^{-16}
$$

and normalizes every accepted Gaussian.

The Metal kernel normalizes again before matrix construction:

```metal
normalize(
    gaussian.rotation)
```

which is a useful defensive measure.

---


For normalized:

$$
q=(w,x,y,z),
$$

one standard column-vector convention gives:

$$
R=
\begin{bmatrix}
1-2(y^2+z^2)
&
2(xy-wz)
&
2(xz+wy)
\\
2(xy+wz)
&
1-2(x^2+z^2)
&
2(yz-wx)
\\
2(xz-wy)
&
2(yz+wx)
&
1-2(x^2+y^2)
\end{bmatrix}.
$$

Maveb CPU oracle uses this convention.

The MSL source appears transposed textually because Metal matrix constructors are column-oriented; semantically it constructs the corresponding rotation used in:

$$
RDR^T.
$$

The lesson is important:

$$
\boxed{
\text{never compare matrix source text without first checking storage/constructor convention}.
}
$$

---


Given decoded:

$$
\mathbf s=
(s_x,s_y,s_z),
$$

construct:

$$
S=
diag(s_x,s_y,s_z).
$$

Then:

$$
S^2=
diag(
s_x^2,
s_y^2,
s_z^2
).
$$

You may either compute:

$$
M=RS
$$

then:

$$
MM^T,
$$

or directly:

$$
R
diag(s^2)
R^T.
$$

Maveb uses the latter.

---


Now the entire physical geometry representation becomes:

$$
\boxed{
\Sigma_w
=
R(q)
\begin{bmatrix}
e^{2\ell_x}&0&0\\
0&e^{2\ell_y}&0\\
0&0&e^{2\ell_z}
\end{bmatrix}
R(q)^T.
}
$$

This is worth memorizing.

Stored representation:

$$
(\ell_x,\ell_y,\ell_z,q).
$$

Rendered geometric representation:

$$
\Sigma.
$$

---

**Example**

Suppose:

$$
s=(2,1,0.25)
$$

and:

$$
R=I.
$$

Then:

$$
\Sigma=
diag(
4,
1,
0.0625
).
$$

One-sigma ellipsoid axes:

$$
2,\ 1,\ 0.25.
$$

Three-sigma axes:

$$
6,\ 3,\ 0.75.
$$

This Gaussian resembles a flat elongated ellipsoid.

Rotate it by $R$ and the covariance rotates without changing eigenvalues.

---


A Gaussian needs a base opacity:

$$
\alpha_0.
$$

At screen offset $\Delta p$, final local alpha will later resemble:

$$
\alpha(\Delta p)
=
\alpha_0
\exp
\left(
-\frac12
\Delta p^T
\Sigma_{2D}^{-1}
\Delta p
\right).
$$

At its center:

$$
\Delta p=0
$$

so:

$$
\alpha(0)=\alpha_0.
$$

Thus $\alpha_0$ is maximum primitive opacity before any additional clamps.

---


Just as scale is stored in an unconstrained domain, opacity is stored as a logit:

$$
o\in\mathbb R.
$$

Decode:

$$
\boxed{
\alpha_0
=
\sigma(o)
=
\frac1{1+e^{-o}}.
}
$$

Therefore:

$$
0<\alpha_0<1.
$$

---

**Why logits help optimization**

Directly optimizing:

$$
\alpha\in[0,1]
$$

would require explicit constraints.

Logit is unconstrained.

The sigmoid maps it naturally.

Derivative:

$$
\boxed{
\frac{d\alpha}{do}
=
\alpha(1-\alpha).
}
$$

Maximum derivative occurs around:

$$
\alpha=0.5.
$$

Near 0 or 1 gradients saturate.

That is the standard tradeoff.

---

**Stable sigmoid evaluation**

Naive:

$$
\frac1{1+e^{-x}}
$$

may overflow in $e^{-x}$ for very negative $x$.

Vulkax implements a numerically stable branch:

for:

$$
x\ge0:
$$

$$
z=e^{-x},
\quad
\sigma(x)=\frac1{1+z}.
$$

for:

$$
x<0:
$$

$$
z=e^x,
\quad
\sigma(x)=\frac{z}{1+z}.
$$

Maveb's rasterizer instead clamps:

$$
x\in[-30,30]
$$

then evaluates the ordinary sigmoid.

Both avoid dangerous exponential ranges.

---

**Maveb deliberately preserves log/logit domains**

Its canonical structure is:

```cpp
struct Gaussian
{
    float position[3];

    float logScale[3];

    float rotation[4];

    float opacityLogit;

    float dc[3];

    float rest[45];

    size_t restCount;
};
```

The loader does **not** permanently decode scale or opacity.

That is excellent because import remains lossless with respect to the trained 3DGS parameterization.

---

### SPHERICAL HARMONICS, PLY AND IDENTITY


If each Gaussian stored only:

$$
RGB
$$

then appearance would be view independent.

But real scenes contain:

- specularities,
- reflections,
- directional highlights,
- view-dependent effects.

3DGS therefore stores color as coefficients of directional basis functions.

For direction:

$$
\mathbf d
$$

color becomes approximately:

$$
\boxed{
C(\mathbf d)
=
\sum_i
c_iY_i(\mathbf d).
}
$$

Each:

$$
Y_i
$$

is a spherical harmonic basis function.

Each:

$$
c_i
$$

is learned.

---


You could store an arbitrary function:

$$
C:S^2\rightarrow\mathbb R^3.
$$

But storing an entire function for every Gaussian is impossible.

Instead approximate it with a finite basis:

$$
C(\mathbf d)
\approx
\sum_{l=0}^{L}
\sum_{m=-l}^{l}
c_{lm}
Y_l^m(\mathbf d).
$$

This is analogous to Fourier series.

Fourier basis decomposes periodic 1D signals.

Spherical harmonics decompose functions on the sphere.

---


A normalized Cartesian direction:

$$
\mathbf d=
(x,y,z),
\quad
x^2+y^2+z^2=1
$$

can be written using angles.

Under one common convention:

$$
x=\sin\theta\cos\phi,
$$

$$
y=\sin\theta\sin\phi,
$$

$$
z=\cos\theta.
$$

Where:

$$
\theta
$$

is polar angle and:

$$
\phi
$$

is azimuth.

However, you do **not** need to compute:

$$
\theta,\phi
$$

to evaluate low-order real SH.

Polynomial expressions in:

$$
x,y,z
$$

are cheaper.

Maveb does exactly that.

---

**Direction convention is part of your data ABI**

Maveb computes:

$$
\mathbf d
=
\frac{
\mu-\text{cameraPosition}
}{
\|\mu-\text{cameraPosition}\|
}.
$$

That is:

$$
\boxed{
\text{camera}\rightarrow\text{Gaussian}.
}
$$

If you accidentally use:

$$
-\mathbf d,
$$

odd-order SH terms change sign.

Degree 1 and degree 3 are odd under inversion.

So a direction-convention error can produce plausible but wrong colors.

This is a classic 3DGS debugging failure.

---


SH functions are indexed by:

$$
l\ge0
$$

and:

$$
-l\le m\le l.
$$

For each degree $l$, number of functions is:

$$
2l+1.
$$

Total coefficients through degree $L$:

$$
N(L)
=
\sum_{l=0}^{L}
(2l+1).
$$

Use:

$$
\sum_{l=0}^{L}(2l+1)
=
(L+1)^2.
$$

Therefore:

$$
\boxed{
N(L)=(L+1)^2.
}
$$

---


| Degree $L$ | Basis coefficients per color | Total RGB coefficients | `f_rest_*` count |
|---:|---:|---:|---:|
| 0 | 1 | 3 | 0 |
| 1 | 4 | 12 | 9 |
| 2 | 9 | 27 | 24 |
| 3 | 16 | 48 | 45 |

Why does degree 3 have:

$$
45
$$

rest values?

There are:

$$
16
$$

coefficients per color.

The DC term is already stored separately:

$$
1.
$$

Remaining:

$$
15
$$

per color.

Three colors:

$$
3\times15
=
45.
$$

Exactly Maveb's representation.

---


Maveb follows the standard real SH convention used by the GraphDECO renderer.

Degree zero:

$$
Y_0=
C_0
$$

with:

$$
C_0=
0.28209479177387814.
$$

Degree one:

$$
Y_1=-C_1y,
$$

$$
Y_2=C_1z,
$$

$$
Y_3=-C_1x,
$$

where:

$$
C_1=
0.4886025119029199.
$$

---

**Degree-two functions**

Maveb evaluates:

$$
Y_4=
1.0925484305920792xy,
$$

$$
Y_5=
-1.0925484305920792yz,
$$

$$
Y_6=
0.31539156525252005
(2z^2-x^2-y^2),
$$

$$
Y_7=
-1.0925484305920792xz,
$$

$$
Y_8=
0.5462742152960396
(x^2-y^2).
$$

These are real SH polynomial basis functions after normalization constants are folded in.

---

**Degree-three functions**

Maveb/GraphDECO degree-three terms are:

$$
Y_9=
-0.5900435899266435
y(3x^2-y^2),
$$

$$
Y_{10}
=
2.890611442640554xyz,
$$

$$
Y_{11}
=
-0.4570457994644658
y(4z^2-x^2-y^2),
$$

$$
Y_{12}
=
0.3731763325901154
z(2z^2-3x^2-3y^2),
$$

$$
Y_{13}
=
-0.4570457994644658
x(4z^2-x^2-y^2),
$$

$$
Y_{14}
=
1.445305721320277
z(x^2-y^2),
$$

$$
Y_{15}
=
-0.5900435899266435
x(x^2-3y^2).
$$

Current Maveb CPU and Metal implementations use these same constants.

The canonical GraphDECO CUDA implementation uses the same polynomial basis and adds a 0.5 color offset after SH evaluation. [GitHub](https://github.com/graphdeco-inria/diff-gaussian-rasterization/blob/main/cuda_rasterizer/forward.cu?utm_source=chatgpt.com)

---


For channel $c$:

$$
C_c(\mathbf d)
=
\sum_{i=0}^{N-1}
a_{i,c}Y_i(\mathbf d).
$$

Maveb then computes:

$$
\boxed{
RGB=
\max(
0,
C(\mathbf d)+0.5
).
}
$$

The 0.5 offset is part of the trained GraphDECO convention.

This is not arbitrary post-processing.

---

**Important Maveb versus Vulkax difference**

Current Maveb performs:

$$
\max(C+0.5,0).
$$

It does **not** clamp the upper side to 1.

Thus:

$$
RGB>1
$$

can survive and participate in HDR processing.

Current Vulkax SH evaluator does:

$$
clamp(C+0.5,0,1).
$$

That is a semantic difference.

So if Vulkax and Maveb are compared as Gaussian appearance oracles, an HDR/brighter-than-one SH case may diverge even when every other part is correct.

This deserves an explicit agreement test rather than silently assuming both evaluators are identical.

---


PLY begins with a text header:

```text id="zw47bm"
ply
format binary_little_endian 1.0
element vertex 1000000

property float x
property float y
property float z

...

end_header
```

Then payload follows.

PLY itself knows only:

> vertices have named scalar properties.

3DGS assigns semantic names such as:

```text id="f1u91b"
x y z
f_dc_0 ...
f_rest_0 ...
opacity
scale_0 ...
rot_0 ...
```

---

**Maveb's required Gaussian schema**

Maveb requires:

```text id="8jo8qo"
x y z
f_dc_0 f_dc_1 f_dc_2
opacity
scale_0 scale_1 scale_2
rot_0 rot_1 rot_2 rot_3
```

Higher SH properties:

```text id="m1flmm"
f_rest_N
```

are optional.

But if present they must be dense and correspond to a complete degree:

$$
9,\ 24,\ 45.
$$

It rejects a partial degree such as:

$$
17
$$

rest coefficients.

That is a strong invariant.

---


Current Maveb parser first validates magic:

```text id="czuxw1"
ply
```

then recognizes:

```text id="qlh0ei"
format
element
property
end_header
comment
obj_info
```

It rejects unknown directives rather than pretending they are understood.

It accepts:

$$
PLY\ 1.0.
$$

Supported formats:

$$
ASCII
$$

and:

$$
binary_little_endian.
$$

Big-endian is rejected.

---

**Hostile-input limits matter**

A renderer importing external PLY should never trust:

```text id="ye44ct"
vertex count
property count
header size
file size
```

blindly.

Maveb has explicit upper limits for:

- file bytes,
- header bytes,
- number of properties,
- number of Gaussians,
- absolute log scale.

This is the difference between a research script loader and production-facing asset infrastructure.

---


PLY allows properties:

```text id="68k852"
property float x
property double opacity
property uint some_id
```

Maveb supports common signed/unsigned 8/16/32-bit integers and float32/float64 scalar fields.

For Gaussian vertices it rejects:

```text id="gkre1f"
property list ...
```

because a 3DGS Gaussian record is expected to be a flat scalar structure.

---

**Parse by name, not fixed column index**

This is important.

Do not assume:

$$
column_0=x,
column_1=y,...
$$

unless schema guarantees it.

Maveb parses property declarations, then assigns each scalar based on its name.

Therefore property order can vary while meaning remains intact.

This is robust.

---


ASCII payload has one textual scalar sequence per Gaussian.

For each vertex Maveb:

reads exactly one line,

reads one scalar for every declared property,

rejects too few values,

rejects extra values,

rejects trailing non-whitespace payload.

This strictness prevents silent misalignment.

---

**ASCII advantages and costs**

ASCII:

Human inspectable.

Easy to debug.

Large.

Slow to parse.

Potential float text conversion differences.

Binary:

Compact.

Faster.

Requires byte-level correctness.

Not human readable.

For multi-million-Gaussian fields, binary is generally much more practical.

---


Binary payload is:

```text id="j6pgmr"
vertex0 property0 bytes
vertex0 property1 bytes
...
vertex1 property0 bytes
...
```

Stride:

$$
S=
\sum_i
sizeof(property_i).
$$

Expected payload:

$$
PayloadBytes
=
VertexCount\times S.
$$

Maveb checks this exactly.

That prevents truncated/oversized malformed payloads.

---

**Manual little-endian reconstruction**

For unsigned integer bytes:

$$
b_0,b_1,\dots
$$

little-endian value:

$$
v=
\sum_i
b_i2^{8i}.
$$

Maveb explicitly reconstructs values from bytes.

That keeps file endianness semantics independent of host assumptions.

Float bit patterns can then be interpreted through the corresponding scalar handling.

---


Maveb mapping is direct:

$$
(x,y,z)
\rightarrow
position.
$$

$$
scale_i
\rightarrow
logScale_i.
$$

$$
rot_i
\rightarrow
quaternion_i.
$$

$$
opacity
\rightarrow
opacityLogit.
$$

$$
f_dc_i
\rightarrow
dc_i.
$$

$$
f_rest_i
\rightarrow
rest_i.
$$

Then quaternion is normalized.

Crucially:

$$
scale_i
$$

is **not** exponentiated during import.

And:

$$
opacity
$$

is **not** sigmoid-decoded during import.

---

**GraphDECO SH PLY order**

Maveb explicitly documents degree-3 higher SH data as:

$$
15\text{ red}
$$

followed by:

$$
15\text{ green}
$$

followed by:

$$
15\text{ blue}.
$$

So its evaluator gets coefficient:

$$
\boxed{
rest[
channel\times15
+
(coefficient-1)
].
}
$$

That matches its loader, which preserves:

$$
f_rest_0,\dots,f_rest_{44}
$$

in property order.

---

**A Vulkax convention issue worth testing**

Current Vulkax loader also places:

$$
f_rest_N
\rightarrow
shRest[N]
$$

without transposing.

But its current SH evaluator indexes:

$$
shRest[
(coefficient-1)\times3
+
channel
].
$$

That is coefficient-major/interleaved RGB indexing.

Maveb's canonical GraphDECO interpretation is channel-major.

Therefore a standard GraphDECO degree-2/3 PLY should be used as a specific regression test here.

If no conversion occurs elsewhere, those indexing conventions represent different coefficient layouts.

This is exactly the kind of subtle cross-repository issue that makes “the data loads successfully” an insufficient test.

---


Suppose Gaussian vector is:

```text id="e6vf6b"
index 0 → A
index 1 → B
index 2 → C
```

Sort by depth:

```text id="riprkz"
index 0 → C
index 1 → A
index 2 → B
```

If identity means:

$$
index,
$$

then Gaussian A changed identity from:

$$
0\rightarrow1.
$$

That is unacceptable for:

- editor selection,
- physics correspondence,
- persistent edits,
- local repair,
- provenance.

---

**Stable identity**

Instead:

```text id="juc7yl"
A → stable ID 1001
B → stable ID 1002
C → stable ID 1003
```

Sorting changes storage indices, not identity.

You maintain:

$$
ID\rightarrow currentIndex
$$

as a transient lookup.

---

**Vulkax has a strong implementation**

Its:

```cpp id="t7ceps"
GaussianId
{
    uint32_t namespaceId;
    uint32_t localId;
}
```

packs to:

$$
64\text{-bit ID}.
$$

`GaussianIndexView` builds:

$$
stableID\rightarrow vectorIndex.
$$

Its source comment is exactly correct:

> rebuild the index view after reorder/filter operations; do not persist vector indices as identity.

That is the right model.

---

**Maveb canonical Gaussian currently differs**

`GaussianAsset::Gaussian` currently contains no stable ID field.

Its CPU reference image uses:

$$
sourceIndex+1
$$

for dominant-contributor ID.

That is deterministic for a fixed unchanged asset vector.

It is **not a persistent semantic identity under reorder/filter/edit**.

Maveb's editable-world systems therefore need a higher-level identity/correspondence mechanism—which is precisely where Phase 25 later goes.

For raw rendering oracle purposes, source index is adequate.

For persistent world editing, it is not.

---

<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/38 - Projecting Ellipsoids into Screen-Space Conics|Next chapter]]
