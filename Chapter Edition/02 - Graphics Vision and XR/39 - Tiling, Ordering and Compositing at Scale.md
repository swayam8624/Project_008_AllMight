# 39 — Tiling, Ordering and Compositing at Scale

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/38 - Projecting Ellipsoids into Screen-Space Conics|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/40 - Lighting, Reconstruction and Surface Motion|Next chapter]]

Many projected kernels must be assigned to tiles, ordered and composited efficiently. This span connects local footprints to scalable rasterization and identity-aware data flow.

**By the end:** Describe the tiling, sorting and compositing stages for large kernel sets.

**Recall question:** Why does ordering matter when overlapping kernels are composited?

<!-- Derived from Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography.md; source lines 87885–91348; source-span SHA256 794c6cd748226737828a41050186e0210a55bc768b09acdd8dfc53183d2d8c9a. -->

<!-- CHAPTER-SOURCE-BEGIN -->
## Rasterize kernels at scale and connect their identities

**Policy distinction:** the source opacity cutoff $A>0.999$ means remaining transmittance below $10^{-3}$. The inspected Graphdeco reference uses $10^{-4}$; these are separate accuracy/performance policies, as derived above.

### + M150

This pass completes **Phase 25**. The authoritative curriculum boundary is exact: **M148 owns Parts 2453–2474**, taking the projected conics from Pass 15 through tiling, prefix sums, reference expansion, ordering and alpha compositing; **M149 owns 2475–2486**, covering the CPU oracle, Metal implementation and quantitative CPU↔GPU validation; and **M150 owns 2487–2500**, where Gaussian splats stop being merely pixels and become an editable captured-world representation connected to persistent entities and physical degrees of freedom.

Pass 15 ended with one visible Gaussian represented approximately as:

$$
G_i^{screen}
=
\{
\mu_i^{2D},
z_i,
C_i=\Sigma_{2D,i}^{-1},
r_i,
\alpha_i,
c_i,
ID_i
\}.
$$

That representation answers:

> Where is the Gaussian and what is its elliptical pixel-space kernel?

It does **not** yet answer:

> Which of millions of Gaussians should every pixel examine, and in what order?

That is the problem M148 solves.

The complete high-performance pipeline becomes:

$$
\boxed{
\text{3D Gaussians}
\rightarrow
\text{project/cull}
\rightarrow
\text{tile-overlap counts}
\rightarrow
\text{exclusive scan}
\rightarrow
\text{tile references}
\rightarrow
\text{stable tile+depth sort}
\rightarrow
\text{tile ranges}
\rightarrow
\text{per-pixel front-to-back composite}
}
$$

Maveb currently implements essentially this entire chain in Metal.

---

### TILING, REFERENCE EXPANSION, SORTING AND COMPOSITING


Imagine:

$$
N=5,000,000
$$

Gaussians and:

$$
1920\times1080
\approx
2.07\times10^6
$$

pixels.

The absolutely naive algorithm is:

```cpp
for every pixel:
    for every Gaussian:
        test Gaussian
```

Cost:

$$
O(PN).
$$

Numerically:

$$
2.07\times10^6
\times
5\times10^6
\approx
10^{13}
$$

Gaussian-pixel tests.

Impossible for interactive rendering.

But each projected Gaussian typically affects only a small screen region.

So instead partition the screen into fixed tiles.

Maveb uses:

$$
\boxed{16\times16}
$$

pixel tiles.

---

**Tile-grid dimensions**

For image:

$$
W\times H
$$

and tile side:

$$
T,
$$

columns:

$$
\boxed{
N_x
=
\left\lceil
\frac WT
\right\rceil
=
\frac{W+T-1}{T}
}
$$

with integer division.

Rows:

$$
\boxed{
N_y
=
\frac{H+T-1}{T}.
}
$$

Total:

$$
N_{tiles}=N_xN_y.
$$

Maveb does:

```cpp
tileColumns =
    (width + 15) / 16;

tileRows =
    (height + 15) / 16;
```

because:

$$
T=16.
$$

For 1920×1080:

$$
N_x=120,
$$

$$
N_y=
\lceil1080/16\rceil
=
68.
$$

So:

$$
N_{tiles}=8160.
$$

Much smaller than pixel count.

---

**Why 16×16?**

There is no universal magical tile size.

Tradeoff:

Small tile:

$$
\text{fewer irrelevant Gaussians per pixel}
$$

but:

$$
\text{more tile references and scheduling metadata}.
$$

Large tile:

$$
\text{fewer references}
$$

but every pixel may iterate more irrelevant splats.

For projected Gaussian rasterization, 16×16 is a strong practical compromise and is also the tile size hardcoded in current Maveb:

```metal
constant uint
aetherGaussianTileSize = 16;
```

---


From Pass 15 we have screen-space center:

$$
(c_x,c_y)
$$

and conservative radius:

$$
r.
$$

Pixel bounds:

$$
x_{min}
=
\max(
0,
\lfloor c_x-r\rfloor
),
$$

$$
x_{max}
=
\min(
W-1,
\lceil c_x+r\rceil
).
$$

Likewise y.

Then tile coordinates:

$$
\boxed{
t_{x,min}
=
\left\lfloor
\frac{x_{min}}T
\right\rfloor
}
$$

and:

$$
\boxed{
t_{x,max}
=
\left\lfloor
\frac{x_{max}}T
\right\rfloor.
}
$$

Same for y.

Maveb computes exactly:

```metal
minimumTile =
    uint2(minimumPixel) / 16;

maximumTile =
    uint2(maximumPixel) / 16;
```

---

**Number of overlapped tiles**

If inclusive tile range:

$$
[t_{x0},t_{x1}]
\times
[t_{y0},t_{y1}],
$$

number of tiles touched:

