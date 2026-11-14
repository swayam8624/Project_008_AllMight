# 29 — Acceleration Structures and Robust Queries

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/28 - Visibility Queries and Ray Intersections|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/30 - Mesh Operators, Boundaries and Diffusion|Next chapter]]

Acceleration structures reduce work only when their bounds and traversal rules preserve query correctness. Robust predicates and invariants keep that optimization from changing the answer.

**By the end:** State BVH invariants and connect them to robust geometric queries.

**Recall question:** How can a bounding-volume hierarchy speed up a query without changing its result?

<!-- Derived from Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography.md; source lines 73707–74455; source-span SHA256 9083775d28488cff20db24e36749a3ebbba967df818f0b4fac9a330d26ae94ca. -->

<!-- CHAPTER-SOURCE-BEGIN -->
## Acceleration structures and robust queries

Acceleration stores reusable spatial bounds so queries can avoid most scene geometry while preserving correctness.

### Geometric queries and acceleration structures

A ray is $r(t)=o+td$, with origin $o$, direction $d$, and parameter $t\ge0$. To intersect triangle $a,b,c$, solve

$$
o+td=a+u(b-a)+v(c-a).
$$

The unknowns are $t,u,v$. A forward hit must satisfy $t\ge0$, $u\ge0$, $v\ge0$, and $u+v\le1$. The barycentric weights are $(1-u-v,u,v)$. A degenerate triangle makes the system singular. A ray exactly on a shared edge can be inconsistently classified by floating-point arithmetic, so robust tests need a consistent edge rule or conservative predicates. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-12 - Geometric Queries|CMU-12]]

Closest-point queries project a point onto a triangle plane, then clamp to the appropriate edge or vertex when the projection is outside the face. Signed distance adds a sign indicating inside versus outside, often determined by orientation or a winding test. These queries occur in collision detection, surface reconstruction, and modeling. The mathematical answer should be independent of the data structure used to accelerate it. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-12 - Geometric Queries|CMU-12]]

Naively testing a ray against every triangle costs $O(n)$ operations for $n$ faces. A bounding-volume hierarchy groups primitives inside boxes. A ray-box slab test computes an interval of possible ray parameters for each coordinate axis; intersect the three intervals, rejecting when their common interval is empty. If a ray is parallel to a slab and its origin is outside, reject; if inside, that axis imposes no extra restriction. The bound must be conservative: a false positive costs time, but a false negative loses a real hit. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-13 - Spatial Data Structures|CMU-13]]

BVH traversal descends only into boxes hit by the ray. Visiting the nearer child first can find a closest hit early; any later node whose nearest possible hit is farther can be pruned. A shadow query can stop at its first blocker, while a transparent visibility query may need several hits. Uniform grids, k-d trees, and BVHs suit different geometry distributions and update patterns. A BVH can be refit after small motion or rebuilt for tighter bounds. Acceleration changes the cost, not the result, of a query. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-13 - Spatial Data Structures|CMU-13]]

### Bounding-volume hierarchy invariants

A BVH is a tree. Every leaf contains one or more primitives; every interior node has children whose primitive sets partition its own. The children’s boxes may overlap spatially, but a primitive belongs to only one child set. A node bound encloses every descendant primitive. This containment property ensures that a ray missing a node cannot hit any geometry below it. The split heuristic affects speed, not correctness. A [[Bounding volume hierarchies|surface-area heuristic]] estimates expected traversal cost by balancing the cost of testing a node against the probability that a ray visits a child.

For an axis-aligned box with coordinate bounds $[b_i^{min},b_i^{max}]$, a ray coordinate is $o_i+td_i$. If $d_i>0$, the slab-entry and exit parameters are $(b_i^{min}-o_i)/d_i$ and $(b_i^{max}-o_i)/d_i$. If $d_i<0$, their ordering reverses. Taking the maximum of entry parameters and minimum of exit parameters across axes gives the intersection interval. The ray hits if the interval is nonempty and overlaps allowed $t$ values. Parallel components require explicit treatment to avoid division by zero. Floating-point implementations often use reciprocal directions and precomputed sign bits, but must preserve conservative behavior near boundaries. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-13 - Spatial Data Structures|CMU-13]]

Dynamic geometry can update a BVH by refitting node boxes from child bounds. Refit preserves correctness but can leave poor partitions after large deformation. Rebuilding costs more immediately but may produce tighter bounds and faster future traversal. If a scene receives many rays between updates, rebuilding can pay off; for rapidly changing scenes with few rays, a quick refit may be preferable. This is a workload decision. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-13 - Spatial Data Structures|CMU-13]]

### Data structures shape the algorithm

A uniform grid maps each primitive to intersected cells. Ray traversal steps cell by cell, but a large primitive may occupy many cells and produce duplicate tests. A k-d tree partitions space with planes; splitting a primitive can require storing it in multiple leaves. A BVH partitions primitives into disjoint sets while allowing spatially overlapping bounds. This is especially useful when objects are clustered or empty space is irregular. Different acceleration structures are not interchangeable abstractions in performance terms, though each should produce the same geometric answer.

The choice of query and structure should be tested against expected rays. Primary camera rays tend to be coherent; neighboring rays point in similar directions. Shadow rays point toward light sources. Diffuse bounce rays are more scattered. A structure optimized for one ray distribution may perform differently for another. This is a general systems lesson: data structures encode assumptions about future queries. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-13 - Spatial Data Structures|CMU-13]]

### Robustness and conditioning are related but different

Conditioning describes how sensitive the exact mathematical answer is to input changes. Algorithmic stability describes whether a numerical method adds additional error beyond that sensitivity. Nearly parallel line intersection is ill-conditioned: a tiny angular change can move the intersection far away. Higher precision cannot make the geometric problem well-conditioned, but a stable method can avoid making it worse. For a well-conditioned problem, a numerically unstable method can still return a poor answer. Robust geometry needs both an understanding of conditioning and careful arithmetic.

Orientation predicates, such as determining whether one point lies to the left or right of a directed edge, often reduce to the sign of a determinant. Near zero, roundoff can flip the sign, causing inconsistent topology decisions. Adaptive precision evaluates cheaply in ordinary cases and uses more precision only when the result is uncertain. Such predicates matter in triangulation, ray-edge tests, and mesh repair. The point is not to eliminate all floating-point error; it is to prevent uncertain local calculations from violating global invariants. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-12 - Geometric Queries|CMU-12]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-13 - Spatial Data Structures|CMU-13]]

### Acceleration: spend scene work once so each ray can skip most geometry

A ray tracer's brute-force inner loop compares each ray with every primitive. With $P$ image samples, $N$ triangles, and an average of $R$ rays generated per sample (including shadow and secondary rays), a crude work model is $O(PRN)$. If an object contains $1{,}300$ triangles, a bounding-volume miss can save all $1{,}300$ triangle tests for that ray. The bounding volume must be conservative: it may contain extra empty space, but it cannot exclude any part of the object. A loose bound wastes work; a costly, tight bound can cost more than it saves. Construction is amortized across queries: if setup costs $B$ and it saves $s$ work per ray, it becomes worthwhile after about $B/s$ rays. This changes for a deforming animated object, where bounds may need refitting or rebuilding each frame; a wider bound can instead cover several poses at the cost of weaker culling. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-12 - Ray tracing II|MITG-12]] (00:07:00–00:20:00)

For an axis-aligned bounding box, store minimum corner $b_{\min}$ and maximum corner $b_{\max}$. Along each axis, the box defines a slab interval. For one coordinate $x(t)=o_x+td_x$, intersect the two planes $x=b_{\min,x}$ and $x=b_{\max,x}$ to get two parameters; sort them as $t_{x,\min}$ and $t_{x,\max}$. Repeat for $y$ and $z$. The ray enters the box at the largest of the three entry parameters and exits at the smallest of the three exit parameters:

$$
t_{\mathrm{enter}}=\max(t_{x,\min},t_{y,\min},t_{z,\min}),\qquad
t_{\mathrm{exit}}=\min(t_{x,\max},t_{y,\max},t_{z,\max}).
$$

The intervals overlap if $t_{\mathrm{enter}}\leq t_{\mathrm{exit}}$. For a forward ray, also require $t_{\mathrm{exit}}\geq0$; when recursively testing a subregion, intersect with its inherited interval $[t_{\mathrm{start}},t_{\mathrm{end}}]$. If one direction component is zero, the ray is parallel to that pair of slab planes: it misses if its origin lies outside the slab, otherwise that axis places no bound on $t$. This is the slab method and it reduces an AABB query to a few min/max operations. A triangle bound is formed by coordinatewise minima/maxima of its three vertices; a sphere centered at $c$ with radius $R$ has bounds $c\pm(R,R,R)$. An infinite plane has no useful finite AABB. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-12 - Ray tracing II|MITG-12]] (00:20:30–00:30:00)

A bounding volume hierarchy (BVH) groups primitives under a tree of conservative boxes. Each primitive belongs to one leaf, while sibling boxes may overlap. Each internal node has the bound of its descendants. To query, intersect a node box; if it misses, discard the entire subtree. If it hits, visit its children, then test primitives in reached leaves. The data structure is useful only when bounds prune enough work: a camera inside a single bunny-sized box, or a million identical triangles, may yield little improvement. Simple split rules divide a list in half, sort by centroid and take the median, or reuse an artist's modeling hierarchy. The lecture stresses that the common constructions are heuristics, not a proof of $O(\log N)$ traversal. A balanced-looking tree can still overlap heavily or be poorly aligned to actual rays. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-12 - Ray tracing II|MITG-12]] (00:30:00–00:38:30; 00:45:45–00:46:40)

A BVH must not stop merely because it finds a primitive in the first child box it intersects. Sibling boxes can overlap: a ray may enter the right sibling's box first, hit a primitive geometrically assigned to that box, and still encounter a nearer primitive assigned to the left sibling. The safe general rule is to examine all candidate children, retain the least valid $t$, and prune only when a node's near bound is farther than the best hit already found. The BVH stores each object once, so avoiding duplicate primitive tests costs little memory. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-12 - Ray tracing II|MITG-12]] (00:38:30–00:46:40)

A $k$-d tree makes a different trade. It partitions space with axis-aligned split planes, so the cells do not overlap, but a triangle crossing a split may be stored in both children. A node records the split axis, split coordinate, and front/back children or leaf primitives. This avoids overlapping spatial regions and enables near-to-far traversal, at the expense of duplicated storage and possibly repeated primitive tests. There is no universal construction rule: splitting the world cell in half can leave one child full and one almost empty; splitting by median primitive centroid balances counts but not necessarily ray cost. The surface area heuristic (SAH) estimates traversal cost from the areas of candidate child regions and the number of primitives assigned to them, reflecting the likelihood that rays visit those regions. It is a useful engineering heuristic rather than a guarantee. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-12 - Ray tracing II|MITG-12]] (00:46:40–00:54:10; 01:01:45–01:05:10)

Traversal correctness comes from parameter intervals. At a node, let the parent region be intersected by the ray over $[t_s,t_e]$. Find the ray parameter $t_m$ at the split plane. If $t_m<t_s$, the ray reaches only the back child in this interval. If $t_m>t_e$, it reaches only the front child. Otherwise it reaches both; determine which is near from the ray direction, recurse into that child over its subinterval first, and search the far child only if no valid hit was found before $t_m$. A primitive duplicated into both cells may have a geometric hit outside the current cell interval; reject such a hit here, or the first cell could report a surface that lies beyond the split and incorrectly hide a nearer object. The parent bounds and inherited interval are therefore not bookkeeping trivia—they are what make early termination safe. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-12 - Ray tracing II|MITG-12]] (00:50:45–01:01:30)

A regular grid provides another organization. Insert each primitive into every cell its bound or geometry overlaps. Traverse the cells in the order the ray crosses them and test only their contents. The 3D digital differential analyzer (DDA) keeps the next crossing parameter for each axis. From current cell entry $t$, advance to the smallest next boundary time, update the corresponding cell index, and update that axis's next time by a constant increment. For example, if the next x face is at $t_x=2.4$ and the next y face is at $t_y=2.1$, advance y first; then compare the updated times again. Stop once a valid primitive hit lies within the current cell's interval. A primitive spanning several cells can be tested repeatedly unless the implementation marks already-tested primitives. Tiny cells increase walking overhead; large cells put many primitives in each bucket. Uniform grids suit scenes with roughly uniform object scale, and their ordered loops can be simpler to parallelize than recursive trees. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-12 - Ray tracing II|MITG-12]] (01:09:20–01:18:50)

### Additional geometry notes for rasterization

These lecture examples are integrated by topic. The 2009 interface material below is explicitly historical; it is not a modern OpenGL core recipe.

#### Visibility: compare depth locally or trace paths

