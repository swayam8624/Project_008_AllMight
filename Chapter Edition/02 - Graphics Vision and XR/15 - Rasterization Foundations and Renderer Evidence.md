# 15 — Rasterization Foundations and Renderer Evidence

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/14 - Transparency, Blending and Visibility|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/16 - GPU Memory, Divergence and Execution Order|Next chapter]]

The renderer snapshot provides concrete evidence about a particular implementation before the text turns to general rasterization principles. This span establishes that distinction and develops the foundational pipeline.

**By the end:** Separate implementation-specific renderer evidence from general rasterization concepts.

**Recall question:** Which claims describe this renderer’s state, and which are general pipeline rules?

<!-- Derived from Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography.md; source lines 35300–37331; source-span SHA256 a78b04e715d3b81b09dbc46ac29d254d8f057bb0a3210dcc2c4a1812faa8adc1. -->

<!-- CHAPTER-SOURCE-BEGIN -->
### What Kairo specifically taught us in this pass

The live implementation is currently a fairly clean traditional forward Vulkan renderer rather than a disguised “modern feature showcase,” which is useful for learning. It has an HDR offscreen color target, integer object picking, conventional D32 depth, no stencil, no MSAA, no backface culling, dynamic viewport/scissor, straight-alpha transparent blending, cutout discard, per-object indexed draws, small hand-created PSO permutations, a large camera/light uniform block, 128-byte per-draw push constants, no application Vulkan pipeline cache in this path, and a frame lifecycle explicitly decomposed into acquire → record → submit → present.

That is an excellent baseline because later phases can introduce GPU-driven draws, render graphs, reversed-Z, Hi-Z, clustered lighting and more sophisticated transparency as measurable architectural changes instead of starting from an opaque engine where everything is already hidden.

### Mastery gate

If you can now explain **why stencil needs three result operations**, derive source-over blending, explain exactly why premultiplied alpha behaves better under filtering, derive why transparency ordering matters, explain why front-to-back benefits opaque rendering but conflicts with back-to-front transparency, derive the Hi-Z reduction direction for conventional and reversed depth, trace a compute-written indirect command through its synchronization into a draw, distinguish instance/uniform/storage/push data, explain why shader permutations explode, and reconstruct the CPU rasterizer from edge equations rather than memorizing its code, then Phase 19 is in good shape.

**Depth gate: passed for Pass 2.**

### Next — Pass 03

**M115 + M116, Parts 1901–1944:** radiometry and color science → radiance/irradiance/solid angle → linear RGB vs sRGB/gamma → HDR/exposure/tone mapping → texture signal theory → sampling/reconstruction → mipmaps/LOD derivatives → anisotropic filtering → compression → tangent-space normal mapping → parallax/displacement foundations.


Rasterization converts projected geometry into covered pixel samples and uses depth to decide which fragment contributes.

### Triangle coverage and sampling

Triangles are convenient primitives because polygons can be triangulated, edges are straight, and attributes can be interpolated across a face. A raster pipeline clips geometry, transforms vertices to screen coordinates, sets up triangles, finds covered sample locations, interpolates attributes, performs depth tests, and updates color samples. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-04 - Drawing a Triangle and an Intro to Sampling|CMU-04]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-08 - Depth and Transparency|CMU-08]]

The exact coverage of a pixel is the fraction of its area lying inside a triangle. For pixel region $P$, the exact coverage is $|P|^{-1}\int_P \mathbf 1_T(x)\,dx$, where $\mathbf 1_T$ is 1 inside the triangle and 0 outside. Computing that integral exactly for every pixel is expensive. A center sample estimates it as either 0 or 1, so a tiny edge shift can flip the whole result. The jagged boundary is aliasing: high spatial frequencies in a sharp edge exceed what the pixel grid can represent. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-04 - Drawing a Triangle and an Intro to Sampling|CMU-04]]

Supersampling evaluates multiple points inside one pixel. If there are $N$ subpixel samples $x_i$, estimated coverage is

$$
\widehat c(P)=\frac1N\sum_{i=1}^N\mathbf 1_T(x_i).
$$

Three covered samples out of four estimate coverage as $3/4$. More samples improve the estimate but cost computation. Filtering combines samples into a reconstructed pixel value. This is the same signal-processing problem as sampling a 1D waveform: samples alone do not recover arbitrary frequencies. A signal must be low-pass filtered before downsampling to prevent high frequencies from folding into lower ones. The Nyquist condition says a band-limited signal must be sampled at more than twice its highest frequency. A hard triangle edge is not band-limited, so practical renderers approximate antialiasing with finite sample patterns and reconstruction filters. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-04 - Drawing a Triangle and an Intro to Sampling|CMU-04]]

Nearest-neighbor reconstruction can look blocky; a box filter averages a footprint but may ring in frequency space; smoother filters trade sharpness against ringing. A regular grid has even coverage but can produce structured moiré when aligned with image features. Jittered random points make the error less structured; stratification ensures every subregion is represented. Blue-noise patterns avoid large clumps while suppressing low-frequency structure. Sample patterns change the appearance of error, so the most uniform geometric distribution is not always the visually best. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-04 - Drawing a Triangle and an Intro to Sampling|CMU-04]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-19 - Variance Reduction|CMU-19]]

### Barycentric coordinates, depth, and alpha

For triangle vertices $v_0,v_1,v_2$, an interior point can be expressed as $p=\lambda_0v_0+\lambda_1v_1+\lambda_2v_2$, with each $\lambda_i\ge0$ and $\sum_i\lambda_i=1$. The $\lambda_i$ are barycentric coordinates. They interpolate vertex color, normals, or texture coordinates. Perspective projection requires the depth correction described earlier. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-04 - Drawing a Triangle and an Intro to Sampling|CMU-04]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-07 - Perspective Projection and Texture Mapping|CMU-07]]

The z-buffer stores nearest depth per sample. Under a smaller-is-nearer convention, a fragment with new depth $z$ is accepted if $z<z_b$, where $z_b$ is stored depth. Accepted fragments replace both depth and color. For two overlapping opaque triangles at depths 8 and 3, the depth 3 fragment wins regardless of drawing order. Multisampling may store separate depth values at subpixel samples so a partly covered edge can retain correct visibility. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-08 - Depth and Transparency|CMU-08]]

Transparency requires compositing because a transparent layer reveals what lies behind it. If foreground color is $C_f$ with opacity $\alpha_f$ and background color is $C_b$, foreground-over-background is

$$
C_{out}=\alpha_fC_f+(1-\alpha_f)C_b.
$$

The operation is order-dependent. **Premultiplied alpha** stores $C'_f=\alpha_fC_f$, making the over operation $C'_{out}=C'_f+(1-\alpha_f)C'_b$. It also avoids colored fringes. For example, a fully transparent red pixel has straight color red and opacity zero. Averaging its straight color with opaque blue creates purple contamination even though the red was invisible. Premultiplied storage turns the transparent red into zero color, so filtering cannot introduce hidden red. [[Depth buffering and visibility order|Opaque visibility]] can use depth order; blended translucent layers usually need back-to-front composition or another order-aware method. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-08 - Depth and Transparency|CMU-08]]

### How a triangle reaches the framebuffer

The framebuffer is the array of samples that eventually becomes the displayed image. A color buffer stores color values; a depth buffer stores visibility values. A graphics pipeline turns vertex data into fragments, where a fragment is a candidate contribution to a sample. The following trace links geometry to pixels without treating any one stage as magic.

Start with three triangle vertices in object coordinates. A model transform places the object in the scene. A view transform expresses the scene relative to the camera. Projection maps camera coordinates into a clip-space representation, and clipping discards or cuts geometry outside the view volume. Perspective division converts clip coordinates to normalized device coordinates; viewport mapping scales them to screen pixels. At this point the triangle’s edges can be tested against sample locations. If a sample is covered, barycentric weights interpolate attributes. A depth test rejects a fragment hidden behind another. The accepted sample writes shaded color into the color buffer. Multiple samples per pixel can retain separate edge coverage and depth until a resolve step combines them.

