# 41 — Per-Vertex and Per-Corner Mesh Attributes

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/40 - Lighting, Reconstruction and Surface Motion|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/42 - XR Radiometry, Optics and Perception|Next chapter]]

A mesh vertex can carry a single shared value, while a corner can carry values that differ across incident faces. This distinction explains seams, normals, UVs and the relationship between topology and shading.

**By the end:** Choose per-vertex or per-corner attributes based on mesh continuity.

**Recall question:** When should two triangles share a vertex position but keep separate corner attributes?

<!-- Derived from Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography.md; source lines 94606–97863; source-span SHA256 c8d8b33f6c039e15153ca041dae1dd9fdbdecdebf8e8bea1c5148b831d7a5ad9. -->

<!-- CHAPTER-SOURCE-BEGIN -->
### Per-vertex vs per-corner

Consider cube corner shared by three faces.

Geometric vertex position:

$$
p
$$

is one point.

But face normals:

$$
n_x,n_y,n_z
$$

are three different values.

A GPU vertex typically has one normal.

Therefore same topological vertex must be duplicated:

```text
position identical
normal different
```

Likewise UV seam.

So:

$$
\boxed{
\text{GPU vertex}
\neq
\text{topological vertex}.
}
$$

A GPU vertex is usually a unique tuple of all active corner attributes.

---

### Index sharing

You may share two corners only if every indexed attribute agrees:

$$
(position,normal,uv,tangent,\ldots).
$$

If position same but UV differs:

duplicate.

If position+UV same but normal differs:

duplicate.

This becomes crucial in OBJ importer later.

---

### Topology vs geometry

Two meshes can have identical positions but different topology.

Example:

Two triangles sharing coordinates.

Mesh A:

indices share edge vertices.

Mesh B:

duplicates vertices along edge.

Visually identical in flat render.

Topologically:

A connected.

B disconnected.

Algorithms like smoothing, subdivision, simulation and adjacency behave differently.

---

### Boundary edges

For triangle mesh edge:

$$
e=(v_i,v_j).
$$

Count incident faces.

Boundary if:

$$
\boxed{
|F(e)|=1.
}
$$

Interior manifold edge:

$$
|F(e)|=2.
$$

---

### Manifold and non-manifold

A 2-manifold interior edge has exactly two incident faces.

Non-manifold if:

$$
|F(e)|>2.
$$

Example three triangles sharing one edge.

Vertex can also be non-manifold if incident faces form disconnected fans.

Why care?

Many algorithms assume local disk topology.

Half-edge traversal, subdivision, normals and simplification can fail on non-manifold data.

Current Kairo `EditableMesh::Validate()` explicitly counts canonical edges and rejects any edge used by more than two faces.

That is a strong invariant.

---

### Connected components

Construct graph where faces are nodes.

Two faces adjacent if sharing edge.

Run DFS/BFS.

Each traversal produces one connected component.

Complexity:

$$
O(V+E+F).
$$

Useful for:

isolating objects, cleanup, import diagnostics, topology repair.

---

### Face adjacency

Map canonical edge:

$$
\{\min(v_i,v_j),\max(v_i,v_j)\}
$$

to incident face list.

Then:

```text
edge -> [faceA,faceB]
```

gives face adjacency.

For manifold interior edge:

two faces.

Boundary:

one.

Non-manifold:

more than two.

---

### Vertex adjacency

For every edge:

$$
(v_i,v_j)
$$

add:

$$
v_j\in N(v_i)
$$

and:

$$
v_i\in N(v_j).
$$

One-ring:

$$
\boxed{
N(v)
=
\text{vertices directly connected to }v.
}
$$

This neighborhood drives smoothing, curvature, subdivision, local editing.

---

### Half-edge

Each undirected edge becomes two directed half-edges.

A canonical half-edge record:

```cpp
struct HalfEdge
{
    VertexID origin;
    FaceID face;

    HalfEdgeID twin;
    HalfEdgeID next;
};
```

If:

$$
h=(a\rightarrow b),
$$

then:

$$
twin(h)
=
(b\rightarrow a).
$$

Within a face:

$$
next(h)
$$

moves around boundary.

For triangle:

$$
h_0\rightarrow h_1\rightarrow h_2\rightarrow h_0.
$$

---

### One-ring traversal

Suppose outgoing half-edge:

$$
h_0
$$

from vertex $v$.

A common manifold traversal:

$$
\boxed{
h_{next}
=
twin(next(h))
}
$$

or variant depending whether half-edge stores origin/destination convention.

Repeated application walks neighboring faces around vertex.

This gives ordered ring, not merely unordered adjacency set.

---

### Current Kairo representation

Kairo `EditableHalfEdge` currently stores:

```cpp
From
To
Face
Corner
optional<Twin>
```

It does not store an explicit `Next` index.

`next` is implicit from the face's corner sequence:

$$
corner+1\pmod{faceVertexCount}.
$$

That is perfectly workable.

Its topology representation also uses persistent 64-bit editable vertex and face IDs rather than raw vector indices—good for editor operations.

---

### NORMALS, TANGENTS, BOUNDS AND OBJ

---

### Flat face normal

Triangle:

$$
p_0,p_1,p_2.
$$

Edges:

$$
e_1=p_1-p_0,
$$

$$
e_2=p_2-p_0.
$$

Unnormalized normal:

$$
\tilde n=
e_1\times e_2.
$$

Normalized:

$$
\boxed{
n=
\frac{
e_1\times e_2
}{
\|e_1\times e_2\|
}.
}
$$

Magnitude:

$$
\|e_1\times e_2\|
=
2A_{triangle}.
$$

So cross product naturally contains area weight.

Current Kairo's `FaceNormal()` computes exactly this.

---

### Smooth normals

Simplest:

$$
n_v=
normalize
\left(
\sum_{f\ni v}
n_f
\right).
$$

This gives equal face weight.

Current Kairo `RecalculateSmoothNormals` uses this simple equal-normal accumulation.

But a large triangle and tiny triangle count equally.

---

### Area-weighted normals