After projection, screen x and y say where a sample lands; depth says which opaque surface is in front. A depth buffer stores one depth per pixel next to the color buffer. When a triangle rasterizes a pixel, interpolate its depth and compare it with the stored value. If the new fragment is nearer, replace both color and stored depth; otherwise leave the current pixel alone. This makes the result independent of the order in which opaque triangles were submitted and reduces to a local comparison that GPUs can perform in parallel. (P3-07)

The comparison direction and clear value are paired with the chosen depth mapping. In this lecture’s image-space convention, near is larger than far, so it initializes to a very negative value and keeps the larger depth. In a common modern window-depth setup, nearer values are smaller, the buffer is cleared to 1, and a `LESS` test keeps the smaller value. Reversing one part without the other makes occlusion wrong. Perspective depth is nonlinear after divide: depth values are crowded toward the far end, so a needlessly tiny near distance relative to the far distance reduces precision. (P3-05–07)

A depth buffer solves visibility for opaque samples, not every light-transport problem. Transparent surfaces need blending and ordering or a different method; a single nearest-surface value cannot represent multiple layers. A shadow asks what is hidden from a light, while the camera depth test asks what is hidden from the eye. These are separate visibility queries. (P3-07)

Ray tracing asks the visibility question differently. For each pixel sample, cast a primary ray from the eye through the image plane, intersect it with scene objects, and keep the nearest hit. A naïve implementation costs about $O(PN)$ for $P$ pixel samples and $N$ objects. As a scale check, one million pixels times one thousand objects is roughly one billion intersection tests. A ray-sphere test can reduce to distance from the ray to the sphere center; triangles need a ray-triangle intersection test. Each pixel ray can be processed independently, but a straightforward implementation may perform much more work than rasterization.

Secondary rays add effects: send a shadow ray from a surface point toward a light and mark it shadowed if another object is encountered first; send a reflected ray in the mirror direction; send a refracted/transmitted ray through transparent material. Recursive effects can branch, so realism brings a potentially large cost. This is the geometric-optics view of light paths traced backward from the eye. The lecture contrasts its cost with the practical depth-buffer solution and previews later ray-tracing work. (P3-07)

Painter’s algorithm sorts surfaces back-to-front and draws them in order, but cyclic overlap can make a global ordering impossible unless geometry is split. A Warnock-style algorithm handles a complex screen region by subdividing it into four smaller regions and repeating until each region is simple enough to resolve. These methods illustrate different ways to turn a global visibility problem into a useful order or smaller local problems. (P3-07)

#### Choose visibility algorithms by their ordering and data needs

The 2009 survey groups five approaches by where they solve visibility:

| Method | Core decision | Main strength | Limitation or use |
|---|---|---|---|
| Depth buffer | Per pixel, keep the nearest depth | Submission order can be arbitrary; maps well to parallel hardware | Extra memory; transparent layers need more than one depth |
| Scanline | Process polygon spans row by row, updating depth/order | Incremental edge/attribute bookkeeping | More complex active-edge state; often combined with depth buffer |
| Painter / BSP | Sort or partition geometry, then draw back-to-front | Can reuse a spatial ordering for a moving camera; useful in constrained game scenes | Cyclic overlap forces splitting; tree balance and fragments matter |
| Warnock subdivision | Split the image into smaller windows until each is simple | Window tasks distribute across processors; can oversample edges | Recursive work and repeated polygon/window tests |
| Ray tracing | Send rays through samples and choose nearest intersections | Natural reflection, shadow and refraction paths | Intersection cost grows; acceleration structures required |

A painter’s algorithm is like painting distant scenery first and nearer objects over it. If every polygon can be placed in one back-to-front list, later drawing hides earlier color. Intersecting polygons can form a cycle—A in front of B in one region, B in front of C elsewhere, C in front of A—so there is no global scalar sort that works. A binary space partition (BSP) tree chooses a polygon plane, classifies other polygons against it, and splits polygons that cross. The tree stores an ordering, and traversal visits the far subtree, the node polygon, and the near subtree; which subtree is “far” depends on the camera’s side of the plane. Bounding volumes can avoid unnecessary split tests. A badly chosen splitter produces a deep, unbalanced tree and many tiny fragments, so random or heuristic plane selection can help. (P3-07, P3-14)

Warnock’s method works in screen space. For a viewport region, collect the polygons that overlap it. If there are none, leave it blank; if one surface alone determines the region, draw it; if a nearer polygon covers the whole region (a surrounder), discard surfaces hidden behind it. Otherwise divide the rectangle into four child windows and repeat with smaller polygon lists. At a one-pixel region, choose the nearest candidate; subdividing below pixel size and averaging samples improves edge coverage. This is a quadtree. Its tasks can be sent to separate processors, which explains why its recursive subdivision remains useful even when the original algorithm is not the main renderer. (P3-14)

The methods are complementary. Rasterization still needs coverage and interpolated attributes; the depth test chooses visible opaque samples. A BSP or painter order can avoid a per-pixel depth store in special geometry, but incurs scene sorting and splitting. Ray tracing resolves nearest intersections per ray and naturally adds secondary visibility queries. The lecture also mentions scanline/depth hybrids. Do not read the historical claim “this algorithm is dead” as a timeless API fact: hardware shifts the balance, and components of older methods reappear inside newer parallel renderers. (P3-14)

#### Ray intersection, recursive paths, and acceleration

A ray is an origin $O$ and direction $D$: $R(t)=O+tD$ for $t\ge0$. Normalize $D$ if you want $t$ to measure distance. For a plane through point $Q$ with normal $N$, an intersection lies on both the ray and plane, so

$$
N\cdot(O+tD-Q)=0,\qquad t=\frac{N\cdot(Q-O)}{N\cdot D}.
$$

If $N\cdot D=0$, the ray is parallel; if it also lies in the plane, the intersection is non-unique and must be handled separately. Reject negative t because those hits lie behind the ray origin. This dot-product formula is the same signed-plane test used for clipping, now used to find a location rather than classify a whole endpoint. (P3-15)

For a sphere centered at $C$ with radius $r$, let $V=C-O$ and let $D$ be unit length. Project V onto the ray with $h=V\cdot D$. The squared perpendicular distance is $d_\perp^2=V\cdot V-h^2$. If this exceeds $r^2$, the ray misses. Otherwise its roots are $t_{near}=h-\sqrt{r^2-d_\perp^2}$ and $t_{far}=h+\sqrt{r^2-d_\perp^2}$. Keep the smallest nonnegative root. This geometric derivation shows why spheres are an easy primitive, though a production implementation can solve the quadratic directly and must consider floating-point tolerances. For a triangle, intersect its plane and test the hit’s barycentric coordinates; all three coordinates must be nonnegative and sum to one. (P3-15)

A recursive Whitted-style renderer starts with the camera ray and shades the nearest surface. For each point light, cast a shadow ray toward it; a hit before the light means the direct source is blocked. For a reflective surface, reflect the incoming direction about the normal and trace a new ray; for a dielectric surface, use its refractive index and Snell’s law to trace transmission, with total internal reflection as an edge case. Add each returned radiance scaled by the material’s reflective/transmissive coefficient. Stop at a maximum bounce depth or when contribution becomes negligible, otherwise repeated mirrors can recurse indefinitely. The source transcript says “Turner Whitted”; the correct author is Turner Whitted, whose 1980 paper describes the recursive ray tree ([ACM record](https://doi.org/10.1145/358876.358882), [paper copy](https://my.eng.utah.edu/~cs6958/papers/p343-whitted.pdf)). (P3-07, P3-15)

Backward ray tracing traces from the eye because only paths contributing to visible pixels matter. It efficiently handles direct shadows and ideal mirror/refraction chains. It can miss a narrow light path that does not begin at the camera ray, such as a caustic where a lamp reflects from a mirror onto a diffuse table. Forward light tracing marks where emitted paths land; combining sampling strategies is harder but recovers such paths. Diffuse interreflection requires many scattered directions, so each surface hit can spawn many more rays. More rays improve coverage but increase work. This is the geometric-optics model, not a wave-optics simulation. (P3-15)

The naïve primary-ray cost is roughly $P\times N$ primitive tests for $P$ samples and $N$ objects. Spatial acceleration groups objects. An octree recursively divides 3D space into eight cells; a ray visits only cells it crosses and tests the objects stored there. A kd-tree uses axis-aligned plane splits, often at adaptive positions rather than the exact center. Smaller leaves reduce primitive tests but increase tree traversal and storage. Objects crossing cell boundaries may be referenced in multiple nodes or handled by a spatial split rule. The ray-versus-cell traversal resembles grid line rasterization because both increment through ordered cells. (P3-15)

Implicit surfaces give another difficulty. If a surface is $F(x,y,z)=0$, substitute the ray coordinates to obtain $F(O_x+tD_x,O_y+tD_y,O_z+tD_z)=0$, a one-variable root-finding problem. A quadratic can be solved directly; a generic polynomial of degree five or more has no general formula in radicals. Numerical root-finding is needed for many implicit surfaces. Neighboring pixel rays usually hit nearby points, so the previous ray’s root can seed the next search, but at silhouettes a tiny screen shift may jump to a different branch or miss the surface. This is why ray tracing a sphere or triangle is straightforward while ray tracing a high-order freeform surface is a numerical-analysis problem. (P3-15)

## Scene graphs and articulated geometry

Hierarchies separate reusable local structure from global pose, allowing articulated surfaces to follow moving skeletons.

### Forward kinematics makes inverse kinematics differentiable

A rig is a hierarchy of segments and joints. Forward kinematics starts at a root and composes each joint transform down the hierarchy to compute the position of a hand, foot, or other end effector. For a two-link planar chain with root $p_0$, local segment vectors $u_0,u_1$, and joint angles $\theta_0,\theta_1$, let $R(\theta)$ be the 2D rotation matrix. Then

$$
p_1=p_0+R(\theta_0)u_0,\qquad
p_2=p_1+R(\theta_0)R(\theta_1)u_1.
$$

The second rotation is relative to the first segment, so rotating the root joint also moves every child. Equivalently in the plane, rotations can be written as multiplication by complex numbers $e^{i\theta}$, where $i^2=-1$.

Inverse kinematics reverses the question. Rather than specifying joint angles and asking where the hand goes, specify a target and ask which angles place the hand there. For a target $p^*$, one simple objective is

$$
E(\theta_0,\theta_1)=\frac12\|p^*-p_2(\theta_0,\theta_1)\|^2.
$$

The Jacobian is the matrix of endpoint derivatives with respect to joint angles. The chain rule combines it with the residual $p_2-p^*$ to get a gradient, which can drive gradient descent. A character rig applies the same idea to many joints: asking the hand to reach a prop allows an optimizer to coordinate the shoulder, elbow, wrist, and torso. Joint limits, balance, and pose preferences can be added as constraints or penalties. Different configurations may reach the same target, and a target beyond the limb's reach cannot achieve zero error, so the solver's returned pose is a best feasible fit, not necessarily a unique or globally ideal artistic pose. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-22 - Optimization|CMU-22]]

### Why linear blends can twist a character limb

Suppose one bone transform rotates a vertex +90 degrees and another rotates it -90 degrees. A naïve matrix blend at equal weights can approach a collapsed matrix rather than a halfway rotation. Applied to vertices along a limb, this can create a candy-wrapper twist or volume loss. A better rotation interpolation preserves the group structure of rotations, for example by quaternion interpolation, while translation or stretch is blended separately. This is one reason a transformation representation must reflect the object being interpolated. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-05 - Spatial Transformations|CMU-05]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-06 - 3D Rotations and Complex Representations|CMU-06]]

### Scene graphs: reuse local structure, compute global pose

A scene graph represents a complex object as a hierarchy of local coordinate frames. A robot arm is a useful abstract model: the forearm is positioned relative to the upper arm; the hand is positioned relative to the forearm. If the parent is moved or rotated, every descendant follows automatically. For a node with local transform $M_{\mathrm{local}}$, its world transform is

$$
M_{\text{world}}=M_{\text{parent,world}}M_{\text{local}}.
$$

The ordering is the whole point: local transforms are composed along the path from root to node. A tree can reuse one definition of a hand or wheel and place it in multiple poses. It also provides a natural animation state: joint parameters modify local transforms, and traversal computes the consequences. The lecture explores hierarchical modeling, multiplying transforms along the graph, and inverse-kinematics questions such as choosing joint angles to place an endpoint [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-05 - Hierarchical modeling|MITG-05]] (00:10:00–00:25:00; 00:35:00–00:50:00).

Each edge in the tree has a local transform, and a node's world transform is the product of all edge transforms from the root. For a torso, shoulder, elbow, and wrist, for instance,

$$
M_{\mathrm{wrist}}^{\mathrm{world}}=
M_{\mathrm{torso}}^{\mathrm{world}}
M_{\mathrm{shoulder}}^{\mathrm{torso}}
M_{\mathrm{elbow}}^{\mathrm{shoulder}}
M_{\mathrm{wrist}}^{\mathrm{elbow}}.
$$