Each coordinate space has a different convenient range and meaning. Object coordinates make a model reusable; world coordinates place all objects together; camera coordinates simplify view direction; clip coordinates make clipping efficient; screen coordinates index buffers. A matrix chain is often written $PVM$, meaning projection times view times model for a column-vector convention. In execution the rightmost model matrix acts first. Reversing the order silently changes the result.

The depth buffer stores projected depth, which is not generally a linear measure of distance from the camera. Perspective projection allocates more representable precision near the camera than far away under common mappings. Two surfaces with nearly equal depth may compare inconsistently, producing z-fighting. Solutions include adjusting near and far clipping planes, using a precision-aware depth format, or offsetting a surface intentionally. The depth representation and comparison rule are part of the rendering pipeline contract, not incidental state. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-01 - Course Overview|CMU-01]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-04 - Drawing a Triangle and an Intro to Sampling|CMU-04]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-05 - Spatial Transformations|CMU-05]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-08 - Depth and Transparency|CMU-08]]

### A depth-buffer table trace

For one sample, initialize depth to infinity and color to the background. Process fragment A at depth 8 and color red. Since 8 is nearer than infinity, write red and depth 8. Process fragment B at depth 3 and color blue. Since 3 is nearer than 8, overwrite with blue and depth 3. Process fragment C at depth 6 and color green. Since 6 is farther than 3, discard it. Final state is blue at depth 3. Reordering A and B still leaves B as the visible opaque fragment. If layers have fractional opacity, discarding the farther layer is wrong because it contributes through the foreground. Depth answers the visibility question; alpha compositing answers how visible layers combine.

### Coverage samples and a triangle rasterizer

A triangle’s coverage function is one inside the triangle and zero outside. An ideal pixel value would integrate this indicator over the square footprint of the pixel; for a colored surface, it would integrate the signal over the same footprint. A single sample at the pixel center estimates this integral very cheaply, but it can miss a thin triangle entirely or report full coverage when only a corner is touched. The edge of a triangle is a discontinuity, so it contains arbitrarily high spatial frequencies. The Nyquist–Shannon theorem’s exact reconstruction guarantee assumes a band-limited signal; a hard edge is not band-limited. This is why no finite regular sample rate perfectly solves all rasterization aliasing. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-04 - Drawing a Triangle and an Intro to Sampling|CMU-04]]

Supersampling approximates area coverage by taking several tests within a pixel and averaging their results. Suppose four sample points lie in a pixel and two are inside the triangle. The estimated coverage is $2/4=1/2$. The lecture compared one sample, four samples, sixteen samples, and 1,024 samples per pixel on a receding checkerboard. Each denser pattern looked smoother, but even 1,024 samples retained error because the image still has detail beyond that finite rate. A special checkerboard-on-plane example admits an analytic pixel integral, but arbitrary intersecting geometry, overlapping layers, and transparency make exact integration impractical. Sampling is attractive because the same finite-sample procedure works across general scenes. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-04 - Drawing a Triangle and an Intro to Sampling|CMU-04]]

For one point $q$ and triangle vertices $p_0,p_1,p_2$, a point-in-triangle test checks whether $q$ lies in the interior half-plane of each directed edge. A half-plane is one side of a line; an oriented edge test can use a 2D determinant or a dot product with the edge’s normal. Consistent vertex order matters because reversing an edge swaps its inside and outside sides. A pixel exactly on a shared edge needs a tie rule. The top-left rule assigns shared boundary samples to exactly one of the adjacent triangles; without it, two triangles can leave a hairline gap or double-draw the same sample. These same signed edge values can be normalized into barycentric coordinates, so coverage and interpolation can share arithmetic. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-04 - Drawing a Triangle and an Intro to Sampling|CMU-04]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-07 - Perspective Projection and Texture Mapping|CMU-07]]

To rasterize a whole triangle, one software strategy walks across pixel rows. The half-plane values for neighboring pixels change predictably, so each next test can update the previous result instead of recomputing every dot product. Row traversal also improves memory coherence: nearby framebuffer locations are accessed together. Modern graphics hardware instead evaluates many candidate samples in parallel inside a triangle’s bounding box. This trades extra tests for wide parallel execution and dedicated triangle-test hardware. A long, skinny triangle exposes the weakness: its rectangular bounding box may contain huge empty regions. Tiling adds a coarse test: reject a block wholly outside, accept a block wholly inside, and perform fine tests only on boundary blocks. A recursive hierarchy repeats this process at multiple scales, though traversal overhead can outweigh the saved work for common triangles. The design lesson is to match query granularity to hardware and geometry: exact tests only where coarse tests leave uncertainty. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-04 - Drawing a Triangle and an Intro to Sampling|CMU-04]]

### Modeling a cube by hand

The course introduction builds a cube centered at the origin with side length 2. Its coordinates are all sign combinations of $(1,1,1)$. Each edge joins vertices differing in one coordinate. To draw the cube from a camera at $c=(2,3,5)$, transform its vertices into camera coordinates, project them to the image plane, then draw each of the 12 edges. This exercise separates modeling from rendering: the cube is a list of vertices and edges, while a picture requires a camera and projection. It also makes the hidden pipeline concrete—changing camera position alters every projected coordinate even when the cube model stays the same. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-01 - Course Overview|CMU-01]]

### A cube drawing pipeline

Take a cube centered at the origin, with corners at every combination of $x,y,z\in\{-1,1\}$. To make an image, enumerate its twelve edges. For each edge, transform both endpoints from object coordinates to camera coordinates. Let the camera be at $c=(2,3,5)$ and look toward the origin. Build an orthonormal camera frame: viewing direction from camera to target, right direction from a cross product with the up vector, and camera-up from the cross of those two. Project each endpoint through the perspective divide, then rasterize the resulting screen-space line. The object description is unchanged if the camera moves; only the view transform changes. This separates model, camera, and image construction. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-01 - Course Overview|CMU-01]]

This toy task also exposes hidden conventions. If camera-up is nearly parallel to view direction, their cross product is nearly zero, so the camera frame is ill-conditioned. If an edge crosses the near clipping plane, clipping must trim it rather than projecting an endpoint behind the camera. If several edges overlap, visibility needs depth reasoning. A simple cube becomes an entry point to coordinate frames, projection, clipping, rasterization, and visibility.

### Opaque sample update

The following pseudocode states the invariant for a z-buffer. `nearer` must match the chosen depth convention, and clearing depth must use the corresponding far value.

```text
for each covered sample s of triangle T:
    z_new = interpolate_depth(T, s)
    if nearer(z_new, depth[s]):
        depth[s] = z_new
        color[s] = shade(T, s)
```

The triangle’s coverage rule determines which samples reach this loop. The interpolation rule determines the candidate depth. The depth convention determines `nearer`. If any one of these differs from the rest of the pipeline, visible artifacts appear. Multisampling runs the update per subpixel sample. Transparent blending needs a different update because a fragment contributes without fully replacing the stored color. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-04 - Drawing a Triangle and an Intro to Sampling|CMU-04]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-08 - Depth and Transparency|CMU-08]]

### A final numerical checkpoint: coverage, color, and accumulation

Imagine a black triangle covers exactly one quarter of a white pixel’s area. The physically averaged linear-light color is three quarters white and one quarter black, or 0.75. If the renderer uses four uniformly distributed subpixel samples and exactly one lands inside the triangle, the Monte Carlo estimate is also 0.75. With only one center sample, the result is either 0 or 1, depending on which side of the edge the sample lands. Both sampling schemes estimate the same target, but their variance differs. This tiny construction connects exact area integration, multisampling, and Monte Carlo estimation.

Now suppose the white value is stored in a nonlinear display encoding. The estimate should still average physical light in linear space. Decode the four sample colors to linear values, average to 0.75, and encode once for display. Averaging encoded values directly changes the brightness because encoding is nonlinear. If the triangle is translucent, multiply each sample’s foreground color by opacity before averaging, then composite with the background in premultiplied form. The order of operations follows the meanings of the stored quantities: coverage determines where the surface exists, radiance determines its physical contribution, alpha determines how layers combine, and display encoding happens after those operations. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-04 - Drawing a Triangle and an Intro to Sampling|CMU-04]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-08 - Depth and Transparency|CMU-08]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-14 - Color|CMU-14]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-17 - Numerical Integration|CMU-17]]