Use unnormalized face cross products:

$$
\boxed{
n_v
=
normalize
\left(
\sum_{f\ni v}
(e_1\times e_2)
\right).
}
$$

Since magnitude:

$$
2A_f,
$$

large faces contribute more.

---

### Angle-weighted normals

At vertex $v$, face corner angle:

$$
\theta_f.
$$

Then:

$$
\boxed{
n_v=
normalize
\left(
\sum_f
\theta_fn_f
\right).
}
$$

This often better approximates smooth surface normals under irregular triangulation because contribution depends on wedge angle rather than triangle area.

Hard edges require separate normals/corner vertices rather than averaging everything.

---

### Tangent generation

Triangle position edges:

$$
e_1=p_1-p_0,
$$

$$
e_2=p_2-p_0.
$$

UV deltas:

$$
\Delta uv_1=
(u_1-u_0,v_1-v_0),
$$

$$
\Delta uv_2=
(u_2-u_0,v_2-v_0).
$$

We assume:

$$
e_1=
T\Delta u_1+
B\Delta v_1,
$$

$$
e_2=
T\Delta u_2+
B\Delta v_2.
$$

Solve:

$$
D=
\Delta u_1\Delta v_2
-
\Delta v_1\Delta u_2.
$$

Then:

$$
\boxed{
T=
\frac{
e_1\Delta v_2-
e_2\Delta v_1
}{D}
}
$$

and:

$$
\boxed{
B=
\frac{
e_2\Delta u_1-
e_1\Delta u_2
}{D}.
}
$$

Maveb's glTF loader generates tangents using exactly this formula when authored tangents are absent.

---

### Orthogonalize tangent

After accumulating tangents:

$$
T'
=
T-N(N\cdot T).
$$

Normalize:

$$
\boxed{
\hat T=
normalize(T').
}
$$

This is Gram-Schmidt.

---

### Handedness

Reconstructed bitangent:

$$
B'
=
s(N\times T)
$$

where:

$$
s\in\{-1,+1\}.
$$

Compute:

$$
\boxed{
s=
sign(
(N\times T)\cdot B
).
}
$$

Store tangent:

$$
(T_x,T_y,T_z,s).
$$

Maveb does exactly this.

---

### Degenerate UVs

If:

$$
D
=
\Delta u_1\Delta v_2-
\Delta v_1\Delta u_2
\approx0,
$$

UV mapping has zero/near-zero area.

Cannot invert.

Do not divide.

Fallback tangent can be generated from normal and arbitrary nonparallel axis.

Maveb skips degenerate UV triangle accumulation and later constructs fallback tangent.

Correct.

---

### Bounds

AABB:

$$
min=
(\min x,\min y,\min z),
$$

$$
max=
(\max x,\max y,\max z).
$$

Cheap and tight in fixed coordinate frame.

Bounding sphere simple center:

$$
c=
\frac{min+max}{2}
$$

then:

$$
r=
\max_i\|p_i-c\|.
$$

Ritter algorithm often produces tighter sphere cheaply.

Exact minimum enclosing sphere is more expensive.

Bounds drive:

culling, BVH, picking, LOD, shadow fitting, animated bounds.

---

### OBJ

OBJ positions:

```text
v x y z
```

UV:

```text
vt u v
```

normal:

```text
vn x y z
```

face:

```text
f v/vt/vn v/vt/vn v/vt/vn
```

Important: OBJ uses separate index streams.

A face corner can reference:

$$
(positionIndex,
uvIndex,
normalIndex).
$$

GPU usually wants one unified vertex index.

Therefore importer must deduplicate **tuples**, not positions alone.

---

### OBJ negative indices

Positive:

$$
1
$$

means first element.

OBJ is one-based.

Negative:

$$
-1
$$

means most recently defined element.

For count $N$, negative $i$:

$$
\boxed{
index_{0based}
=
N+i.
}
$$

Example $N=10$:

$$
-1\rightarrow9,
$$

$$
-2\rightarrow8.
$$

---

### Polygon triangulation

OBJ faces can have $n>3$ vertices.

Simple fan:

$$
(v_0,v_1,v_2),
$$

$$
(v_0,v_2,v_3),
$$

...

works for convex polygons.

It can fail for concave polygons.

Robust importer can use ear clipping after projecting polygon to plane.

Do not silently claim fan triangulation supports arbitrary polygons.

---

### Tuple deduplication

Hash key:

```cpp
struct ObjVertexKey
{
    int Position;
    int UV;
    int Normal;
};
```

Map:

$$
key\rightarrow GPUVertexIndex.
$$

When corner tuple first seen, create GPU vertex.

When repeated, reuse index.

This preserves seams and hard normals.

---

### glTF FROM BYTES TO PORTABLE SCENE

glTF is fundamentally a structured graph of metadata pointing into binary data.

The hierarchy:

$$
\boxed{
buffer
\rightarrow
bufferView
\rightarrow
accessor
\rightarrow
attribute
}
$$

must be deeply understood.

---

### Buffers

Buffer is raw bytes.

Could be:

external `.bin`, embedded GLB chunk, data URI.

No semantic type yet.

---

### bufferView

A bufferView selects a byte range:

$$
bufferIndex,
byteOffset,
byteLength,
byteStride.
$$

Think:

$$
\boxed{
\text{raw byte slice}.
}
$$

It may represent interleaved vertex data or index stream.

---

### Accessor

Accessor adds typed interpretation:

$$
componentType,
type,
count,
offset,
normalized.
$$

Example:

```text
componentType = FLOAT
type = VEC3
count = 1000
```

means:

$$
1000
$$

three-component float vectors.

Effective address:

$$
\boxed{
bufferBase
+
bufferView.byteOffset
+
accessor.byteOffset
+
i\times stride.
}
$$

Never forget both offsets.

---

### Primitive attributes

glTF primitive may map:

```text
POSITION -> accessor 4
NORMAL   -> accessor 7
TEXCOORD_0 -> accessor 9
JOINTS_0 -> accessor 11
WEIGHTS_0 -> accessor 12
```

Semantic names drive interpretation.

Position required for rendering primitive.

Normals/tangents optional.

Joint/weights appear for skinning.

---

### Indices

Index accessor uses scalar unsigned integer component type.

Could be:

$$
uint8,uint16,uint32.
$$

Convert into engine index type, often `uint32_t`.

Validate:

$$
index<vertexCount.
$$

Maveb and Kairo both perform strict count/bounds validation rather than trusting source.

---

### Nodes

A glTF node can contain:

mesh, camera, skin, children and transform.

Transform either:

$$
matrix
$$

or decomposed:

$$
T,R,S.
$$

Local:

$$
M_{local}
=
T
R
S
$$

under glTF convention.

World:

$$
\boxed{
M_{world,node}
=
M_{world,parent}
M_{local,node}.
}
$$

Scene contains root node indices.

Not every node in file must belong to active scene.

---

### Kairo's matrix conversion

glTF matrices stored column-major.

Kairo math stores row-major but uses column-vector multiplication.

Current runtime copies:

$$
result(row,column)
=
source[column\times4+row].
$$

This is storage conversion preserving same mathematical transform, not an arbitrary geometric transpose.

This distinction is extremely important.

---

### Materials

Primitive references material index.

Material contains glTF PBR metallic-roughness data:

$$
baseColorFactor,
metallicFactor,
roughnessFactor,
textures,
normal,
occlusion,
emissive,
alphaMode,
doubleSided.
$$

We already covered its shading semantics in M119.

Importer must preserve semantic defaults and texture color spaces.

---

### Images/textures/samplers

Image:

actual image payload/source.

Texture:

links image plus sampler.

Sampler:

mag/min filters and wrap modes.

So:

$$
\boxed{
Image
\neq
Texture
\neq
Sampler.
}
$$

Current Maveb loader resolves wrap modes and all glTF minification/mipmap filter combinations into portable enums.

It currently requires texture coordinate set:

$$
TEXCOORD_0
$$

for its supported material bindings; other coordinate sets are rejected as unsupported rather than misinterpreted.

Good.

---

### Animation structures

A glTF animation has:

samplers and channels.

Animation sampler defines:

$$
input=timeAccessor,
$$

$$
output=valueAccessor,
$$

$$
interpolation.
$$

Channel maps sampler to:

$$
targetNode
$$

and path:

$$
translation,
rotation,
scale,
weights.
$$

Kairo's imported animation representation follows this model.

---

### Skins

Skin stores ordered joint node list:

$$
J_0,\dots,J_{n-1}
$$

and inverse-bind matrices.

`JOINTS_0` vertex values index into **this ordered skin joint array**, not global node indices directly.

That distinction is critical.

---

### Morph targets

Primitive target may contain delta accessors:

```text
POSITION
NORMAL
TANGENT
```

These are deltas, not complete replacement geometry.

For target $k$:

$$
\Delta p_k.
$$

Final morph position:

$$
p'=p+\sum_k w_k\Delta p_k.
$$

More later.

---

### SIMPLIFICATION, SUBDIVISION, SMOOTHING AND PARAMETERIZATION

Current repository boundary first: the inspected KairoAssets code has real editable topology, split/inset/extrude/dissolve/loop-cut and subdivision-modifier infrastructure, but I did **not** find an implemented QEM simplifier, cotangent Laplacian or UV parameterization system in the current repo. So the theory below is curriculum-level, not a claim about current implementation.

---

### Edge collapse

Simplification reduces triangle count while minimizing visual/geometric error.

Edge:

$$
(v_a,v_b)
$$

collapsed to:

$$
v'.
$$

All faces referencing either endpoint now reference $v'$.

Degenerate faces removed.

But collapse can change topology, normals and UVs.

Choosing which edge and new position is the problem.

---

### Quadric Error Metrics

For plane:

$$
ax+by+cz+d=0,
$$

normalized plane vector:

$$
p=
[a,b,c,d]^T.
$$

For homogeneous vertex:

$$
\hat v=
[x,y,z,1]^T,
$$

signed plane equation:

$$
p^T\hat v.
$$

Squared distance-like error:

$$
(p^T\hat v)^2.
$$

Rewrite:

$$
\boxed{
(p^T\hat v)^2
=
\hat v^T
(pp^T)
\hat v.
}
$$

Define plane quadric:

$$
Q_p=pp^T.
$$

For vertex adjacent to planes:

$$
\boxed{
Q_v=
\sum_{p\ni v}
Q_p.
}
$$

For collapsing edge:

$$
Q=
Q_a+Q_b.
$$

Candidate cost:

$$
\boxed{
E(v')
=
\hat v'^TQ\hat v'.
}
$$

Choose position minimizing this.

---

### Solve optimal collapse position

Partition:

$$
Q=
\begin{bmatrix}
A&b\\
b^T&c
\end{bmatrix}
$$

with $A$ 3×3.

For:

$$
E(x)=x^TAx+2b^Tx+c.
$$

Gradient:

$$
\nabla E=
2Ax+2b.
$$

Set zero:

$$
\boxed{
Ax=-b.
}
$$

If $A$ invertible:

$$
x=-A^{-1}b.
$$

If singular, test endpoints and midpoint.

---

### Topology constraints

Do not collapse if it:

inverts triangle orientation, creates duplicate faces, creates non-manifold edge, collapses boundary incorrectly, crosses material/UV seam, destroys protected feature edge.

A common manifold test is link condition.

For edge $(u,v)$, one-ring intersection should match edge link.

Topology preservation matters as much as geometric QEM cost.

---

### Subdivision

Subdivision increases mesh resolution and smooths surface.

Loop subdivision for triangles.

Interior edge new vertex with endpoints:

$$
v_0,v_1
$$

and opposite vertices:

$$
v_2,v_3:
$$

$$
\boxed{
v_e
=
\frac38(v_0+v_1)
+
\frac18(v_2+v_3).
}
$$

Boundary edge typically:

$$
v_e=
\frac12(v_0+v_1).
$$

---

### Existing Loop vertex

For interior valence $n$, one exact Loop weight formulation:

$$
\beta
=
\frac1n
\left[
\frac58-
\left(
\frac38+
\frac14\cos\frac{2\pi}{n}
\right)^2
\right].
$$

Then:

$$
\boxed{
v'
=
(1-n\beta)v
+
\beta
\sum_{j=1}^{n}v_j.
}
$$

Common simplified rules exist, but the above captures canonical valence dependence.

Each old triangle becomes four.

---

### Laplacian smoothing

Uniform Laplacian:

$$
\bar v_i=
\frac1{|N(i)|}
\sum_{j\in N(i)}
v_j.
$$

Laplacian:

$$
\Delta v_i=
\bar v_i-v_i.
$$

Update:

$$
\boxed{
v_i'
=
v_i+
\lambda\Delta v_i.
}
$$

Problem:

mesh shrinkage.

Repeated averaging moves vertices toward center.

Taubin smoothing uses positive/negative passes to reduce shrinkage.

---

### Cotangent Laplacian

For edge $ij$, opposite angles:

$$
\alpha_{ij},\beta_{ij}.
$$

Weight:

$$
\boxed{
w_{ij}
=
\frac12
(
\cot\alpha_{ij}
+
\cot\beta_{ij}
).
}
$$

Discrete Laplacian:

$$
\Delta v_i
=
\frac1{A_i}
\sum_{j}
w_{ij}(v_j-v_i)
$$

with appropriate area normalization.

This reflects local surface geometry better than uniform neighbor average.

Used in:

smoothing, curvature, parameterization, deformation.

---

### Parameterization

Map surface mesh:

$$
(x,y,z)
$$

to UV:

$$
(u,v).
$$

Want low angular/stretch distortion and no overlap.

For disk-like mesh, fix boundary to 2D curve, solve interior harmonic equations:

$$
\boxed{
\sum_j
w_{ij}
(u_i-u_j)=0
}
$$

and same for v.

Cotangent weights yield conformal-ish harmonic maps but can fold depending boundary/topology.

LSCM minimizes conformal distortion.

ABF optimizes angles.

Complex topology requires cuts.

---

### UV seams

Closed surface cannot generally flatten to plane without seams/distortion.

A seam duplicates a topological vertex in UV domain.

Same position:

$$
p.
$$

Different UVs:

$$
uv_A\ne uv_B.
$$

Therefore GPU mesh duplication is required.

This loops directly back to Part 2608:

$$
\boxed{
\text{corner attributes create vertex splits}.
}
$$

---

### SKELETONS, SKINNING AND ANIMATION

---

### Skeleton hierarchy

Skeleton is tree/forest of joints.

Each joint:

$$
j
$$

has parent:

$$
parent(j).
$$

Local transform relative parent:

$$
L_j.
$$

World/global:

$$
\boxed{
G_j
=
G_{parent(j)}
L_j.
}
$$

Root:

$$
G_{root}=L_{root}.
$$

Current Kairo animation runtime resolves this recursively and detects hierarchy cycles defensively.

---

### Local transforms

Usually TRS:

$$
L=
T(t)
R(q)
S(s).
$$

Animation channels update components.

Matrix-authored static glTF nodes may remain exact matrices rather than forcing lossy decomposition.

Kairo explicitly preserves this distinction.

That is good engineering.

---

### Bind pose

Bind pose is skeleton configuration in which mesh was authored/attached.

Let joint bind global:

$$
B_j.
$$

We want vertex transformed from bind coordinate into current joint coordinate.

First undo bind:

$$
B_j^{-1}.
$$

Then apply current:

$$
G_j(t).
$$

Skin transform:

$$
\boxed{
S_j(t)
=
G_j(t)
B_j^{-1}.
}
$$

At bind pose:

$$
G_j=B_j,
$$

so:

$$
S_j=
I.
$$

This is the key sanity check.

---

### glTF inverse-bind matrices

glTF stores:

$$
IBM_j
$$

directly.

Current Kairo EngineCore builds palette:

$$
\boxed{
JointMatrix_j
=
WorldJoint_j
\cdot
InverseBind_j.
}
$$

It intentionally keeps this in asset space and applies outer scene/entity transform later in renderer.

That avoids requiring inverse of mesh-node transform, which may contain zero scale.

This is a thoughtful design choice.

---

### Linear Blend Skinning

Vertex has joint indices:

$$
j_0,\dots,j_3
$$

and weights:

$$
w_0,\dots,w_3.
$$

Weights:

$$
w_i\ge0,
$$

and ideally:

$$
\boxed{
\sum_iw_i=1.
}
$$

Skinned position:

$$
\boxed{
p'
=
\sum_{i=0}^{3}
w_i
S_{j_i}
\begin{bmatrix}
p\\1
\end{bmatrix}.
}
$$

Equivalently blended skin matrix:

$$
S=
\sum_iw_iS_{j_i}.
$$

Then:

$$
p'=Sp.
$$

---

### Weight normalization

Input may not sum exactly one due quantization.

Compute:

$$
s=
\sum_iw_i.
$$

If:

$$
s>0
$$

normalize:

$$
w_i'=w_i/s.
$$

If all zero, importer must define fallback or reject.

Current Kairo renderer validates that four weights sum to one within:

$$
10^{-3}.
$$

It also caps portable palette at:

$$
255
$$

joints because 255 mat4 values fit its 16 KiB baseline uniform-buffer portability target.

---

### Skinning normals/tangents

Position uses $w=1$.

Direction uses $w=0$.

For purely rigid joint matrices, blend upper 3×3 and normalize.

With nonuniform scale, correct normal transformation is inverse transpose.

But:

$$
inverseTranspose
\left(
\sum_iw_iS_i
\right)
$$

is not generally equal to:

$$
\sum_iw_i
inverseTranspose(S_i).
$$

There is no perfect simple LBS normal rule under arbitrary nonrigid blended transforms.

Many engines assume joint transforms are rigid/uniform-scale and use weighted 3×3 then normalize.

Tangents similarly.

---

### Candy-wrapper artifact

Consider cylinder arm twisting 180° between two bones.

LBS linearly blends matrices.

Linear interpolation of rotations is not a rotation.

At midpoint, opposite rotations can partially cancel, shrinking cross-section.

Result:

$$
\boxed{
\text{volume collapse / candy-wrapper twist}.
}
$$

---

### Dual-quaternion skinning

Rigid transform represented by dual quaternion:

$$
\hat q=
q_r+
\epsilon q_d.
$$

Blend dual quaternions then normalize.

This better interpolates rotations/translations and preserves volume under twisting.

It does not naturally handle arbitrary nonuniform scale; hybrid methods are needed.

---

### Animation clips/channels/samplers

Clip:

collection of animation channels over duration.

Channel:

target node/property.

Sampler:

key times + values + interpolation.

Key times:

$$
t_0<t_1<\cdots<t_n.
$$

Current Maveb loader explicitly rejects non-finite, unsorted or duplicate animation times.

Excellent malformed-data defense.

---

### Translation interpolation

For segment:

$$
t\in[t_0,t_1].
$$

Normalized:

$$
u=
\frac{t-t_0}{t_1-t_0}.
$$

Translation:

$$
\boxed{
T(u)
=
(1-u)T_0+uT_1.
}
$$

---

### Scale interpolation

Similarly:

$$
\boxed{
S(u)
=
(1-u)S_0+uS_1.
}
$$

Usually component-wise.

Negative scales complicate interpolation but glTF permits scale values that can be negative.

---

### Quaternion interpolation

Given normalized quaternions:

$$
q_0,q_1.
$$

Cosine:

$$
d=q_0\cdot q_1.
$$

If:

$$
d<0
$$

negate one quaternion because:

$$
q
$$

and:

$$
-q
$$

represent same rotation, but we want shortest path.

Slerp:

$$
\boxed{
q(u)
=
\frac{
\sin((1-u)\theta)
}{
\sin\theta
}
q_0
+
\frac{
\sin(u\theta)
}{
\sin\theta
}
q_1
}
$$

where:

$$
\cos\theta=d.
$$

For small angle, normalized lerp is numerically better.

Current Kairo uses `SLerp`.

---

### Step interpolation

$$
value(t)=value_i
$$

until next key.

No interpolation.

Useful for discrete state or intentionally stepped animation.

---

### Cubic spline

glTF cubic spline stores:

$$
inTangent,
value,
outTangent
$$

per key.

For interval $\Delta t$, Hermite:

$$
P(u)
=
h_{00}P_0
+
h_{10}\Delta t M_0
+
h_{01}P_1
+
h_{11}\Delta t M_1
$$

with:

$$
h_{00}=2u^3-3u^2+1,
$$

$$
h_{10}=u^3-2u^2+u,
$$

$$
h_{01}=-2u^3+3u^2,
$$

$$
h_{11}=u^3-u^2.
$$

Note tangent multiplication by:

$$
\Delta t.
$$

Current Kairo animation runtime implements Hermite sampling and renormalizes quaternion result for rotation channels.

---

### Pose evaluation

Algorithm:

Start rest pose.

Sample each active channel at time $t$.

Override corresponding local TRS component.

Rebuild local matrices.

Resolve hierarchy:

$$
G_j=
G_{parent}L_j.
$$

Build skin palette:

$$
S_j=
G_jB_j^{-1}.
$$

Then render.

That is the complete pose path.

---

### Animation blending

Two local poses A/B.

Translation:

$$
T=
lerp(T_A,T_B,\alpha).
$$

Scale:

$$
S=
lerp(S_A,S_B,\alpha).
$$

Rotation:

$$
R=
slerp(R_A,R_B,\alpha).
$$

Then reconstruct local transform.

Current Kairo `BlendGltfAnimationPoses()` does exactly this.

Do not linearly blend arbitrary 4×4 matrices if you want meaningful rotations/scales.

---

### Crossfade

Fade from clip A to B over duration $D$:

$$
\alpha=
clamp(
(t-t_0)/D,
0,1
).
$$

Evaluate both clips at their own times.

Blend resulting poses.

Once:

$$
\alpha=1,
$$

drop A.

For locomotion, phase synchronization may improve transition.

---

### Additive animation

Additive pose stores delta relative reference pose.

Translation:

$$
\Delta T=
T_{anim}-T_{ref}.
$$

Scale can be ratio or delta.

Rotation:

$$
\Delta R=
R_{anim}R_{ref}^{-1}.
$$

Apply weighted:

$$
R_{out}
=
slerp(
I,
\Delta R,
w
)
R_{base}
$$

with convention-dependent multiplication order.

Useful for:

breathing, recoil, aim offsets, facial layers.

---

### Root motion

Animation root may translate character forward.

If renderer applies root animation while game object also moves, double motion can happen.

Root-motion system extracts root delta:

$$
\Delta T=
T_{root}(t_1)
-
T_{root}(t_0)
$$

and rotation delta.

Apply to entity/world locomotion.

Then remove that motion from local skeleton root as appropriate.

This synchronizes gameplay position with authored movement.

---

### MORPHS, ANIMATED BOUNDS, COMPRESSION AND COMPLETE TRACE

---

### Morph position deltas

Base position:

$$
p.
$$

Target k delta:

$$
\Delta p_k.
$$

Weight:

$$
w_k.
$$

Result:

$$
\boxed{
p_m
=
p+
\sum_kw_k\Delta p_k.
}
$$

Targets can represent:

facial expression, corrective shape, muscle bulge, damage deformation.

---

### Morph normal deltas

Similarly:

$$
n_m=
n+
\sum_kw_k\Delta n_k.
$$

Then normalize:

$$
\boxed{
n_m\leftarrow
normalize(n_m).
}
$$

Morph normals are not unit after linear addition.

Tangents may also have deltas.

---

### Morph weights

Multiple targets combine linearly:

$$
p'
=
p+
w_0\Delta p_0
+
w_1\Delta p_1
+\cdots.
$$

Weights need not necessarily sum to one.

This is not barycentric blending between full shapes.

It is additive delta composition.

---

### Morph before or after skinning

For glTF semantics, morph target displacements are applied to base mesh in object/local space **before skinning**.

So:

$$
\boxed{
p_m=
p+
\sum_kw_k\Delta p_k
}
$$

then:

$$
\boxed{
p_{skin}
=
\sum_j
w_j^{skin}
S_j
p_m.
}
$$

If you skin first and then add object-space morph delta, result is wrong because delta does not follow bones.

---

### Animated bounds

Static mesh bounds no longer valid under:

skinning, morphing, procedural animation.

Options:

Conservative offline bound over all animation samples.

Per-bone bounds transformed each frame.

CPU-evaluate bounds.

GPU reduction.

Inflated rest bound.

Exact per-frame vertex reduction is expensive.

Bone bounds are common compromise.

---

### CPU vs GPU skinning

CPU skinning:

Compute transformed vertices on CPU.

Upload dynamic vertex buffer.

Pros: easy CPU collision/picking, simple shaders.

Cons: CPU work + upload bandwidth.

GPU skinning:

Upload base vertices once.

Upload joint palette every frame.

Vertex shader computes skin.

Pros: scalable, low upload bandwidth.

Cons: deformed vertices unavailable to CPU unless replicated/read back.

Current Kairo is architected for GPU skinning: EngineCore computes palette, renderer receives four-weight influence stream + palette.

This separation is good.

---

### Animation compression

Raw animation can be large.

Compression dimensions:

quantize times, quantize translation/scale, quaternion smallest-three encoding, remove redundant constant tracks, reduce keys by error threshold.

Quaternion unit constraint means four floats are redundant.

Store largest component index and sign, encode remaining three.

Reconstruct fourth:

$$
q_m=
\sqrt{
1-
q_a^2-
q_b^2-
q_c^2
}.
$$

Key reduction removes key $i$ if interpolation between neighbors stays under error tolerance.

Error should be measured after hierarchical effect when possible, not only local component difference.

---

### Coordinate-convention conversion

Importers must reconcile:

handedness, up axis, forward axis, matrix storage, texture UV conventions, winding.

Never sprinkle conversions randomly.

Define canonical engine convention.

Construct conversion transform:

$$
C.
$$

Positions:

$$
p_e=
Cp_s.
$$

Transforms:

$$
\boxed{
M_e=
CM_sC^{-1}.
}
$$

Normals use corresponding linear inverse-transpose where needed.

If conversion changes handedness:

$$
det(C)<0,
$$

triangle winding flips.

Tangent handedness may need sign adjustment.

Animation transforms and inverse-bind matrices must be converted consistently with mesh.

Converting only vertices but not skeleton is catastrophic.

---

### Asset validation

Treat glTF/OBJ as hostile input.

Validate:

counts within budgets, finite floats, accessor byte ranges, stride, component type, indices in range, node hierarchy acyclic, skin joints valid, inverse-bind count matches joint count, weights finite/nonnegative, animation times increasing, quaternion nonzero, morph target counts match base, texture/material indices valid.

Current Kairo and Maveb both do substantial validation.

Maveb explicitly checks file-size budgets, primitive counts, vertex/index growth, finite animation data, increasing key times and nonzero rotation quaternions.

Kairo's scene validator also rejects malformed hierarchy and inconsistent skin data before runtime palette building.

---


A strong pipeline is:

```text
glTF bytes
↓
JSON/GLB parser
↓
buffers / bufferViews / accessors
↓
validated typed attribute streams
↓
portable MeshArtifact
    positions
    normals
    tangents
    UVs
    indices
    morph targets
    joints/weights
↓
portable Materials / Textures
↓
portable node hierarchy
↓
skins + inverse binds
↓
animation clips/channels
↓
EngineCore runtime pose
↓
skin palette
↓
Renderer GPU buffers
```

The important architectural boundary:

$$
\boxed{
\text{renderer should not know glTF parsing details}.
}
$$

Assets owns file semantics.

EngineCore owns animation semantics.

Renderer owns GPU execution.

Current Kairo architecture follows this separation remarkably well.

---


This is the final mastery trace.

Assume glTF contains one mesh vertex.

POSITION accessor eventually decodes:

$$
p=
(0.2,1.0,0.0,1).
$$

Morph target 0:

$$
\Delta p_0=
(0.0,0.1,0.0,0).
$$

Morph weight:

$$
m_0=0.5.
$$

First morph:

$$
p_m
=
p+
0.5\Delta p_0
$$

$$
=
(0.2,1.05,0.0,1).
$$

---

### Skin influence

Vertex:

$$
joints=
(3,7,0,0)
$$

and:

$$
weights=
(0.7,0.3,0,0).
$$

Skin joint index 3 maps to node:

$$
J_3
$$

in skin's ordered joint array.

Palette:

$$
S_3=
G_{J_3}(t)
B_{J_3}^{-1}
$$

and:

$$
S_7=
G_{J_7}(t)
B_{J_7}^{-1}.
$$

Skin:

$$
\boxed{
p_s
=
0.7S_3p_m
+
0.3S_7p_m.
}
$$

---

### Entity/world transform

If asset-space result belongs to entity:

$$
M_{entity},
$$

world:

$$
\boxed{
p_w=
M_{entity}p_s.
}
$$

Then camera:

$$
p_v=
Vp_w.
$$

Clip:

$$
\boxed{
p_c=
Pp_v.
}
$$

NDC:

$$
p_{ndc}
=
p_c.xyz/p_c.w.
$$

Viewport maps to pixel.

That is the final rendered position.

---

### Previous-frame version for TAA

Now connect Pass 18 back to Pass 17.

Previous morph:

$$
m_{0,t-1}.
$$

Previous joint globals:

$$
G_{J,t-1}.
$$

Previous entity matrix:

$$
M_{entity,t-1}.
$$

Previous camera:

$$
P_{t-1}V_{t-1}.
$$

Compute:

$$
p_{c,t-1}.
$$

Current:

$$
p_{c,t}.
$$

Then motion vector:

$$
v=
NDC_t-NDC_{t-1}.
$$

So correct TAA motion for animated character requires every layer of the animation stack.

This is why rendering systems cannot treat animation as an unrelated subsystem.

---

### A COHERENT END-TO-END C++ MODEL

```cpp
// -----------------------------------------------------------------------------
// One portable animated-vertex evaluation.
// This is intentionally explicit to show the ordering of morphing, skinning,
// entity/world transforms, and camera projection.
// -----------------------------------------------------------------------------

struct SkinInfluence
{
    std::array<uint32_t, 4> Joint;
    std::array<float, 4> Weight;
};

Vec4 EvaluateAnimatedVertex(
    const Vec3& basePosition,
    std::span<const Vec3> morphPositionDeltas,
    std::span<const float> morphWeights,
    const SkinInfluence& influence,
    std::span<const Mat4> jointPalette,
    const Mat4& entityWorld,
    const Mat4& view,
    const Mat4& projection)
{
    // Start in mesh/object space.
    Vec3 morphed = basePosition;

    // glTF morph targets are additive deltas and are applied before skinning.
    const std::size_t targetCount =
        std::min(
            morphPositionDeltas.size(),
            morphWeights.size());

    for (std::size_t target = 0;
         target < targetCount;
         ++target)
    {
        morphed +=
            morphPositionDeltas[target]
            * morphWeights[target];
    }

    // Four-weight linear blend skinning.
    Vec4 skinned{0.0f};

    for (std::size_t slot = 0;
         slot < 4;
         ++slot)
    {
        const float weight =
            influence.Weight[slot];

        if (weight == 0.0f)
            continue;

        const uint32_t joint =
            influence.Joint[slot];

        if (joint >=
            jointPalette.size())
        {
            throw std::out_of_range(
                "Vertex references "
                "missing skin joint.");
        }

        skinned +=
            weight *
            (
                jointPalette[joint]
                *
                Vec4{
                    morphed,
                    1.0f
                }
            );
    }

    // The skin palette is asset-space in Kairo's current architecture.
    // The owning entity/world transform is applied outside it.
    const Vec4 world =
        entityWorld *
        skinned;

    const Vec4 viewPosition =
        view *
        world;

    const Vec4 clip =
        projection *
        viewPosition;

    return clip;
}
```

The joint palette generation feeding this function is:

```cpp
for each joint j:
{
    // Global animated joint transform obtained by walking the node hierarchy.
    Mat4 currentJointWorld =
        ResolveWorldTransform(
            jointNode[j],
            sampledPose);

    // Imported inverse-bind takes bind-space vertices into joint-local bind space.
    Mat4 skinMatrix =
        currentJointWorld
        *
        inverseBind[j];

    palette[j] =
        skinMatrix;
}
```

That is essentially Kairo EngineCore's current contract.

---


KairoAssets' editable mesh currently has stable vertex/face IDs, face polygons, edge validation, transactional editing, twin half-edge construction and non-manifold-edge rejection. Its half-edge `next` relationship is implicit through face corner order rather than stored explicitly.

Its current modeling stack includes real split/extrude/inset/dissolve/quad-strip operations and basic subdivision infrastructure. The current repo does **not** yet contain a QEM simplifier, cotangent Laplacian or production UV parameterization implementation, so those remain future implementation opportunities.

Kairo's glTF importer already separates file representation from portable runtime artifacts and handles materials, textures, images, nodes, skins, animation and morph structures. EngineCore evaluates animation and computes asset-space skin palettes; Renderer only needs portable influence streams and matrices. That separation is exactly what a scalable engine should do.

Maveb's glTF path similarly performs strict validation and has a useful tangent generator with explicit UV degeneracy handling and tangent handedness reconstruction. Its animation importer validates time monotonicity and quaternion norms rather than accepting malformed animation silently.

---

### THE CONNECTION BETWEEN PASS 17 AND PASS 18

These phases are not separate islands.

Animation generates:

$$
current\ geometry
$$

and:

$$
previous\ geometry.
$$

Those generate:

$$
motion\ vectors.
$$

Motion vectors feed:

$$
TAA,\ motion\ blur,\ temporal\ upscaling.
$$

Animated bounds feed:

$$
frustum\ culling,\ Hi-Z,\ shadow\ cascades,\ clustered\ light relevance.
$$

Morph/skinning changes shadow caster geometry.

Material/mesh import determines tangent frames used by PBR.

Entity transforms determine light-space positions.

So the real frame dependency is:

$$
\boxed{
Assets
\rightarrow
Animation
\rightarrow
Deformation
\rightarrow
Bounds
\rightarrow
Visibility
\rightarrow
Lighting/Shadows
\rightarrow
Post/Temporal
}
$$

not separate unrelated systems.

---

### RECONSTRUCTION GATE — PASS 17

From blank paper, derive a clustered renderer.

Starting with:

$$
W,H,T_x,T_y,N_z,n,f
$$

derive tile count, linear and logarithmic z slicing.

Show:

$$
slice(z)
=
N_z
\frac{
\ln(z/n)
}{
\ln(f/n)
}.
$$

Construct cluster volume.

Derive point-sphere vs AABB test.

Explain conservative spot-light bounding.

Compare dense:

$$
C\times L_{max}
$$

memory against CSR:

$$
C\times range+
R\times index.
$$

Derive count/scan/scatter.

Explain overflow.

Then design adaptive controller using:

$$
benefit/cost.
$$

Explain hysteresis:

$$
T_{enter}>T_{exit}.
$$

Then derive Hi-Z semantics for conventional and reversed-Z.

Then derive shadow map comparison, constant/slope bias, Peter-panning and PCF.

Derive CSM practical split:

$$
s_i=
\lambda
n(f/n)^{i/N}
+
(1-\lambda)
[n+(f-n)i/N].
$$

Explain stabilization by texel snapping.

Explain VSM moments.

Then distinguish AO from GI.

Explain probe leakage and visibility.

Explain SSR missing-data problem.

Explain radiance-cascade spatial/angular tradeoff.

Then derive motion vector:

$$
NDC_t-NDC_{t-1}.
$$

Show why previous joint palette is mandatory for skinned TAA.

Derive reprojection.

Explain depth/normal/ID rejection and neighborhood clamp.

Then explain bloom in HDR before tone map.

Finally derive a particle alive/dead compaction and a soft-particle depth fade.

If you can reconstruct that, Phase 26 is mastered.

---

### RECONSTRUCTION GATE — PASS 18

From blank paper, define the distinction:

$$
\text{topological vertex}
\neq
\text{GPU vertex}.
$$

Explain UV seam/hard-normal duplication.

Classify:

boundary, manifold, non-manifold edge.

Construct half-edge twin and next.

Traverse one-ring.

Derive:

$$
n_f=
normalize(
(p_1-p_0)\times(p_2-p_0)
).
$$

Compare equal, area and angle-weighted smooth normals.

Derive tangent from UV Jacobian and tangent handedness.

Parse OBJ negative indices and deduplicate tuple:

$$
(v,vt,vn).
$$

Then reconstruct glTF:

$$
buffer
\rightarrow
bufferView
\rightarrow
accessor
\rightarrow
primitive.
$$

Explain scene node hierarchy.

Explain skin joint indexing.

Derive QEM:

$$
Q=
\sum pp^T
$$

and:

$$
E(v)=v^TQv.
$$

Derive optimal collapse:

$$
Ax=-b.
$$

Explain Loop weights, Laplacian and cotangent Laplacian.

Then derive skeleton globals:

$$
G_j=
G_pL_j.
$$

Derive:

$$
S_j=
G_jB_j^{-1}.
$$

Derive four-weight LBS.

Explain candy-wrapper failure and DQS.

Derive lerp, slerp and cubic Hermite animation.

Explain crossfade, additive animation and root motion.

Then derive:

$$
p_m=
p+
\sum_km_k\Delta p_k.
$$

Explain why morphing precedes skinning.

Finally trace:

$$
\boxed{
glTF\ bytes
\rightarrow
accessor
\rightarrow
base\ vertex
\rightarrow
morph
\rightarrow
skin
\rightarrow
entity\ world
\rightarrow
camera
\rightarrow
clip
\rightarrow
pixel
}
$$

and repeat it for previous frame to obtain the TAA motion vector.

If you can do that without a reference, you understand the entire asset-to-rendered-motion path.

---

### STATUS

**Pass 17 complete — M151–M156, Parts 2501–2600.**

**Pass 18 complete — M157–M162, Parts 2601–2700.**

That means the complete **1801–2700 sequence we planned across the 18 passes is now finished**, including Phase 21 ray/path tracing, Phase 22 GPU architecture, Phase 23/24 modern API and multi-backend/render-graph design, Phase 25 Gaussian captured worlds, Phase 26 advanced real-time rendering, and Phase 27 mesh/asset/animation systems.


Graph connections: [[Radiance fields and Gaussian splatting]], [[Expectation, covariance, and dependence]], and [[Prefix scan and stream compaction]]. These link nodes supplement the inline derivations; the equations and examples remain in this book.



#### Check which transmittance the stopping test actually uses

The residual bound above assumes that the **actual unprocessed remainder** has transmittance below the stated cutoff. A test on the next candidate value is different. In the inspected [Graphdeco forward CUDA kernel](https://github.com/graphdeco-inria/diff-gaussian-rasterization/blob/main/cuda_rasterizer/forward.cu), the code computes a candidate transmittance, checks it against the cutoff, and can stop before accumulating that candidate layer or assigning the candidate transmittance. The stored transmittance can therefore remain above the cutoff. Reading the numeric constant alone does not establish the omitted-energy bound.

For example, let current transmittance be $T=0.009$ and the next opacity be $0.99$. The candidate is $0.00009$, below $10^{-4}$. If this layer is skipped when that test succeeds, a red layer over black loses a contribution of $0.00891$, which is much larger than $10^{-4}$. This is an arithmetic counterexample to applying the after-accumulation remainder bound to a before-accumulation stopping policy; it is not a measured artifact in a trained scene.

If candidate-based stopping uses $T(1-\alpha)<\tau$ and a proven opacity cap $\alpha\le a_{max}<1$, then the discarded current remainder instead satisfies

$$
T<\frac{\tau}{1-a_{max}}.
$$

With $a_{max}=0.99$, that worst-case bound is one hundred times the cutoff. It is loose but follows from the actual branch condition. A tighter implementation-specific bound must inspect the actual discarded layer and remainder. Footprint truncation and small-alpha rejection are additional approximation policies; they are not automatically covered by an early-termination allowance. Define whether the comparison target is an ideal untruncated compositor or the same renderer with a particular stop rule disabled.


For color edits, changing order and a complete fixed-order derivation, see [[Chapter Edition/03 - Game Engine/45 - Rasterizing kernels and connecting world identity#An image-change certificate must name the edits it covers|the image-change certificate contract]].

---

## From a rendered frame to a viewer's experience

The renderer now gives us a concrete image-formation and execution model. We can ask the next question: which parts of that image survive display sampling, optics, motion and perception? The following foundation develops those mechanisms using the mathematical and rendering tools already introduced. It is the preserved perceptual/XR section, relocated here so its prerequisites come first.

## A correct image must survive the viewer — perceptual rendering and XR

The workshop image is only the beginning of the measurement. Light passes through a camera or display, reaches an eye and is interpreted under changing gaze, contrast and motion. This full XR foundation is integrated here before later camera and rendering algorithms. Source claims remain explicitly distinguished from measured or implemented results.

---
tags:
  - continuous-notes
  - xr
  - independent-series
series: XR Foundations
source: XR01
curriculum: outside-3000-part-plan
---

<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/40 - Lighting, Reconstruction and Surface Motion|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/42 - XR Radiometry, Optics and Perception|Next chapter]]