The wrist mesh can be authored once in wrist-local coordinates and transformed by this product. If the shoulder rotates, the elbow and wrist move because their world transforms inherit that factor; the wrist's own local geometry does not need to be rewritten. A useful tree drawing is: world → torso → left upper arm → left forearm → left hand; the right arm is a second child branch. This is also why a scene graph is a reuse structure, not just a list of objects. An instanced chair can reuse a single chair mesh under several chair-to-world transforms. A character's parts use parent-child transforms that encode attachment. The 2021 lecture gives the nested arm/hand/finger example and describes matrix push/pop as a traversal technique: descend while accumulating transforms, then restore the parent's transform when returning to a sibling branch [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-04 - Transformations|MITG-04]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-05 - Hierarchical modeling|MITG-05]] (00:53:20–00:54:33; , 00:02:14–00:04:46).

Rotation around a joint requires one more composition detail. A rotation matrix rotates around its own origin, so a forearm joint displaced from the local origin needs $T(c)RT(-c)$: move the pivot $c$ to the origin, rotate, then move it back. With column vectors, the rightmost operation happens first. Forgetting either translation makes the bone spin around the model origin rather than bend at the elbow. This same “translate to pivot, rotate, translate back” pattern appears in mechanical parts, doors, and wheels.

An inverse-kinematics (IK) problem reverses the usual direction of evaluation. Forward kinematics says, “given joint values, where is the hand?” IK says, “given a target for the hand, what joint values reach it?” There may be many solutions, no exact solution, or solutions that violate joint constraints. Even a two-link planar arm makes the geometry clear: with lengths $L_1,L_2$, a target at distance $r$ is reachable only when $|L_1-L_2|\le r\le L_1+L_2$. Animation then needs a policy to choose among valid poses, such as smoothness over time or a preferred bend direction. A hierarchy supplies the forward model; solving for a pose is a separate optimization problem.

For a planar two-link arm with lengths $L_1,L_2$ and joint angles $\theta_1,\theta_2$, the wrist position is

$$
x=L_1\cos\theta_1+L_2\cos(\theta_1+\theta_2),\qquad
y=L_1\sin\theta_1+L_2\sin(\theta_1+\theta_2).
$$

The cosine rule gives $\cos\theta_2=(r^2-L_1^2-L_2^2)/(2L_1L_2)$, where $r=\sqrt{x^2+y^2}$. This immediately gives the reach condition $|L_1-L_2|\le r\le L_1+L_2$. If both links are one unit long and the target is at $(1,1)$, then $r=\sqrt2$ and $\cos\theta_2=0$, so the elbow angle has the two familiar bent configurations $\theta_2=\pm\pi/2$. The endpoint alone does not choose between them; a rig needs an elbow-side preference, joint limits, or a continuity rule from the previous frame.

For a longer chain, define the forward-kinematics function $e=f(\theta)$, mapping all joint parameters to an end-effector position. Given target $e^*$, one seeks $f(\theta)=e^*$. Near a current guess $\theta_0$, first-order Taylor expansion gives

$$
f(\theta_0+\Delta\theta)\approx f(\theta_0)+J_f(\theta_0)\Delta\theta.
$$

The Jacobian $J_f$ contains the partial derivative of each end-effector coordinate with respect to each joint parameter. Solve $J_f\Delta\theta\approx e^*-f(\theta_0)$, update the joint values, and repeat. This is a Newton-style local solve: it works well when the current pose is close and the Jacobian is well-conditioned, but may fail when the target is unreachable, constraints are active, or a poor initial guess crosses into another bend configuration. The lecture explicitly frames the IK problem as nonlinear inversion, introduces Jacobian linearization/Newton iteration, and warns that solutions may be absent, non-unique, or hard to find [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-05 - Hierarchical modeling|MITG-05]] (00:38:40–00:49:25). It also mentions mesh-based IK as a different approach that searches within a span of example poses rather than only the joint-angle chain [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-05 - Hierarchical modeling|MITG-05]] (00:49:35–00:50:45).

Representing orientation with Euler angles is compact and readable but can suffer gimbal lock, where one degree of freedom is lost for a particular alignment. Unit quaternions encode rotation without that singularity and allow interpolation along a rotation path, but $q$ and $-q$ represent the same orientation. The 2021 hierarchy lecture mentions the quaternion double-cover issue while discussing rotation and IK [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-05 - Hierarchical modeling|MITG-05]] (00:25:00–00:45:00). This is a reminder that representation choices have side effects: a representation that makes composition easy may need care when interpolating or enforcing continuity.

A kinematic chain records rigid transforms and joint degrees of freedom. A hinge contributes one angle; a ball joint can contribute three rotational degrees of freedom; a slider contributes translation. Each joint may also have limits. The lecture uses a robot end effector to motivate the same problem in character animation and notes that a virtual character can take a pose that would fling a physical robot because the motion curves are not constrained by hardware forces [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-05 - Hierarchical modeling|MITG-05]] (00:04:12–00:05:48; 00:21:23–00:25:24). Forward kinematics is therefore a deterministic evaluation of a pose, while inverse kinematics is an optional tool for fitting the pose to a target.

### Skinning: attach a surface to a moving skeleton

Before the deformation formula, the sixth lecture frames animation as several tools with different jobs. Traditional cel animation separates a painted background from transparent moving layers, and animators draw key poses while “in-betweeners” fill the frames between them. Computer keyframing keeps the same idea: an artist sets important poses, then a time spline interpolates position or joint angles. That spline is a function of time, so its shape controls both the path and the timing. Procedural animation instead computes motion from rules or code, useful for repeated clocks, crowds, or generated road networks. Physically based animation produces secondary motion—cloth, flags, or fluid—that would be tedious to keyframe, though an artist may have less direct control over it. The lecture's bouncing-ball example highlights squash-and-stretch and anticipation: an appealing animated ball can deform before impact, intentionally violating literal causality to communicate the action. A film can spend many hours rendering one frame; a game must satisfy an interactive frame budget, so the fidelity and control trade-offs differ [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-06 - Animation:  Skinning/enveloping|MITG-06]] (00:03:47–00:06:27; 00:20:14–00:25:30; 00:25:34–00:31:27).

Motion capture records a performer's movement and maps it onto a virtual rig. When the performer and target character have different proportions or joints, the transfer is called retargeting; raw angles cannot simply be copied because they can produce implausible poses on a different skeleton. The lecture also mentions markerless capture and its smoothing, and separates the captured pose stream from the later deformation of the character's surface [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-06 - Animation:  Skinning/enveloping|MITG-06]] (00:42:00–00:47:08). These methods supply motion; skinning solves the distinct geometric question of how a posed skeleton moves the mesh vertices.

A skeleton provides a compact pose; skinning maps that pose to the many vertices of a surface. In linear blend skinning, also called skeleton-subspace deformation, vertex (x) receives a weighted blend of bone transformations:

$$
x'(t)=\sum_{j=1}^{m}w_j(x)M_j(t)M_j(0)^{-1}x,\qquad \sum_j w_j(x)=1.
$$

The inverse bind transform $M_j(0)^{-1}$ first expresses the vertex in bone $j$'s bind-pose frame; the current bone transform sends it into the animated pose. The weights say how strongly each bone influences a vertex. A vertex at a joint might be influenced by both adjacent bones; a vertex in the forearm mostly follows that bone. Weights should sum to one so the blend remains normalized. Skinning can transform thousands of vertices using a small set of joint matrices and weights, allowing artists to shape a mesh once and then animate it through the skeleton.

To unpack the formula, let $B_j$ map bone $j$'s local coordinates into world coordinates in the bind pose, and let $P_j(t)$ map that bone's local frame into world coordinates in the current pose. The vertex's bone-local bind coordinate is $B_j^{-1}x$. Its posed candidate from that bone is $P_j(t)B_j^{-1}x$. Then blend those candidates:

$$
x'(t)=\sum_{j=1}^{m}w_j(x)P_j(t)B_j^{-1}x,\qquad
w_j(x)\ge0,\quad\sum_jw_j(x)=1.
$$

The inverse bind matrix matters because a bone's motion is measured relative to its rest placement. If a joint is translated one unit from the model origin, applying only its current transform would rotate a vertex around the origin. Multiplying by $B_j^{-1}$ first removes the rest placement; applying $P_j(t)$ then moves it with the animated bone. If the current pose equals the bind pose, $P_j=B_j$, so every bone candidate is $B_jB_j^{-1}x=x$ and the mesh stays exactly in its rest position. This identity is a valuable implementation check.

For a simple numerical example in 2D homogeneous coordinates, take a rest vertex $x=(2,0,1)^T$ and a bone whose bind transform is $B=T(1,0)$. Its bone-local coordinate is $B^{-1}x=(1,0,1)^T$. If the posed bone transform is $P=T(1,0)R_{90^\circ}$, then $PB^{-1}x=(1,1,1)^T$: the point has rotated about the bone origin at $x=1$, not about the model origin. With two bones, calculate each posed candidate this way first, then blend the resulting points using their weights.

Linear blend skinning is fast because each vertex needs a small number of matrix-vector products, but averaging rotations can shrink the mesh. Let a point be $x=(1,0)$ and let two bones rotate it by $+90^\circ$ and $-90^\circ$ around the same joint. The candidate points are $(0,1)$ and $(0,-1)$. Equal weights produce

$$
\frac12(0,1)+\frac12(0,-1)=(0,0).
$$

The point collapses onto the joint even though neither bone transform collapses anything. Along a real elbow or wrist this appears as volume loss or a twisting “candy-wrapper” pinch. More weights or smoother interpolation do not remove the underlying issue: a weighted average of rotation matrices need not be a rotation matrix. Common improvements include dual-quaternion blending for rigid motions, additional corrective shapes, or artist-authored weight adjustments; each trades extra machinery or authoring work for better shape preservation. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-06 - Animation:  Skinning/enveloping|MITG-06]] (00:47:17–00:51:00)

The 2021 skinning lecture separates the artist-authored character, the skeleton pose, and the deformation caused by changing pose. It discusses how the surface follows a moving joint, and uses interpolated sensor/pose information as an example of estimating motion between samples [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-06 - Animation:  Skinning/enveloping|MITG-06]] (00:05:00–00:30:00; 00:35:00–00:55:00). The important weakness of ordinary matrix blending is that a weighted sum of rigid rotations is generally not itself a rigid rotation. Near joints, this can shrink or twist volume, producing the “candy-wrapper” or collapsing-joint artifact. Dual-quaternion skinning provides one alternative for blending rigid transforms, while corrective shapes or nonlinear methods handle artist-specific deformations. These later options address the limits of the simple baseline rather than making the baseline useless: linear blend skinning is fast, easy to implement, and often adequate when the rig and weights are designed with its behavior in mind.

### Additional geometry notes for acceleration structures

These lecture examples are integrated by topic. The 2009 interface material below is explicitly historical; it is not a modern OpenGL core recipe.

#### Hierarchical modeling: reuse geometry through transforms

A scene is easier to build from reusable pieces than from one long list of hard-coded world coordinates. The lecture’s 1977-era Midas example treats a text file as a small modeling program. A polygon command defines a unit square. A transform command moves or rotates it; a create command makes a transformed copy while keeping the source; an extrude command sweeps a face into side polygons; an assemble command combines named pieces. Six instances of a square, each with its own transform, make the six faces of a cube. Repeating the cube with translations creates a larger regular arrangement. (P3-11)

This command list naturally forms a directed graph. The leaf stores the original points; each parent stores which children to include and the transform to apply. A table can reuse one cube model for four legs and a top, changing only each instance’s transform. In a row-vector system, define clearly whether a local transform is applied before or after its parent transform; a child’s world transform is the ordered product along the path from leaf to root. In the column-vector convention, that product order reverses. The same parent-child rule makes an articulated character work: rotate the torso and all descendant parts follow; rotate the upper arm and the forearm follows, while the hand remains attached at its own joint. (P3-11)

The source examples reinforce two useful modeling habits. First, identify repeated structure: many chairs, floor tiles, railroad boards, or background houses can be instances of a few models. Second, keep geometry local and move instances with transforms. This lowers authoring effort and makes the model easier to edit. A graph can be a strict tree, or a DAG if geometry resources are shared; distinguish shared immutable shape data from per-instance transform/material state so changing one chair does not accidentally change every chair. The 2009 fixed-function matrix stack offers push/pop for nested transforms. Current renderers usually traverse an explicit scene graph and pass composed matrices to shaders, but the composition idea is the same. (P3-11)

## Animation curves and motion paths

Motion paths and animation curves describe how poses vary through time and how smoothly cameras or characters move.

### A camera path is animation plus a frame choice

A camera path specifies position $x(t)$, viewing direction, and an up vector. Interpolating position alone does not define orientation. The view direction must remain normalized; the up vector must avoid becoming parallel to it, or the camera frame becomes singular. One approach interpolates position and orientation separately, using a quaternion for orientation, then reconstructs a camera basis at each time. This camera-path example links animation curves, 3D rotations, and coordinate frames. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-20 - Introduction to Animation|CMU-20]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-06 - 3D Rotations and Complex Representations|CMU-06]]

