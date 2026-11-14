# 40 — Lighting, Reconstruction and Surface Motion

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/39 - Tiling, Ordering and Compositing at Scale|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/41 - Per-Vertex and Per-Corner Mesh Attributes|Next chapter]]

Rendering captured kernels still requires lighting, reconstruction and temporal surface behavior. This span considers those extensions and how motion changes the representation over time.

**By the end:** Connect lighting and reconstruction choices to animated surface representations.

**Recall question:** What additional information is needed when a captured surface changes over time?

<!-- Derived from Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography.md; source lines 91349–94605; source-span SHA256 a55a6beecf9a393c115d469a2d7f91cc09f84521eeced8863d8c7b2727ef8229. -->

<!-- CHAPTER-SOURCE-BEGIN -->
## Illuminate, reconstruct and animate the surface

**Coordinate-space clarification:** glTF morph targets precede skinning and node transforms. A global-joint skin palette already produces world-space positions; a mesh-local palette needs the matching inverse mesh transform. Bind-pose identity requires the declared bind-shape/output-space convention, not merely the presence of inverse-bind matrices.

We are doing both in one continuous pass, with no curriculum boundary drift.

**Pass 17 = M151–M156, Parts 2501–2600**, covering advanced real-time rendering: many-light indexing, clustered lighting, adaptive work, Hi-Z, shadows, AO, GI/probes/SSR/radiance cascades, TAA/post, particles/decals and performance-quality selection.

**Pass 18 = M157–M162, Parts 2601–2700**, covering indexed meshes, topology, half-edges, normals/tangents, OBJ/glTF, simplification/subdivision/smoothing, skeletons, skinning, animation, morphing and complete animated-vertex ingestion.

---

### ADVANCED REAL-TIME RENDERING
### M156

---

### MANY-LIGHT RENDERING AND CLUSTERED LIGHTING

#### Reconstruction checkpoint — make the cluster coordinate measurable

The new `tools/labs/realtime_rendering_reference_lab.cpp` turns the depth-slice equation into an executable contract. It checks near/far clamping, monotonic slice assignment over a logarithmic depth sweep, and the fact that equal depth decades occupy equal widths in log space. This proves the z partition only; full light-volume/cluster overlap, GPU count/scan/fill and overflow handling remain later implementation evidence. See [[Clustered lighting depth slicing]].


The central problem of many-light rendering is not the BRDF anymore. We already know how to evaluate one light:

$$
L_o \mathrel{+}=
f_r(\omega_i,\omega_o)
L_i
(n\cdot\omega_i).
$$

The problem is finding:

$$
\boxed{
\text{which lights are actually relevant to this shading point?}
}
$$

If the scene has:

$$
N_L=10,000
$$

lights and:

$$
N_P=2,000,000
$$

pixels, naïvely evaluating all lights gives:

$$
2\times10^{10}
$$

light/pixel tests.

That is why Part 2501 describes many-light rendering as a **visibility and indexing problem**.

---

### Forward rendering

Traditional forward shading:

```cpp
for every object:
    bind material

    for every pixel produced:
        for every light:
            shade
```

Conceptually:

$$
Cost
\sim
Pixels\times Lights.
$$

Forward rendering has several strengths. Material data is naturally available while the object is drawn, transparency works naturally, MSAA is straightforward, and the geometry need not be encoded into an intermediate G-buffer.

Its many-light weakness is that the fragment shader does not inherently know a small relevant subset of lights.

You can CPU-cull lights per object, but a large object spanning the screen may still intersect hundreds of lights that affect different regions.

---

### Deferred rendering

Deferred rendering separates geometry from lighting.

First pass writes a G-buffer, perhaps:

$$
G=
\{
normal,
baseColor,
roughness,
metallic,
depth
\}.
$$

Then lighting is evaluated later using screen-space data.

A representative layout might be:

```cpp
struct GBuffer
{
    RGBA8_SRGB   BaseColorMetallic;
    RGBA16F      NormalRoughness;
    R32F         Depth;
};
```

Actual formats vary.

The benefit is that lighting occurs only for visible pixels and geometry cost does not multiply per light.

But G-buffer bandwidth can be large.

At 1920×1080, suppose:

$$
32\text{ bytes/pixel}.
$$

One full write:

$$
1920\times1080\times32
\approx66.4\text{ MB}.
$$

At 60 FPS, ignoring reads and compression:

$$
\approx4\text{ GB/s}.
$$

So deferred shading trades some repeated shading work for memory bandwidth and storage.

Transparency also generally needs a separate forward path.

---

### Tiled lighting

Divide screen into tiles:

$$
16\times16
$$

or:

$$
32\times32.
$$

For each tile, determine:

$$
L_t=
\{\text{lights overlapping tile}\}.
$$

Then every pixel in that tile loops only:

$$
L_t
$$

rather than every light.

This changes:

$$
O(PN_L)
$$

toward something closer to:

$$
O(
N_L\cdot tilesTouched
+
P\cdot \bar L_{tile}
).
$$

But tiles only partition x/y.

A light close to the camera and another 500 meters away can occupy the same screen tile.

That motivates clustered lighting.

---

### Clustered lighting

A cluster adds depth.

Instead of:

$$
(x,y)
$$

we partition:

$$
(x,y,z).
$$

A cluster is a sub-frustum.

Think:

```text
screen
┌─────────────────────────────┐
│ x/y tile grid               │
└─────────────────────────────┘
            ×
depth slices
```

Cluster index:

$$
\boxed{
C(x,y,z)
=
(zN_y+y)N_x+x.
}
$$

Now a pixel can find:

$$
clusterID
$$

and fetch only lights affecting that 3D region.

---

### Screen-tile construction

With tile dimensions:

$$
T_x,T_y
$$

and framebuffer:

$$
W,H,
$$

tile counts are:

$$
N_x=
\left\lceil\frac{W}{T_x}\right\rceil,
$$