### Rasterization and depth

Rasterization starts with projected primitives and asks which pixels they cover. A triangle's projected vertices define edge functions; evaluating the signs at a pixel sample determines whether it lies inside. Interpolated barycentric coordinates then carry depth and attributes across the triangle. A depth buffer stores the closest depth encountered so far. For a candidate fragment at depth (z), keep it if it is nearer than the saved value, then update that value. This resolves visibility without explicitly constructing a global sort of all surfaces. The rasterization lectures discuss triangle coverage, interpolation, depth testing, and the cost of arranging the work over screen tiles [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-17 - Rasterization|MITG-17]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-18 - Rasterization II|MITG-18]] (00:10:00–00:35:00; , 00:20:00–00:50:00).

The z-buffer is simple and order-independent when depth values compare consistently, but precision is not uniform in camera distance under a perspective projection. Nearly equal depths can produce z-fighting. Reversing near/far choices, increasing precision, or moving the near plane can help; a depth buffer does not solve transparency ordering automatically. Rasterization also brings sampling back: a triangle may cover only part of a pixel, so single-sample coverage creates jagged edges. Multisample antialiasing evaluates geometric coverage at several sample positions while often shading the triangle less frequently than full supersampling. It improves edge coverage, but does not automatically antialias every shader signal or texture.

A tiled renderer bins primitives into screen-space tiles, then processes the primitives relevant to each tile. This improves locality and parallel scheduling: a tile's pixel region can be worked on with nearby data and an independent depth buffer. The 2021 rasterization lecture discusses the reordering and tiling of work, including space-filling curves as an ordering strategy [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-17 - Rasterization|MITG-17]] (00:05:00–00:20:00; 00:45:00–01:10:00). Its algorithmic point is that spatial coherence is a performance resource: pixels near each other usually touch many of the same primitives and memory.

### Rasterization converts projected triangles into depth-tested fragments

A ray tracer spends work on rays that might hit many objects. A rasterizer streams triangles and asks which screen samples each projected triangle covers. This is useful under a hard frame deadline: at 30 frames per second, the entire frame budget is about $1/30$ second, and missed deadlines are visible as stutter or latency. A typical pipeline transforms each triangle to camera/clip space, clips it to the view volume, projects vertices to the screen, generates fragments, shades them, depth-tests them, then presents a completed color buffer. Modern systems mix rasterization with ray tracing, but the pipeline makes the real-time tradeoffs explicit. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-17 - Rasterization|MITG-17]] (00:01:29–00:05:12; 00:19:32–00:25:26)

Projection maps a 3D triangle to a 2D triangle under the usual perspective camera, which lets scan conversion work in screen space. The vertex shader transforms one vertex at a time; the fragment stage handles candidate pixel samples generated by triangles. A fragment is a candidate sample associated with a primitive, not necessarily a final visible pixel. A million overlapping triangles can generate a million fragments even if they all land on one screen pixel. The depth test is what decides whether a fragment survives. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-17 - Rasterization|MITG-17]] (00:20:39–00:23:50; 01:06:31–01:08:30)

For a triangle with projected vertices $v_0,v_1,v_2$, each oriented edge defines a signed linear function $E_i(x,y)=a_i x+b_i y+c_i$. Choose a consistent winding so the interior satisfies all three inequalities $E_i(x,y)\ge 0$. A simple scan converter tests pixels in a bounding rectangle and evaluates these edge functions. The rectangle limits work, though a long diagonal triangle can have a large bounding rectangle and few covered pixels. Robust rasterizers also define exact shared-edge ownership rules so adjacent triangles neither leave cracks nor shade the same sample twice. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-17 - Rasterization|MITG-17]] (00:38:09–00:45:53)

Depth buffering stores the nearest depth at each pixel. A candidate fragment passes when its depth is nearer than the recorded value; the color and depth are then updated. It avoids sorting triangles globally, unlike the painter’s algorithm, which draws far surfaces first and nearer ones over them. Three overlapping bars can form a cyclic ordering, so no single global sort works. Limited depth precision can cause Z-fighting: nearly coplanar surfaces quantize to indistinguishable or fluctuating depth values. Double buffering keeps the image being scanned out separate from the one currently being rendered; swapping buffers presents a completed frame at once. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-17 - Rasterization|MITG-17]] (00:24:14–00:25:23; 00:32:00–00:33:50; 01:11:22–01:14:03)

Clipping removes or splits geometry outside the [[Camera view and clip-space pipeline|view frustum]] before perspective division, preventing division-by-zero and behind-camera degeneracies. Frustum culling tests a whole object bound before streaming its triangles. Homogeneous rasterization retains 3D structure longer: each triangle edge and the eye define a plane, and candidate samples are tested against the three oriented half-spaces. This rejects regions behind the eye without first dividing coordinates by depth. Clipping can still be faster when it removes whole primitives early; the choice depends on where work runs and the hardware. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-17 - Rasterization|MITG-17]] (00:55:00–01:04:35)

The Z buffer need store only depth evidence, not the name of the object at a pixel. Shading can often run independently across fragments because the BRDF evaluation uses local position, normal, material, and light data. Shadows and reflections require extra passes or buffers because a single forward triangle stream does not automatically answer what is visible from a light or reflected viewpoint. The next topics therefore add screen-space attributes and specialized passes rather than assuming every ray-tracing operation transfers directly. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-17 - Rasterization|MITG-17]] (01:05:23–01:08:30; 01:14:05–01:16:38)

### Perspective changes the interpolation weights inside a rasterized triangle

Rasterization finds a screen point first, but texture coordinates belong to the original 3D triangle. Linear barycentric weights in the projected triangle do not equal the weights in 3D: perspective division is nonlinear. Let the camera transform the homogeneous vertices into $q_i=Cp_i=(x_i',y_i',z_i',w_i')$. For a 3D point $p=\sum_i\lambda_i p_i$, the transformed point is $Cp=\sum_i\lambda_i q_i$. After perspective division, the screen coordinate is the ratio of weighted sums, not the weighted average of divided vertex coordinates. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-18 - Rasterization II|MITG-18]] (00:15:01–00:24:05)

A practical fragment recipe is: form the projected homogeneous vertex columns $Q=[q_0\ q_1\ q_2]$ in the coordinate convention used by the screen mapping. For fragment homogeneous screen coordinate $r=(x,y,1)^T$, solve $Q\mu\propto r$ using $\mu=Q^{-1}r$. Normalize the three entries: $\lambda_i=\mu_i/(\mu_0+\mu_1+\mu_2)$. These are the perspective-correct 3D barycentrics. Then interpolate a texture coordinate or other vertex attribute $a=\sum_i\lambda_i a_i$. The per-triangle inverse can be precomputed; each fragment needs a matrix-vector multiply and normalization. The same weights interpolate depth and material attributes consistently. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-18 - Rasterization II|MITG-18]] (00:18:01–00:34:11)

Antialiasing is costly in a raster pipeline because shading can be much more expensive than deciding whether a sample is covered. Supersampling renders a larger image and filters it down. Multisampling instead stores multiple subpixel coverage and depth samples but evaluates the shader once for a pixel, sharing that color across covered samples. This concentrates extra work at object boundaries, where coverage changes sharply, while avoiding repeated BRDF work across a smoothly shaded interior. It is an approximation: strong texture or lighting variation inside a pixel may still need multiple shading samples. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-18 - Rasterization II|MITG-18]] (00:35:09–00:53:00)