$$
\boxed{
K_i
=
(t_{x1}-t_{x0}+1)
(t_{y1}-t_{y0}+1).
}
$$

Maveb stores this as:

```metal
overlap
```

and writes:

```metal
tileCounts[index] =
    overlap;
```

Important nuance:

Despite the name `tileCounts`, this buffer is initially indexed by **Gaussian**.

So:

$$
tileCounts[i]
=
\text{number of tile references Gaussian }i\text{ must generate}.
$$

It is not initially:

$$
\text{number of Gaussians in tile }t.
$$

That distinction matters.

---


Suppose:

$$
G_0
$$

touches 4 tiles,

$$
G_1
$$

touches 2,

$$
G_2
$$

touches 5.

Then reference counts:

$$
K=
[4,2,5].
$$

Total flattened references required:

$$
\boxed{
M=
\sum_i K_i
=
11.
}
$$

Why references rather than storing each Gaussian once?

Because one Gaussian may need to appear in several tiles.

The relation is many-to-many:

$$
Gaussian_i
\leftrightarrow
Tile_j.
$$

A Gaussian touching:

$$
20
$$

tiles must appear in:

$$
20
$$

tile work lists.

---

**Direct tile counting versus Maveb's Gaussian counting**

There are two broad constructions.

A traditional tile-centric approach first counts:

$$
count[t]
=
\text{number of Gaussians touching tile }t.
$$

Then scan tile counts.

Maveb instead counts:

$$
count[g]
=
\text{number of tiles touched by Gaussian }g.
$$

Then scans Gaussian counts to allocate one contiguous output region per Gaussian.

After all references are generated, it sorts them by tile and depth.

Both approaches ultimately produce:

$$
\text{tile}\rightarrow
\text{ordered Gaussian references}.
$$

Maveb's approach is especially natural because every projection thread already knows the entire tile rectangle of its Gaussian.

---


We now know how many references each Gaussian emits:

$$
K_0,K_1,\ldots,K_{N-1}.
$$

We need non-overlapping write ranges.

Exclusive prefix sum:

$$
O_0=0
$$

and:

$$
\boxed{
O_i=
\sum_{j=0}^{i-1}K_j.
}
$$

Then Gaussian $i$ writes references beginning at:

$$
O_i.
$$

Example:

$$
K=
[4,2,5,1].
$$

Exclusive scan:

$$
O=
[0,4,6,11].
$$

Writes become:

$$
G_0:[0,4)
$$

$$
G_1:[4,6)
$$

$$
G_2:[6,11)
$$

$$
G_3:[11,12).
$$

No atomic increment is needed for every emitted reference.

That is why prefix sum is such a central GPU primitive.

---

**Why not one global atomic counter?**

Could do:

```metal
uint destination =
    atomic_fetch_add(
        globalCounter,
        overlap);
```

once per Gaussian.

That can work.

But a scan gives deterministic offsets and avoids potentially severe contention around a global allocator.

More generally:

$$
\boxed{
\text{count}
\rightarrow
\text{scan}
\rightarrow
\text{scatter}
}
$$

is one of the canonical parallel-GPU patterns.

You already learned this in Phase 22.

Here it becomes a real renderer.

---

**Maveb's parallel scan**

Current default Metal path uses 256-thread blocks.

Each workgroup loads:

$$
256
$$

counts into:

```metal
threadgroup uint scratch[256];
```

Then performs a Blelloch-style exclusive scan.

Up-sweep:

$$
1,2,4,8,\ldots
$$

combines partial sums.

At the root:

$$
scratch[255]
$$

contains block total.

That total is saved to:

```text
blockSums[groupIndex].
```

Root then becomes:

$$
0.
$$

Down-sweep propagates exclusive prefixes.

Finally:

$$
offsets[index]
=
scratch[localIndex].
$$

---

**Why Blelloch has two phases**

Suppose values:

$$
[3,1,7,0,4,1,6,3].
$$

Up-sweep reduces tree toward total:

$$
25.
$$

But that alone does not provide every prefix.

Set root to zero.

Down-sweep distributes sum of all elements preceding each node.

Result:

$$
\boxed{
[0,3,4,11,11,15,16,22].
}
$$

That is an exclusive scan.

---

**Multi-block scan**

If Gaussian count exceeds 256, each workgroup gives only local prefixes.

Suppose block $b$ total:

$$
S_b.
$$

You need prefix of block totals:

$$
B_b
=
\sum_{k<b}S_k.
$$

Then each local offset:

$$
\boxed{
O_i=
O_i^{local}
+
B_{block(i)}.
}
$$

Maveb pipeline therefore executes:

```text id="4z8tmw"
Gaussian Block Scan
        ↓
Gaussian Block Prefix
        ↓
Gaussian Block Offset Propagation
```

This is exactly the hierarchical scan algorithm.

---

**Maveb has a deterministic serial fallback**

If hardware cannot provide required:

$$
256
$$

threads per threadgroup, Maveb keeps:

```text id="81ri4m"
Gaussian Serial Scan Fallback.
```

This is an important engineering principle:

$$
\boxed{
\text{correctness path remains executable independently of optimized path}.
}
$$

Its ADR explicitly preserves a bounded fallback while performance claims must use the parallel path.

---


CSR normally means Compressed Sparse Row.

For sparse matrix:

```text id="jurd6v"
rowOffsets
columnIndices
```

For Gaussian tiles, analogous structure is:

```text id="re9vks"
tileRanges
sortedGaussianIndices
```

For tile $t$:

$$
begin=
ranges[t].x
$$

$$
end=
ranges[t].y.
$$

Its splats are:

$$
sortedValues[
begin:end
].
$$

That is essentially CSR.

---

**Example**

Suppose final sorted tile references are:

```text id="usz654"
Tile 0 → G2 G7 G9
Tile 1 → G1
Tile 2 → G0 G4
```

Flatten:

```text id="m5jiq5"
values =
[
 G2,G7,G9,
 G1,
 G0,G4
]
```

Ranges:

$$
range_0=[0,3),
$$

$$
range_1=[3,4),
$$

$$
range_2=[4,6).
$$

Instead of:

```cpp
vector<vector<Gaussian>>
```

GPU sees two flat buffers.

Much better for memory access.

---

**Maveb builds ranges after sorting**

It initializes every tile range to:

$$
(0xffffffff,0).
$$

Then walks sorted keys.

When tile ID changes, it closes previous range and opens next one.

Thus:

```text id="sxtu5b"
sorted keys
→ contiguous same-tile runs
→ tile ranges
```

No separate per-tile vector allocation exists.

---


After scan, Gaussian $i$ has:

$$
destination=offset[i].
$$

Then loop over its tile rectangle:

```cpp id="4hi99e"
for tileY:
    for tileX:
        values[destination] =
            gaussianIndex;

        keys[destination] =
            MakeSortKey(
                tile,
                depth);

        ++destination;
```

Current Maveb does precisely this.

---

**Bounded capacity**

The total number of tile references can explode.

A huge Gaussian can touch thousands of tiles.

Millions of Gaussians can produce tens or hundreds of millions of references.

So Maveb gives the pipeline an explicit budget:

$$
maximumTileEntries.
$$

Default currently:

$$
4,194,304.
$$

Writes are guarded:

```metal
if (
    destination <
    maximumTileEntries)
{
    ...
}
```

Overflow amount is counted.

This is critical.

Never let a projected Gaussian field turn into an out-of-bounds GPU write.

---

**Saturated work**

Maveb's scan/prefix path tracks:

$$
requested
$$

versus:

$$
stored=
\min(requested,capacity).
$$

It increments:

```text id="hhf4kg"
overflowedEntries
```

for excess references.

So failure mode becomes:

$$
\boxed{
\text{bounded visual degradation + telemetry}
}
$$

rather than:

$$
\boxed{
\text{memory corruption}.
}
$$

That is production engineering.

---


Each tile reference needs two ordering dimensions:

$$
tileID
$$

and:

$$
depth.
$$

Ultimately we want:

```text id="rgchv5"
Tile 0:
    near → far

Tile 1:
    near → far

Tile 2:
    near → far
```

So logically key is:

$$
\boxed{
(tileID,depth).
}
$$

Maveb stores:

```metal
uint2 key
```

as:

```metal
key.x =
    as_type<uint>(depth);

key.y =
    tileID;
```

At first glance this looks reversed.

But radix-pass ordering makes it correct.

---

**Why positive float bits can serve as depth key**

IEEE-754 positive finite floats preserve numerical order under unsigned bit interpretation.

If:

$$
0<a<b,
$$

then for ordinary positive finite values:

$$
bits(a)<bits(b).
$$

Maveb's camera depth is positive:

$$
Z>near>0.
$$

Therefore:

```metal
as_type<uint>(depth)
```

can be sorted numerically as unsigned integer.

If negative depths were allowed, this trick would fail without sign-bit transformation.

But they have already been culled.

---


Suppose foreground A:

$$
C_A,\alpha_A
$$

and background B:

$$
C_B,\alpha_B.
$$

A over B gives:

$$
C=
\alpha_A C_A
+
(1-\alpha_A)
\alpha_B C_B.
$$

Reverse:

$$
C'=
\alpha_B C_B
+
(1-\alpha_B)
\alpha_A C_A.
$$

Generally:

$$
\boxed{
C\ne C'.
}
$$

Therefore alpha blending is not commutative.

Depth order matters.

---

**Concrete example**

Red A:

$$
C_A=(1,0,0),
\quad
\alpha_A=0.5.
$$

Blue B:

$$
C_B=(0,0,1),
\quad
\alpha_B=0.5.
$$

A in front:

$$
C=
0.5(1,0,0)
+
0.5\cdot0.5(0,0,1)
$$

$$
=
(0.5,0,0.25).
$$

B in front:

$$
C'=
(0.25,0,0.5).
$$

Very different.

Sorting is not optional.

---


One option:

Sort every visible Gaussian globally by depth:

$$
G_{near}\rightarrow G_{far}.
$$

Then each tile can reference Gaussians in that order.

This is conceptually simple.

But if we duplicate a Gaussian across tiles anyway, sorting expanded references can directly produce per-tile contiguous ranges.

Maveb sorts:

$$
(tile,depth)
$$

references.

That simultaneously groups by tile and orders by depth.

---

**Why per-tile ordering is enough**

A pixel only receives Gaussians whose footprints overlap its tile.

Therefore relative ordering of:

$$
G_A
$$

in tile 12 and:

$$
G_B
$$

in tile 98 is irrelevant.

They can never affect the same pixel.

So a global total ordering contains unnecessary ordering constraints.

The true requirement is:

$$
\boxed{
\text{all splats that may affect the same pixel must be correctly ordered}.
}
$$

Tile-local sort is a conservative way to ensure that.

---


Suppose two splats have equal depth:

$$
z_A=z_B.
$$

Their alpha blend order still potentially changes output.

If GPU sorting arbitrarily permutes equal keys each frame, you can get:

- flickering,
- nondeterministic captures,
- CPU/GPU mismatch,
- difficult debugging.

So sort should be stable.

A stable sort preserves earlier order among equal keys.

---

**Maveb's stability**

References are generated deterministically:

Gaussian index ascending.

Within each Gaussian, tile Y then tile X ascending.

The radix scatter uses prefix ranks preserving input order within each bucket.

Thus equal sort keys preserve initial relative order.

That gives deterministic tie behavior.

This matters much more than it first appears.