$$
N_y=
\left\lceil\frac{H}{T_y}\right\rceil.
$$

Pixel:

$$
(x_p,y_p)
$$

maps to:

$$
t_x=
\lfloor x_p/T_x\rfloor,
$$

$$
t_y=
\lfloor y_p/T_y\rfloor.
$$

Exactly the same spatial partitioning principle we just used for Gaussian rasterization.

The difference is that now we're assigning **lights** rather than splats.

---

### Logarithmic depth slicing

Perspective distributes perceptual/geometric importance nonlinearly with depth.

A logarithmic partition is:

$$
\boxed{
z_i
=
n
\left(
\frac fn
\right)^{i/N_z}
}
$$

for:

$$
i=0,\dots,N_z.
$$

Given depth $z$, solve for slice.

Start:

$$
z=
n(f/n)^{i/N_z}.
$$

Divide:

$$
z/n=
(f/n)^{i/N_z}.
$$

Log:

$$
\ln(z/n)
=
\frac i{N_z}
\ln(f/n).
$$

Therefore:

$$
\boxed{
i
=
N_z
\frac{
\ln(z/n)
}{
\ln(f/n)
}.
}
$$

Cluster index:

$$
slice=
clamp(
\lfloor i\rfloor,
0,
N_z-1
).
$$

Current Maveb's `Lighting.cpp` implements precisely this normalized logarithmic mapping.

---

### Why log depth is good

Suppose:

$$
n=0.1,
f=1000.
$$

A linear 16-slice scheme gives:

$$
\sim62.5m
$$

per slice.

The entire region from:

$$
0.1m
$$

to:

$$
62.6m
$$

becomes one slice.

Terrible for nearby lighting.

Log slicing gives much finer near-camera partitioning and increasingly large far slices.

This aligns much better with perspective projection.

---

### Linear slicing

Linear slices are:

$$
\boxed{
z_i=
n+
(f-n)\frac iN.
}
$$

Slice:

$$
\boxed{
i=
\left\lfloor
N
\frac{z-n}{f-n}
\right\rfloor.
}
$$

Advantages are simplicity and predictable physical size.

Disadvantage is poor depth distribution when:

$$
f/n
$$

is huge.

Linear slicing can still make sense in bounded scenes or orthographic cameras.

---

### Frustum-cluster bounds

A cluster is not really an axis-aligned world-space box.

It is the volume between four screen-space tile rays and two depth planes.

For one tile, construct NDC corners:

$$
(x_0,y_0),(x_1,y_1).
$$

At near depth $z_a$ and far depth $z_b$, back-project:

$$
p=
P^{-1}
\begin{bmatrix}
x_{ndc}\\
y_{ndc}\\
z_{ndc}\\
1
\end{bmatrix}
$$

then divide by homogeneous w.

You obtain eight view-space points defining a frustum cell.

For exact intersection tests you can derive its six planes.

For faster conservative tests, you may approximate cluster with:

$$
AABB
$$

or bounding sphere in view space.

---

### Light bounding volumes

Directional lights:

$$
\boxed{\text{affect all clusters}}
$$

unless further masked.

Point light naturally has a sphere:

$$
\|p-L\|\le r.
$$

Spot light naturally has a cone.

Area lights may use bounding spheres/AABBs or custom conservative volumes.

Bounding volumes must be conservative:

$$
\boxed{
false\ positive
\Rightarrow
extra\ shading
}
$$

but:

$$
\boxed{
false\ negative
\Rightarrow
missing\ light.
}
$$

Correct renderer prefers conservative false positives.

---

### Point-light sphere vs cluster

If cluster is approximated by AABB:

$$
[min,max],
$$

find closest point on box to sphere center:

$$
q=
clamp(
L,
min,
max
).
$$

Then:

$$
d^2=
\|L-q\|^2.
$$

Sphere intersects if:

$$
\boxed{
d^2\le r^2.
}
$$

Very cheap.

Exact frustum-plane sphere test is possible:

For each cluster plane:

$$
n_i\cdot L+d_i.
$$

If sphere lies completely outside any plane:

$$
n_i\cdot L+d_i<-r
$$

then reject.

---

### Spot cone vs cluster

Spotlight cone defined by apex $A$, axis $\hat d$, range $R$, half-angle $\theta$.

A point $p$ is inside infinite cone when:

$$
t=
(p-A)\cdot\hat d>0
$$

and radial distance:

$$
r_\perp
\le
t\tan\theta.
$$

Exact cone-frustum intersection is more expensive.

Typical clustered renderers use conservative approximations such as:

$$
\text{cone bounding sphere}
$$

or cone-vs-plane tests.

Important repository detail: current Maveb validates spotlight cone parameters, but its inspected CPU cluster assignment uses the light's **range sphere/projected range bound** rather than a dedicated exact cone-vs-cluster refinement. That is conservative and correct, but it admits extra clusters for spots.

---


A naïve dense representation:

```cpp
LightIndex clusterLights[
    ClusterCount
][MaxLightsPerCluster];
```

Memory:

$$
M=
C
L_{max}
sizeof(index).
$$

Suppose:

$$
C=120\times68\times24
=
195,840
$$

clusters and:

$$
L_{max}=128.
$$

With 4-byte indices:

$$
M\approx100\text{ MB}.
$$

Mostly wasted if average cluster contains 8 lights.

---

### Compact representation

Use:

```cpp
struct ClusterRange
{
    uint32_t Offset;
    uint32_t Count;
};

std::vector<uint32_t>
    LightIndices;
```

For cluster $c$:

$$
indices[
offset_c:
offset_c+count_c
].
$$

Storage:

$$
O(C+R)
$$

where:

$$
R=
\text{total light references}.
$$

This is CSR again.

Exactly the same storage pattern as Gaussian tile lists.

Current Maveb's `ClusteredLightLists` ends with:

```text
clusters:
    offset,count

lightIndices:
    flat list
```

so the final representation is already CSR-like.

---

### GPU construction

If building on GPU:

First count:

$$
count[c].
$$

Then exclusive scan:

$$
offset[c]
=
\sum_{j<c}count[j].
$$

Then second pass scatters light indices into:

$$
flatIndices[
offset[c]+localRank
].
$$

Again:

$$
\boxed{
count
\rightarrow
scan
\rightarrow
scatter.
}
$$

This should now feel like a universal GPU pattern rather than a one-off algorithm.

---

### Current Maveb construction

Current inspected `Lighting.cpp` is CPU-side and simpler:

```cpp
vector<vector<uint32_t>>
    temporary(clusterCount);
```

It conservatively appends each light to every overlapped cluster and then flattens the vectors into:

$$
(offset,count)
+
flatIndices.
$$

That is perfectly acceptable as a reference/CPU builder.

A future GPU implementation should replace nested dynamic vectors with count/scan/scatter.

---

### Overflow handling

Never allow light assignment to silently overflow.

If:

$$
references>budget,
$$

options include:

Reject frame/configuration.

Clamp and report.

Drop lowest-priority lights.

Increase buffer next frame.

Use fallback global-light list.

Current Maveb uses a configured:

$$
maximumLightReferences
$$

and fails with:

```text
resourceExhausted
```

if exceeded.

That is conservative correctness-first behavior.

---

### Memory budgeting

You should budget:

$$
M_{cluster}
=
C\times sizeof(range)
+
R\times sizeof(index).
$$

For:

$$
C=200k,
$$

8-byte range:

$$
1.6MB.
$$

For:

$$
R=2M
$$

references:

$$
8MB.
$$

Total:

$$
\sim9.6MB.
$$

Reasonable.

But worst-case:

Every local light touches every cluster:

$$
R=C\times L.
$$

That explodes.

Hence robust renderer must define explicit:

$$
\boxed{
cluster budget,
reference budget,
overflow telemetry.
}
$$

---

### Debug visualization

A clustered renderer should visualize cluster occupancy.

For pixel cluster:

$$
c,
$$

map:

$$
count_c
$$

to heat.

For example:

$$
h=
clamp(
count_c/K,
0,1
).
$$

Then display low occupancy dark, high occupancy bright.

Useful views include cluster ID, z-slice, light count, overflow clusters and selected light volumes.

If one region suddenly has 300 lights/cluster, performance mystery becomes obvious.

---

### ADAPTIVE WORK, HYSTERESIS AND HI-Z

The unifying idea is:

$$
\boxed{
\text{not every possible rendering calculation deserves equal work every frame}.
}
$$

Suppose a renderer has 11 ms budget.

You cannot simply enable every expensive feature at maximum quality.

You need an allocation rule.

---

### Adaptive work

For task $i$, estimate:

$$
E_i=
\text{expected visual error if skipped/reduced}
$$

and:

$$
C_i=
\text{cost}.
$$

A simple benefit score:

$$
\boxed{
S_i=
\frac{E_i}{C_i}.
}
$$

Then execute highest-value work until budget exhausted.

This resembles knapsack optimization.

Examples:

A shadow cascade covering center screen might have high benefit.

Updating a tiny reflection probe behind camera may have low benefit.

In foveated XR, center-of-gaze work gets much higher benefit.

---

### Error metrics

Possible lighting error proxy:

$$
E_{cluster}
\sim
\sum_{l\in omitted}
I_l
A_l
$$

where:

$$
I_l=\text{estimated intensity},
$$

$$
A_l=\text{screen relevance}.
$$

Or use gradient/sensitivity:

$$
E_i
\approx
\left|
\frac{\partial Q}{\partial x_i}
\right|
\Delta x_i.
$$

This is conceptually close to Vulkax's current adaptive-material-influence research code, which ranks regions by absolute adjoint gradient contribution and selects enough work to cover a target fraction of total sensitivity.

That code is not itself a renderer controller, but the allocation mathematics is directly analogous.

---

### Work budget

Suppose tasks sorted by score:

$$
S_1\ge S_2\ge\cdots.
$$

Choose largest $k$ such that:

$$
\boxed{
\sum_{i=1}^{k}C_i
\le B.
}
$$

Budget may be measured as:

$$
GPU\ ms,
rays,
light\ evaluations,
shadow\ texels,
particles,
memory.
$$

For FoveaBench-style work, a very useful unit is:

$$
\boxed{
\text{GPU time under fixed image-quality target}.
}
$$

---

### Online controller

Target frame time:

$$
T^*.
$$

Measured:

$$
T_t.
$$

Error:

$$
e_t=
T^*-T_t.
$$

If frame too slow:

$$
e_t<0.
$$

A proportional controller:

$$
B_{t+1}
=
B_t
+
K_p e_t.
$$

A PI controller:

$$
\boxed{
B_{t+1}
=
B_t
+
K_p e_t
+
K_i\sum_{\tau\le t}e_\tau.
}
$$

Clamp:

$$
B_{min}\le B\le B_{max}.
$$

Do not react too aggressively or quality oscillates.

---

### Temporal hysteresis

Suppose expensive feature activates when importance:

$$
S>0.5.
$$

If signal fluctuates:

$$
0.49,0.51,0.49,0.51
$$

feature toggles every frame.

Visible flicker and scheduling instability.

Instead use separate thresholds:

$$
\boxed{
T_{enter}>T_{exit}.
}
$$

If inactive:

activate only when:

$$
S>T_{enter}.
$$

If active:

remain active until:

$$
S<T_{exit}.
$$

Example:

$$
T_{enter}=0.6,
\quad
T_{exit}=0.4.
$$

That dead band is hysteresis.

You can also require persistence:

$$
N_{enter}
$$

consecutive qualifying frames.

---

### Hi-Z

A depth pyramid stores progressively coarser depth information.

Level 0:

$$
W\times H.
$$

Level 1:

$$
W/2\times H/2.
$$

Each texel summarizes a:

$$
2\times2
$$

region.

Continue until:

$$
1\times1.
$$

---

### Min-depth vs max-depth pyramid

Which reduction you need depends on depth convention and query.

For conventional depth:

$$
near=0,\quad far=1.
$$

If asking:

> Is an object's nearest depth behind **every** occluder across its screen rectangle?

you need the **farthest occluder** in that region:

$$
\boxed{
max(depth)
}
$$

for conventional depth.

If even the farthest occluder is nearer than object:

$$
maxDepth_{occluders}
<
z_{objectNear},
$$

then every pixel is occluded.

For reversed-Z:

$$
near=1,\quad far=0,
$$

the analogous conservative farthest-depth summary is:

$$
\boxed{
min(depth).
}
$$

So never copy “Hi-Z uses max” without knowing the depth convention and query semantics.

---

**2×2 reduction**

For conventional max pyramid:

$$
D_{l+1}(x,y)
=
\max
\begin{cases}
D_l(2x,2y),\\
D_l(2x+1,2y),\\
D_l(2x,2y+1),\\
D_l(2x+1,2y+1)
\end{cases}.
$$

For reversed-Z conservative farthest:

replace max with min.

This is a perfect compute-shader workload.

---

### Screen-space bounding rectangle

Project object's conservative bounding volume.

AABB corners or bounding sphere give:

$$
[x_0,x_1]
\times
[y_0,y_1].
$$

Choose mip roughly matching rectangle size.

If rectangle spans:

$$
w\times h
$$

pixels, choose:

$$
mip\approx
\lfloor\log_2(\max(w,h))\rfloor.
$$

Sample enough pyramid texels conservatively.

Then compare object's near depth against stored aggregate.

---

### Occlusion false positives and false negatives

Conservative occlusion culling must prefer:

$$
\boxed{
\text{false visible}
}
$$

rather than:

$$
\boxed{
\text{false occluded}.
}
$$

False visible wastes draw.

False occluded creates holes.

Temporal occlusion systems often also delay culling by a frame or use history to avoid flicker around boundaries.

---

### SHADOWS AND AMBIENT OCCLUSION

---

### Shadow mapping

A shadow map asks:

> From the light's viewpoint, what is the nearest visible depth?

First render scene from light into depth texture.

For world point $p_w$:

$$
p_l=
P_LV_Lp_w.
$$

Perspective divide:

$$
p_{ndc}
=
p_l/p_l.w.
$$

Convert x/y to shadow texture UV.

Compare current light-space depth:

$$
z_{current}
$$

with stored:

$$
z_{map}.
$$

Conventional depth test:

$$
\boxed{
z_{current}
>
z_{map}+bias
\Rightarrow
shadow.
}
$$

Exact sign changes with reversed-Z conventions.

---

### Shadow acne

Suppose receiver surface generated shadow map.

Due to finite raster precision and sample mismatch, shading point depth may be slightly behind its own stored depth.

Then self-shadowing stripes appear.

This is shadow acne.

---

### Constant bias

Offset comparison:

$$
z_{current}
\leftarrow
z_{current}-b
$$

or equivalently adjust stored threshold depending convention.

Too small:

acne.

Too large:

shadow disconnects from caster.

---

### Slope-scaled bias

Problem grows when surface is grazing relative to light.

Bias can depend on angle:

$$
\boxed{
b=
b_c+
b_s\tan\theta
}
$$

where $\theta$ is surface/light slope.

Raster APIs often implement slope-scaled depth bias directly using polygon depth slope.

---

### Peter-panning

Excessive bias moves shadow away from caster.

Object appears to float:

```text
caster
  █

    shadow
```

That is Peter-panning.

Bias tuning is always compromise:

$$
acne
\leftrightarrow
peter\ panning.
$$

Normal-offset shadowing is another technique: offset receiver along normal.

---

### PCF

Instead of one binary comparison:

$$
V(u,v)\in\{0,1\},
$$

sample a neighborhood:

$$
\boxed{
V_{PCF}
=
\frac1N
\sum_{i=1}^{N}
Compare(
z,
D(u+\Delta u_i,v+\Delta v_i)
).
}
$$

This produces soft transitions.

It is filtering **comparison results**, not averaging depths and comparing once.

Important distinction.

Hardware comparison samplers can implement this efficiently.

---

### Cascaded shadow maps

Directional light with huge view range cannot maintain resolution everywhere.

Split camera frustum into cascades.

Near cascades cover small physical region with high texel density.

Far cascades cover large region.

Current Maveb supports up to:

$$
8
$$

cascades.

---

### Cascade split formula

Uniform split:

$$
u_i=
n+(f-n)i/N.
$$

Log split:

$$
l_i=
n(f/n)^{i/N}.
$$

Practical split:

$$
\boxed{
s_i=
\lambda l_i+
(1-\lambda)u_i.
}
$$

Current Maveb implements exactly this with:

```text
splitLambda
```

clamped:

$$
0\le\lambda\le1.
$$

---

### Stabilization

If cascade projection changes continuously with camera movement, shadow texel grid swims.

Snap light-space cascade center to texel increments.

If cascade width:

$$
W_c
$$

and resolution:

$$
R,
$$

texel world size:

$$
t=
W_c/R.
$$

Snap:

$$
\boxed{
x_c'
=
round(x_c/t)t.
}
$$

Same y.

Maveb currently performs this center quantization.

It also quantizes its cascade radius conservatively before constructing light-space projection.

That is classic cascade stabilization.

---

### Shadow atlas

Instead of one texture per light/cascade, pack shadow maps into one atlas.

Each allocation gives:

$$
rect=
(x,y,w,h).
$$

Shading transforms local shadow UV:

$$
uv_{local}
$$

to atlas:

$$
uv_{atlas}
=
offset+
scale\cdot uv_{local}.
$$

Benefits:

fewer resource bindings.

Problems:

allocation fragmentation, padding, filtering bleed, differing resolutions.

---

### Point-light cubemap shadows

Point light emits all directions.

Render six faces:

$$
\pm X,\pm Y,\pm Z.
$$

For shaded point:

$$
d=p-L.
$$

Direction selects cubemap face.

Stored value often radial distance:

$$
\|d\|.
$$

Compare with current radial distance.

Cost:

$$
6
$$

shadow renders per point light.

Expensive.

Dual-paraboloid or octahedral shadow maps exist, but cubemap is straightforward.

---

### VSM

Variance shadow maps store moments:

$$
m_1=E[z],
$$

$$
m_2=E[z^2].
$$

Variance:

$$
\sigma^2=
m_2-m_1^2.
$$

For receiver depth $t>m_1$, Chebyshev bound:

$$
\boxed{
P(z\ge t)
\le
\frac{
\sigma^2
}{
\sigma^2+(t-m_1)^2
}.
}
$$

This gives filterable soft visibility.

Problem:

$$
\boxed{\text{light bleeding}}
$$

because probability bound can be loose.

EVSM transforms depth exponentially to improve separation, with its own precision issues.

---

### Contact shadows

Shadow map resolution often misses tiny near-contact occlusion.

Screen-space contact shadows ray march a short segment from shading point toward light using scene depth.

Good for:

feet on floor, object contact, small creases.

Limitations:

screen-space only, missing off-screen occluders, finite depth thickness.

Use as supplement, not full shadow replacement.

---

### Ambient occlusion

AO estimates how blocked local hemisphere is.

Conceptually:

$$
\boxed{
AO(p)
=
\frac1\pi
\int_{\Omega^+}
V(p,\omega)
(n\cdot\omega)
d\omega.
}
$$

If fully open:

$$
AO\approx1.
$$

Highly occluded:

$$
AO\rightarrow0.
$$

It is not global illumination.

It is an approximation to local visibility, generally used to attenuate indirect diffuse.

---

### SSAO

Reconstruct view-space position from depth.

Sample neighboring offsets.

Compare whether neighbors occlude hemisphere around point.

A simple sample:

$$
p_i=
p+
TBN\cdot k_i\cdot radius.
$$

Project $p_i$ to screen.

Fetch depth.

If scene geometry lies in front of sample position by appropriate threshold, count occlusion.

---

### Sample kernels

Generate hemisphere samples biased near origin.

Example:

$$
k_i=
normalize(randomHemisphere)
\cdot r_i.
$$

Use nonlinear radius:

$$
r_i=
lerp(
0.1,
1,
(i/N)^2
).
$$

More samples near center.

---

### Noise rotation

If every pixel uses same kernel, visible repeated pattern appears.

Use small tiled noise texture containing random tangent-plane rotations.

Construct TBN:

$$
T=
normalize(
r-n(n\cdot r)
),
$$

$$
B=n\times T.
$$

Rotate kernel around normal per pixel.

Then blur AO.

Noise becomes high-frequency and easier to filter.

---

### HBAO

Horizon-based AO considers horizon angle along screen-space directions.

For direction $\phi$, find maximum elevation angle of surrounding geometry:

$$
h(\phi).
$$

Occlusion is derived from angular horizon relative to normal.

This tends to produce more geometrically grounded AO than simple random depth tests.

Modern techniques like GTAO build further on horizon/integral reasoning.

---

### GI, PROBES, SSR AND RADIANCE CASCADES

Global illumination means light paths involving indirect bounces.

Direct:

$$
camera
\leftarrow
surface
\leftarrow
light.
$$

Indirect:

$$
camera
\leftarrow
surface_A
\leftarrow
surface_B
\leftarrow
light.
$$

Rendering equation:

$$
L_o=
L_e+
\int
f_rL_i(n\cdot\omega_i)d\omega_i.
$$

$L_i$ itself may contain light bounced from many previous surfaces.

That recursive dependency is GI.

---

### Irradiance probes

Irradiance probe stores:

$$
E(p,n)
$$

or low-frequency directional representation near position $p$.

For diffuse surfaces:

$$
L_{diffuse}
=
\frac{\rho}{\pi}
E.
$$

Since diffuse irradiance is low frequency, spherical harmonics are common.

A probe might store 9 SH coefficients per color for degree 2.

---

### Radiance probes

Radiance depends on direction:

$$
L(p,\omega).
$$

A radiance probe stores more directional information than a scalar irradiance probe.

Useful for glossy response or propagation.

Can use:

cubemaps, octahedral maps, SH, SGs, learned representations.

---

### Probe interpolation

For probes $p_i$ around shading point $x$:

$$
w_i=
\frac{
\tilde w_i
}{
\sum_j\tilde w_j
}.
$$

For inverse-distance:

$$
\tilde w_i=
\frac1{
(\|x-p_i\|+\epsilon)^k
}.
$$

Then:

$$
\boxed{
E(x)
\approx
\sum_iw_iE_i.
}
$$

Grid probes often use trilinear interpolation.

---

### Probe leakage

Suppose wall separates bright room and dark room.

Probe on bright side may be spatially close to point on dark side.

Pure distance interpolation leaks light through wall.

This is one of the major problems of probe GI.

---

### Visibility-aware probes

Weight can include visibility:

$$
w_i
\propto
distanceWeight
\times
visibility(x,p_i).
$$

Visibility may use:

depth moments, ray tests, signed distance, probe relocation/classification, geometry normals.

DDGI-style systems maintain visibility information to reduce leakage.

---

### Reflection probes

Reflection probe stores environment radiance, often cubemap.

Reflection direction:

$$
R=
reflect(-V,N).
$$

Sample:

$$
L_{env}(R).
$$

For rough surfaces, use prefiltered mip level:

$$
LOD
\sim
roughness.
$$