Mipmapping stores a pyramid of prefiltered textures. Choosing only the nearest level can cause a discontinuity as an object moves through a level threshold. Trilinear filtering blends the two neighboring mip levels; each lookup is bilinear in UV, so a trilinear lookup conceptually combines eight texel values. For a tilted triangle, the screen pixel footprint maps to a stretched ellipse in UV. Let $J=\partial(u,v)/\partial(x,y)$; its four partial derivatives describe how one screen pixel expands and rotates in texture space. The [[Singular value decomposition|singular directions]] of $J$ estimate major and minor footprint axes. [[Convolution and filter footprints|Anisotropic filtering]] approximates this ellipse with several circular/mip samples, improving detail along the short axis without aliasing along the long one. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-18 - Rasterization II|MITG-18]] (00:53:00–01:03:20)

Transparency complicates a single nearest-depth buffer because multiple layers contribute. A simple approximation draws opaque geometry first, then transparent surfaces with depth testing enabled but depth writes disabled. This lets a transparent window be occluded by a nearer opaque object while allowing multiple transparent layers to blend, though overlapping transparent surfaces still require careful order or more elaborate per-pixel fragment storage. A deferred shading pass stores visible depth and material/BRDF parameters first, then shades only the surface that survived visibility. A pre-Z pass writes depth before expensive shading; tile-based rendering limits active buffers to screen regions, which is especially useful on memory-limited mobile hardware. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-18 - Rasterization II|MITG-18]] (01:16:39–01:22:02)

## From measured samples to parallel GPU execution

The triangle pipeline needs an explicit shader-memory ABI and a model of parallel work before an API call is meaningful. This full existing teaching chapter connects alignment, buffers, invocation scheduling, synchronization, arithmetic intensity and reusable GPU algorithms to those pixel calculations.

## Make a shader record an explicit binary contract

The CPU writes bytes; the shader interprets them using offsets, strides, scalar widths and layout rules. A [[Concepts/Shader buffer ABI|shader ABI]], or application binary interface, is that agreement at the byte boundary. A C++ field name does not communicate its intended offset to a GPU. The same is true of a C++ `bool`: its representation is not an automatic portable shader representation. Prefer an explicit integer flag and verify the complete packed record.

Alignment is the set of byte addresses at which a value may begin. Size is how much storage represents the value. Array stride is the distance from the start of one element to the next. These quantities can differ. Under ordinary extended-alignment `std140` rules, a `vec3` begins at a 16-byte-aligned offset but its three float components occupy 12 bytes. A following scalar can occupy the fourth slot when the layout rules permit it. Treating every `vec3` as an unconditional 16-byte payload gives wrong predictions for some legal structures. Inspect compiler-generated member offsets and array strides, and assert the corresponding CPU offsets.