---


Comparison sort costs roughly:

$$
O(M\log M).
$$

Keys are fixed-width integers.

Radix sort can achieve:

$$
O(kM)
$$

where $k$ is number of digit passes.

---

**LSD radix sort**

Least-significant-digit radix sort processes low-order digit first.

For a multi-component composite key:

$$
(tile,depth),
$$

if stable sorting is used, sort **secondary** key first, then primary key.

We want tile primary, depth secondary.

So:

$$
\boxed{
\text{sort depth first}
\rightarrow
\text{sort tile second}.
}
$$

Because tile sort is stable, depth order inside each tile is preserved.

That is exactly Maveb's pass ordering.

---

**Maveb parallel radix**

Each `uint2` has:

$$
64\text{ bits total}.
$$

Default optimized path uses:

$$
4\text{-bit digits}.
$$

Each 32-bit component therefore needs:

$$
32/4=8
$$

passes.

Depth:

$$
8
$$

passes.

Tile:

$$
8
$$

passes.

Total:

$$
\boxed{16\text{ radix passes}.}
$$

The shader:

```metal
component =
    pass < 8
        ? key.x
        : key.y;
```

is therefore deliberate.

---

**One radix pass**

For one digit with:

$$
16
$$

possible buckets, Maveb performs:

```text id="asjrz1"
Histogram
↓
Global/group prefix offsets
↓
Stable scatter
```

Histogram determines how many elements belong to each bucket.

Prefix determines output starting address.

Stable scatter places each key/value at destination.

This is again:

$$
\boxed{
\text{count}
\rightarrow
\text{scan}
\rightarrow
\text{scatter}.
}
$$

The exact same GPU pattern appears again.

---

**SIMD-group acceleration**

Maveb's histogram uses:

```metal
simd_sum()
```

for bucket counts.

Scatter uses:

```metal
simd_prefix_exclusive_sum()
```

for rank inside the SIMD group.

For one element belonging to bucket $b$, local destination is:

$$
base_{group,b}
+
rank_{withinWorkgroup,b}.
$$

This avoids one atomic per sorted element.

---

**Why ping-pong buffers**

Sorting cannot safely scatter arbitrary elements in-place.

So Maveb owns:

```text id="t35rr1"
Keys A
Keys B

Values A
Values B.
```

Each pass:

$$
A\rightarrow B
$$

then:

$$
B\rightarrow A.
$$

After an even number of passes:

$$
16,
$$

result returns to A.

That is why range construction later reads:

```cpp
keysA_
```

and composite reads:

```cpp
valuesA_.
```

---

**Serial radix fallback**

Fallback uses:

$$
8\text{-bit digits}.
$$

Each 32-bit component:

$$
4
$$

passes.

Two components:

$$
8
$$

passes.

It is intentionally serial and deterministic.

Again:

$$
\text{fallback exists for correctness},
$$

not performance claims.

---


For pixel:

$$
p=
(x+0.5,y+0.5)
$$

and Gaussian center:

$$
\mu=(c_x,c_y),
$$

define:

$$
\boxed{
d=
p-\mu.
}
$$

So:

$$
d_x=
x+0.5-c_x,
$$

$$
d_y=
y+0.5-c_y.
$$

Maveb computes exactly this.

Pixel center convention:

$$
+0.5
$$

is important.

Do not evaluate at integer pixel corner unless the entire renderer is defined that way.

---


Conic:

$$
Q=
\Sigma^{-1}
=
\begin{bmatrix}
A&B\\
B&C
\end{bmatrix}.
$$

Mahalanobis squared distance:

$$
\boxed{
q=
d^TQd.
}
$$

Expanding:

$$
\boxed{
q=
Ad_x^2
+
2Bd_xd_y
+
Cd_y^2.
}
$$

Maveb:

```metal
distance =
    conic.x * dx * dx
  + 2 * conic.y * dx * dy
  + conic.z * dy * dy;
```

---


Gaussian kernel:

$$
\boxed{
g(d)
=
e^{-\frac12q}.
}
$$

If:

$$
q=0,
$$

then:

$$
g=1.
$$

If:

$$
q=1,
$$

then:

$$
g\approx0.6065.
$$

If:

$$
q=4,
$$

then:

$$
g=e^{-2}\approx0.1353.
$$

If:

$$
q=9,
$$

then:

$$
g=e^{-4.5}\approx0.0111.
$$

Maveb rejects:

$$
q>9.
$$

This is the 3σ ellipse.

---


Current code:

```metal
exp(
    -0.5f *
    distance)
```

is expensive relative to simple arithmetic, but only performed after:

- camera culling,
- screen culling,
- tile culling,
- 3σ conic rejection.

That ordering matters.

Do not compute exponent for obviously irrelevant splats.

---


Stored decoded opacity:

$$
\alpha_0.
$$

Pixel alpha:

$$
\boxed{
\alpha_i(p)
=
\alpha_{0,i}
e^{-q_i/2}.
}
$$

Maveb additionally caps:

$$
\boxed{
\alpha_i
=
\min(
0.99,
\alpha_{0,i}e^{-q/2}
).
}
$$

and rejects:

$$
\alpha_i<\frac1{255}.
$$

---

**Why cap at 0.99?**

Without cap, one Gaussian can produce:

$$
\alpha=1.
$$

Then transmittance becomes exactly:

$$
T=0.
$$

A cap preserves a small amount:

$$
T\ge0.01
$$

after one splat.

This avoids an individual primitive becoming mathematically perfectly opaque and improves stability around learned saturation.

It also mirrors standard 3DGS-style implementations where alpha is capped below one.

---


Start accumulated color:

$$
C_0=0.
$$

Start transmittance:

$$
T_0=1.
$$

For ordered Gaussian $i$:

$$
\boxed{
w_i=
T_i\alpha_i.
}
$$

Then:

$$
\boxed{
C_{i+1}
=
C_i
+
w_i c_i.
}
$$

Update transmittance:

$$
\boxed{
T_{i+1}
=
T_i(1-\alpha_i).
}
$$

That is the central 3DGS compositing recurrence.

---

**Accumulated opacity formulation**

Instead of storing $T$, Maveb stores accumulated opacity:

$$
A=1-T.
$$

Then:

$$
T=1-A.
$$

Contribution:

$$
\boxed{
w_i=
(1-A_i)\alpha_i.
}
$$

Update:

$$
A_{i+1}
=
A_i+w_i.
$$

Maveb code:

```metal
contribution =
    (1.0f - opacity)
    * alpha;

color +=
    contribution
    * gaussian.color;

opacity +=
    contribution;
```

Exactly equivalent.

---

**Prove opacity recurrence**

Suppose:

$$
A_i=1-T_i.
$$

Then:

$$
A_{i+1}
=
A_i
+
(1-A_i)\alpha_i.
$$

Substitute:

$$
1-T_i
+
T_i\alpha_i
$$

$$
=
1-T_i(1-\alpha_i).
$$

Thus:

$$
T_{i+1}
=
T_i(1-\alpha_i).
$$

Correct.

---


After $n$ splats:

$$
\boxed{
T_n
=
\prod_{i=0}^{n-1}
(1-\alpha_i).
}
$$

Therefore accumulated opacity:

$$
\boxed{
A_n
=
1-
\prod_i(1-\alpha_i).
}
$$

Contribution of splat $i$:

$$
\boxed{
w_i
=
\alpha_i
\prod_{j<i}
(1-\alpha_j).
}
$$

This formula is worth memorizing.

It says:

> A Gaussian contributes its own alpha multiplied by how much light remains visible through everything in front of it.

---

**Background**

After all splats, background contribution is:

$$
\boxed{
C_{bg}^{contribution}
=
T_{final}C_{bg}.
}
$$

Thus final color:

$$
\boxed{
C=
\sum_i
T_i\alpha_i c_i
+
T_{final}C_{bg}.
}
$$

Maveb CPU oracle adds exactly:

$$
(1-A)C_{background}.
$$

The Metal Gaussian target stores its accumulated Gaussian color/alpha and later scene composition can determine how it participates in the wider render.

---


If:

$$
T
$$

becomes tiny, remaining splats cannot substantially change pixel.

Maveb terminates when:

$$
A>0.999.
$$

Equivalent:

$$
\boxed{
T<0.001.
}
$$

Remaining maximum contribution from bounded color $C_{max}$:

$$
\le
0.001C_{max}.
$$

That gives an actual error interpretation.

---

**Early termination saves exactly the expensive tail**

Dense scenes often have many splats behind already-opaque foreground.

Without early out:

```text id="ss9s8z"
evaluate
exp
blend
evaluate
exp
blend
...
```

for potentially hundreds/thousands of hidden splats.

With front-to-back compositing, once $T$ is tiny:

$$
\boxed{\text{stop}.}
$$

This is one reason front-to-back ordering is operationally valuable.

Maveb counts:

```text id="7ar1nv"
earlyTerminations
```

in GPU statistics.

That is useful performance evidence.

---


Traditional “over” can also be evaluated back-to-front.

Start with background:

$$
C=C_{bg}.
$$

For splats from far to near:

$$
\boxed{
C
\leftarrow
\alpha_i c_i
+
(1-\alpha_i)C.
}
$$

This yields the same ideal result as front-to-back when order is exact and arithmetic exact.

---

**Why front-to-back is preferable here**

Back-to-front cannot easily stop because near splats have not yet been processed.

A background splat that looks important may later become completely occluded.

Front-to-back knows remaining transmittance directly.

So:

$$
\boxed{
\text{front-to-back}
\rightarrow
\text{natural early termination}.
}
$$

For millions of semi-transparent splats, this matters.

---


Define premultiplied sample color:

$$
\tilde C_i
=
\alpha_i c_i.
$$

Then front-to-back:

$$
C
\leftarrow
C+
T\tilde C_i.
$$

and:

$$
T
\leftarrow
T(1-\alpha_i).
$$

Premultiplication cleanly separates:

$$
\text{color energy}
$$

from:

$$
\text{remaining transparency}.
$$

Maveb effectively computes premultiplied contribution on the fly:

```metal
contribution *
gaussian.color
```

where:

$$
contribution=T\alpha.
$$

---

**Why straight-color interpolation causes edge problems**

If transparent colors are stored as straight RGB, fully transparent pixels can carry arbitrary RGB.

Filtering/interpolation may pull those hidden colors into visible borders.

Premultiplied representation satisfies:

$$
\alpha=0
\Rightarrow
\tilde C=0.
$$

This generally behaves better under filtering/compositing.

---


Naively:

$$
e^{-q/2}
$$

can underflow for huge:

$$
q.
$$

For float32, sufficiently negative exponents become subnormal then zero.

But Maveb does something better:

```metal
if (distance > 9)
    continue;

alpha =
    opacity *
    exp(-0.5 * distance);
```

Therefore exponent argument always lies:

$$
[-4.5,0]
$$

for evaluated samples.

No meaningful exponential underflow can occur.

This is a perfect example of:

$$
\boxed{
\text{algorithmic culling improves numerical robustness as well as performance}.
}
$$

---

**Transmittance underflow**

Another issue is:

$$
T=
\prod_i(1-\alpha_i).
$$

After many splats, $T$ may become extremely small.

But Maveb stops at:

$$
T<10^{-3}.
$$

So it never multiplies through thousands of values toward true floating-point underflow.

Again:

$$
\boxed{
\text{early termination prevents both wasted work and pathological numerics}.
}
$$

---

### THE M148 DATAFLOW IN CURRENT MAVEB

The current Metal pipeline owns:

```text id="f7zvxr"
Projected Gaussians
Tile Counts
Tile Offsets
Scan Block Sums
Radix Histograms
Radix Group Offsets
Indirect Dispatch
Keys A/B
Values A/B
Tile Ranges
Counters
```

and executes:

```text id="1ly8ek"
Counter Clear
↓
Gaussian Projection
↓
Block Scan
↓
Block Prefix
↓
Block Offset Propagation
↓
Key Generation
↓
16×
    Radix Histogram
    Radix Group Prefix
    Stable Scatter
↓
Range Initialization
↓
Range Construction
↓
Tile Composite
```

This is not a conceptual example.

It is the inspected Maveb production path.

---

### CPU ORACLE, METAL EXECUTION AND QUANTITATIVE AGREEMENT


Maveb's CPU oracle first projects all valid Gaussians.

Then:

```cpp
std::ranges::stable_sort(
    projected,
    {},
    &Projected::depth);
```

Since reference camera uses:

$$
+Z
$$

forward and smaller positive $Z$ is closer:

$$
\boxed{
\text{ascending depth}
=
\text{front-to-back}.
}
$$

Stable sort preserves source order for equal depth.

This is deliberately compatible with deterministic GPU ordering.

---

**Why a CPU oracle should be simple**

The CPU reference implementation should optimize primarily for:

$$
\boxed{
\text{clarity + determinism + correctness}.
}
$$

Not maximum performance.

If CPU and GPU share the same clever algorithm and same bug, comparison proves nothing.

A useful oracle should be independent enough that implementation mistakes are unlikely to correlate.

Maveb's CPU rasterizer does not run the GPU's radix pipeline.

It globally stable-sorts projected records, then rasterizes them.

Good.

---

**Yet CPU still avoids pathological $P\times N$**

Maveb partitions the image into horizontal:

$$
16\text{-row bands}.
$$

For each projected Gaussian it computes pixel AABB and inserts a `BandEntry` into every intersected horizontal band.

Then each band processes only relevant Gaussians.

This is not the exact same 16×16 tile algorithm as GPU.

That difference is useful.

---

**Why deterministic threading survives**

Global projected list is already sorted.

Band lists receive Gaussians in that sorted order.

Every band owns disjoint rows.

So threads never concurrently update the same pixel.

Thus:

$$
\boxed{
\text{thread scheduling does not affect pixel accumulation order}.
}
$$

This gives deterministic multi-threaded CPU output.

That is an excellent oracle design.

---


A CPU implementation could use the same conceptual tile ranges:

```cpp id="tfx8be"
for each tile:
{
    const auto [begin,end] =
        ranges[tile];

    for each pixel in tile:
    {
        float T = 1.0f;
        Vec3 color{};

        for (
            reference = begin;
            reference < end;
            ++reference)
        {
            evaluate splat;

            if (outside ellipse)
                continue;

            ...

            if (T < threshold)
                break;
        }
    }
}
```

This has excellent cache locality because one tile's Gaussian list is shared by its pixels.

Maveb's oracle currently uses horizontal bands instead because it is simpler and remains independent from GPU scheduling machinery.

That independence is valuable for validation.

---


Current shared CPU/MSL ABI:

```cpp id="zxyqtm"
struct AetherGaussianGpu
{
    float4 positionOpacity;
    float4 logScaleRestCount;
    float4 rotation;
    float4 dc;
    float4 shRest[12];
};
```

Size:

$$
\boxed{256\text{ bytes}.}
$$

Why 12 `float4`s?

There are:

$$
45
$$

possible higher SH floats.

Twelve float4s provide:

$$
48
$$

float slots.

Three padding positions remain.

This keeps 16-byte alignment straightforward.

---

**Packed semantic fields**

`positionOpacity`:

$$
(x,y,z,opacityLogit).
$$

`logScaleRestCount`:

$$
(\ell_x,\ell_y,\ell_z,restCount).
$$

`rotation`:

$$
(w,x,y,z).
$$

`dc`:

$$
(R_{DC},G_{DC},B_{DC},padding).
$$

`shRest[12]` stores the 45 higher coefficients.

This is a GPU-friendly fixed-size record.

---


Projection:

$$
\boxed{
1\ GPU\ thread
\leftrightarrow
1\ Gaussian.
}
$$

Kernel receives:

```metal
uint index
[[thread_position_in_grid]]
```

and exits if:

$$
index\ge GaussianCount.
$$

This is embarrassingly parallel:

Each Gaussian independently computes:

- camera-space center,
- covariance,
- conic,
- radius,
- SH color,
- tile bounds,
- overlap count.

Only shared counter:

$$
visibleGaussians
$$

needs atomic increment.

---

**Projection threadgroup width**

Maveb's generic `dispatch1D()` chooses:

$$
\min(
pipeline.maxTotalThreadsPerThreadgroup(),
256
).
$$

This is a conservative implementation.

A more performance-tuned path might consider:

$$
threadExecutionWidth
$$

and occupancy, but 256 is a conventional GPU workgroup size.

---

**Scan mapping**

Scan:

$$
256\text{ threads/workgroup}.
$$

One element:

$$
\leftrightarrow
$$

one Gaussian count.

Threadgroup shared scratch:

$$
256\times4
=
1024\text{ bytes}.
$$

Very cheap.

---

**Radix mapping**

Radix histogram/scatter:

$$
256
$$

threads/group.

SIMD-group intrinsics compute local bucket rank/count.

This is exactly the SIMT material from Phase 22 applied to a real engine workload.

---

**Composite mapping**

Current Metal composite dispatch:

```cpp
dispatchThreads(
    {width,height,1},
    {8,8,1});
```