Probe parallax is a problem because cubemap assumes rays originate at probe center.

Box projection/parallax correction partially fixes this for interiors.

---

### SSR

Screen-space reflection begins with view-space point $P$, normal $N$, view vector $V$.

Reflection direction:

$$
R=
reflect(V,N)
$$

according to convention.

Ray:

$$
P(t)=P+tR.
$$

Project successive points into screen.

Compare ray depth against depth buffer.

When ray crosses scene depth, infer intersection.

---

### Ray marching

Simple fixed step:

$$
t_{k+1}=t_k+\Delta.
$$

Better hierarchical march uses Hi-Z to skip empty regions.

Binary refinement around crossing improves hit precision.

---

### Thickness problem

Depth buffer gives one front surface.

Suppose ray projected depth:

$$
z_r
$$

and scene depth:

$$
z_s.
$$

If:

$$
z_r>z_s
$$

you may think ray passed behind geometry.

But is geometry infinitesimally thin?

Usually assign thickness:

$$
0<
z_r-z_s
<
\tau
$$

for hit.

Too small:

misses.

Too large:

false hits/reflecting through objects.

---

### Missing data

SSR fundamentally cannot reflect:

off-screen objects, back-facing surfaces not in depth, hidden geometry, geometry behind visible front layer.

Therefore:

$$
\boxed{
SSR
=
\text{high-quality local screen-space supplement}
}
$$

not complete reflection solution.

Fallback can be reflection probe, environment map or ray tracing.

---

### Radiance cascades

The core motivation is the **spatial-angular resolution tradeoff**.

Nearby radiance details require high spatial resolution.

Distant radiance changes slowly across position but requires more angular resolution to represent directional light structure.

Radiance-cascade-style thinking therefore uses approximately:

$$
\boxed{
near:
high\ spatial,\ lower\ angular
}
$$

and:

$$
\boxed{
far:
lower\ spatial,\ higher\ angular.
}
$$

Successive cascades represent increasingly distant radiance intervals with different sampling density.

This avoids storing full:

$$
L(x,\omega)
$$

at equally high spatial and angular resolution everywhere, which would be enormous.

The same idea should feel familiar from:

mipmaps, clipmaps, cascaded shadows, hierarchical grids.

The resolution allocation follows signal structure.

---

### TEMPORAL RENDERING AND POST

#### Reconstruction checkpoint — history is a hypothesis with rejection tests

The same real-time witness declares one motion-vector sign convention and proves that current UV plus motion reaches the previous UV. Its history predicate rejects out-of-bounds reprojection, history-generation changes such as camera cuts, object-identity mismatch, scale-aware depth disagreement and normal disagreement; invalid history contributes no temporal weight. The lab also evaluates a hand-checkable 3×3 PCF visibility fraction and a variance-shadow upper bound that decreases as the receiver moves farther behind the stored mean.

This is not full TAA or full shadow rendering. Jitter generation, neighborhood/variance clipping, reactive masks, cascade selection, SSR/SSAO, volumetrics, particles and GPU execution remain distinct. [[Temporal history validation and reprojection]] and [[Shadow filtering PCF and variance bounds]] turn those boundaries into explicit next tests rather than hidden omissions.


TAA is not simply:

> average current and previous image.

Correct temporal rendering requires motion correspondence, validity tests and anti-ghosting.

---

### Motion vectors

For current clip position:

$$
c_t=
P_tV_tM_tp.
$$

Previous:

$$
c_{t-1}
=
P_{t-1}
V_{t-1}
M_{t-1}
p.
$$

NDC:

$$
n_t=
c_t.xy/c_t.w,
$$

$$
n_{t-1}
=
c_{t-1}.xy/c_{t-1}.w.
$$

Motion:

$$
\boxed{
v=
n_t-n_{t-1}.
}
$$

Convert to UV scale if needed:

$$
v_{uv}=0.5v_{ndc}
$$

with y sign depending convention.

---

### Previous-position vertex data

For deforming mesh you cannot reconstruct previous position using only previous camera.

You need:

$$
p_{t-1}.
$$

For rigid object:

$$
M_{t-1}p.
$$

For skinned object:

$$
\sum_j
w_j
M_{j,t-1}p.
$$

For morph:

previous morph weights.

For procedural animation:

previous procedural state.

This is why motion-vector correctness propagates deep into animation systems.

---

### Camera motion

For static geometry you can reconstruct world position from current depth.

Then project it with previous:

$$
P_{t-1}V_{t-1}.
$$

This avoids storing previous clip position per vertex for static scene.

But dynamic objects still need object/animation history.

---

### Skinned motion

Current skinned position:

$$
p_t=
\sum_j
w_j
S_{j,t}p.
$$

Previous:

$$
p_{t-1}
=
\sum_j
w_j
S_{j,t-1}p.
$$

Using only current skin and previous camera misses character motion.

This causes TAA ghosting trails on limbs.

A serious renderer keeps previous joint palette.

---

### Procedural animation

Vertex shader displacement such as:

$$
p'=p+
N\sin(
kx+\omega t
)
$$

needs previous time:

$$
t-\Delta t.
$$

Otherwise motion vector says zero for waving vegetation/ocean while image visibly moves.

Any procedure that affects position must have a previous-frame equivalent.

---

### Reprojection

If motion defined current minus previous:

$$
v_{uv}
=
uv_t-uv_{t-1},
$$

then:

$$
\boxed{
uv_{history}
=
uv_t-v_{uv}.
}
$$

Sample previous history there.

Convention can be opposite; document it.

Half of temporal bugs are sign/convention bugs.

---

### History buffers

Store previous temporally accumulated HDR or post-lighting image.

Often also need previous:

depth, normal, object/material ID or reactive mask.

History validity is not a single global boolean.

Different pixels can become invalid due to disocclusion.