### Camera orientation continuity along a spline path

A camera path must interpolate both position and orientation while maintaining a valid up direction. A curve can be smooth in position yet rotate abruptly if orientation is interpolated inconsistently. The camera therefore combines geometric curve design, a rotation representation, and time parameterization; its valid orientation frame is part of the animation state. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-20 - Introduction to Animation|CMU-20]]

### Additional geometry notes for scene graphs

#### Smooth interpolating paths and camera motion

A Bezier control polygon is a handle: its interior controls generally do not lie on the curve. A camera path often needs to pass through chosen waypoints, so Catmull–Rom uses interpolating cubic segments. Between waypoints $P_1$ and $P_2$, a uniform Catmull–Rom segment takes endpoint tangents from neighbors, commonly $m_1=(P_2-P_0)/2$ and $m_2=(P_3-P_1)/2$. With these tangents, adjacent pieces meet at each waypoint and share a tangent direction and scale, giving $C^1$ continuity for uniform parameter spacing. For unevenly spaced waypoints, chord-length or centripetal parameterization is needed to avoid inappropriate speed and loops; the uniform formula assumes equal parameter steps. (P3-20)

The Hermite basis writes this segment directly from endpoints and tangents:

$$
P(t)=(2t^3-3t^2+1)P_1+(t^3-2t^2+t)m_1+(-2t^3+3t^2)P_2+(t^3-t^2)m_2.
$$

At $t=0$ the weights select $P_1$ and its derivative is $m_1$; at $t=1$ the curve reaches $P_2$ with derivative $m_2$. These four boundary conditions determine the four coefficients of a cubic polynomial. Bezier, Ferguson/Hermite, and Catmull–Rom are different ways to specify a cubic; converting between their coefficient/control-point forms is a matrix change of basis, not a different geometric degree. (P3-20)

For a numeric intuition, let neighboring one-dimensional waypoints be $P_0=0,P_1=0,P_2=10,P_3=10$. Uniform Catmull–Rom gives both tangents as 5. At $t=1/2$, the Hermite basis has weights $1/2,1/8,1/2,-1/8$, so $P(1/2)=0.5(0)+0.125(5)+0.5(10)-0.125(5)=5$. The path reaches the midpoint smoothly; unlike straight interpolation, its endpoint speeds match the neighboring waypoint context. A small perturbation to a point affects nearby segments rather than making a single global Lagrange polynomial swing wildly across the entire route. (P3-20)

Continuity labels describe different experiences. $C^0$ means neighboring pieces meet in position but may have a corner. $C^1$ also matches first derivatives, removing a velocity jump. $C^2$ matches second derivatives, so curvature changes less abruptly. A camera can still look jerky on a geometrically smooth path if it starts at full speed, stops instantly, or rotates its view direction abruptly. Treat time as a second function: ease speed from zero to cruise and back, and smooth orientation (look direction and up vector) separately from position. A cubic easing curve with a horizontal tangent at its start and end can ramp velocity gently. The path spline determines where the camera goes; timing and orientation determine how the shot feels. (P3-20)

## Dynamics, forces, and numerical integration

Dynamics predicts changing state from forces or rules, while numerical integration controls how a continuous model behaves in discrete time.

### Animation, dynamics, optimization, and PDEs

Animation makes scene state change over time. Keyframes specify selected values, and interpolation fills the intervals. Position often lies in Euclidean space; orientations lie on a curved space, such as the unit quaternion sphere, so componentwise interpolation can take a poor path. Cubic splines use polynomial segments and control continuity: positional continuity means segments meet, tangent continuity means velocities agree, and higher continuity constrains acceleration. Smoothness and temporal coherence matter as much as matching endpoints. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-20 - Introduction to Animation|CMU-20]]

Animation has long used sequences of still images to suggest movement. A spinning phenakistoscope hides the changing disc behind a slit so each drawing appears as a separate snapshot rather than a blur. Muybridge's photographs answered whether all four horse legs leave the ground during a gallop. Digital animation continues this pattern: a new display or computer changes how artists can construct and show motion. The perceptual explanation is not simply that an image lingers on the retina. The brain compares successive visual events and anticipates likely movement (the beta and phi phenomena); a flashing pattern can therefore make equal-speed objects appear to move differently. These cues help explain why the timing and spacing of animated frames matter. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-20 - Introduction to Animation|CMU-20]]

Hand-drawn production assigns key poses to a senior animator and fills the intervening drawings with assistants. Computer keyframing applies the same division to scene parameters: key values can be position, color, light intensity, camera zoom, or the degrees of freedom in a character rig. A bouncing ball, for example, may have keys at launch, ground contact, and successive rises and falls. The computer interpolates the parameter values between these events. This saves repetitive work, but the interpolation rule determines the perceived motion. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-20 - Introduction to Animation|CMU-20]]

A spline is a piecewise polynomial function. Let $t_i$ denote an ordered key time (a knot), and let $f_i$ be the value required at that time. Interpolation means $f(t_i)=f_i$. Linear interpolation joins keys with straight segments; position stays continuous, but the velocity jumps at each key, which implies an abrupt acceleration. A cubic polynomial has four coefficients, so two endpoint positions alone leave two degrees of freedom unresolved. Supplying both endpoint velocities makes a unique cubic Hermite segment. On normalized time $s=(t-t_0)/(t_1-t_0)$, where $s=0$ is the first key and $s=1$ is the second, the segment is

$$
p(s)=h_{00}(s)p_0+h_{10}(s)m_0+h_{01}(s)p_1+h_{11}(s)m_1,
$$

where $p_0,p_1$ are endpoint positions, $m_0,m_1$ are endpoint tangent vectors scaled for the segment's time interval, and

$$
h_{00}=2s^3-3s^2+1,\quad h_{10}=s^3-2s^2+s,\quad h_{01}=-2s^3+3s^2,\quad h_{11}=s^3-s^2.
$$

These four basis polynomials satisfy the four endpoint constraints: $p(0)=p_0$, $p(1)=p_1$, $p'(0)=m_0$, and $p'(1)=m_1$. A cubic is useful partly because a slightly bent physical strip minimizes an energy involving the squared second derivative; it also provides convenient continuity with few parameters. Raising polynomial degree is not a universal cure: high-degree interpolation can oscillate sharply near the ends (the Runge phenomenon). [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-20 - Introduction to Animation|CMU-20]]

For a sequence of keys, each segment has its own polynomial coefficients. Matching the value at every knot gives interpolation; matching first derivatives gives continuous velocity; matching second derivatives gives continuous acceleration, or $C^2$ continuity. A natural cubic spline closes the remaining degrees of freedom by setting second derivative to zero at the first and last keys. It interpolates and is $C^2$, but the segment coefficients are coupled through a global linear system, so moving one key can affect distant parts of the curve. A Bezier/Hermite chain instead gives each segment its endpoints and endpoint tangents independently. It has interpolation and local editing, but second derivatives may jump. Catmull–Rom splines infer tangent at key $i$ from neighboring values, approximately $(f_{i+1}-f_{i-1})/(t_{i+1}-t_{i-1})$, avoiding the need to hand-author velocities; this makes it a practical starting point for camera paths. A cubic B-spline basis has local support and smooth joins, but the curve generally approximates rather than passes through its control points. These are tradeoffs among interpolation, smoothness, and local control; no one cubic spline provides all three automatically. NURBS can exactly represent circles and ovals, and clothoid curves are chosen in road design for their acceleration behavior. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-20 - Introduction to Animation|CMU-20]]

Motion can also come from measurements or rules. Motion capture records reflective markers from multiple cameras and triangulates their 3D positions over time; the resulting joint motion can drive another character's rig. Procedural animation computes motion from equations, with physical simulation as one example. In a hierarchical rig, transformations form a tree, and splines animate its joint angles and translations. Inverse kinematics reverses the usual question: specify a hand or tool target and solve for the joint angles that place the end effector there, typically as an optimization problem. Skeletal animation deforms a mesh by blending the transformed positions supplied by several bones. Each mesh vertex has weights that indicate the influence of each bone; linear blend skinning averages those transformed positions. This can collapse volume at elbows or shoulders, motivating more physical deformation models. Blend shapes store alternate vertex configurations—such as happy, sad, sleepy, or awake facial poses—and animate time-varying weights for those shapes. Splines can interpolate those weights. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-20 - Introduction to Animation|CMU-20]]

Dynamics describes state change under forces. For particle position $x$, velocity $v$, and acceleration $a$, an ODE is $\dot x=v$ and $\dot v=a(x,v,t)$. Forward Euler advances with the current derivative. For a spring with acceleration $a=-kx/m$, a large time step can add energy and make an oscillation explode. [[ODE integration and stability|Symplectic Euler]] updates velocity before position and often preserves qualitative mechanical behavior better. Implicit methods use the unknown next state and can remain stable for stiff systems, at the cost of solving equations. Step size controls accuracy and stability. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-21 - Dynamics and Time Integration|CMU-21]]

Optimization minimizes an objective. For energy $E(\theta)$ over parameters $\theta$, gradient descent uses $\theta_{k+1}=\theta_k-\eta\nabla E(\theta_k)$. The gradient points toward increasing error, so the negative gradient descends. Step size $\eta$ that is too small wastes iterations; too large may overshoot. Conditioning describes how differently curved directions affect progress. Newton methods use Hessian curvature; quasi-Newton methods approximate it. In a least-squares fit of line $y=mx+b$ to samples $(x_i,y_i)$, minimize $\tfrac12\sum_i(mx_i+b-y_i)^2$. The residual gradient tells how to change slope and intercept. This same method fits shapes, cameras, and image parameters. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-22 - Optimization|CMU-22]]

PDEs describe fields changing over space and time. The heat equation $u_t=\kappa\Delta u$ diffuses temperature toward neighborhood averages. The wave equation $u_{tt}=c^2\Delta u$ propagates disturbances. Initial conditions define the starting field; boundary conditions constrain the domain edge. Dirichlet conditions prescribe the field value; Neumann conditions prescribe flux or derivative. A finite-difference approximation of the second derivative is

$$
u_{xx}(x_i)\approx\frac{u_{i-1}-2u_i+u_{i+1}}{(\Delta x)^2}.
$$

Substitution into heat diffusion moves each value toward its neighbors. Explicit time stepping can become unstable if the time step is too large relative to spatial resolution. Many physical shapes also minimize an energy functional. The Euler-Lagrange equation gives the stationarity condition; discretizing the field turns that continuous problem into a finite optimization problem. Geometry, optimization, and numerical integration thus share the task of approximating continuous behavior with finite data. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-03 - Vector Calculus (P)Review|CMU-03]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-22 - Optimization|CMU-22]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-23 - Physically Based Animation and PDEs|CMU-23]]

### ODE integration and stability

An ordinary differential equation specifies the derivative of a state. If state $y$ obeys $\dot y=f(y,t)$, explicit Euler advances by $y_{n+1}=y_n+h f(y_n,t_n)$. Its local truncation error is proportional to $h^2$ and accumulated global error is generally proportional to $h$. Halving the time step roughly doubles work while often reducing error by about a factor of two. Higher-order Runge-Kutta methods sample the derivative several times per step to improve accuracy.

Stability is distinct from accuracy. A method may have small one-step error but amplify it over repeated steps. For a damped oscillator or stiff spring, explicit Euler becomes unstable if the step is too large relative to the natural frequency. Implicit Euler evaluates $f$ at the next state and requires solving an equation, but it can remain stable for much larger steps. Symplectic Euler updates momentum-like variables before positions and often preserves the qualitative energy behavior of mechanical systems. No method is best for all tasks: real-time games may favor predictable cost, offline simulation may favor accuracy, and stiff material models may need implicit solves. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-21 - Dynamics and Time Integration|CMU-21]]

Constraints such as fixed joints or nonpenetration conditions restrict the state to a feasible set. A solver can project a tentative update back onto that set or use Lagrange multipliers to introduce constraint forces. The numerical step must respect these conditions; otherwise joints stretch or objects interpenetrate. Time integration, collision queries, and optimization therefore interact in physically based animation. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-12 - Geometric Queries|CMU-12]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-21 - Dynamics and Time Integration|CMU-21]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-23 - Physically Based Animation and PDEs|CMU-23]]

### Motion models from a tossed rock to crowds

For a rock of mass $m$ under gravity $g$, Newton’s second law gives $m\ddot x=mg$. The acceleration is constant, so position is a parabola in time. A pendulum adds a geometric constraint: the bob moves on a circle and gravitational force is projected onto its tangent direction. A double pendulum couples two angles and can show complex, sensitive motion despite simple equations. These examples show that nonlinear behavior can arise from constraints and coupling, not only from exotic forces.