For the GLSL block `float exposure; vec3 color; float roughness; vec2 uvScale;`, the ordinary member offsets are $0,16,28,32$ and the block's rounded structure extent is 48 bytes. This is a layout calculation, not proof that an arbitrary C++ type has that representation. Arrays of floats have stride 16 under `std140` and stride 4 under ordinary base-alignment `std430`. Vulkan's permitted uniform-block layouts depend on enabled features; feature promotion to a core API version does not make every optional feature automatically enabled. [Khronos shader memory layout guide](https://docs.vulkan.org/guide/latest/shader_memory_layout.html)

Texture descriptor indexing changes how a shader finds an image; it does not remove resource lifetime or synchronization obligations. A descriptor is a binding record. An index into a table is an identifier. Neither means the allocation remains alive until a pending draw finishes. Track generation, ownership and completion independently. The material-facing interpretation remains in [[Chapter Edition/02 - Graphics Vision and XR/05 - Reflectance, Materials and Light Transport#Ask a surface where incoming energy can go|Ask a surface where incoming energy can go]].

## Let many invocations cooperate without inventing a global clock

A GPU dispatch launches many invocations: individual executions of a kernel. A workgroup is a set of invocations that can use a common shared-memory region and workgroup synchronization. A subgroup is a smaller execution collective exposed by an API. NVIDIA often calls its hardware collective a warp, while other vendors use wave terminology. Width and supported operations must be queried or constrained; 32 lanes is an example, not a portable constant.

SIMT means single instruction, multiple threads. Many logical invocations execute instructions through grouped hardware. If lanes disagree on a branch, hardware may execute different paths with different active-lane masks. This is divergence. Its cost depends on the actual paths, reconvergence, compiler lowering and hardware. Removing a useful branch can increase work, so measure the kernel rather than memorizing that every branch is expensive.

Latency is how long an individual operation takes. Throughput is how much work completes per unit time. A GPU can hide a memory wait by issuing ready work from another resident group. Occupancy measures the fraction of a relevant hardware residency limit occupied by active work; it is not a direct measure of useful work completed. Registers, shared memory, group size and implementation-specific allocation granularity constrain residency. More occupancy can help a latency-bound kernel and still lose to a lower-occupancy kernel that performs less work.

Coalescing combines suitable nearby lane accesses into fewer memory transactions. Shared-memory bank conflicts arise when concurrent accesses need conflicting service from the same hardware bank. These are different mechanisms. A padded transpose tile can remove a bank conflict while leaving global access unchanged. Texture caches and unified physical memory likewise do not remove the need to reason about access patterns.

### Scan turns local counts into nonoverlapping output regions

Suppose four visible objects emit $c=(2,0,3,1)$ records. The [[Concepts/Prefix scan and stream compaction|exclusive prefix sum]] is $o=(0,2,2,5)$. Object $i$ owns the interval $[o_i,o_i+c_i)$. These intervals do not overlap; their total output size is six. This explains why scan appears in visibility compaction, mesh generation, radix sorting and variable-length expansion. The mathematics is an allocation proof, not just a trick for adding numbers.

For large arrays, first scan blocks locally, scan their totals, then add each block's offset. If all workgroups cannot synchronize within the dispatch, use separate dispatch phases with explicit dependencies. A dispatch boundary by itself is not a universal memory-visibility guarantee. The consumer must wait for the relevant producer writes in the API's execution and memory model. In Vulkan, stage and access scopes identify those producer and consumer operations. [Khronos synchronization specification](https://docs.vulkan.org/spec/latest/chapters/synchronization.html)

```mermaid
flowchart LR
  A["Counts: 2, 0, 3, 1"] --> B["Exclusive scan: 0, 2, 2, 5"] --> C["Owned ranges: two, zero, three, one"] --> D["Scatter six records"]
  classDef allocation fill:#fff1dc,stroke:#a46a24,color:#55340f;
  class A,B,C,D allocation;
```

Atomicity, ordering and rendezvous answer different questions. An atomic addition prevents that counter update from being torn or lost. Memory ordering governs visibility and order of related operations. A workgroup barrier makes the participating invocations meet at a synchronization point and establishes the specified memory effects. An atomic counter does not by itself establish that unrelated payload writes are ready for a reader. A workgroup barrier does not make another workgroup finish.

Consider a final partially filled group in a tiled matrix multiply. Some lanes represent out-of-range matrix elements. They should write neutral zero values into their assigned shared slots and still participate in every required group barrier. Returning before a barrier that the other lanes reach can violate the synchronization contract. After consuming a shared tile, another barrier may be needed before any lane overwrites it for the next iteration. The first barrier protects consumers from incomplete loads; the second protects slower consumers from premature reuse.

### Load a partial matrix tile with two different bounds

The supplied tiled multiplication excerpt uses a placeholder `valid` for both input loads. That placeholder is not a sufficient complete bounds rule. Let the left matrix have shape $M\times K$ and the right matrix shape $K\times N$. For tile width $T$, row $r$, column $c$, inner-dimension base $b$, and local coordinates $(l_x,l_y)$, the two legal-load predicates are

$$
\text{left valid}:\quad r<M\ \land\ b+l_x<K,
\qquad
\text{right valid}:\quad b+l_y<K\ \land\ c<N.
$$

A thread whose output column is out of range can still need to load a left-hand element for another thread's valid output. Similarly, an out-of-range output row does not by itself make its right-hand load unnecessary. Mask the two loads separately, write zero for each missing element, and let every participating thread reach both barriers. Finally store only when $r<M$ and $c<N$. A zero-padded last tile makes the full inner tile loop valid without reading past either input.

```metal
// Illustrative tile body, with T x T participating threads.
// lhsTile and rhsTile are threadgroup arrays declared by the kernel.
for (uint base = 0; base < K; base += T) {
    lhsTile[local.y][local.x] =
        (row < M && base + local.x < K)
        ? lhs[row * K + base + local.x] : 0.0f;
    rhsTile[local.y][local.x] =
        (base + local.y < K && column < N)
        ? rhs[(base + local.y) * N + column] : 0.0f;
    threadgroup_barrier(mem_flags::mem_threadgroup);
    for (uint j = 0; j < T; ++j)
        sum += lhsTile[local.y][j] * rhsTile[j][local.x];
    threadgroup_barrier(mem_flags::mem_threadgroup);
}
if (row < M && column < N)
    output[row * N + column] = sum;
```

This is a kernel fragment with explicit assumptions, not a standalone compiled Metal program. CPU emulation of its load masks and arithmetic is recorded separately; that check does not establish Metal compilation, GPU synchronization or device behavior.

The following supplied GPU lessons retain implementation-specific examples as source observations. Their Kairo and Maveb claims describe the supplied snapshot; this integration has not established that they match today's repositories or tested their kernels on a GPU.


Implementation-specific passages retain observations from the supplied Kairo/Maveb snapshot. They are not a fresh audit of those repositories. Source exercises and illustrative code are teaching material; only explicitly recorded checks have been executed here.

**Material representation has several layers**

A healthy engine should distinguish at least these domains:

```text
authoring/interchange material
        ↓
persistent engine artifact
        ↓
runtime renderer-neutral material
        ↓
GPU material record
        ↓
shader representation
```

These do **not** need to have identical C++ structs.

For Kairo today:

```text
glTF material
    ↓
GltfMaterialData
    ↓
MakeGltfPBRMaterial()
    ↓
PBRMaterial
    ↓
MaterialUniform + descriptors
    ↓
GLSL MaterialData
```

That is already the correct broad layering.

The danger begins when one layer's ABI accidentally leaks into another.

---

**GPU data layout is an ABI**

Suppose C++ contains:

```cpp
struct Material
{
    Vec3 baseColor;
    float roughness;
    float metallic;
};
```

and GLSL says:

```glsl
layout(std140) uniform Material
{
    vec3 baseColor;
    float roughness;
    float metallic;
};
```

You cannot assume:

```text
same field names
→ same bytes.
```

CPU and GPU must agree on:

$$
\text{offset},
$$

$$
\text{alignment},
$$

$$
\text{stride},
$$

$$
\text{matrix order},
$$

$$
\text{scalar size}.
$$

This is a binary ABI contract.

---

**Alignment versus size**

These are different concepts.

A scalar `float` has size:

$$
4
$$

bytes.

Its alignment is typically:

$$
4.
$$

A `vec4` has:

$$
16\text{-byte size}
$$

and:

$$
16\text{-byte alignment}.
$$

A `vec3` contains 12 bytes of useful numeric data but frequently carries 16-byte alignment requirements in GPU interface layouts.

That mismatch is where many CPU/GPU struct bugs begin.

---

**std140**

`std140` is the traditional extended-alignment layout used especially for uniform blocks.

The broad rules to internalize are:

| Type | Typical std140 base alignment |
|---|---:|
| `float` / `int` | 4 |
| `vec2` | 8 |
| `vec3` | 16 |
| `vec4` | 16 |
| arrays | element alignment/stride rounded to vec4-like 16-byte boundaries |
| structs | maximum member alignment, rounded to a 16-byte boundary |
| matrices | treated essentially as aligned vector arrays |

Vulkan documentation refers to std140 as **extended alignment**. [docs.vulkan.org](https://docs.vulkan.org/guide/latest/shader_memory_layout.html?utm_source=chatgpt.com)

The primary characteristic is conservative 16-byte padding for aggregates such as arrays and structures.

---

**Example std140 struct**

Consider:

```glsl
layout(std140) uniform Example
{
    float exposure;
    vec3  color;
    float roughness;
    vec2  uvScale;
};
```

Start offset:

$$
0.
$$

`exposure`:

$$
\text{offset}=0,\quad\text{size}=4.
$$

Next `vec3` requires:

$$
16\text{-byte alignment}.
$$

So we jump:

$$
4\rightarrow16.
$$

`color` begins at:

$$
16.
$$

This immediately introduces 12 bytes between the first float and the vector.

Depending on exact member-layout rules and representation, relying on packing around `vec3` is notoriously error-prone.

A much safer CPU-side shape is:

```cpp
struct alignas(16) MaterialGPU
{
    std::array<float, 4> colorAndExposure;
    std::array<float, 4> params;
};
```

which exactly mirrors:

```glsl
vec4 colorAndExposure;
vec4 params;
```

---

**Why Kairo's current material block is smart**

Kairo shader:

```glsl
layout(set = 0, binding = 7) uniform MaterialData
{
    vec4 baseColorFactor;
    vec4 emissiveAndNormalScale;
    vec4 factors;
    vec4 flags;
} material;
```

That is:

$$
4\times16=64\text{ bytes}.
$$

CPU:

```cpp
struct MaterialUniform
{
    std::array<float, 16> Values{};
};
```

also:

$$
16\times4=64\text{ bytes}.
$$

There are no:

- `vec3` packing questions,
- bool representation questions,
- irregular offsets,
- nested arrays.

This is deliberately boring, which is excellent for an ABI.

---

**Kairo's actual 64-byte mapping**

Current packing is essentially:

$$
\texttt{Values[0..3]}
=
(\text{baseRGB},\alpha).
$$

$$
\texttt{Values[4..7]}
=
(\text{emissiveRGB},\text{normalScale}).
$$

$$
\texttt{Values[8..11]}
=
(\text{metallic},
\text{roughness},
\text{AO},
\text{alphaCutoff}).
$$

$$
\texttt{Values[12..15]}
=
(\text{alphaMode},
\text{doubleSided},
\text{unused},\text{unused}).
$$

That maps exactly onto four shader `vec4`s.

This is a very robust GPU-data layout.

---

**Why GPU-side booleans are awkward**

A CPU `bool` may occupy:

$$
1\text{ byte}.
$$

A shader boolean in an interface block may obey entirely different representation/alignment requirements.

Therefore sending:

```cpp
bool doubleSided;
```

directly into a raw GPU buffer is risky.

Kairo instead converts:

```cpp
DoubleSided
```

into:

$$
0.0\text{ or }1.0
$$

inside a float slot.

This is simple and portable.

For packed material tables, integers/bitfields are also common.

---

**std430**

`std430` is Vulkan/GLSL's **base alignment** layout.

It relaxes some std140 aggregate padding.

The biggest practical difference is arrays and structures of scalar/small-vector values.

For example, under std140:

```glsl
float values[4];
```

commonly has:

$$
16\text{-byte stride}.
$$

Thus four floats may consume:

$$
64\text{ bytes}.
$$

Under std430:

$$
\text{stride}=4
$$

is possible, so the same array uses:

$$
16\text{ bytes}.
$$

Vulkan documents std430 as base alignment, and Vulkan 1.2's `uniformBufferStandardLayout` feature allows std430-style layout for uniform buffers as well; storage buffers commonly use it already. [docs.vulkan.org](https://docs.vulkan.org/guide/latest/shader_memory_layout.html?utm_source=chatgpt.com)

---

**Why std430 matters for large material arrays**

Suppose you want:

$$
100,000
$$

materials.

Each material contains many scalars.

If every scalar/array element is unnecessarily promoted to 16-byte stride, memory waste becomes substantial.

For huge structured material tables, std430 is much more appropriate.

A modern material SSBO could look like:

```glsl
layout(std430, set = 1, binding = 0)
readonly buffer MaterialTable
{
    MaterialGPU materials[];
};
```

Then each draw carries:

$$
\texttt{materialIndex}
$$

instead of rebinding one UBO/descriptor set per material.

That is a major architectural improvement toward GPU-driven rendering.

---

**Example production material GPU record**

A scalable representation might be:

```cpp
struct alignas(16) MaterialGPU
{
    // 16 bytes
    float BaseColorR;
    float BaseColorG;
    float BaseColorB;
    float BaseColorA;

    // 16 bytes
    float EmissiveR;
    float EmissiveG;
    float EmissiveB;
    float NormalScale;

    // 16 bytes
    float Metallic;
    float Roughness;
    float AO;
    float AlphaCutoff;

    // 16 bytes
    uint32_t BaseColorTexture;
    uint32_t NormalTexture;
    uint32_t MetallicRoughnessTexture;
    uint32_t EmissiveTexture;

    // 16 bytes
    uint32_t OcclusionTexture;
    uint32_t Flags;
    uint32_t TexCoordMask;
    uint32_t Reserved;
};
```

Total:

$$
80\text{ bytes}.
$$

The key change is that textures are no longer Vulkan handles inside the struct.

They are **indices into a resource table**.

That is the bridge to bindless rendering.

---

**Padding is part of the format**

Suppose CPU believes:

```text
roughness offset = 12
```

but shader believes:

```text
roughness offset = 16.
```

Then the shader does not get a subtly inaccurate roughness.

It reads **different bytes**.

A float bit pattern from some unrelated field may become:

$$
-3.4\times10^{20}
$$

or:

$$
NaN.
$$

This can then propagate into:

$$
D_{\text{GGX}}
$$

and the entire framebuffer.

So GPU layout mismatches are not “serialization bugs.”

They are shader execution corruption.

---

**Compile-time layout defense**

For explicit CPU GPU structures, use assertions:

```cpp
struct alignas(16) MaterialGPU
{
    float BaseColor[4];
    float EmissiveNormal[4];
    float Factors[4];
    float Flags[4];
};

static_assert(
    sizeof(MaterialGPU) == 64);

static_assert(
    alignof(MaterialGPU) == 16);

static_assert(
    offsetof(MaterialGPU, BaseColor) == 0);

static_assert(
    offsetof(MaterialGPU, EmissiveNormal) == 16);

static_assert(
    offsetof(MaterialGPU, Factors) == 32);

static_assert(
    offsetof(MaterialGPU, Flags) == 48);
```

That turns a silent rendering bug into a compile-time failure.

Even better, generate CPU/shader layouts from one schema where possible.

---

**Why `std::array<float,16>` is robust but not expressive**

Kairo currently avoids ABI mistakes with:

```cpp
std::array<float,16>
```

which is good.

But:

```cpp
uniform.Values[10]
```

does not explain itself.

The long-term better pattern is to retain both:

$$
\text{explicit semantic field names}
$$

and:

$$
\text{layout guarantees}.
$$

For example:

```cpp
struct alignas(16) MaterialGPU
{
    Vec4f BaseColorFactor;
    Vec4f EmissiveNormalScale;
    Vec4f Factors;
    Vec4f Flags;
};
```

provided `Vec4f` is explicitly guaranteed to be 16 bytes/aligned as needed.

Then shader and CPU become mechanically readable.

---

**Bindless: what problem are we solving?**

Current Kairo has approximately:

```text
draw 0
→ bind descriptor set for material 0
→ draw

draw 1
→ bind descriptor set for material 1
→ draw

draw 2
→ bind descriptor set for material 2
→ draw
```

Every set contains several texture descriptors plus material UBO state.

This is perfectly reasonable for a compact renderer.

But at very large scene scales, explicit per-material rebinding becomes expensive and complicates GPU-driven submission.

Bindless-style rendering changes the model to:

```text
one large texture/resource table
+
material contains integer indices
+
shader dynamically chooses resources
```

---

**Bindless conceptual model**

Suppose:

```glsl
layout(set = 0, binding = 0)
uniform sampler2D textures[];
```

Material record:

```glsl
struct MaterialGPU
{
    uint baseColorTexture;
    uint normalTexture;
    uint mrTexture;
};
```

Then:

```glsl
MaterialGPU m =
    materials[materialIndex];

vec4 base =
    texture(
        textures[
            nonuniformEXT(
                m.baseColorTexture)],
        uv);
```

Now thousands of materials can coexist without rebinding a different descriptor set for every draw.

---

**Why `nonuniformEXT` exists**

Imagine one GPU wave processes fragments from different materials.

Lane 0 chooses texture:

$$
17
$$

while lane 1 chooses:

$$
42.
$$

The descriptor index is not uniform across the wave.

Vulkan descriptor indexing explicitly provides support for non-uniform dynamic indexing of descriptor arrays, along with runtime arrays, partially bound arrays and update-after-bind behavior. [docs.vulkan.org](https://docs.vulkan.org/refpages/latest/refpages/source/VK_EXT_descriptor_indexing.html?utm_source=chatgpt.com)

The shader/feature declarations must tell the implementation that indexing can differ across invocations.

This is not merely syntax.

It changes assumptions the compiler may make about descriptor selection.

---

**Descriptor indexing capabilities**

A Vulkan bindless-like design may rely on capabilities such as:

$$
\texttt{runtimeDescriptorArray},
$$

$$
\texttt{descriptorBindingPartiallyBound},
$$

$$
\texttt{descriptorBindingVariableDescriptorCount},
$$

$$
\texttt{shaderSampledImageArrayNonUniformIndexing},
$$

and possibly:

$$
\texttt{updateAfterBind}.
$$

These features must be queried and enabled; they are not magical properties of declaring `sampler2D[]`. [docs.vulkan.org](https://docs.vulkan.org/guide/latest/extensions/VK_EXT_descriptor_indexing.html?utm_source=chatgpt.com)

---

**Partially bound arrays**

Suppose your shader declares capacity for:

$$
100,000
$$

textures.

But currently only:

$$
11,000
$$

exist.

Without suitable descriptor-indexing behavior, descriptor validity requirements can force every declared slot to be populated.

With partially bound descriptors, unused dynamically unreferenced entries can remain invalid/unpopulated under the extension's rules. [docs.vulkan.org](https://docs.vulkan.org/spec/latest/chapters/descriptorsets.html?utm_source=chatgpt.com)

That makes large resource tables practical.

---

**Update-after-bind**

Ordinarily, descriptor contents are treated as stable while pending GPU work may use them.

Update-after-bind relaxes some of those rules.

This is useful for streaming:

```text
frame running
    ↓
new texture finishes loading
    ↓
descriptor table slot updated
```

without rebuilding every material.

But this flexibility introduces strict lifetime/synchronization responsibilities. Vulkan's descriptor-indexing documentation emphasizes that descriptor updates and resource lifetime still need precise correctness rules. [docs.vulkan.org](https://docs.vulkan.org/tutorial/latest/Building_a_Simple_Engine/Advanced_Topics/Descriptor_Indexing_UpdateAfterBind.html?utm_source=chatgpt.com)

“Bindless” does not mean “lifetime-less.”

---

**A bindless handle is not ownership**

Suppose material stores:

$$
\text{textureIndex}=317.
$$

If texture 317 is destroyed while a command buffer still references that index, the integer remains 317.

It has not become safe.

A scalable renderer therefore needs some policy around:

- descriptor-slot lifetime,
- deferred destruction,
- frame fences,
- generation counters,
- residency,
- streaming.

The hard part of bindless architecture is often resource lifetime, not shader syntax.

---

**Descriptor indexing and GPU-driven rendering fit together**

From Pass 2:

GPU-driven rendering wants the GPU to generate draw commands.

If each generated draw requires the CPU to bind a unique material descriptor set, we lose much of the point.

Instead:

```text
draw command
contains materialID
        ↓
shader
loads MaterialGPU[materialID]
        ↓
MaterialGPU
contains texture indices
        ↓
descriptor table
```

Now the entire rendering decision can stay on GPU-visible data.

This is why:

$$
\boxed{
\text{bindless resources}
+
\text{material tables}
+
\text{indirect draws}
}
$$

form such a common modern architecture.

---



### + M129

Phase 22 begins by changing the question again.

In Phase 21 we asked:

$$
\boxed{\text{How do I construct a correct Monte Carlo estimator?}}
$$

Now the question is:

$$
\boxed{\text{How do I make millions of copies of useful work execute efficiently on throughput hardware?}}
$$


The curriculum's own repository anchors are unusually useful here because Maveb already contains a real Metal Gaussian pipeline with **Blelloch scan, subgroup operations, threadgroup barriers, radix histograms, prefix sums and stable key/value scatter**. So this pass is not merely explaining textbook GPU algorithms—we can reverse-engineer code you already have.

---

### GPU EXECUTION ARCHITECTURE

**CPU and GPU optimization start from different goals**

A modern high-performance CPU is strongly optimized for **low latency on relatively few instruction streams**.

It invests substantial silicon in things such as sophisticated branch prediction, speculative/out-of-order execution, large caches, powerful individual cores and mechanisms for extracting instruction-level parallelism from comparatively narrow serial programs.

A GPU spends vastly more of its resources on arithmetic throughput.

Instead of saying:

> Make one thread finish this operation as soon as physically possible.

the GPU philosophy is closer to:

> Keep an enormous number of arithmetic lanes busy, and when some work stalls, immediately run other work.

The distinction is:

$$
\boxed{
\text{CPU}\approx\text{latency-oriented}
}
$$

versus:

$$
\boxed{
\text{GPU}\approx\text{throughput-oriented}.
}
$$

Neither statement is absolute. CPUs have SIMD and many cores; GPUs have caches and sophisticated schedulers. It is the architectural emphasis that differs.

---

**A simple latency versus throughput example**

Imagine a memory operation takes notionally:

$$
400
$$

cycles before the requested data is ready.

A CPU might try to avoid waiting by using caches, prefetchers, out-of-order execution and other independent instructions from the same few threads.

A GPU instead may have many groups of threads ready.

Warp A issues a memory load and stalls.

Rather than waiting 400 cycles, the scheduler can issue:

$$
B,\ C,\ D,\ E,\dots
$$

provided those groups are ready.

Hence:

$$
\boxed{
\text{GPU latency hiding is primarily achieved by having other independent execution groups resident}.
}
$$

This is the origin of the occupancy discussion later.

---

**Compute unit / streaming multiprocessor concept**

Different vendors use different terminology.

NVIDIA commonly talks about an SM, or streaming multiprocessor.

AMD commonly uses compute-unit/wave terminology.

Apple exposes Metal execution abstractions rather than requiring you to reason in exact vendor-internal units.

Generically, call it a **compute unit**.

A compute unit contains some combination of execution lanes, warp/wave scheduling hardware, a large register file, fast shared/threadgroup memory, load/store hardware, special-function units and caches/interfaces to the wider GPU memory system.

Many workgroups can potentially reside on one compute unit simultaneously.

The central hierarchy is:

```text id="5kw7e9"
GPU
    compute units
        resident workgroups
            subgroups / warps / waves
                lanes / invocations
```

These levels are not interchangeable.

---

**SIMT**

GPU shader programming exposes what appears to be many scalar threads:

```cpp id="e937zu"
kernel void Compute(uint id)
{
    output[id] =
        input[id] * 2.0f;
}
```

Each invocation appears independent.

But hardware typically groups several invocations and issues the same instruction across their lanes.

This is **Single Instruction, Multiple Threads**:

$$
\boxed{\text{SIMT}.}
$$

Each lane has its own logical registers and IDs, but the subgroup often shares instruction issue.

---

**SIMT versus CPU SIMD**

This distinction matters.

Your `KairoSIMD` implementation explicitly executes:

$$
8
$$

floats with AVX2:

```cpp id="qpohrx"
_mm256_add_ps(...)
```

and:

$$
16
$$

with AVX-512:

```cpp id="6vk2sf"
_mm512_add_ps(...)
```

There the programmer explicitly says:

$$
\boxed{\text{perform one vector operation across N lanes}.}
$$

On the GPU, you write something that appears scalar:

```metal id="2o38s4"
kernel void add(
    device const float* a,
    device const float* b,
    device float* out,
    uint id [[thread_position_in_grid]])
{
    out[id] =
        a[id] + b[id];
}
```

Hardware groups many such invocations.

So:

$$
\boxed{
\text{SIMD: vector lanes explicit in the programming model}
}
$$

while:

$$
\boxed{
\text{SIMT: scalar-looking invocations grouped by execution hardware}.
}
$$

This distinction explains divergence.

---

**Warp, wavefront and subgroup**

The hardware execution group has different names.

Conceptually:

$$
\boxed{
W=\{\text{lane}_0,\dots,\text{lane}_{S-1}\}.
}
$$

The width:

$$
S
$$

is hardware/platform dependent.

Do not write an engine assuming:

$$
S=32
$$

unless your target guarantees it.

In cross-platform shader terminology, **subgroup** is the safest conceptual name.

Metal calls corresponding operations `simdgroup` operations.

Vulkan has subgroup operations.

CUDA historically talks about warps.

AMD literature commonly talks about wavefronts.

---

**Maveb already exposes this hierarchy explicitly**

Your Gaussian shader takes:

```metal id="z8wxwy"
uint lane
    [[thread_index_in_simdgroup]],

uint simdGroup
    [[simdgroup_index_in_threadgroup]],

uint simdWidth
    [[threads_per_simdgroup]],

uint threadsPerGroup
    [[threads_per_threadgroup]]
```

That means the shader can distinguish:

$$
\text{global invocation},
$$

$$
\text{threadgroup-local invocation},
$$

$$
\text{subgroup},
$$

$$
\text{lane within subgroup}.
$$

That is the complete execution hierarchy M127 is trying to teach.

---

**Invocation IDs**

For a one-dimensional dispatch, global index is conceptually:

$$
\boxed{
i=
gG+l
}
$$

where:

$$
g=\text{workgroup index},
$$

$$
G=\text{threads per workgroup},
$$

$$
l=\text{local thread index}.
$$

GPU APIs usually provide this already as a global invocation ID.

In Metal:

```metal id="0x8pi8"
uint index
    [[thread_position_in_grid]]
```

is exactly that.

---

**KairoGPU's vector kernel is the canonical 1D mapping**

Current KairoGPU:

```metal id="v6nvzv"
kernel void kairo_vector_multiply(
    device const float* lhs,
    device const float* rhs,
    device float* output,
    constant uint& count,
    uint index [[thread_position_in_grid]])
{
    if (index < count)
        output[index] =
            lhs[index] * rhs[index];
}
```

The mapping is perfect:

$$
\boxed{
\text{one invocation}
\leftrightarrow
\text{one array element}.
}
$$

No invocation communicates with another.

This is the easiest class of GPU workload.

---

**Workgroups / threadgroups**

Invocations are organized into workgroups.

For one workgroup:

$$
T=
T_xT_yT_z.
$$

All invocations in the group can generally:

$$
\text{share threadgroup/shared memory}
$$

and:

$$
\text{synchronize at workgroup barriers}.
$$

Different workgroups must be considered independent during one dispatch.

There is no ordinary barrier inside a kernel that says:

> Wait until every workgroup on the entire GPU reaches this point.

That restriction drives many multi-dispatch algorithms.

---

**Why workgroups exist**

Suppose 256 threads need to collaboratively reduce 256 values.

If those threads belong to one workgroup, they can place values into fast threadgroup storage:

```metal id="et2xom"
threadgroup float scratch[256];
```

then synchronize:

```metal id="yx8mk8"
threadgroup_barrier(
    mem_flags::mem_threadgroup);
```

and collaboratively operate on the array.

Without workgroup locality, every intermediate exchange would need slower device memory plus more complicated synchronization.

---

**Dispatch grids**

A dispatch consists of many workgroups or a requested grid of threads.

For a 1D array of $N$ elements with workgroup width $G$, conventional APIs dispatch:

$$
\boxed{
N_g=
\left\lceil
\frac NG
\right\rceil
}
$$

groups.

For example:

$$
N=1000,
\qquad
G=256.
$$

Then:

$$
N_g=
\lceil3.90625\rceil
=
4.
$$

Total threads:

$$
4\times256=1024.
$$

Therefore 24 invocations are outside valid input range and must perform:

```cpp id="9wj2yb"
if (id >= count)
    return;
```

This is called **over-dispatch**.

---

**Metal's `dispatchThreads`**

Metal also provides a dispatch interface where the requested logical thread-grid size is explicit:

```objc id="d2ts06"
dispatchThreads(
    MTLSizeMake(count, 1, 1),
    threadsPerThreadgroup);
```

KairoGPU uses exactly this for vector addition and multiplication.

It still has:

```metal id="ccmw1t"
if (index < count)
```

which is harmless defensive correctness.

---

**Workgroup scheduling**

A workgroup is assigned to some compute unit.

Multiple workgroups may be resident on that unit.

Within them, the hardware schedules subgroups.

The essential scheduling object is usually the subgroup/warp/wave rather than an individual invocation.

Conceptually:

```text id="r4h73i"
cycle:
    choose ready warp
    issue instruction

later:
    warp stalls on memory
    choose another ready warp
```

This explains why GPU performance can remain excellent despite high individual memory latency.

---

**Latency hiding**

Imagine each subgroup performs:

```text id="6d38i1"
load
wait
multiply
store
```

If only one subgroup is resident, memory latency directly stalls the execution unit.

If 16 are resident:

```text id="bnfz56"
warp 0 → load, stalls
warp 1 → execute
warp 2 → execute
...
```

By the time the scheduler returns to warp 0, its load may be ready.

Thus:

$$
\boxed{
\text{latency hiding requires enough independent ready work}.
}
$$

This is one major role of occupancy.

---

**GPU occupancy**

Occupancy is roughly:

$$
\boxed{
O=
\frac{
\text{resident active subgroups}
}{
\text{hardware maximum resident subgroups}
}.
}
$$

A workgroup consumes finite per-compute-unit resources.

The number of simultaneously resident groups is constrained by the minimum of several limits:

$$
N_{\text{resident groups}}
\le
\min
\left(
N_{\text{thread limit}},
N_{\text{register limit}},
N_{\text{shared-memory limit}},
N_{\text{group limit}}
\right).
$$

Conceptually:

$$
N_{\text{reg}}
=
\left\lfloor
\frac{
R_{\text{CU}}
}{
R_{\text{thread}}
T_{\text{group}}
}
\right\rfloor.
$$

And:

$$
N_{\text{shared}}
=
\left\lfloor
\frac{
S_{\text{CU}}
}{
S_{\text{group}}
}
\right\rfloor.
$$

Whichever is smallest can become the occupancy bottleneck.

---

**Occupancy example**

Imagine a hypothetical compute unit with:

$$
65536
$$

register units available.

A kernel uses:

$$
R_t=64
$$

registers/thread.

Workgroup:

$$
T=256.
$$

Registers/group:

$$
64\times256=16384.
$$

Register limit allows:

$$
65536/16384=4
$$

resident groups.

If the compiler increases register usage to:

$$
128
$$

per thread:

$$
128\times256=32768,
$$

so only:

$$
2
$$

groups fit.

Without changing one line of dispatch code, occupancy has potentially halved.

---

**Occupancy is not synonymous with performance**

A kernel at:

$$
100\%
$$

occupancy is not automatically faster than one at:

$$
50\%.
$$

Suppose lowering register usage to increase occupancy forces values to spill into memory.

You gained resident warps but added expensive memory traffic.

Or a compute-heavy kernel may already have enough instruction-level parallelism to hide latency with fewer warps.

Thus:

$$
\boxed{
\text{occupancy is a constraint/diagnostic, not a score to maximize blindly}.
}
$$

M132 will return to this tradeoff in even more detail.

---

**Registers**

Every invocation has logical private variables:

```metal id="t9jsa8"
float sum;
float3 position;
float4 color;
```

The compiler tries to place these in registers.

Registers are extremely fast.

But the physical register file is finite.

That means a deceptively innocent shader variable has a system-level cost:

$$
\boxed{
\text{registers/thread}
\times
\text{resident threads}
\le
\text{register-file capacity}.
}
$$

---

**Register pressure**

Register pressure increases from long-lived values, large unrolled loops, large temporary expressions, complex material/path states and local arrays whose indexing cannot be optimized well.

Consider:

```cpp id="g2gruk"
float a0, a1, a2, a3;
float b0, b1, b2, b3;
float c0, c1, c2, c3;
```

If all remain live simultaneously, compiler must retain them.

If some can be recomputed or have shorter live ranges, registers can be released/reused.

Shader optimization is therefore partly **lifetime engineering**.

---

**Register spilling**

When register demand exceeds what the compiler/hardware can comfortably allocate, some values may spill into a thread-private address space that is physically backed by lower-level memory/cache.

Instead of:

```text id="i9imdl"
register → ALU
```

you can get:

```text id="x9gdnc"
load spill
ALU
store spill
```

This is enormously slower.

Hence a kernel can become slower when you “optimize” it by aggressively unrolling everything.

---

**Shared / threadgroup memory**

Threadgroup memory is fast on-chip storage shared by all invocations of one workgroup.

KairoGPU's tiled matrix multiplication uses:

```metal id="mmlugb"
threadgroup float lhs_tile[16][16];
threadgroup float rhs_tile[16][16];
```

That is the canonical shared-memory use case.

Why?

Without tiling, computing:

$$
C_{ij}
=
\sum_{k=0}^{K-1}
A_{ik}B_{kj}
$$

requires every output thread to repeatedly fetch rows and columns from device memory.

With a:

$$
16\times16
$$

tile, each workgroup cooperatively loads chunks of $A$ and $B$ once, then reuses those values many times.

---

**KairoGPU matrix multiplication**

Current kernel:

```metal id="tun1bn"
const uint row =
    group.y * 16 + local.y;

const uint column =
    group.x * 16 + local.x;

float sum = 0.0f;

for (uint base = 0;
     base < K;
     base += 16)
{
    lhs_tile[local.y][local.x] =
        valid
        ? lhs[row * K +
              base + local.x]
        : 0.0f;

    rhs_tile[local.y][local.x] =
        valid
        ? rhs[
            (base + local.y) *
            columns +
            column]
        : 0.0f;

    threadgroup_barrier(
        mem_flags::mem_threadgroup);

    for (uint offset = 0;
         offset < 16;
         ++offset)
    {
        sum +=
            lhs_tile[local.y][offset] *
            rhs_tile[offset][local.x];
    }

    threadgroup_barrier(
        mem_flags::mem_threadgroup);
}
```

Each loaded tile element is reused by many threads.

That increases arithmetic intensity dramatically.

---

**Why there are two barriers**

The first:

```metal id="lzwp5h"
threadgroup_barrier(...)
```

guarantees every thread has finished loading its tile entries before any thread reads the shared tile.

The second guarantees no thread begins overwriting the shared arrays for the next $K$-tile while another is still using the current one.

Without either barrier, you have a race.

---

**Shared-memory bank conflicts**

Shared/threadgroup memory is physically partitioned into banks so multiple lanes can access it simultaneously.

Conceptually:

$$
\text{bank}(address)
=
(address/\text{wordSize})\bmod B
$$

for some hardware-dependent bank count $B$.

If many lanes access different addresses mapping to the same bank, accesses can serialize.

That is a **bank conflict**.

If lanes access different banks, accesses can proceed in parallel.

---

**Why transpose-like accesses can create bank conflicts**

Imagine a 2D shared array:

```cpp id="32aupj"
float tile[32][32];
```

and each lane reads:

```cpp id="dov08i"
tile[lane][constantColumn]
```

depending on bank mapping, stride:

$$
32\times4\text{ bytes}
$$

may align repeatedly to the same bank.

A classic mitigation is padding:

```cpp id="ucsz3u"
float tile[32][33];
```

The extra column changes row stride and therefore bank mapping.

Do not blindly apply CPU cache-line intuition here; shared-memory banks are a distinct hardware organization.

---

<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/14 - Transparency, Blending and Visibility|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/16 - GPU Memory, Divergence and Execution Order|Next chapter]]