Current Maveb's `TemporalInvalidation` subsystem focuses specifically on invalidating history regions when captured-world edits change local content—a valuable connection between editing and temporal rendering.

---

### Temporal blending

Simple:

$$
\boxed{
C_t^{TAA}
=
(1-\alpha)
H_{t-1}
+
\alpha
C_t.
}
$$

Small $\alpha$:

stable but slow response.

Large $\alpha$:

responsive but less temporal accumulation.

Often adaptive based on velocity, variance, disocclusion or reactive content.

---

### History rejection

Reject if reprojected sample does not represent same surface.

Possible tests:

Depth difference:

$$
|z_t-z_{history}|>\epsilon_z.
$$

Normal:

$$
n_t\cdot n_h<\tau_n.
$$

Object ID mismatch.

Large motion.

Outside viewport.

Known camera cut.

Local edit invalidation.

---

### Neighborhood clamping

History may contain color no longer plausible in current neighborhood.

Compute current 3×3 min/max:

$$
C_{min},C_{max}.
$$

Clamp history:

$$
\boxed{
H'
=
clamp(
H,
C_{min},
C_{max}
).
}
$$

More sophisticated methods operate in YCoCg or use variance:

$$
\mu\pm\gamma\sigma.
$$

This strongly reduces ghost trails.

---

### Ghosting

Ghosting means outdated history remains visible after object moved/disappeared.

Causes:

wrong velocity, missing velocity, disocclusion not detected, over-aggressive history weight, insufficient clamp.

Debug:

current frame, raw history reprojection, velocity, rejection mask, history weight.

Do not debug final TAA output only.

---

### Jitter

Without subpixel jitter each frame samples same locations.

Temporal accumulation cannot reconstruct extra subpixel information.

Projection jitter:

$$
P_t
\rightarrow
P_t+\delta_t.
$$

Common sequences:

Halton base 2/3, Sobol, blue-noise temporal sequences.

Scale jitter in pixel units:

$$
\delta_{ndc,x}
=
2j_x/W,
$$

$$
\delta_{ndc,y}
=
2j_y/H.
$$

Motion vectors must account correctly for current and previous jitter.

Often velocities use unjittered matrices to represent scene motion while reprojection handles jitter explicitly.

---

### Temporal upscaling

Render internally at:

$$
W_r<H_{display}.
$$

Jitter samples different subpixel locations each frame.

Motion vectors map samples across frames.

Temporal reconstruction accumulates enough information to synthesize higher-resolution output.

Conceptually:

$$
\boxed{
\text{low-resolution spatial samples}
+
\text{time}
+
\text{motion}
\rightarrow
\text{higher-resolution estimate}.
}
$$

But disocclusions and thin geometry remain hard.

---

### Bloom

Bloom represents optical scattering of bright light.

Extract bright signal:

$$
B=
max(
C-T,
0
)
$$

or smooth threshold.

Better soft-knee threshold avoids hard discontinuity.

Then downsample and blur.

---

### Separable Gaussian blur

2D Gaussian:

$$
G(x,y)
=
G_x(x)G_y(y).
$$

Thus instead of:

$$
K\times K
$$

samples per pixel, use:

horizontal:

$$
K
$$

then vertical:

$$
K.
$$

Cost:

$$
O(K)
$$

rather than:

$$
O(K^2).
$$

For large bloom, mip/downsample chain is often even cheaper.

---

### Tone mapping

Correct post chain conceptually keeps lighting/bloom in linear HDR.

Typical:

$$
HDR\ scene
$$

$$
+
HDR\ bloom
$$

then exposure:

$$
C_e=
2^{EV}C
$$

then tone-map:

$$
C_{tm}=f(C_e)
$$

then display transform/sRGB encoding.

Do not tone-map the scene before bloom if you want bright HDR energy to drive bloom.

This is the issue we identified earlier in Kairo's current realtime shader path: tone mapping too early destroys true HDR values before later HDR post effects can consume them.

---

### Color grading LUT

3D LUT maps color:

$$
(r,g,b)
\rightarrow
(r',g',b').
$$

A:

$$
32^3
$$

or:

$$
64^3
$$

LUT is common.

Trilinear/tetrahedral interpolation between LUT samples.

Placement depends grading pipeline, but often after tone mapping or within a defined display-referred grading stage.

The key is consistency of color space.

A LUT authored in Rec.709/sRGB display space should not blindly consume scene-linear HDR.

---

### Motion blur

Velocity gives blur path.

Simple screen-space integration:

$$
C_{blur}(p)
=
\frac1N
\sum_i
C(
p+s_iv(p)
).
$$

Problems:

foreground/background bleeding, disocclusion, nonlinear motion, rotational blur.

Tile-max velocity buffers can accelerate and stabilize neighborhood sampling.

---

### Depth of field

Thin lens circle of confusion depends on focal parameters.

A useful proportional form:

$$
CoC
\propto
\left|
\frac1z-
\frac1{z_f}
\right|.
$$

Near/far blur sign distinguishes which side of focus plane.

Real-time DoF generally:

compute CoC, split near/far, gather/scatter blur, composite.

Depth discontinuities are hard because foreground blur must spread over background without background bleeding into foreground incorrectly.

---

### PARTICLES, DECALS AND TECHNIQUE SELECTION

---

### GPU particle buffers

Particle state:

```cpp
struct Particle
{
    Vec3 Position;
    Vec3 Velocity;
    float Age;
    float Lifetime;

    Vec4 Color;

    float Size;
    uint Flags;
};
```

SoA can be better for compute:

```text
positions[]
velocities[]
ages[]
lifetimes[]
```

because simulation kernels may read only subsets.

Update:

$$
v_{t+\Delta}
=
v_t+a\Delta t,
$$

$$
x_{t+\Delta}
=
x_t+v_{t+\Delta}\Delta t.
$$

---

### Alive/dead management

Keep:

```text
AliveList
DeadList
```

Spawn:

pop from dead.

Initialize.

Append to alive.

Update:

if:

$$
age\ge lifetime
$$

append index back to dead.

GPU can use atomic counters or prefix compaction.

Again:

$$
count/mark
\rightarrow
scan
\rightarrow
compact.
$$

---

### Transparency sorting

Particles often alpha blend.

Need back-to-front for ordinary blend or front-to-back explicit compositor.

Sort by depth.

For hundreds of thousands of particles:

radix sort.

Exact sorting may be expensive; approximate bucket sorting is often enough for soft effects.

Weighted blended OIT can avoid full sorting at different visual tradeoffs.

---

### Soft particles

Hard particle polygon intersecting geometry produces obvious edge.

Use scene depth.

Particle depth:

$$
z_p.
$$

Scene depth:

$$
z_s.
$$

Fade as intersection approaches:

$$
\boxed{
fade=
clamp(
(z_s-z_p)/d_{soft},
0,1
)
}
$$

according to depth convention.

Multiply alpha by fade.

Smoke disappears smoothly into wall/floor.

---

### Decals

A decal projects material information onto existing surface.

Examples:

bullet holes, blood, dirt, graffiti.

Deferred decal often renders a box volume.

For pixels inside projected box, reconstruct world position from depth, transform into decal local space.

If:

$$
|x|,|y|,|z|\le1
$$

sample decal texture from local coordinates.

Then blend into G-buffer/material attributes.

---

### Screen-space decals

Advantages:

no mesh topology modification.

Works on arbitrary geometry.

Problems:

normal-angle stretching, overlap ordering, depth discontinuities, transparency, MSAA.

Angle rejection:

$$
dot(N_{surface},N_{decal})
>
threshold.
$$

---

### Overdraw

Particles and decals can become fill-rate monsters.

Overdraw:

$$
O(p)
=
\text{number of fragment invocations covering pixel}.
$$

Total shaded fragments:

$$
\sum_pO(p).
$$

Transparent fragments cannot benefit from normal early-Z rejection as effectively because many layers contribute.

A particle system with tiny geometry cost can still destroy performance through fill.

Debug with overdraw heatmap.

---

### Ablations

For any advanced renderer feature, test:

$$
Feature\ OFF
$$

versus:

$$
Feature\ ON.
$$

Measure:

frame time, GPU pass time, memory, image error/quality.

For adaptive feature, compare several budget points.

Do not claim improvement because screenshot looks better.

Construct Pareto curve:

$$
\boxed{
Quality
\quad vs\quad
Cost.
}
$$

That is especially relevant to FoveaBench and your adaptive-rendering research.

---


This is the synthesis of Phase 26.

Exact methods are appropriate when correctness cost is acceptable.

Approximate methods exploit signal structure.

Spatial methods use nearby pixels/clusters/probes.

Temporal methods reuse previous frames.

Adaptive methods spend work where predicted benefit is highest.

A robust decision framework is:

| Workload | Prefer |
|---|---|
| few lights, simple scene | forward |
| many opaque lights | clustered/deferred |
| huge static occlusion | Hi-Z |
| sharp direct shadows | shadow maps / RT |
| diffuse indirect | probes/DDGI/radiance structures |
| glossy on-screen reflection | SSR |
| off-screen reflection | probes/RT |
| subpixel stability | TAA |
| high-resolution output from lower render | temporal upscale |
| massive soft effects | GPU particles + approximate sorting |
| strict frame budget | adaptive allocation |

The principle is:

$$
\boxed{
\text{the best rendering algorithm is workload-dependent}.
}
$$

---


Current Maveb's clustered light builder already uses logarithmic depth slicing, screen/depth conservative bounds, explicit reference budgets and flattened offset-count lists. It does not yet perform a dedicated exact cone-vs-cluster test for spots; local lights use conservative range bounds. Its shadow system implements practical CSM split blending and texel-grid stabilization. Its image-based-lighting code contains real irradiance/specular/BRDF preprocessing rather than a placeholder environment lookup. Temporal invalidation is already connected to local world edits, which is unusually relevant to your captured-world work.

The deeper opportunity is to join these systems: clustered light assignment, Hi-Z visibility, temporal reuse and adaptive work allocation can all become **budgeted spatial work distributions**. That same mathematical structure already appears in Gaussian tiling and Vulkax's gradient-ranked adaptive material regions.

**Pass 17 complete: Parts 2501–2600.**

---

### MESH TOPOLOGY, GEOMETRY PROCESSING, GLTF, SKINNING, ANIMATION AND MORPHS
### M162

---

### INDEXED MESHES, TOPOLOGY AND HALF-EDGES

A triangle mesh has two fundamentally different kinds of information:

$$
\boxed{
\text{geometry}
}
$$

and:

$$
\boxed{
\text{topology}.
}
$$

Geometry says:

> Where are points?

Topology says:

> Which points are connected into edges/faces?

Never conflate them.

---

### Indexed vertices and faces

Unindexed triangles:

```text
triangle 0:
    v0 v1 v2

triangle 1:
    v3 v4 v5
```

Even if:

$$
v_1=v_3
$$

position-wise, they are separate storage.

Indexed:

```cpp
vertices =
[
    V0,V1,V2,V3
];

indices =
[
    0,1,2,
    2,1,3
];
```

Two triangles share:

$$
V_1,V_2.
$$

Memory improves and topology becomes more explicit.

---

### Attributes

Vertex may contain channels:

$$
position
$$

$$
normal
$$

$$
uv
$$

$$
tangent
$$

$$
color
$$

$$
joints
$$

$$
weights.
$$

These are not all intrinsically per-vertex.

Position often represents topological vertex.

UV frequently belongs to **face corner**.

Normal can be per-corner at hard edges.

This distinction is critical.

---

<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/39 - Tiling, Ordering and Compositing at Scale|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/41 - Per-Vertex and Per-Corner Mesh Attributes|Next chapter]]