The same simulation framework extends to flocking, crowds, particle fluids, granular materials, molecular dynamics, cosmological particles, mass-spring systems, and hair. A spring connecting positions $x_1,x_2$ with rest length $L_0$ exerts force according to its extension and stiffness. The resulting ordinary differential equations may be stiff when springs are very strong. Crowds add collision avoidance and bottleneck questions; flocking adds local alignment, cohesion, and separation rules. Physical and behavioral models share state integration but differ in the forces or rules driving state change. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-21 - Dynamics and Time Integration|CMU-21]]

### Keep physical and numerical error separate

An image can look wrong for several distinct reasons. The scene model can be wrong, the chosen physical model can omit effects, the discretization can be coarse, the estimator can have high variance, or the display transform can be inappropriate. These errors should not be conflated. More samples reduce Monte Carlo variance but do not repair a biased material model. More mesh triangles reduce geometric discretization error but do not fix a bad camera. A correct linear-light solution can still look unexpected when mapped into a limited display gamut. A stable simulation can solve the wrong PDE if boundary conditions were encoded incorrectly.

When diagnosing a result, identify the mathematical target first. For rasterization, is the target pixel area coverage or a single center sample? For a light integral, is the measure solid angle or area? For geometry processing, is the target a smooth surface, a discrete mesh, or preserved topology? For animation, should the curve match only positions or also velocities? For optimization, are constraints part of the problem or just penalties? Good questions reveal which error source must be addressed. This precision is the common thread across the course’s rendering, imaging, geometry, and animation topics.

### Crowd bottlenecks and particle models

Crowd simulation can ask where a building plan creates bottlenecks. Each agent has a desired velocity and collision-avoidance behavior; the combined flow reveals congestion. Particle-based fluids represent a continuum with moving samples that exchange forces or density estimates. Granular material simulation treats contacts and friction between particles. Molecular dynamics and cosmological simulations use related particle state integration at very different physical scales. These systems have distinct force laws, but share data structures, neighbor queries, time stepping, and stability constraints. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-21 - Dynamics and Time Integration|CMU-21]]

### First-order state formulation

Many second-order mechanical equations can be rewritten as first-order systems by including velocity in the state. A particle satisfies $\dot x=v$ and $\dot v=F(x,v,t)/m$. The state vector is $(x,v)$. This allows the same integrator to advance different systems. For a thrown rock, force is gravity and acceleration is constant. For a spring, force depends on displacement and produces oscillation. For a pendulum, position is constrained to a circle, so the force must be projected onto the tangent or represented with an angle coordinate. For a double pendulum, the coupled state produces complex motion even though each force law is simple. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-21 - Dynamics and Time Integration|CMU-21]]

### Lagrangian mechanics and generalized coordinates

Newtonian mechanics tracks forces in Cartesian coordinates. Lagrangian mechanics instead defines kinetic energy $T$ and potential energy $V$, then uses Lagrangian $L=T-V$. For generalized coordinate $q$, the Euler-Lagrange equation is

$$
\frac{d}{dt}\frac{\partial L}{\partial\dot q}-\frac{\partial L}{\partial q}=Q,
$$

where $Q$ is generalized nonconservative force. A pendulum can use one angle $\theta$ instead of two Cartesian coordinates plus a length constraint. This can reduce the number of unknowns and enforce the constraint automatically. For an object with many degrees of freedom, choice of generalized coordinates is a modeling decision that affects numerical conditioning and implementation complexity. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-21 - Dynamics and Time Integration|CMU-21]]

### Reduce constraints with a coordinate that fits the motion

A generalized coordinate is a number (or compact vector of numbers) that specifies a system's configuration without storing redundant positions. A freely moving point in the plane needs two coordinates. Six freely moving billiard balls need twelve. A pendulum bob in the plane appears to need two Cartesian coordinates, but its distance from the pivot is fixed. Recording both coordinates and separately enforcing the circle constraint carries an unnecessary degree of freedom. Instead, use the angle $\theta$ from the downward vertical: the bob location is $x=\ell\sin\theta$, $y=-\ell\cos\theta$. Every value of $\theta$ already lies on the circle of radius $\ell$.

The Lagrangian method makes the benefit concrete. The bob's speed is $\ell\dot\theta$, so its kinetic energy is $T=\tfrac12m\ell^2\dot\theta^2$. Taking zero potential energy at the bottom gives $V=mg\ell(1-\cos\theta)$. The Lagrangian is $L=T-V$. Its derivatives are

$$
\frac{\partial L}{\partial\dot\theta}=m\ell^2\dot\theta,\qquad
\frac{d}{dt}\frac{\partial L}{\partial\dot\theta}=m\ell^2\ddot\theta,\qquad
\frac{\partial L}{\partial\theta}=-mg\ell\sin\theta.
$$

With no external torque, Euler–Lagrange says the time derivative on the left equals the coordinate derivative on the right. Dividing by $m\ell^2$ gives

$$
\ddot\theta=-\frac{g}{\ell}\sin\theta.
$$

This is already a complete equation of motion. Near the stable resting position, $\theta$ is small and $\sin\theta\approx\theta$. The equation becomes $\ddot\theta+\frac{g}{\ell}\theta=0$, a harmonic oscillator with angular frequency $\omega=\sqrt{g/\ell}$. Its motion is $\theta(t)=A\cos(\omega t)+B\sin(\omega t)$, where $A$ and $B$ are set by initial angle and velocity. This approximation predicts that a small pendulum's period is nearly independent of amplitude. At larger angles, the sine cannot be replaced by its argument; the nonlinear equation is generally advanced numerically. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-21 - Dynamics and Time Integration|CMU-21]]

### A model can be simple locally and rich globally

Particle systems illustrate a useful modeling pattern: give each element a small state and simple local rules, then let interactions produce a large-scale phenomenon. A particle's state typically includes position and velocity. A flocking agent can experience three forces: attraction toward a nearby group's center, separation from neighbors that are too close, and alignment toward their average velocity. These rules are a model of motion, not a claim that the simulation contains a complete psychology of birds or people. They can still reproduce recognizable group motion or estimate where a crowd may bottleneck during evacuation.

The same representation appears at very different scales. Particle interactions approximate fluid motion; contact and friction rules produce granular motion such as sand or popcorn; atom-scale forces support molecular dynamics; gravitational interactions model stars or dark matter. A naive all-pairs force calculation costs roughly $N^2$ interactions for $N$ particles. Hierarchical approaches such as Barnes–Hut or the fast multipole method group distant particles and approximate their combined influence, reducing work for appropriate spatial distributions. For sufficiently dense fluids or elastic materials, a continuum model may be a better description: it assigns fields such as velocity and density at every point and expresses their evolution with partial differential equations.

A mass-spring system is a discrete elastic model. For two endpoints at positions $x_1$ and $x_2$, the current length is $\ell=\|x_2-x_1\|$. A spring with rest length $\ell_0$ and stiffness $k$ stores potential energy

$$
V_{spring}=\frac12 k(\ell-\ell_0)^2.
$$

The spring force pulls the endpoints toward the rest length; differentiating the energy gives its direction and magnitude. A mesh can turn this small rule into cloth: each vertex is a particle, edges connect springs, and some vertices can be pinned to a moving character. Hair strands can be chains of particles joined by springs. These models need collision detection and collision response as well: otherwise a cloth point may pass through a character's body. Their simplicity is attractive, but spring networks can become stiff when springs strongly resist stretching. That stiffness is exactly where integration choices matter. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-21 - Dynamics and Time Integration|CMU-21]]

### Time steps change the behavior of a simulated system

An ordinary differential equation describes change continuously in time. A computer instead stores snapshots $q_k$ separated by time step $h$. The derivative is approximated by a difference,

$$
\dot q(t_k)\approx\frac{q_{k+1}-q_k}{h}.
$$

For an equation $\dot q=f(q,t)$, forward Euler evaluates the rate at the known current state and steps along it:

$$
q_{k+1}=q_k+h f(q_k,t_k).
$$

This is explicit: the right-hand side uses only known values. Imagine trying to follow a curved path by looking at its current tangent, then walking a distance proportional to $h$. If the time step is large, the tangent may point away from the path as it curves, so each step can make the next error worse.

A scalar equation exposes the stability limit without requiring a picture. Let $u$ decay according to $\dot u=-a u$, where $a>0$. The exact solution shrinks exponentially: $u(t)=u(0)e^{-at}$. Forward Euler gives

$$
u_{k+1}=(1-ah)u_k,
\qquad
u_n=(1-ah)^n u_0.
$$

For numerical values to shrink rather than grow without bound, the multiplier must have magnitude below one: $|1-ah|<1$, which means $0<h<2/a$. If $a$ is large, the allowed step is small. A stiff spring behaves like a system with a large rate constant: hair barely stretches, so explicit Euler may require thousands of internal steps per rendered frame to keep the simulation stable.

Backward Euler uses the unknown next state to evaluate the derivative:

$$
q_{k+1}=q_k+h f(q_{k+1},t_{k+1}).
$$

It is implicit because both sides depend on $q_{k+1}$. The solver must find a state satisfying the equation, often through an iterative method for nonlinear forces. For the scalar decay test this rearranges to $u_{k+1}=u_k/(1+ah)$, which shrinks for every positive $h$; this property is called unconditional stability for this test equation. But stability is not the same as physical accuracy. Backward Euler can drain energy from a frictionless pendulum, making the swing decay even though the model includes no friction.

Symplectic Euler staggers the update: first compute velocity using the current configuration, then update position using the new velocity. For acceleration $a(q)$,

$$
v_{k+1}=v_k+h a(q_k),\qquad q_{k+1}=q_k+h v_{k+1}.
$$

For simple conservative systems this method tends to keep energy bounded over long runs, often producing a much more convincing pendulum than either forward Euler's energy growth or backward Euler's numerical damping. Related methods include leapfrog and Verlet. No integrator wins on every criterion: stability, local accuracy, long-run conservation, symmetry and computational cost matter differently in visual animation, engineering and scientific prediction. A frame rate does not dictate a safe physics step; a simulation may need multiple internal substeps before each displayed frame. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-21 - Dynamics and Time Integration|CMU-21]]

### A short coupled-simulation example

Take two particles connected by a spring. Their state is positions $x_1,x_2$ and velocities $v_1,v_2$. Spring direction is the normalized displacement from one particle to the other; extension is current length minus rest length $L_0$. Hooke’s law applies force proportional to extension, with equal and opposite forces on the two particles. Gravity adds another force. Each time step computes forces, converts them to acceleration by dividing by mass, updates velocities, and advances positions. A large spring stiffness creates a high natural frequency, which makes explicit integration require small time steps. If particles also collide, closest-point or distance queries identify contact, and contact forces or projection enforce nonpenetration. The example combines vector calculus, geometry queries, ODE integration, and numerical stability without changing the basic state-update framework. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-12 - Geometric Queries|CMU-12]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-21 - Dynamics and Time Integration|CMU-21]]

### Simulation: from forces to trajectories

A particle is a useful abstraction for a point-like object whose state changes over time. Its state might contain position (x(t)) and velocity (v(t)). Newton's law gives

$$
\dot{x}=v,\qquad \dot{v}=m^{-1}F(x,v,t).
$$

The differential equation defines a continuous-time evolution, but a computer advances in finite time steps. Explicit Euler evaluates the derivative at the current state:

$$
x_{n+1}=x_n+h v_n,\qquad v_{n+1}=v_n+h a_n.
$$

This is simple and inexpensive, but the error accumulates and stiff systems can become unstable unless (h) is very small. For a pendulum, the acceleration depends on position, so each step's estimated angle changes the next acceleration. With a coarse time step, the numerical trajectory can gain or lose energy even if the ideal pendulum conserves it. The particle lecture uses ODE stepping, a swinging pendulum, and flocking/bird-like particles to connect an abstract integrator with animation behavior [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-07 - Particle systems|MITG-07]] (00:30:00–00:55:00; 01:05:00–01:15:00).

More accurate methods sample the derivative multiple times per step. Fourth-order Runge–Kutta (RK4) uses four slope estimates, including mid-step estimates, and combines them with weights (1,2,2,1):

$$
y_{n+1}=y_n+\frac{h}{6}(k_1+2k_2+2k_3+k_4).
$$

Here $k_1=f(t_n,y_n)$, $k_2=f(t_n+h/2,y_n+hk_1/2)$, $k_3=f(t_n+h/2,y_n+hk_2/2)$, and $k_4=f(t_n+h,y_n+hk_3)$. RK4 is much more accurate than Euler at the same step size for smooth dynamics, but it can still fail on stiff systems and does not enforce physical constraints automatically. The cloth lecture compares integration accuracy, Taylor expansions, and RK4-like divided differences, while emphasizing that numerical methods are approximations to the differential equation, not exact physics [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-08 - Cloth simulation|MITG-08]] (00:05:00–00:45:00).