Therefore:

$$
\boxed{
1\ GPU\ thread
\leftrightarrow
1\ pixel.
}
$$

Each workgroup contains:

$$
8\times8=64
$$

threads.

---

**Relationship between 8×8 compute groups and 16×16 Gaussian tiles**

Gaussian scheduling tile:

$$
16\times16.
$$

Compute workgroup:

$$
8\times8.
$$

Therefore one Gaussian tile covers:

$$
2\times2
$$

compute workgroups.

All four workgroups independently calculate the same tile ID and read the same:

$$
range[tile].
$$

That is correct.

The scheduling tile size does not have to equal compute threadgroup dimensions.

They solve different problems.

---


Current projected record:

```cpp id="2lb4al"
struct AetherProjectedGaussian
{
    float4 centerDepthRadius;
    float4 conicOpacity;
    float4 color;

    uint4 tileBounds;
    uint4 sourceCountValid;
};
```

Size:

$$
\boxed{80\text{ bytes}.}
$$

---

**Field semantics**

$$
centerDepthRadius
=
(c_x,c_y,z,r).
$$

$$
conicOpacity
=
(A,B,C,\alpha_0).
$$

$$
color
=
(R,G,B,1).
$$

`tileBounds` contains:

$$
(minX,minY,maxX,maxY)
$$

in tile coordinates.

`sourceCountValid` currently stores:

$$
(sourceIndex,\ overlapCount,\ valid,\ SHdegree).
$$

This is a well-designed projection product because composite kernel no longer needs:

- world-space covariance,
- quaternion,
- log scale,
- SH coefficients,
- camera projection math.

All that expensive work was amortized once per Gaussian.

---

**Projection is a representation-lowering stage**

This resembles compiler lowering.

Rich representation:

```text id="jyhwui"
3D mean
quaternion
scale
SH coefficients
opacity logit
```

becomes:

```text id="z54oi2"
2D center
depth
conic
opacity
color
tile bounds
```

The second form is exactly what rasterization needs.

This is why GPU pipelines benefit from explicit intermediate representations.

---


There are at least two broad ways to splat.

Raster approach:

Create one quad per projected Gaussian.

Rasterize quad.

Fragment shader evaluates Gaussian.

Use blending.

Compute approach:

Tile/bin Gaussians.

Dispatch one thread per pixel.

Explicitly loop ordered splats and perform compositing.

Maveb currently chooses compute.

---

**Raster splat advantages**

Hardware rasterization gives:

- coverage generation,
- primitive setup,
- blend units,
- easy quad projection.

A simple Gaussian renderer can be built quickly this way.

Vulkax even contains projected-raster paths using generated quad geometry.

---

**Raster disadvantages**

Correct ordering for millions of translucent splats remains difficult.

Blend hardware cannot magically solve arbitrary per-pixel ordering.

A global splat order may work approximately but has problems with overlapping anisotropic geometry and per-tile ordering.

You may also pay:

- huge quad overdraw,
- fragment invocation outside true ellipse,
- per-splat draw/instance management.

---

**Compute advantages**

Compute lets renderer explicitly control:

$$
\text{tile list}
$$

$$
\text{ordering}
$$

$$
\text{early termination}
$$

$$
\text{debug IDs/depth}
$$

$$
\text{overflow handling}.
$$

This makes it an excellent fit for serious 3DGS rasterization.

---

**Compute disadvantage**

You implement much more yourself:

- scan,
- sort,
- ranges,
- pixel loops,
- synchronization.

Which is precisely why Maveb has 13 compute pipelines for this subsystem.

---


Dependencies:

```text id="sd09sx"
Projection writes:
    projected
    counts

Scan reads counts
writes offsets

Key generation reads:
    projected
    offsets
writes:
    keys
    values

Radix reads/writes:
    keys
    values

Range construction reads keys
writes ranges

Composite reads:
    projected
    sorted values
    ranges

Composite writes:
    color/depth/ID textures
```

These are real RAW dependencies.

---

**Current Metal synchronization strategy**

Maveb ends the compute encoder after each stage.

Then creates the next encoder on the **same command buffer**.

The relevant buffers use normal tracked Metal hazard semantics; the pipeline is not using an explicit untracked-memory scheme here.

Thus Metal orders and tracks producer→consumer dependencies across encoders.

There is no long sequence of Vulkan-style manual barriers in `GaussianPipeline::encode()`.

That does **not** mean synchronization is absent.

It means:

$$
\boxed{
\text{dependency is expressed through ordered encoders + Metal resource hazard tracking}.
}
$$

Exactly the Metal-vs-Vulkan distinction from Pass 13.

---

**Why separate encoders are convenient here**

Each stage has a clear boundary:

```text id="9acmg5"
Projection encoder ends
↓
Scan encoder starts
```

so Metal can understand that a producer phase precedes consumer phase.

Within one compute encoder, if dispatches read data written by previous dispatch without sufficient implicit scope, memory-barrier semantics may be needed.

Maveb's design avoids ambiguity by giving major stages separate encoders.

---


A serious GPU renderer needs an oracle.

Test conditions should use identical:

$$
\text{Gaussian asset},
$$

$$
\text{camera intrinsics/extrinsics},
$$

$$
\text{viewport},
$$

$$
\text{background},
$$

$$
\text{SH convention},
$$

$$
\text{alpha thresholds}.
$$

Then render:

$$
I_{CPU}
$$

and:

$$
I_{GPU}.
$$

Compare in:

$$
\boxed{\text{linear color space}.}
$$

Do not compare after arbitrary tone mapping if testing the Gaussian integrator itself.

---

**Maveb's current correctness philosophy**

The repository's accepted Metal Gaussian ADR says the standard renderer includes:

- projection,
- anisotropic covariance,
- culling,
- overlap counts,
- exclusive offsets,
- key generation,
- stable ordering,
- tile ranges,
- front-to-back composition,
- depth,
- IDs,
- early termination,
- bounded overflow.

It also specifies a Metal test spanning more than one scan/radix block with shuffled depths and varying colors, compared against the CPU oracle.

That is exactly the kind of test needed.

I did **not** find a current repository implementation named specifically around all four curriculum metrics—RMSE, PSNR, maximum error and changed-pixel fraction—in the inspected Gaussian test paths, so the formulas below are the validation methodology required by the curriculum rather than a claim that Maveb already exposes each metric as a production API.

---


For two images with:

$$
N
$$

scalar channel samples:

$$
e_i=
G_i-R_i.
$$

MSE:

$$
\boxed{
MSE=
\frac1N
\sum_{i=1}^{N}
e_i^2.
}
$$

RMSE:

$$
\boxed{
RMSE=
\sqrt{
\frac1N
\sum_i
(G_i-R_i)^2
}.
}
$$

Units are same as image values.

If image is linear normalized RGB, RMSE of:

$$
10^{-3}
$$

means average magnitude of numerical difference is small.

But exact acceptable threshold depends on scene, precision and expected backend differences.

Do not invent universal threshold.

---

**RGB RMSE**

For $P$ pixels:

$$
\boxed{
RMSE_{RGB}
=
\sqrt{
\frac{
\sum_{p}
[
(\Delta R_p)^2+
(\Delta G_p)^2+
(\Delta B_p)^2
]
}{
3P
}
}.
}
$$

You may separately compute alpha RMSE.

Depth should have its own metric.

IDs should generally be discrete agreement, not RMSE.

---


For range:

$$
[0,MAX],
$$

PSNR:

$$
\boxed{
PSNR
=
10\log_{10}
\frac{MAX^2}{MSE}.
}
$$

If:

$$
MAX=1,
$$

then:

$$
PSNR
=
-10\log_{10}(MSE).
$$

If MSE:

$$
10^{-4},
$$

then:

$$
PSNR=40\text{ dB}.
$$

If:

$$
10^{-6},
$$

then:

$$
60\text{ dB}.
$$

Higher is better.

---

**PSNR caveat with HDR**

If HDR values exceed:

$$
1,
$$

what is:

$$
MAX?
$$

You must define it.

Possible approaches:

- fixed physical/reference range,
- measured reference peak,
- tone-mapped LDR metric.

Do not blindly use:

$$
MAX=1
$$

for unclamped HDR.

For backend numerical validation, RMSE plus max error is often less ambiguous.

---


Define:

$$
\boxed{
E_{\max}
=
\max_{p,c}
|G_{p,c}-R_{p,c}|.
}
$$

RMSE can hide one catastrophic pixel.

Example:

One pixel error:

$$
1
$$

among a million perfect pixels gives small RMSE.

But max error reveals:

$$
1.
$$

So both matter.

---

**Location of max error**

Do not store only scalar max.

Store:

```cpp id="vgsyyh"
struct MaxError
{
    float Error;
    uint32_t X;
    uint32_t Y;
    uint32_t Channel;
};
```

Then debugger can inspect:

- Gaussian ID,
- depth,
- tile,
- sort rank,
- conic.

That turns a metric into a diagnostic.

---


Choose threshold:

$$
\epsilon.
$$

Define changed indicator:

$$
I_p=
\begin{cases}
1,&
\max_c|\Delta_{p,c}|>\epsilon\\
0,&
\text{otherwise}.
\end{cases}
$$

Then:

$$
\boxed{
F_{changed}
=
\frac1P
\sum_p I_p.
}
$$

This distinguishes:

> tiny errors everywhere

from:

> large/localized errors in one region.

---

**Why RMSE + max + changed fraction is much more informative**

Case A:

Every pixel differs by:

$$
10^{-4}.
$$

Low RMSE.

Low max.

Changed fraction maybe 100% for tiny epsilon.

Likely systematic floating-point variation.

Case B:

Only 1% pixels wrong by:

$$
0.5.
$$

RMSE might still look moderate.

Max huge.

Changed fraction 1%.

Likely culling/boundary/sorting issue.

Metrics together reveal error structure.

---

**Validate semantic buffers separately**

Maveb already produces:

$$
color,
depth,
ID.
$$

Its debug modes expose:

```text id="zp6hzo"
appearance
logarithmic depth
source ID
tile occupancy
accumulated opacity
SH bands
sort rank
```

That is excellent for fault isolation.

Color mismatch with matching depth/ID suggests:

$$
SH/alpha.
$$

ID mismatch suggests:

$$
ordering/culling.
$$

Tile occupancy mismatch suggests:

$$
projection/bounds/scheduler.
$$

Sort-rank mismatch suggests:

$$
key/radix/ranges.
$$

---

**ID validation**

Away from equal-depth/raster-boundary ambiguities, dominant source ID should often match exactly:

$$
\boxed{
ID_{CPU}(p)=ID_{GPU}(p).
}
$$

A color tolerance cannot excuse a completely different dominant Gaussian unless there is a documented near-tie.

This is an extremely strong validation signal.

---

**Depth validation**

Maveb stores depth of first accepted contributing splat:

CPU:

```cpp
if depth == infinity:
    depth =
        gaussian.depth;
```

GPU:

```metal
if (!isfinite(depth))
    depth =
        gaussian.centerDepthRadius.z;
```

So CPU/GPU should agree closely if sort and alpha thresholds agree.

If they do not:

$$
\boxed{
\text{suspect order before shading}.
}
$$

---


<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/38 - Projecting Ellipsoids into Screen-Space Conics|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/40 - Lighting, Reconstruction and Surface Motion|Next chapter]]