A mass-spring cloth model turns a sheet into particles connected by springs. Stretch springs resist changes in edge length; shear springs resist diagonal distortion; bending springs resist folding. A spring from particle (i) to (j) exerts a force approximately

$$
F_{ij}=-k(\|x_i-x_j\|-\ell_0)\frac{x_i-x_j}{\|x_i-x_j\|}.
$$

where (k) is stiffness and ell_0 its rest length. Damping removes energy and can make motion settle. As stiffness grows, explicit integration needs smaller steps for stability. An implicit step solves for a future state that depends on its own future forces, which is more expensive per step but can tolerate stiffer springs. The lecture contrasts integrators and refers to cloth/mass-spring simulation, including the practical difficulty of obtaining stable behavior [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-08 - Cloth simulation|MITG-08]] (00:15:00–00:25:00; 00:50:00–01:05:00).

Cloth needs collision handling too: with a body or with itself, vertices and triangles can pass through one another if a time step is too large. A practical pipeline predicts motion, detects penetrations, then applies constraints or corrective forces. The result is only as convincing as the integration step, collision rule, and material parameters together. A visually plausible cloth is not simply “the right equation”; it is a numerical system whose errors and constraints are managed.

### Particle rules, state vectors, and a stability calculation

The 2021 particle lecture begins with a broad definition of a particle system: software must emit particles, maintain them while alive, apply forces, and remove them when their lifetimes end. A linked list is one reasonable structure when particle counts vary; a fixed-size ring or pool can work when a capacity bound matters. A fountain can be made with no interparticle forces at all: emit particles at one point with randomized initial velocities, apply gravity, and draw their ballistic paths. A stylized fire can reverse gravity, add a little curl, and map particle age to color and opacity. These are intentionally simplified rules: the visual goal may matter more than reproducing fluid mechanics. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-07 - Particle systems|MITG-07]] (00:02:00–00:15:00; 00:25:00–00:30:00)

For a particle of mass $m$ and position $\boldsymbol{x}(t)$, Newton's law is second order:

$$
m\ddot{\boldsymbol{x}}=\boldsymbol{F}(\boldsymbol{x},\dot{\boldsymbol{x}},t).
$$

Most general ODE solvers are written for first-order systems. Introduce velocity $\boldsymbol{v}=\dot{\boldsymbol{x}}$ and stack position and velocity into one state $\boldsymbol{z}=(\boldsymbol{x},\boldsymbol{v})$. Then

$$
\dot{\boldsymbol{z}}=
\begin{bmatrix}\dot{\boldsymbol{x}}\\\dot{\boldsymbol{v}}\end{bmatrix}
=\begin{bmatrix}\boldsymbol{v}\\m^{-1}\boldsymbol{F}(\boldsymbol{x},\boldsymbol{v},t)\end{bmatrix}
=\boldsymbol{f}(\boldsymbol{z},t).
$$

This “reduction to first order” trades one higher derivative for twice as many state variables. For $N$ point masses in three dimensions, the state has $6N$ scalar entries. It gives a software interface: a force model computes $\boldsymbol{f}$, an integrator advances the state, and a particle manager handles emission and death. The lecture distinguishes Lagrangian simulation, which follows material packets as moving particles, from Eulerian simulation, which stores fields such as density and velocity on fixed spatial cells. Smooth particle hydrodynamics (SPH) is a Lagrangian fluid family; grid methods for smoke are often Eulerian. Hybrid methods can use a grid for bulk water and particles for spray. Coupling matters: if a heavy body pushes a fluid, one-way coupling may update the fluid while ignoring the tiny feedback on the body; two-way coupling computes both directions at greater cost. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-07 - Particle systems|MITG-07]] (00:18:00–00:25:00; 00:32:00–00:40:00)

Forward Euler replaces the local trajectory by its tangent line:

$$
\boldsymbol{z}_{n+1}=\boldsymbol{z}_n+h\boldsymbol{f}(\boldsymbol{z}_n,t_n).
$$

The parameter $h$ is the time step. This is an explicit method because the right side uses only known values at step $n$. Taylor expansion gives the exact solution

$$
\boldsymbol{z}(t+h)=\boldsymbol{z}(t)+h\dot{\boldsymbol{z}}(t)
+\frac{h^2}{2}\ddot{\boldsymbol{z}}(t)+O(h^3),
$$

so the one-step error is $O(h^2)$. But a fixed duration $T$ takes about $T/h$ steps; accumulated global error is typically $O(h)$. Halving the step roughly doubles work and halves first-order global error. The Fall 2021 lecture makes this distinction explicit: “order of accuracy” describes error over a fixed interval, not the power on the first omitted term of one Taylor expansion. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-07 - Particle systems|MITG-07]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-08 - Cloth simulation|MITG-08]] (00:42:00–00:45:00; , 00:23:00–00:32:00)

A simple scalar test reveals stability. Consider decay $\dot{x}=-kx$ for $k>0$, whose exact solution is $x(t)=x_0e^{-kt}$. Forward Euler gives

$$
x_{n+1}=(1-hk)x_n.
$$

After $n$ steps, $x_n=(1-hk)^nx_0$. The solution decays only when the amplification factor has magnitude at most one:

$$
|1-hk|\le1\quad\Longleftrightarrow\quad 0\le hk\le2.
$$

For $k=4$ and $h=0.6$, the factor is $-1.4$: values alternate sign and grow in magnitude even though the exact motion decays smoothly. For $h=0.2$, the factor is $0.2$: discrete values decay but alternate sign rather than reproducing a smooth exponential. The first failure is instability; the second may be stable but visibly inaccurate. The lecture emphasizes that stability and accuracy are distinct properties. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-08 - Cloth simulation|MITG-08]] (00:08:00–00:18:00; 00:22:00–00:32:00)

This fast/slow mismatch is called stiffness. Large spring constants create fast oscillations; small time steps are then required even when the scene's visible motion is slow. A protein has tiny molecular vibrations and much slower folding; a cloth model can have tiny fast elastic modes and large-scale draping. The Courant–Friedrichs–Lewy (CFL) condition is a related spatial-temporal restriction for grid methods: information must not travel across many grid cells in one step if the discretization exchanges flux locally between neighbors. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-08 - Cloth simulation|MITG-08]] (00:19:00–00:22:30)

For the oscillator $\ddot{x}=-\omega^2x$, simultaneous forward Euler updates $x_{n+1}=x_n+h v_n$ and $v_{n+1}=v_n-h\omega^2x_n$ amplify the squared state norm by $1+\omega^2h^2$ per step. Even a small step injects energy over time. A semi-implicit (symplectic) Euler variant updates velocity first and then uses that new velocity for position:

$$
v_{n+1}=v_n-h\omega^2x_n,\qquad x_{n+1}=x_n+h v_{n+1}.
$$

For $\omega=2$, $h=0.25$, and initial $(x_0,v_0)=(1,0)$, this gives $v_1=-1$ and $x_1=0.75$. The exact oscillator energy $E=\frac12(v^2+\omega^2x^2)$ begins at $2$ and the first discrete value is $1.625$. This is not exact energy preservation, but the symplectic update avoids the systematic radial blow-up of simultaneous forward Euler for suitable steps; long-term error tends to oscillate rather than grow without bound. This is a useful extension of the lecture's stability discussion, not a claim that its captions derive this update. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-07 - Particle systems|MITG-07]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-08 - Cloth simulation|MITG-08]] (00:45:00–00:55:00; , 00:12:00–00:18:00)

Backward Euler evaluates the derivative at the unknown future state:

$$
\boldsymbol{z}_{n+1}=\boldsymbol{z}_n+h\boldsymbol{f}(\boldsymbol{z}_{n+1},t_{n+1}).
$$

For $\dot{x}=-kx$, rearranging gives $x_{n+1}=x_n/(1+hk)$. Its amplification factor remains between zero and one for every $h>0$, so this test problem cannot explode. The price is solving an equation for the unknown future state; for nonlinear forces this often needs Newton iteration and a linear-system solve. Large steps also damp away real motion. That artificial dissipation is numerical viscosity: cloth looks limp because the integrator removes energy, not because the material model specified that amount of damping. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-08 - Cloth simulation|MITG-08]] (00:43:00–00:56:00)

Higher-order explicit methods evaluate the force at multiple points in a step. The trapezoid method averages slopes at both ends and has $O(h^3)$ local error, hence second-order global accuracy. The midpoint method samples the slope halfway through. Classical Runge–Kutta 4 (RK4) combines four slopes with weights $1,2,2,1$ and is fourth-order accurate for smooth problems. It can still fail on stiff systems and does not enforce physical constraints automatically. Adaptive integration estimates whether a step was too large and retries with a smaller one, but choosing a reliable error indicator is difficult because the true solution is unknown. Accuracy, stability, energy behavior, force cost, and solve cost all matter. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-08 - Cloth simulation|MITG-08]] (00:32:00–00:43:00)

### A spring is a force derived from an energy

Represent a cloth vertex by a point mass at $\boldsymbol{x}_i$. A spring joins vertices $i$ and $j$, with rest length $\ell_0$ and stiffness $k$. Define $\boldsymbol{d}=\boldsymbol{x}_i-\boldsymbol{x}_j$, its length $\ell=\|\boldsymbol{d}\|$, and unit direction $\boldsymbol{u}=\boldsymbol{d}/\ell$. The elastic potential energy is

$$
U(\boldsymbol{x}_i,\boldsymbol{x}_j)=\frac{k}{2}(\ell-\ell_0)^2.
$$

The force is the negative energy gradient. Since $\nabla_{\boldsymbol{x}_i}\ell=\boldsymbol{u}$,

$$
\boldsymbol{F}_i=-\nabla_{\boldsymbol{x}_i}U
=-k(\ell-\ell_0)\boldsymbol{u},\qquad
\boldsymbol{F}_j=-\boldsymbol{F}_i.
$$

The sign has a direct interpretation. If the edge is too long, $\ell>\ell_0$, then $\boldsymbol{F}_i$ points toward $j$, pulling it shorter. If it is compressed, the force pushes the vertices apart. Hooke's law is linear in extension magnitude, but the direction also depends on vertex positions. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-08 - Cloth simulation|MITG-08]] (00:56:00–01:01:00)

For example, place $i$ at $(0,0)$ and $j$ at $(1.2,0)$, with $\ell_0=1$ and $k=10$. Then $\ell=1.2$, $\boldsymbol{u}=(-1,0)$, and $\boldsymbol{F}_i=(2,0)$; vertex $i$ is pulled right toward $j$ while $j$ receives $(-2,0)$. If both endpoints have mass one, these are their spring accelerations before gravity, damping, or other incident springs are added. At a mesh vertex, sum every incident force vector. A vertex attached to several springs receives the vector sum, not the average of their magnitudes.

The derivative of the force is needed by implicit integration and Newton solvers. Differentiating the unit direction as well as the length gives the block Jacobian

$$
K_{ii}=\frac{\partial\boldsymbol{F}_i}{\partial\boldsymbol{x}_i}
=-k\left[\left(1-\frac{\ell_0}{\ell}\right)I
+\frac{\ell_0}{\ell}\boldsymbol{u}\boldsymbol{u}^{T}\right],
\qquad
K_{ij}=\frac{\partial\boldsymbol{F}_i}{\partial\boldsymbol{x}_j}=-K_{ii}.
$$

The matrix $\boldsymbol{u}\boldsymbol{u}^{T}$ projects onto the spring axis; $I-\boldsymbol{u}\boldsymbol{u}^{T}$ projects perpendicular to it. At the stretched example, $K_{ii}$ is diagonal with entries $(-10,-5/3)$: moving the endpoint along the spring changes force more strongly than a small sideways perturbation. This is why a spring network's Jacobian couples nearby vertices and why an implicit system is sparse rather than dense. At $\ell\approx0$, the direction is undefined; robust code must avoid division by zero or handle coincident endpoints explicitly.

Velocity damping can be added as a separate force. A common axial dashpot is

$$
\boldsymbol{F}^{d}_i=-c\big((\boldsymbol{v}_i-\boldsymbol{v}_j)\cdot\boldsymbol{u}\big)\boldsymbol{u},
$$

where $c\ge0$ is a damping coefficient. It opposes relative motion along the spring, reducing oscillation. It does not replace elasticity: a damped spring still tends toward $\ell_0$. Gravity contributes $m_i\boldsymbol{g}$ at each vertex. Total force is the sum of gravity, all structural spring forces, damping, contact forces, and any artist-authored forces.

### A cloth mesh needs more than the visible triangle edges

A grid cloth has particles at lattice sites. Structural springs join immediate horizontal and vertical neighbors, resisting stretch. Shear springs join diagonal corners of each cell, resisting in-plane skew. Flexion (or bending) springs link vertices two cells apart, penalizing folds over a wider stencil. The 1995 model discussed in lecture uses these three families. Its diagonal triangulation example exposes anisotropy: if the mesh is triangulated in only one diagonal direction, folding along one edge can change no spring length, while folding along the other diagonal compresses many springs. The resulting stiffness depends on arbitrary tessellation direction rather than just fabric. Adding shear and flexion springs makes a sparse practical model more directionally balanced, though it remains a hand-designed network rather than a full continuum law. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-08 - Cloth simulation|MITG-08]] (01:06:00–01:11:00)

The material must also be constrained. A pinned curtain corner can be handled by fixing its position and velocity at every step or by eliminating the pinned degrees of freedom from the unknown vector. Contact with a table can be represented by a normal force or by projecting a penetrating vertex back onto the allowed side. Self-collision is much harder: candidate triangles must be found and penetrations resolved before fabric passes through itself. Refining a mesh adds degrees of freedom; a convergent model should approach consistent material behavior as spacing shrinks. If spring constants are not rescaled with edge length, refinement can make the system artificially stiff. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-08 - Cloth simulation|MITG-08]] (01:12:00–01:20:30)

For position vector $\boldsymbol{x}$, velocity $\boldsymbol{v}$, mass matrix $M$, elastic force $\boldsymbol{F}(\boldsymbol{x})$, and damping force $D\boldsymbol{v}$, an implicit step linearizes force around the current state:

$$
\boldsymbol{F}(\boldsymbol{x}+h\boldsymbol{v}+h^2\boldsymbol{a})
\approx \boldsymbol{F}(\boldsymbol{x})
+J_F(h\boldsymbol{v}+h^2\boldsymbol{a}),
\qquad J_F=\frac{\partial \boldsymbol{F}}{\partial \boldsymbol{x}}.
$$

Substitution into the future acceleration equation yields a sparse linear system for the unknown acceleration (or velocity increment), with matrix terms involving $M$, $hD$, and $h^2J_F$. The exact signs depend on whether force Jacobians are defined as derivatives of force or positive stiffness; use one convention consistently. The computational benefit is that a stiff spring's fast mode is treated in the solve instead of being chased by tiny explicit steps. The cost is assembling and solving a coupled system. The lecture motivates backward Euler for stiff cloth and identifies the solve as the price of stability; this linearization spells out that mechanism. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-08 - Cloth simulation|MITG-08]] (00:47:00–00:56:00; 01:12:00–01:16:00)

An ordinary spring chain also cannot model every material property. Hair may need curvature and twist states; twisting an elastic rod can cause it to buckle, a behavior a chain of point masses and length springs cannot express. Discrete elastic rods augment centerline positions with material-frame twist. Real fabric likewise has thickness, weave, friction, bending stiffness, and collision behavior. A simple spring mesh is useful because it chooses a manageable subset. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-08 - Cloth simulation|MITG-08]] (00:59:00–01:06:40; 01:11:00–01:12:10)

### Contact dynamics with a nonintersection guarantee

*Connect to:* mesh geometry and deformation; numerical optimization; finite-element dynamics; collision detection; friction.

**Incremental Potential Contact (IPC)** addresses a difficult guarantee: an elastic mesh should not pass through itself, an obstacle, or itself invert between time samples. The method solves each implicit time step by minimizing an incremental action that combines inertial, elastic, and external-force terms. It treats admissible motion as a trajectory through configurations that stay intersection-free. This distinction matters: checking only that the two endpoints of a time step do not overlap can miss a collision that happened in between.

The CCD guarantee preserves feasibility from an admissible starting configuration. If the elastic model also forbids inversion, the initial state must satisfy that condition as well. For a set of nonadjacent surface primitive pairs, compute unsigned distances $d_k(\mathbf{x})$. A trajectory is intersection-free when these distances stay positive for every time in the step. The admissible set is defined as the closure of the intersection-free trajectories: a motion that just touches and turns away is a limit of valid motions, but a motion that crosses through a surface is not. This definition handles surface obstacles, curves, and points in one framework. It is stronger than merely demanding a nonnegative unsigned distance at the endpoints.

The method adds a **localized barrier** to the incremental potential. Let $\widehat d$ be the user-set contact-distance accuracy. For $0<d<\widehat d$, the barrier is

$$b(d,\widehat d)=-(d-\widehat d)^2\ln\left(\frac{d}{\widehat d}\right),$$

and it is zero when $d\ge\widehat d$. It rises without bound as $d$ approaches zero, but vanishes for pairs farther apart than the target distance. The barrier-augmented potential is

$$B_t(\mathbf{x})=E_t(\mathbf{x})+\kappa\sum_{k\in C}b(d_k(\mathbf{x}),\widehat d),$$

where $E_t$ is the time-step potential and $\kappa$ adjusts barrier stiffness. The clamping is smooth enough for Newton optimization, and distant pairs need no barrier evaluation. The chosen $\widehat d$ controls geometric accuracy; the solver adapts $\kappa$ to balance force strength against conditioning. Too small a $\kappa$ allows surfaces to get unnecessarily close before the barrier repels; too large a value makes the system stiff.

The barrier by itself does **not** guarantee that a numerical descent step stays valid. A Newton direction can pass through a collision and land in a lower-energy configuration on the other side. IPC uses a collision-aware line search: continuous collision detection (CCD) finds an upper bound on the step before any primitive pair intersects; then backtracking reduces the step until the potential decreases. For noninverting elastic models, an inversion-aware limit is combined with the collision limit. This is the mechanism behind the paper's intersection-free trajectory claim. Floating-point CCD is used in the solver; the authors ran slower exact rational CCD afterward on selected difficult examples as a check, and report that rational CCD was roughly 30 times slower.

The Newton solver projects local elasticity and barrier Hessians to positive semidefinite form before assembly. The assembled system is then positive definite with the inertial mass contribution. Each iteration solves a linear system, takes a CCD-limited descent step, rebuilds the nearby contact set, and repeats until a velocity-scaled step tolerance is met. The paper measures momentum-balance accuracy through the optimization residual. Barrier derivatives yield nonnegative contact-force magnitudes $\lambda_k$; a force is applied only within the selected contact distance. This gives an $\widehat d$-complementarity condition: force-bearing pairs must be within $\widehat d$ of contact.

The barrier needs accurate distance derivatives. A vertex-triangle distance and an edge-edge distance each have several closest-point cases: vertex-to-vertex, point-to-edge, or interior projection. Each active case has a closed-form distance function and analytic derivatives. The method stores the active case with its culled contact set. At parallel edge-edge configurations, the ordinary distance function is not differentiable. IPC multiplies the edge-edge barrier by a smooth **mollifier** that fades it out near parallel degeneracy. Point-triangle pairs remain available to prevent missing collisions. This smoothing protects Newton convergence without claiming that the raw distance formula is smooth at every configuration.

#### Friction inside the same solve

Coulomb friction depends on the normal contact force and opposes tangential sliding. At a contact pair, write the relative tangential displacement as $\mathbf{u}_k=T_k(\mathbf{x})^T(\mathbf{x}-\mathbf{x}_t)$, where $T_k$ gives a two-dimensional tangent basis. The sliding velocity is $\mathbf{u}_k/h$. Under the maximal dissipation principle, friction uses the largest opposing force allowed by $\mu\lambda_k$. This law is difficult for Newton's method because it switches nonsmoothly between sliding and sticking, and the force depends on the yet-unknown contact reaction.

IPC smooths the force near zero sliding speed with a user-controlled velocity tolerance $\epsilon_v$. Then, within a nonlinear solve, it **lags** the tangent basis and normal force at values from the preceding iterate. With those quantities held fixed, the smoothed friction force becomes the gradient of a scalar dissipative potential. The method minimizes the barrier-augmented potential plus this lagged friction potential, then updates the basis and contact force and solves again. This is a practical variational approximation: the geometric contact and the frozen friction potential fit in one Newton framework. The authors do not guarantee convergence of every lagged force/basis update in all settings, so that limitation should be retained.

#### What the paper tests

The paper reports simulations of thin elastic rods and mats, card houses, arches, high-speed ball impact, funnel and point/edge obstacles, large mass and stiffness ratios, and persistent chains of contact. It reports no tunneling at speeds up to 1000 m/s for its ball-and-thin-board example, intersection-free high-speed tests, and stable 100-link chains. Its friction tests recover the expected threshold: a block on a slope stays still when $\mu=0.5$ and slides when $\mu=0.49$ for the chosen slope. It also demonstrates stick-slip oscillation and contact with codimensional obstacles. Most experiments used CPU sparse direct solvers; the largest “squishy ball” example used AMGCL. Runtime and memory therefore depend on the paper's mesh size, time step, solver, hardware, and convergence tolerances.

The paper compares IPC with graphics and engineering codes, then implements several sequential quadratic programming (SQP) contact approaches in a common benchmark. The authors report that the compared methods often need small time steps or large contact offsets to avoid failure, while IPC succeeds on their chosen tests. This is evidence about those methods, settings, and benchmark examples. It is not a proof that every contact solver fails or that IPC is inexpensive at arbitrary mesh scale. Cost still rises with resolution and active contacts; exact rational CCD is expensive; friction updates are lagged; and solving the barrier system may be poorly conditioned under extreme deformation.

**Source:** Li et al., “Incremental Potential Contact: Intersection- and Inversion-free, Large-Deformation Dynamics” (2020), complete paper body and conclusion, PDF pp. 1–20. [Primary PDF](https://ipc-sim.github.io/file/IPC-paper-350ppi.pdf).

### Friction and empirical tests for contact dynamics

#### Friction inside the same solve

Coulomb friction depends on the normal contact force and opposes tangential sliding. At a contact pair, write the relative tangential displacement as $\mathbf{u}_k=T_k(\mathbf{x})^T(\mathbf{x}-\mathbf{x}_t)$, where $T_k$ gives a two-dimensional tangent basis. The sliding velocity is $\mathbf{u}_k/h$. Under the maximal dissipation principle, friction uses the largest opposing force allowed by $\mu\lambda_k$. This law is difficult for Newton's method because it switches nonsmoothly between sliding and sticking, and the force depends on the yet-unknown contact reaction.

IPC smooths the force near zero sliding speed with a user-controlled velocity tolerance $\epsilon_v$. Then, within a nonlinear solve, it **lags** the tangent basis and normal force at values from the preceding iterate. With those quantities held fixed, the smoothed friction force becomes the gradient of a scalar dissipative potential. The method minimizes the barrier-augmented potential plus this lagged friction potential, then updates the basis and contact force and solves again. This is a practical variational approximation: the geometric contact and the frozen friction potential fit in one Newton framework. The authors do not guarantee convergence of every lagged force/basis update in all settings, so that limitation should be retained.

#### What the paper tests

The paper reports simulations of thin elastic rods and mats, card houses, arches, high-speed ball impact, funnel and point/edge obstacles, large mass and stiffness ratios, and persistent chains of contact. It reports no tunneling at speeds up to 1000 m/s for its ball-and-thin-board example, intersection-free high-speed tests, and stable 100-link chains. Its friction tests recover the expected threshold: a block on a slope stays still when $\mu=0.5$ and slides when $\mu=0.49$ for the chosen slope. It also demonstrates stick-slip oscillation and contact with codimensional obstacles. Most experiments used CPU sparse direct solvers; the largest “squishy ball” example used AMGCL. Runtime and memory therefore depend on the paper's mesh size, time step, solver, hardware, and convergence tolerances.

The paper compares IPC with graphics and engineering codes, then implements several sequential quadratic programming (SQP) contact approaches in a common benchmark. The authors report that the compared methods often need small time steps or large contact offsets to avoid failure, while IPC succeeds on their chosen tests. This is evidence about those methods, settings, and benchmark examples. It is not a proof that every contact solver fails or that IPC is inexpensive at arbitrary mesh scale. Cost still rises with resolution and active contacts; exact rational CCD is expensive; friction updates are lagged; and solving the barrier system may be poorly conditioned under extreme deformation.

**Source:** Li et al., “Incremental Potential Contact: Intersection- and Inversion-free, Large-Deformation Dynamics” (2020), complete paper body and conclusion, PDF pp. 1–20. [Primary PDF](https://ipc-sim.github.io/file/IPC-paper-350ppi.pdf).

### Figure U15 - The contact barrier grows near zero separation

![[Visual Recall/University Graphics and Vision/contact-barrier.png]]

*The plot checks the positive barrier shape for the paper distance model. It is not a complete collision solver: continuous collision detection and a feasibility-preserving line search are still required.*

## Optimization, constraints, and inverse kinematics

Optimization makes inverse problems explicit: define variables, constraints, objectives, and derivative information.

### Inverse kinematics as constrained optimization

Inverse kinematics asks for joint parameters that place an end effector at a target. Forward kinematics multiplies transforms along a skeleton chain to map joint angles to hand position. Inverse kinematics reverses that relationship: find joint variables whose forward result matches the target while satisfying joint limits or pose preferences. Define objective $E(\theta)=\tfrac12\|x_{hand}(\theta)-x_{target}\|^2$. Its gradient uses the kinematic Jacobian. A damped least-squares update avoids unstable large joint motion near singular configurations. Multiple poses can reach the same target, so regularization can prefer a natural pose or smaller change from the current configuration. This is an optimization problem embedded in a transform hierarchy. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-05 - Spatial Transformations|CMU-05]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-22 - Optimization|CMU-22]]

### Feasibility, constraints, and optimality

An optimization problem has a domain of possible parameters, an objective to minimize, and possibly constraints. A feasible point satisfies every constraint. If two constraints have no common solution, the problem is infeasible; no optimizer can find an answer that satisfies both. For an unconstrained differentiable objective, a local minimum generally has zero gradient, and a twice-differentiable minimum has positive-semidefinite Hessian. Under constraints, these conditions change: the gradient need not be zero because motion may be blocked by a boundary. Lagrange multipliers combine objective and active constraints into stationarity equations.

Convexity means every local minimum is global. A quadratic objective with positive-semidefinite Hessian is convex. Convexity is valuable because optimization guarantees become stronger; nonconvex graphics problems such as camera pose, shape recovery, and neural rendering can have many local minima. Starting points and parameterization matter. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-22 - Optimization|CMU-22]]

### Quadratic objectives and linear systems

A least-squares problem has residual vector $r=A\theta-b$ and objective $E(\theta)=\tfrac12\|A\theta-b\|^2$. Differentiation yields gradient $A^T(A\theta-b)$; setting it to zero gives normal equations $A^TA\theta=A^Tb$. If columns of $A$ are nearly dependent, the system is poorly conditioned and small data changes can cause large parameter changes. QR or SVD factorization can solve least squares more stably than directly forming normal equations. The connection between optimization and numerical linear algebra is direct: the objective’s geometry is encoded in matrix conditioning. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-02 - Linear Algebra (P)Review|CMU-02]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-22 - Optimization|CMU-22]]

Newton’s method locally approximates the objective with a quadratic and solves $H\Delta\theta=-\nabla E$, where $H$ is the Hessian. For a true quadratic objective, one exact Newton step reaches the minimizer if $H$ is invertible. For nonlinear objectives, the Hessian changes, so line searches or trust regions limit step size. Quasi-Newton methods estimate curvature from gradients and avoid explicitly forming a large Hessian. Gradient descent is cheaper per step but may require many iterations in a narrow valley. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-22 - Optimization|CMU-22]]

### Differentiating through a graphics pipeline

If an image $I(\theta)$ depends on shape, camera, or material parameters, define loss $E(\theta)=\|I(\theta)-I_{target}\|^2$. The chain rule propagates derivatives through transformations, projection, interpolation, shading, and compositing. Some operations are smooth, such as matrix multiplication; others are discontinuous, such as visibility changes at an edge. Derivative-based optimization can struggle at visibility boundaries because a tiny parameter change suddenly reveals or hides a surface. This motivates soft visibility approximations, gradient estimators, or alternative objectives. The same derivative machinery used for simple curve fitting therefore reaches inverse graphics and data fitting. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-03 - Vector Calculus (P)Review|CMU-03]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-22 - Optimization|CMU-22]]

### Feasible sets, existence, and the difference between local and global

A useful standard form is: choose a parameter vector $x\in\mathbb R^n$ to minimize an objective $f_0(x)$ subject to inequalities $g_i(x)\le b_i$. The objective assigns a cost to each candidate; the constraints determine which candidates are allowed. A maximization can be written as minimization by negating the score. An equality $g(x)=c$ can be encoded by two inequalities, $g(x)\le c$ and $-g(x)\le -c$. For example, maximize the area enclosed by a curve while fixing its perimeter. The circle solves this classical isoperimetric problem, but the optimization setup is simply “minimize negative area subject to the perimeter constraint.”

Before searching for a minimum, check whether an allowed point exists. That is feasibility. Contradictory requirements such as $x=1$ and $x=-1$ leave no feasible point; a solver cannot satisfy both. Even when a feasible point exists, a minimum need not be attained. The function $f(x)=x$ over the whole real line decreases without bound, so there is no smallest value. The function $f(x)=e^{-x}$ is bounded below by zero but approaches zero only as $x\to\infty$; no finite $x$ attains it. Sufficient existence conditions include a continuous objective over a nonempty compact feasible set (closed and bounded), or a coercive objective on a closed feasible set, where “coercive” means the cost grows toward $+\infty$ as the parameter moves arbitrarily far away.

A global minimum is no worse than every feasible alternative. A local minimum is no worse than candidates in a small neighborhood. The two can differ: pulling a string tight along a path tangled around buildings can produce a locally shortened path that is still globally much longer than a direct route. Multiple global minima are possible too. In the lecture's visualization, minimizing $f(x_1,x_2)=x_1^2-x_2^2$ over the unit disk $x_1^2+x_2^2\le1$ gives two equal optima, $(0,1)$ and $(0,-1)$. Global search can be expensive, so many practical graphics methods seek a useful local solution and depend on initialization.

For a differentiable unconstrained function, a local minimum must have zero gradient: to first order, no direction can reduce the value. But this only identifies a critical point, which may also be a maximum or saddle. The Hessian $H$ is the matrix of second derivatives and describes local curvature. At a twice-differentiable local minimum it must be positive semidefinite, meaning $v^T H v\ge0$ for every direction $v$. A bowl passes this test; a saddle has a direction with negative curvature. With active constraints, the unconstrained gradient need not vanish: the lowest point of a bowl restricted to a boundary can have a slope pointing outside the allowed region. The Karush–Kuhn–Tucker conditions add multipliers for active constraints and encode that balance. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-22 - Optimization|CMU-22]]

### Convexity and why quadratic energy often reduces to linear algebra

A set is convex if the straight segment between any two points in it stays inside it. A function is convex if its graph lies below every chord joining two points on the graph. This rules out hidden bowls separated by hills: every local minimum of a convex function is global. Strong convexity adds curvature in every direction and gives a unique minimum. These properties make convex problems attractive in geometry processing, image processing, and animation, though “convex” alone does not guarantee every large problem is cheap.

A central example is a quadratic objective

$$
f(x)=\frac12 x^T A x-b^T x,
$$

where $x$ is a vector of unknowns, $A$ is a symmetric positive semidefinite matrix, and $b$ is a fixed vector. The gradient is $\nabla f(x)=Ax-b$ and the Hessian is $A$. Setting the gradient to zero yields $Ax=b$. Thus a linear system with a positive semidefinite matrix is also the condition for the bottom of a convex quadratic bowl. If $A$ is positive definite, the bowl curves upward in every direction and the minimizer is unique. If $A$ is only semidefinite, the bowl may be flat along some directions, so many points can achieve the same minimum. If $A$ is indefinite, the graph has saddle directions and may fall without bound. The quadratic error used to choose a mesh-collapse position is one graphics example of this structure. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-22 - Optimization|CMU-22]]

### Gradient descent, step control, and curvature

Gradient descent is a repeated local improvement rule. At a candidate $x_k$, the gradient $\nabla f(x_k)$ points toward steepest increase, so a small move in the opposite direction decreases the objective:

$$
x_{k+1}=x_k-\tau_k\nabla f(x_k).
$$

The positive step length $\tau_k$ determines how far to travel. This update is forward Euler applied to the gradient-flow differential equation $\dot x=-\nabla f(x)$. The connection explains why step size matters: a huge step can overshoot and jump around the objective; a tiny step makes slow progress. A line search tests candidate lengths and selects one that decreases the objective while meeting sufficient-decrease and curvature conditions, such as Armijo–Wolfe conditions. A small fixed step is simpler but can waste iterations or fail on poorly scaled problems.

The method follows only the slope at the current point. In a long, narrow valley, the steep direction across the valley can dominate the gradient even though progress toward the minimum is along the valley. Repeated steps zigzag across it. Newton's method uses the Hessian to scale directions according to curvature:

$$
x_{k+1}=x_k-\tau_k H(x_k)^{-1}\nabla f(x_k).
$$

This is obtained by minimizing a local quadratic approximation of the objective. Near a well-behaved minimum it can converge much faster than gradient descent. However, an indefinite Hessian can direct a step toward a saddle instead of a minimum, and computing or solving with a full Hessian may be costly. Quasi-Newton methods such as L-BFGS approximate curvature from gradients; trust-region methods limit the update to a neighborhood where the local approximation is credible. There is no universal optimizer for all nonconvex graphics problems; objective structure, scaling, initialization, constraints, and prior experience guide the choice. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-22 - Optimization|CMU-22]]

### Function spaces and image error

Vectors need not be finite coordinate lists. A function can be treated as a vector in a function space. For image functions $I(x,y)$, an inner product can be $\langle I,J\rangle=\int I(x,y)J(x,y)dxdy$. Its induced $L^2$ norm is

$$
\|I\|_{L^2}=\sqrt{\int |I(x,y)|^2dxdy}.
$$

This measures total squared difference across an image domain. A discrete image approximates the integral with a sum over pixels. The distinction between a continuous field and its finite samples matters: changing pixel resolution changes the discrete sum unless sample spacing or area weights are handled correctly. Function-space gradients are functions too. For objective $E[I]=\tfrac12\int (I(x)-T(x))^2dx$, the functional derivative is $I-T$; at every point it says whether the image is too bright or too dark relative to target. This is the continuous analogue of an ordinary least-squares gradient. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-02 - Linear Algebra (P)Review|CMU-02]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-03 - Vector Calculus (P)Review|CMU-03]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-22 - Optimization|CMU-22]]

### Automatic differentiation

Automatic differentiation computes derivatives by applying the chain rule to the arithmetic operations in a program. A value can be paired with its derivative; addition and multiplication propagate both. If $z=xy$, then derivative with respect to a parameter is $\dot z=\dot x y+x\dot y$. This differs from finite differences, which estimate a derivative by evaluating nearby values and can suffer cancellation or step-size error. It also differs from symbolic differentiation, which transforms an expression into a new symbolic formula. In graphics optimization, automatic differentiation can produce gradients through complex energy computations, though discrete visibility changes and branching still require care. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-21 - Dynamics and Time Integration|CMU-21]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-22 - Optimization|CMU-22]]

### Computing derivatives: four different contracts

A simulation or optimizer often needs a derivative of a complicated energy with respect to thousands of positions. The derivative is not merely a formula: the method used to obtain it has its own accuracy and cost contract.

**Finite differences** perturb one input and compare two outputs. For scalar $f$, a forward difference is

$$
f'(x)\approx\frac{f(x+h)-f(x)}{h}.
$$

This works even when $f$ is a black box, which is why it is useful for debugging and checking a derivative implementation. But choosing $h$ is delicate. A large step measures an average slope over a nonlocal interval (truncation error); a tiny step subtracts nearly equal floating-point numbers, losing significant digits through cancellation. In many dimensions, estimating every component this way may require one or more function evaluations per input variable.

**Automatic differentiation** carries derivative information through the operations the program performs. In forward mode, a scalar is paired with its value and derivative, $(x,\dot x)$. For a product $z=xy$, the product rule gives $\dot z=\dot x y+x\dot y$. For instance, initialize $x=(2,1)$, where the second component means $dx/dx=1$. Multiplying it by itself gives $x^2=(4,4)$; multiplying by a constant $a$ gives $(4a,4a)$, the value and derivative of $ax^2$ at $x=2$. This computes derivatives to floating-point precision without selecting a perturbation size. It requires arithmetic types that propagate derivatives. Forward mode is especially natural when there are few inputs; reverse mode is advantageous for many inputs and a small number of outputs, which is common when differentiating one scalar loss with respect to many parameters.

**Symbolic differentiation** transforms an expression tree into a new tree for its derivative. Once built, the derivative expression can be evaluated repeatedly. Symbolic tools are helpful for algebra and analysis, but repeated chain-rule expansion can create very long formulas, especially for vector and matrix expressions. The result may be mathematically correct yet obscure a simple geometric structure.

**Geometric differentiation** asks what perturbation changes the quantity and by how much. For a triangle with fixed edge vector $e$ as base and a moving opposite vertex $p$, area is $A=\tfrac12\|e\|h$, where $h$ is the perpendicular height. Moving $p$ parallel to $e$ changes neither the base nor height, so the gradient has no component along $e$. Moving it one unit in the height-increasing direction changes area by $\tfrac12\|e\|$, so the gradient's magnitude is half the base length and its direction is perpendicular to $e$. In 3D, if $n$ is the oriented unit normal, that direction is given up to sign by $n\times e$; the sign depends on which triangle vertex moves and the orientation convention. This compact answer is easier to reason about than an expanded symbolic formula. The gradient points toward fastest increase; for potential energy $V$, the corresponding conservative force is $F=-\nabla V$, pointing toward decreasing potential. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-21 - Dynamics and Time Integration|CMU-21]]

<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/28 - Visibility Queries and Ray Intersections|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/30 - Mesh Operators, Boundaries and Diffusion|Next chapter]]
