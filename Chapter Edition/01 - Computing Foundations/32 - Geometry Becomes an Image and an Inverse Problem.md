# 32 — Geometry Becomes an Image and an Inverse Problem

[[Chapter Edition/01 - Computing Foundations/00 - Reading Map|Reading map]] · [[Chapter Edition/01 - Computing Foundations/31 - Implement Probability with Numerical Contracts|Previous chapter]] · [[Chapter Edition/01 - Computing Foundations/33 - Projection, Clipping, and Raster Sampling|Next chapter]]

The chapter begins a continuous camera journey from geometric frames through projection, clipping, raster coordinates, and inverse reconstruction. Derivatives, uncertainty, and numerical rank now explain what an image can reveal about the 3D scene that produced it.

**By the end:** Carry points and directions through camera transforms and projection, then identify what depth, multiple views, or calibration can recover.

**Recall question:** Which information is lost at projection, and what additional observation is needed to recover depth?

<!-- Derived from Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md; source lines 78258–80693; source-span SHA256 47065f3eba66be05eb316ce5cc54acdd45eb5da3962043b362a293ae65ebc2e5. -->

<!-- CHAPTER-SOURCE-BEGIN -->
# Geometry becomes an image and an inverse problem

Carry one geometric object through frames, projection, clipping and raster coordinates, then invert the process using depth or multiple views. Derivatives, covariance, rank, numerical error and sampling now explain calibrated cameras, visibility, optics and reconstruction.

<!-- begin V11.00 -->
## From Coordinate Frames to Camera Images: Projective Geometry and Cameras

**Received coverage: Parts 1001–1100 / M061–M066.** This volume continues the vector, transform, calculus, probability, and numerical-analysis books. Essential derivations and complete examples stay here; the recall companion provides short retrieval prompts and direct figure links.

Our default mathematical camera is right-handed and looks down local $-Z$. Vectors are columns, and $PVM$ applies model, then view, then projection. Matrix memory layout is a separate choice. Raster examples declare image origin and dimensions. Positive axial distance is $s=-z$; some local derivations call it $d$ and reserve $q$ for hardware depth. Focal length and the far-plane distance have different units and meanings even where a local section uses the letter $f$ for one of them.

[[Visual Recall/11 - Projective Geometry and Cameras - Visual Recall|Recall, figures, and retrieval prompts]] · [[Chapter Edition/01 - Computing Foundations/18 - Vectors for Light and Frames#Keep the promises visible while moving between representations|Transform foundations]] · [[Chapter Edition/01 - Computing Foundations/19 - Coordinate Frames and Linear Systems#The graphics problem hiding inside a matrix equation|Solves and conditioning]] · [[Chapter Edition/01 - Computing Foundations/20 - Choose Matrix Factorizations from Structure#Functions, limits, continuity, and scalar change|Jacobians and optimization]] · [[Chapter Edition/01 - Computing Foundations/39 - Research Navigation and Source Boundaries#Learning route|Uncertainty and numerical reliability]]

A rendered image is the end of a chain of coordinate changes. A mesh point starts in a frame attached to its object, the camera describes it in a view frame, projection places it in homogeneous clip coordinates, and rasterization assigns samples to a pixel grid. Each stage has its own conventions and numerical limits.

We will keep one reference convention visible throughout: a right-handed world, $+X$ right, $+Y$ up, camera forward along local $-Z$, column vectors, and left-multiplied matrices. API depth ranges, depth direction, memory layout, framebuffer origin, and texture origin remain separate choices. The narrative first builds the frames and camera inverse, derives projection and depth, then follows the result through raster sampling, frustum tests, time-dependent views, projective geometry, and calibrated optics.

The last chapters turn the pipeline around: measured pixels constrain camera rays and world geometry. That reverse path exposes the same conventions again, alongside lens distortion and uncertainty.


<!-- end V11.00 -->


## a vector is a measured displacement, a matrix is a frame rule

A vector is a displacement or other directed quantity. Its coordinate tuple is the list of measurements made along a chosen basis. If the basis changes, the tuple changes even when the displacement does not. A point instead identifies a place relative to an origin; subtracting two points produces a displacement, while adding two points has no frame-independent geometric meaning.

A matrix is useful when a rule must transform every vector in a space. The simplest way to specify a linear rule is to state where it sends each basis direction. For a 2D rule whose basis images are $(2,1)$ and $(-1,3)$, the matrix is

$$
A=\begin{bmatrix}2&-1\\1&3\end{bmatrix}.
$$

For input $\mathbf{x}=(4,2)$, the output is

$$
A\mathbf{x}=4(2,1)+2(-1,3)=(6,10).
$$

The columns are not arbitrary table entries: they are the transformed unit directions. This reading makes multiplication order testable. If $A$ changes camera coordinates into world coordinates, its inverse changes world coordinates into camera coordinates. A pose matrix carries both an orientation and an origin; its inverse reverses that mapping. State the from-frame and to-frame on every transform, because the array shape alone cannot say what it means.

A camera then adds a projection: it maps a 3D direction from the optical center to a 2D image measurement. A pinhole model first expresses a point in camera coordinates. For camera coordinates $(X,Y,Z)$ in a forward-positive optical convention, ideal normalized coordinates are $(X/Z,Y/Z)$, provided $Z>0$. Intrinsic parameters then turn these into pixels. Rendering APIs often use a camera looking down negative $z$ and homogeneous clip coordinates; that is a different convention, not a contradiction. Define the convention before writing the formula.

<!-- begin V11.01 -->
## A coordinate is a claim about a frame

```mermaid
flowchart LR
 L["Local point"] --> M["Model to world"] --> V["Inverse camera pose"] --> P["Projection to clip"] --> C["Clip inequalities"] --> D["Divide by valid w"] --> R["Viewport to raster"]
 class L,M,V,P blue; class C,D amber; class R green;
classDef blue fill:#e5eff9,stroke:#2463a6,color:#18354c;
classDef green fill:#e3f3ed,stroke:#16806a,color:#174b3c;
classDef amber fill:#fff0dc,stroke:#c57820,color:#64400d;
```


*Coordinate frames, local geometry, world conventions, and large-world context.*

We begin before the camera: a tuple becomes meaningful only after we declare the origin and basis that interpret it.

**A coordinate system is a basis plus an origin**

The most foundational idea is that a coordinate tuple is not a geometric object by itself.

Consider the tuple:

$$
 \begin{bmatrix}2\\1\\0\end{bmatrix}. 
$$

What does it represent?

Without a coordinate frame, we do not know. It could identify a point two metres right and one metre up from the world origin, a vertex relative to the centre of a mesh, or a position relative to a moving camera.

A coordinate frame consists of an origin and basis vectors.

Let:

$$
 \mathcal F=(\mathbf o,\mathbf b_1,\mathbf b_2,\mathbf b_3). 
$$

Here $\mathbf o$ is the frame's origin, while $\mathbf b_1,\mathbf b_2,\mathbf b_3$ identify its coordinate directions.

A point with coordinates:

$$
 \mathbf p_{\mathcal F}= \begin{bmatrix}x\\y\\z\end{bmatrix} 
$$

represents the geometric point:

$$
 \boxed{ \mathbf p= \mathbf o+x\mathbf b_1+y\mathbf b_2+z\mathbf b_3 }. 
$$

This equation is the actual meaning of a coordinate system.

### Deriving matrix notation

Arrange the basis vectors as columns:

$$
 B= \begin{bmatrix} |&|&|\\ \mathbf b_1&\mathbf b_2&\mathbf b_3\\ |&|&| \end{bmatrix}. 
$$

Then:

$$
 B\mathbf p_{\mathcal F} = x\mathbf b_1+y\mathbf b_2+z\mathbf b_3. 
$$

Therefore:

$$
 \boxed{ \mathbf p=\mathbf o+B\mathbf p_{\mathcal F} }. 
$$

The basis matrix must be invertible if we want a unique coordinate representation for every point in the space.

If two basis vectors are linearly dependent, coordinates cannot uniquely describe arbitrary 3D positions.

### Numerical example

Suppose a frame has origin:

$$
 \mathbf o= \begin{bmatrix}10\\2\\0\end{bmatrix} 
$$

and basis:

$$
 \mathbf b_1= \begin{bmatrix}0\\1\\0\end{bmatrix},\quad \mathbf b_2= \begin{bmatrix}-1\\0\\0\end{bmatrix},\quad \mathbf b_3= \begin{bmatrix}0\\0\\1\end{bmatrix}. 
$$

Its basis matrix is:

$$
 B= \begin{bmatrix} 0&-1&0\\ 1&0&0\\ 0&0&1 \end{bmatrix}. 
$$

Consider the local coordinates:

$$
 \mathbf p_{\mathcal F}= \begin{bmatrix}2\\1\\3\end{bmatrix}. 
$$

Then:

$$
 B\mathbf p_{\mathcal F} = \begin{bmatrix}-1\\2\\3\end{bmatrix}. 
$$

Add the frame origin:

$$
 \boxed{ \mathbf p= \begin{bmatrix}9\\4\\3\end{bmatrix} }. 
$$

Thus the local tuple $(2,1,3)$ and the global tuple $(9,4,3)$ describe the same geometric point in different frames.

### General versus orthonormal bases

A basis need not be perpendicular or have unit-length vectors.

If $B$ is a general invertible matrix, recovering local coordinates requires:

$$
 \boxed{ \mathbf p_{\mathcal F} = B^{-1}(\mathbf p-\mathbf o) }. 
$$

If the basis is orthonormal:

$$
 B^TB=I. 
$$

Therefore:

$$
 B^{-1}=B^T. 
$$

The inverse becomes:

$$
 \boxed{ \mathbf p_{\mathcal F} = B^T(\mathbf p-\mathbf o) }. 
$$

This is much cheaper and simpler than a general inverse.

An ordinary rigid camera uses an orthonormal basis.

A transformed mesh may have nonuniform scale or shear, so its full basis need not be orthonormal.

**Local or object coordinate space**

Local space describes geometry relative to an object's own coordinate frame.

Suppose you create a cube centred at its local origin:

$$
 (-1,-1,-1),\quad (1,-1,-1),\quad\ldots 
$$

The vertices do not know where the cube is placed in the game world.

Their coordinates describe the cube's shape.

The object's transform provides its world placement.

Let the object have scale $S$, rotation $R$, and translation $\mathbf t$.

Then:

$$
 \boxed{ \mathbf p_{\mathrm{world}} = \mathbf t+RS\mathbf p_{\mathrm{local}} }. 
$$

With homogeneous matrices:

$$
 \boxed{ M=TRS }. 
$$

The rightmost operation happens first:

1. Scale in local axes.
2. Rotate the scaled geometry.
3. Translate it into world space.

This order is not arbitrary.

### Transformation order changes the result

Consider:

$$
 \mathbf p= \begin{bmatrix}1\\0\\0\end{bmatrix}. 
$$

Let $R$ rotate 90° counterclockwise about $Z$, and let translation be:

$$
 \mathbf t= \begin{bmatrix}3\\0\\0\end{bmatrix}. 
$$

Rotate first:

$$
 R\mathbf p= \begin{bmatrix}0\\1\\0\end{bmatrix}. 
$$

Then translate:

$$
 \mathbf t+R\mathbf p= \begin{bmatrix}3\\1\\0\end{bmatrix}. 
$$

But translate first:

$$
 \mathbf p+\mathbf t= \begin{bmatrix}4\\0\\0\end{bmatrix}. 
$$

Then rotate:

$$
 R(\mathbf p+\mathbf t)= \begin{bmatrix}0\\4\\0\end{bmatrix}. 
$$

Thus:

$$
 \boxed{TR\mathbf p\ne RT\mathbf p} 
$$

in general.

A very common engine bug is applying translation, rotation, and scale in the wrong order.

### Parent-child transformations

For a child object attached to a parent, the world matrix is:

$$
 \boxed{ M_{\mathrm{child,world}} = M_{\mathrm{parent,world}} M_{\mathrm{child,local}} }. 
$$

For a deeper hierarchy:

$$
 M_{\mathrm{world}} = M_0M_1\cdots M_k. 
$$

The transformations compose from the root toward the child.

This is the mathematical basis of scene graphs, skeletal hierarchies, robot arms, cameras attached to vehicles, and editor gizmos.

An important limitation: multiplying two general TRS matrices can introduce shear even if neither transform explicitly stores shear. A hierarchy with rotated nonuniform scales may not be exactly representable by a simple translation–quaternion–scale decomposition.

**World coordinate space**

World space is a shared reference frame used to place scene objects relative to one another.

If a mesh has a local vertex:

$$
 \mathbf p_L, 
$$

its world position is:

$$
 \mathbf p_W=M\mathbf p_L 
$$

using homogeneous coordinates.

Two objects can have completely different local coordinate systems but coexist in the same world frame.

### World space is a chosen convention

The physical world does not come with a privileged $+X,+Y,+Z$ basis.

An engine chooses one.

One project might use $+Y$ as up.

Another might use $+Z$ as up.

Similarly, an engine may choose metres, centimetres, or arbitrary game units.

These are modeling conventions that must remain consistent.

A coordinate conversion between conventions can include axis permutations, sign changes, scale changes, and translation.

For example, converting centimetres to metres requires multiplication by:

$$
 0.01. 
$$

If one subsystem treats positions as metres and another as centimetres, a matrix may be algebraically correct yet physically wrong.

### World origin and large scenes

Suppose a player is far from the world origin, with coordinates near:

$$
 (10^8,0,10^8). 
$$

Single-precision arithmetic loses fine local positional resolution at such magnitudes.

A camera-relative system can keep high-precision world coordinates but send nearby geometry to the GPU in coordinates relative to a local origin.

This is one reason to separate world-space modeling from view- or camera-relative rendering.

The large-world chapter will make this precision problem explicit.


<!-- end V11.01 -->

<!-- begin V11.02 -->
## A camera is an inverse pose

*View space, points and directions, affine frames, camera basis, look-at construction, view inverse, passive motion.*

Once object and world frames are explicit, the camera is another frame in the scene. Rendering expresses the world relative to that moving frame.

**View or camera coordinate space**

View space represents geometric positions relative to the camera.

In our reference convention:

- Camera position becomes the origin.
- Camera right becomes $+X$.
- Camera up becomes $+Y$.
- Camera forward corresponds to $-Z$.

Therefore a point directly in front of the camera has:

$$
 z_{\mathrm{view}}<0. 
$$

For example:

$$
 \boxed{ \mathbf p_{\mathrm{view}}= \begin{bmatrix}0\\0\\-5\end{bmatrix} } 
$$

describes a point five units in front of a camera located at the view-space origin.

The negative $Z$ coordinate is a convention, not a universal requirement of cameras.

Other systems use $+Z$ forward.

### Why use view space?

Projection becomes particularly simple when the camera is placed at the origin and aligned with the coordinate axes.

Instead of projecting rays from an arbitrary camera position and orientation, we first transform the world so that the camera becomes canonical.

Then every scene point can be projected using the same matrix.

This is the purpose of the view matrix:

$$
 \boxed{ \mathbf p_V=V\mathbf p_W }. 
$$

The actual camera does not need to move physically. We are changing coordinates.

**Clip coordinate space**

Clip space is produced by applying the projection matrix:

$$
 \boxed{ \mathbf p_C=P\mathbf p_V }. 
$$

A projected vertex is represented as a homogeneous four-vector:

$$
 \mathbf p_C= \begin{bmatrix} x_c\\y_c\\z_c\\w_c \end{bmatrix}. 
$$

Clip space is not yet normalized device coordinates.

The values $x_c,y_c,z_c$ are not generally restricted to $[-1,1]$ or $[0,1]$.

Instead, the visibility boundaries depend on $w_c$.

For the conventional symmetric clip volume:

$$
 -w_c\le x_c\le w_c, 
$$

$$
 -w_c\le y_c\le w_c. 
$$

For OpenGL's default clip-depth convention:

$$
 -w_c\le z_c\le w_c. 
$$

For Vulkan's ordinary zero-to-one depth convention:

$$
 0\le z_c\le w_c. 
$$

The role of $w_c$ becomes essential during homogeneous clipping and perspective division.

### Why clip before perspective division?

Suppose a triangle has one vertex behind the camera and two in front.

If we immediately divide every vertex by $w_c$, a behind-camera vertex may have a negative or zero $w_c$, creating invalid or misleading projected geometry.

The homogeneous clipping stage clips primitives against the clip volume before perspective division.

This allows a triangle intersecting the near plane to generate a valid visible polygon rather than projecting inappropriate geometry across the screen.

Hardware performs clipping according to the active graphics pipeline's clip conventions. User-defined clip distances and API-specific options may modify the process.

**Normalized device coordinate space and conventions**

For a valid homogeneous clip coordinate with:

$$
 w_c\ne0, 
$$

normalized device coordinates, or NDC, are obtained by:

$$
 \boxed{ x_n=\frac{x_c}{w_c}, \qquad y_n=\frac{y_c}{w_c}, \qquad z_n=\frac{z_c}{w_c} }. 
$$

For visible geometry under a conventional symmetric XY clip volume:

$$
 -1\le x_n\le1, 
$$

$$
 -1\le y_n\le1. 
$$

The depth range depends on API conventions.

| Convention                 | NDC depth interval |
| -------------------------- | ------------------ |
| Traditional OpenGL default | $[-1,1]$       |
| Vulkan default             | $[0,1]$        |
| Metal                      | $[0,1]$        |

Modern OpenGL can also use zero-to-one clip depth through `glClipControl`. Thus “OpenGL always uses $[-1,1]$” is not correct for every modern OpenGL configuration.&#x20;


### NDC does not mean raster pixels

A point at:

$$
 (x_n,y_n)=(0,0) 
$$

lies at the centre of normalized device space.

Its raster coordinate depends on the viewport dimensions and origin convention.

For a $1920\times1080$ viewport, it corresponds to the viewport centre, but its exact integer pixel index depends on how continuous raster coordinates are interpreted.

**Screen or raster coordinates**

Raster space converts normalized coordinates into framebuffer coordinates.

Assume a viewport with top-left origin, width $W$, height $H$, and $Y$ increasing downward.

If NDC $Y$ increases upward, a typical mapping is:

$$
 \boxed{ x_p=\frac{x_n+1}{2}W } 
$$

and:

$$
 \boxed{ y_p=\frac{1-y_n}{2}H }. 
$$

For:

$$
 W=1920,\quad H=1080, 
$$

consider:

$$
 x_n=0.5,\qquad y_n=0.5. 
$$

Then:

$$
 x_p=\frac{1.5}{2}(1920)=1440, 
$$

$$
 y_p=\frac{0.5}{2}(1080)=270. 
$$

Thus the continuous raster coordinate is:

$$
 \boxed{(1440,270)}. 
$$

### Continuous raster positions versus discrete pixel indices

A raster position is a continuous coordinate.

The framebuffer contains discrete pixels indexed by integers.

Under the common convention in which pixel centres are at:

$$
 (i+0.5,j+0.5), 
$$

a raster coordinate of $(1440,270)$ lies on grid boundaries rather than at the centre of a unique pixel.

Rasterization coverage rules determine which fragments are generated.

This distinction matters for texture sampling, picking, camera rays, antialiasing, and temporal reprojection.

**Texture coordinate space**

Texture coordinates, often written:

$$
 (u,v), 
$$

identify positions in texture parameter space.

They are not necessarily world-space or raster-space coordinates.

A typical normalized texture mapping uses:

$$
 0\le u\le1,\qquad0\le v\le1. 
$$

But texture coordinates may extend outside this interval to support wrapping, repetition, and mirroring.

### Texture coordinates are attached to geometry

A triangle has three positions:

$$
 \mathbf p_0,\mathbf p_1,\mathbf p_2 
$$

and three associated texture coordinates:

$$
 \mathbf t_0,\mathbf t_1,\mathbf t_2. 
$$

Rasterization interpolates the texture coordinates across the projected triangle according to the shader interpolation mode.

For perspective projection, the default smooth interpolation typically accounts for the homogeneous $w$ values.

Perspective-correct interpolation follows later, where screen-space weights are related back to the original triangle.

### Texture origin conventions

A texture's $v=0$ direction depends on asset and pipeline conventions.

Some image-processing libraries treat the top row as row zero.

Some mathematical UV conventions depict $v$ increasing upward.

Texture storage row order and conceptual UV origin are separate choices.

A mismatch can invert textures vertically even when the camera projection is correct.

### Texture filtering

Sampling a normalized UV does not generally mean reading exactly one texel.

Nearest-neighbour, bilinear, trilinear mip filtering, and anisotropic filtering may combine multiple texels.

The texture coordinate defines a sampling location; the filtering method defines how a value is reconstructed around it.

This relates directly to perspective-correct interpolation of homogeneous attributes.

**Tangent coordinate space**

Tangent space is a local frame associated with a surface.

It is especially important for normal mapping.

A typical tangent frame contains:

- Tangent vector $\mathbf T$, approximately aligned with increasing texture $u$.
- Bitangent vector $\mathbf B$, approximately aligned with increasing texture $v$.
- Surface normal $\mathbf N$.

Together these form the TBN matrix:

$$
 \boxed{ TBN= \begin{bmatrix} |&|&|\\ \mathbf T&\mathbf B&\mathbf N\\ |&|&| \end{bmatrix} }. 
$$

A tangent-space normal:

$$
 \mathbf n_t 
$$

can be transformed into world space:

$$
 \boxed{ \mathbf n_W= \operatorname{normalize}(TBN\,\mathbf n_t) }. 
$$

### Deriving surface tangents from UV coordinates

Consider a triangle with world-space edges:

$$
 \mathbf e_1=\mathbf p_1-\mathbf p_0, \qquad \mathbf e_2=\mathbf p_2-\mathbf p_0. 
$$

Let its UV differences be:

$$
 (\Delta u_1,\Delta v_1), 
$$

$$
 (\Delta u_2,\Delta v_2). 
$$

Assume the local surface displacement satisfies:

$$
 \mathbf e_1=\mathbf T\Delta u_1+\mathbf B\Delta v_1, 
$$

$$
 \mathbf e_2=\mathbf T\Delta u_2+\mathbf B\Delta v_2. 
$$

Write this as a matrix equation:

$$
 \begin{bmatrix} |&|\\ \mathbf e_1&\mathbf e_2\\ |&| \end{bmatrix} = \begin{bmatrix} |&|\\ \mathbf T&\mathbf B\\ |&| \end{bmatrix} \begin{bmatrix} \Delta u_1&\Delta u_2\\ \Delta v_1&\Delta v_2 \end{bmatrix}. 
$$

If the UV matrix is invertible, then:

$$
 \boxed{ [\mathbf T\ \mathbf B] = [\mathbf e_1\ \mathbf e_2] \begin{bmatrix} \Delta u_1&\Delta u_2\\ \Delta v_1&\Delta v_2 \end{bmatrix}^{-1} }. 
$$

For the $2\times2$ UV matrix, define:

$$
 D=\Delta u_1\Delta v_2-\Delta u_2\Delta v_1. 
$$

Then:

$$
 \boxed{ \mathbf T= \frac{ \Delta v_2\mathbf e_1-\Delta v_1\mathbf e_2 }{D} } 
$$

and:

$$
 \boxed{ \mathbf B= \frac{ -\Delta u_2\mathbf e_1+\Delta u_1\mathbf e_2 }{D} }. 
$$

If $D=0$, the UV mapping is locally degenerate and the tangent basis cannot be constructed by this inverse.

### Orthogonalization

The calculated tangent may not be exactly perpendicular to the vertex normal.

A common correction uses Gram–Schmidt orthogonalization:

$$
 \boxed{ \mathbf T'= \operatorname{normalize} \left[ \mathbf T-\mathbf N(\mathbf N\cdot\mathbf T) \right] }. 
$$

The bitangent can be reconstructed using:

$$
 \boxed{ \mathbf B'=h(\mathbf N\times\mathbf T') } 
$$

where $h\in\\{-1,+1\\}$ encodes tangent-frame handedness.

This handedness is important for mirrored UV islands.

An engine that discards it can render incorrect normal-map directions across mirrored geometry.

### Nonuniform scaling and normals

Surface normals do not generally transform using the same $3\times3$ matrix as positions and tangent directions.

For a general invertible linear transform $A$:

$$
 \boxed{ \mathbf n'\propto(A^{-1})^T\mathbf n }. 
$$

Why?

A normal must remain perpendicular to every transformed tangent.

If:

$$
 \mathbf n\cdot\mathbf t=0 
$$

and the transformed tangent is:

$$
 \mathbf t'=A\mathbf t, 
$$

we need:

$$
 \mathbf n'\cdot A\mathbf t=0. 
$$

Choose:

$$
 \mathbf n'=(A^{-1})^T\mathbf n. 
$$

Then:

$$
 \mathbf n'^TA\mathbf t = \mathbf n^TA^{-1}A\mathbf t = \mathbf n^T\mathbf t=0. 
$$

This proves the inverse-transpose rule.

For pure rotations, $A^{-T}=A$, so normals and directions transform the same way.

For nonuniform scale and shear, they generally do not.

**Converting points between coordinate spaces**

A point has position relative to an origin.

For a rigid transform with rotation $R$ and translation $\mathbf t$:

$$
 \boxed{ \mathbf p_W=R\mathbf p_L+\mathbf t }. 
$$

To convert backward:

$$
 \mathbf p_W-\mathbf t=R\mathbf p_L. 
$$

Multiply by $R^T$, using $R^{-1}=R^T$:

$$
 \boxed{ \mathbf p_L=R^T(\mathbf p_W-\mathbf t) }. 
$$

### Numerical example · V11.02.12

Suppose a local point is:

$$
 \mathbf p_L= \begin{bmatrix}2\\0\\0\end{bmatrix}. 
$$

Rotate it by +90° about $Z$:

$$
 R= \begin{bmatrix} 0&-1&0\\ 1&0&0\\ 0&0&1 \end{bmatrix}. 
$$

Then:

$$
 R\mathbf p_L= \begin{bmatrix}0\\2\\0\end{bmatrix}. 
$$

Translate by:

$$
 \mathbf t= \begin{bmatrix}3\\4\\0\end{bmatrix}. 
$$

The world point is:

$$
 \boxed{ \mathbf p_W= \begin{bmatrix}3\\6\\0\end{bmatrix} }. 
$$

Convert it back:

$$
 \mathbf p_W-\mathbf t= \begin{bmatrix}0\\2\\0\end{bmatrix}. 
$$

Then:

$$
 R^T \begin{bmatrix}0\\2\\0\end{bmatrix} = \begin{bmatrix}2\\0\\0\end{bmatrix}. 
$$

We recover the original coordinates.

### Homogeneous point representation

A point is represented as:

$$
 \boxed{ \tilde{\mathbf p}= \begin{bmatrix}x\\y\\z\\1\end{bmatrix} }. 
$$

An affine matrix is:

$$
 M= \begin{bmatrix} A&\mathbf t\\ 0&1 \end{bmatrix}. 
$$

Multiplication gives:

$$
 M\tilde{\mathbf p}= \begin{bmatrix} A\mathbf p+\mathbf t\\1 \end{bmatrix}. 
$$

The final coordinate of one allows translation to affect the point.

**Converting directions between coordinate spaces**

A direction is not a point.

It represents a displacement, not a location relative to an origin.

For a rigid transform:

$$
 \boxed{ \mathbf d_W=R\mathbf d_L }. 
$$

There is no translation term.

Its homogeneous representation is:

$$
 \boxed{ \tilde{\mathbf d}= \begin{bmatrix}x\\y\\z\\0\end{bmatrix} }. 
$$

Multiply by the same affine matrix:

$$
 \begin{bmatrix} A&\mathbf t\\ 0&1 \end{bmatrix} \begin{bmatrix} \mathbf d\\0 \end{bmatrix} = \begin{bmatrix} A\mathbf d\\0 \end{bmatrix}. 
$$

Translation disappears because:

$$
 \mathbf t\cdot0=0. 
$$

This is why homogeneous points use $w=1$ and affine direction vectors use $w=0$.

### Why it matters for camera rays

A camera ray has an origin:

$$
 \mathbf o 
$$

and direction:

$$
 \mathbf d. 
$$

Under an affine transform:

$$
 \boxed{ \mathbf o'=A\mathbf o+\mathbf t, \qquad \mathbf d'=A\mathbf d }. 
$$

Do not add translation to the direction.

If the transformation includes nonuniform scale, the transformed direction's length may change. Normalizing it changes the ray parameterization: a world ray parameter $t$ may no longer represent the same distance units.

Intersection code must handle this consistently.

**Affine frames**

An affine frame combines an origin with a basis, allowing us to describe points and directions differently.

An affine transform has the form:

$$
 \boxed{ f(\mathbf p)=A\mathbf p+\mathbf t } 
$$

where $A$ is linear.

Such a transformation preserves collinearity and affine combinations.

If:

$$
 \mathbf p=(1-\alpha)\mathbf p_0+\alpha\mathbf p_1, 
$$

then:

$$
 \begin{aligned} f(\mathbf p) &= A[(1-\alpha)\mathbf p_0+\alpha\mathbf p_1]+\mathbf t\\ &=(1-\alpha)(A\mathbf p_0+\mathbf t) +\alpha(A\mathbf p_1+\mathbf t). \end{aligned} 
$$

Therefore:

$$
 \boxed{ f((1-\alpha)\mathbf p_0+\alpha\mathbf p_1) = (1-\alpha)f(\mathbf p_0)+\alpha f(\mathbf p_1) }. 
$$

Straight lines remain straight.

Ratios along straight-line segments are preserved.

But affine transforms do not necessarily preserve lengths, angles, areas, or volumes.

### Volume scaling

For an invertible linear transformation $A$, a volume element transforms as:

$$
 \boxed{ dV'=|\det A|\,dV }. 
$$

For a pure rotation:

$$
 \det R=1, 
$$

so volume is preserved.

For uniform scale $s$ in three dimensions:

$$
 A=sI, 
$$

so:

$$
 \det A=s^3. 
$$

Volume scales by:

$$
 |s|^3. 
$$

For a reflection:

$$
 \det A<0, 
$$

orientation reverses.

This determinant sign is relevant to winding order, mirrored transforms, normals, and tangent-frame handedness.

### Why camera transforms should generally be rigid

A physically meaningful camera pose uses a position and orientation.

Its basis should be orthonormal.

If a parent hierarchy introduces nonuniform scale into the camera transform, simply taking the entire affine inverse may cause a scaled or sheared camera coordinate system.

Whether that is acceptable depends on the rendering contract.

A camera API should generally separate pose from object scaling or explicitly define how scale is removed.

**Camera position and orientation**

A camera pose has two principal components:

$$
 \boxed{ \text{Pose}=(\mathbf c,R) } 
$$

where $\mathbf c$ is the camera's world position and $R$ describes its orientation.

The orientation may be stored as:

- A rotation matrix.
- A unit quaternion.
- Euler angles with a declared order.
- A forward direction plus an up reference.

These representations have different properties.

A quaternion is convenient for composition and interpolation.

A rotation matrix is convenient for transforming vectors and constructing a view matrix.

Euler angles may be convenient for user controls but can suffer parameterization singularities and order-dependent behavior.

### Camera-to-world transform

For a rigid camera pose:

$$
 \boxed{ C= \begin{bmatrix} R&\mathbf c\\ 0&1 \end{bmatrix} }. 
$$

It converts camera-local points into world space:

$$
 \mathbf p_W=C\mathbf p_C. 
$$

This is called the camera-to-world transform.

### View transform

The view transform is the inverse:

$$
 \boxed{V=C^{-1}}. 
$$

It converts world-space points into camera space.

The distinction is fundamental:

- Camera-to-world tells us where the camera's local basis lies in the world.
- World-to-camera tells us how a world point is described relative to that basis.

**Camera forward, right, and up vectors**

For our right-handed convention, the camera's local axes are:

$$
 \mathbf e_x= \begin{bmatrix}1\\0\\0\end{bmatrix}, \qquad \mathbf e_y= \begin{bmatrix}0\\1\\0\end{bmatrix}, \qquad \mathbf e_z= \begin{bmatrix}0\\0\\1\end{bmatrix}. 
$$

The camera looks toward local $-Z$.

Thus its local forward direction is:

$$
 \boxed{ \mathbf f_C= \begin{bmatrix}0\\0\\-1\end{bmatrix} }. 
$$

Let $R$ be the camera-to-world rotation.

The world-space right vector is:

$$
 \boxed{ \mathbf r_W=R\mathbf e_x }. 
$$

The world-space up vector is:

$$
 \boxed{ \mathbf u_W=R\mathbf e_y }. 
$$

The world-space forward vector is:

$$
 \boxed{ \mathbf f_W=-R\mathbf e_z }. 
$$

The third column of $R$ points backward, not forward.

This distinction causes many sign mistakes in view-matrix implementations.

### Orthonormality requirements

For a rigid camera basis:

$$
 \\|\mathbf r\\|=\\|\mathbf u\\|=\\|\mathbf f\\|=1. 
$$

Also:

$$
 \mathbf r\cdot\mathbf u=0, 
$$

$$
 \mathbf r\cdot\mathbf f=0, 
$$

$$
 \mathbf u\cdot\mathbf f=0. 
$$

For the chosen convention, right and up and backward form a right-handed basis:

$$
 \mathbf r\times\mathbf u=-\mathbf f. 
$$

Equivalently:

$$
 \mathbf r=\mathbf f\times\mathbf u 
$$

for an orthonormal camera basis with our stated forward convention.

To verify the identity, take the canonical case:

$$
 \mathbf f=(0,0,-1),\quad \mathbf u=(0,1,0). 
$$

Then:

$$
 \mathbf f\times\mathbf u=(1,0,0)=\mathbf r. 
$$

### Why the basis must be recomputed carefully

If the user supplies an approximate up vector, it may not be perpendicular to forward.

We should construct an orthonormal basis rather than assuming the supplied vectors are already valid.

A robust look-at function performs this construction.

**Look-at camera construction**

Suppose the camera position is:

$$
 \mathbf c 
$$

and the target point is:

$$
 \mathbf t. 
$$

We also have an approximate world-up direction:

$$
 \mathbf u_0. 
$$

We want a camera aimed at the target.

### Step 1: Construct forward

The direction from camera to target is:

$$
 \mathbf t-\mathbf c. 
$$

Normalize:

$$
 \boxed{ \mathbf f= \frac{\mathbf t-\mathbf c} {\\|\mathbf t-\mathbf c\\|} }. 
$$

The target must not coincide with the camera position.

If:

$$
 \mathbf t=\mathbf c, 
$$

the forward direction is undefined.

### Step 2: Construct right

Under our reference convention:

$$
 \boxed{ \mathbf r= \frac{\mathbf f\times\mathbf u_0} {\\|\mathbf f\times\mathbf u_0\\|} }. 
$$

Why this cross-product order?

Because for canonical forward $(0,0,-1)$ and world up $(0,1,0)$, it produces right $(1,0,0)$.

Reversing the cross product would produce left.

### Step 3: Recompute up

The supplied up vector may not be exactly perpendicular to forward.

Construct:

$$
 \boxed{ \mathbf u=\mathbf r\times\mathbf f }. 
$$

Now $\mathbf r,\mathbf u,-\mathbf f$ form an orthonormal right-handed camera basis.

### Step 4: Construct camera-to-world rotation

Arrange the local camera axes expressed in world coordinates as columns:

$$
 \boxed{ R= \begin{bmatrix} |&|&|\\ \mathbf r&\mathbf u&-\mathbf f\\ |&|&| \end{bmatrix} }. 
$$

Then:

$$
 R \begin{bmatrix}0\\0\\-1\end{bmatrix} = \mathbf f. 
$$

This confirms that the camera's local forward direction maps to the intended world forward direction.

### Step 5: Construct the view matrix

The view matrix is the inverse rigid transform.

We will derive it fully in the next part.

### Degenerate up vectors

Suppose the forward direction is parallel to the supplied world-up vector.

Then:

$$
 \mathbf f\times\mathbf u_0=\mathbf0. 
$$

Right is undefined.

A robust implementation should detect this and either reject the pose or choose a fallback reference axis not parallel to forward.

For example, choose the coordinate axis least aligned with $\mathbf f$, then construct the cross-product basis.

The fallback policy can cause an orientation discontinuity when the chosen reference axis changes, so applications requiring smooth camera orientation should manage camera roll and orientation continuously rather than rebuilding an arbitrary frame each time.

### Worked look-at example

Let:

$$
 \mathbf c=(0,0,5), 
$$

$$
 \mathbf t=(0,0,0), 
$$

$$
 \mathbf u_0=(0,1,0). 
$$

The forward direction is:

$$
 \mathbf f= \frac{(0,0,-5)}5 = (0,0,-1). 
$$

Right:

$$
 \mathbf r=(0,0,-1)\times(0,1,0) =(1,0,0). 
$$

Up:

$$
 \mathbf u=(1,0,0)\times(0,0,-1) =(0,1,0). 
$$

Thus:

$$
 R=I. 
$$

The camera has no rotation relative to our canonical axes.

It is simply translated five units along world $+Z$.

**The view matrix is the inverse camera transform**

We have the camera-to-world matrix:

$$
 C= \begin{bmatrix} R&\mathbf c\\ 0&1 \end{bmatrix}. 
$$

Because the camera rotation is orthonormal:

$$
 R^{-1}=R^T. 
$$

We seek:

$$
 V=C^{-1}. 
$$

Suppose:

$$
 V= \begin{bmatrix} R^T&\mathbf q\\ 0&1 \end{bmatrix}. 
$$

Require:

$$
 VC=I. 
$$

Multiply:

$$
 VC= \begin{bmatrix} R^TR&R^T\mathbf c+\mathbf q\\ 0&1 \end{bmatrix}. 
$$

Because:

$$
 R^TR=I, 
$$

the translation block must satisfy:

$$
 R^T\mathbf c+\mathbf q=0. 
$$

Therefore:

$$
 \mathbf q=-R^T\mathbf c. 
$$

Thus:

$$
 \boxed{ V= \begin{bmatrix} R^T&-R^T\mathbf c\\ 0&1 \end{bmatrix} }. 
$$

This is the exact inverse of a rigid camera transform.

### Expanded look-at view matrix

Recall:

$$
 R=[\mathbf r\ \mathbf u\ -\mathbf f]. 
$$

Therefore:

$$
 R^T= \begin{bmatrix} \mathbf r^T\\ \mathbf u^T\\ -\mathbf f^T \end{bmatrix}. 
$$

The view matrix is:

$$
 \boxed{ V= \begin{bmatrix} r_x&r_y&r_z&-\mathbf r\cdot\mathbf c\\ u_x&u_y&u_z&-\mathbf u\cdot\mathbf c\\ -f_x&-f_y&-f_z&\mathbf f\cdot\mathbf c\\ 0&0&0&1 \end{bmatrix} }. 
$$

This is a row-by-row mathematical display for column-vector multiplication. It does not prescribe whether the 16 numbers are stored row-major or column-major in CPU memory.

### Verify camera position maps to origin

Apply $V$ to the camera's own world position:

$$
 \mathbf p_W=\mathbf c. 
$$

Then:

$$
 \mathbf p_V=R^T(\mathbf c-\mathbf c)=0. 
$$

So:

$$
 \boxed{ V\begin{bmatrix}\mathbf c\\1\end{bmatrix} = \begin{bmatrix}0\\0\\0\\1\end{bmatrix} }. 
$$

A correct view matrix must put the camera at the origin of view space.

### Verify a target point lies on the forward axis

Let:

$$
 \mathbf t=\mathbf c+d\mathbf f 
$$

for $d>0$.

Then:

$$
 \mathbf t-\mathbf c=d\mathbf f. 
$$

Transform:

$$
 R^T(d\mathbf f). 
$$

Because $\mathbf f=-R\mathbf e_z$:

$$
 R^T(d\mathbf f) = -d\mathbf e_z. 
$$

Therefore:

$$
 \boxed{ \mathbf t_V=(0,0,-d) }. 
$$

The look-at target lies on the view-space negative-Z axis.

This is an excellent property-based test.

### Numerical example · V11.02.31

For:

$$
 \mathbf c=(0,0,5) 
$$

with identity orientation:

$$
 \boxed{ V= \begin{bmatrix} 1&0&0&0\\ 0&1&0&0\\ 0&0&1&-5\\ 0&0&0&1 \end{bmatrix} }. 
$$

The world origin:

$$
 (0,0,0,1)^T 
$$

transforms to:

$$
 \boxed{(0,0,-5,1)^T}. 
$$

The point is five units in front of the camera, exactly as intended.

**Why moving the camera transforms the world oppositely**

Imagine standing still while the camera moves to the right.

Visually, stationary objects appear to shift left in the camera's image.

Why?

In one dimension, the world position of an object is $x_W$, while the camera position is $c$.

Its camera-relative position is:

$$
 \boxed{x_C=x_W-c}. 
$$

If the camera moves right by $\Delta c$:

$$
 c'=c+\Delta c. 
$$

The new camera-relative coordinate is:

$$
 x_C'=x_W-(c+\Delta c). 
$$

Therefore:

$$
 \boxed{x_C'=x_C-\Delta c}. 
$$

Moving the camera right makes world points shift left in camera coordinates.

### Rotation behaves similarly

Suppose the camera rotates by a world-space rotation $R$.

The view-space transformation uses:

$$
 R^T=R^{-1}. 
$$

If the camera rotates left, the coordinates of stationary world objects rotate right relative to the camera.

The inverse transform is not a rendering workaround.

It is the mathematical consequence of changing the reference frame.

### Passive versus active transformations

This provides an important conceptual distinction.

An active transformation moves or rotates a geometric object in a fixed coordinate frame.

A passive transformation changes the coordinate frame used to describe the same geometric object.

The matrix expressions can look similar, but the interpretation differs.

A camera view matrix performs a passive coordinate transformation from world coordinates to camera coordinates.

This is why an object's model matrix and the camera's view matrix have conceptually opposite roles.

### Connection to robotics and reconstruction

In robotics, camera calibration, SLAM, and reconstruction, transform names must identify the direction of conversion.

For example, define:

$$
 T_{W\leftarrow C} 
$$

to mean camera coordinates converted into world coordinates.

Then:

$$
 T_{C\leftarrow W} = T_{W\leftarrow C}^{-1}. 
$$

This arrow convention avoids ambiguous names such as `cameraTransform`, which might mean either camera-to-world or world-to-camera.

For a graphics and reconstruction codebase, explicit transform direction is often worth the additional type or naming complexity.

The preceding chapter put the camera at the origin of its own coordinate frame. We now explain how the camera converts three-dimensional points into a form suitable for hardware rasterization.

There are two fundamental camera models in this module.

An orthographic camera projects along parallel lines, so an object's apparent size does not decrease with distance.

A perspective camera projects along rays converging at the camera centre, so apparent size decreases with depth.

Their difference is mathematical, not merely aesthetic.


<!-- end V11.02 -->

<!-- begin V11.09 -->
## Perspective belongs to projective geometry

*Affine versus projective maps, ideal points, homogeneous equivalence, projective lines and planes.*

Homogeneous coordinates were introduced as a convenient camera calculation; projective geometry explains why that representation is natural.

**Projective geometry motivation**

Consider two parallel railway tracks extending into the distance.

In ordinary Euclidean geometry, the tracks remain parallel.

They do not intersect at any finite point.

But in a perspective image, their projected lines appear to meet at a vanishing point.

How can a mathematical transformation turn parallel lines into intersecting lines?

The answer is that perspective projection is not an affine transformation.

### An ordinary affine transformation

An affine transformation has the form:

$$
 \boxed{ \mathbf y=A\mathbf x+\mathbf t }. 
$$

Take a straight line:

$$
 \mathbf x(\lambda)=\mathbf p+\lambda\mathbf d. 
$$

Apply the affine transform:

$$
 \begin{aligned} \mathbf y(\lambda) &= A(\mathbf p+\lambda\mathbf d)+\mathbf t\\ &= (A\mathbf p+\mathbf t)+\lambda A\mathbf d. \end{aligned} 
$$

The image is another straight line with direction $A\mathbf d$.

If two lines share the same direction $\mathbf d$, their transformed lines share direction $A\mathbf d$.

Thus nonsingular affine transformations preserve parallelism.

### Perspective does not preserve parallelism

For a pinhole camera looking along the positive optical Z direction, normalized projection is:

$$
 \boxed{ x'=\frac XZ,\qquad y'=\frac YZ }. 
$$

Consider a 3D line:

$$
 \mathbf X(t)= \begin{bmatrix} a+tv_x\\ b+tv_y\\ c+tv_z \end{bmatrix}. 
$$

Its projected coordinates are:

$$
 x'(t)=\frac{a+tv_x}{c+tv_z}, 
$$

$$
 y'(t)=\frac{b+tv_y}{c+tv_z}. 
$$

If:

$$
 v_z\ne0, 
$$

take the limit as $t\to\infty$:

$$
 \lim_{t\to\infty}x'(t)=\frac{v_x}{v_z}, 
$$

$$
 \lim_{t\to\infty}y'(t)=\frac{v_y}{v_z}. 
$$

Thus the projected line approaches:

$$
 \boxed{ \left( \frac{v_x}{v_z}, \frac{v_y}{v_z} \right) }. 
$$

This limiting image point depends on the line's direction, not its starting position.

Two parallel 3D lines with the same direction approach the same vanishing point.

### Why a new geometry is useful

In Euclidean geometry, we describe parallel lines as never meeting.

In projective geometry, we extend the space to include points at infinity.

Parallel lines can then be described as meeting at an ideal point associated with their shared direction.

This is not merely philosophical. It creates algebraic rules that work uniformly for ordinary line intersections and parallel-line configurations.

That uniformity is extremely useful for camera geometry, calibration, image rectification, and projective reconstruction.

**Points at infinity**

In two-dimensional projective geometry, a homogeneous point is represented by a nonzero triple:

$$
 \boxed{ \mathbf p= \begin{bmatrix} x\\y\\w \end{bmatrix} }. 
$$

If:

$$
 w\ne0, 
$$

the corresponding finite Cartesian point is:

$$
 \boxed{ \left(\frac xw,\frac yw\right) }. 
$$

But if:

$$
 \boxed{w=0}, 
$$

we cannot convert the representation to a finite Cartesian point.

Instead, the homogeneous point represents a point at infinity.

### Directional interpretation

Consider the homogeneous point:

$$
 \boxed{ \mathbf p_\infty= \begin{bmatrix} d_x\\d_y\\0 \end{bmatrix} }. 
$$

It represents the ideal point associated with direction:

$$
 (d_x,d_y). 
$$

Because homogeneous coordinates are defined only up to nonzero scale, the representations:

$$
 (d_x,d_y,0) 
$$

and:

$$
 (2d_x,2d_y,0) 
$$

represent the same projective point.

The magnitude of the direction is irrelevant.

Its orientation is the important feature.

### Parallel lines meet at an ideal point

Consider two horizontal lines:

$$
 y=1 
$$

and:

$$
 y=2. 
$$

They have no finite Euclidean intersection.

In homogeneous line form, their equations are:

$$
 y-w=0, 
$$

$$
 y-2w=0. 
$$

An intersection must satisfy both.

Subtract the equations:

$$
 w=0. 
$$

Then:

$$
 y=0. 
$$

The remaining nonzero homogeneous coordinate may be $x$.

Thus the intersection is:

$$
 \boxed{ (1,0,0) }. 
$$

This is the point at infinity in the horizontal direction.

### The line at infinity

In the projective plane, all points at infinity satisfy:

$$
 w=0. 
$$

That is itself a projective line, called the line at infinity:

$$
 \boxed{ \ell_\infty=(0,0,1) }. 
$$

Its equation is:

$$
 0x+0y+1w=0. 
$$

The line at infinity contains the ideal points associated with all directions in the affine plane.

A general projective transformation need not preserve the chosen line at infinity. This is why it can transform parallel affine lines into converging image lines.

### Three-dimensional projective points at infinity

In projective 3D, a point has homogeneous representation:

$$
 \boxed{ (X,Y,Z,W) }. 
$$

Finite points have:

$$
 W\ne0. 
$$

Points at infinity have:

$$
 \boxed{W=0}. 
$$

The ideal points form a plane at infinity in projective 3D.

This concept also appears computationally when unprojecting the far boundary of an infinite-far camera.

The returned homogeneous coordinate can have $W=0$, representing an unbounded direction rather than a finite position.

**Projective equivalence of homogeneous coordinates**

A homogeneous coordinate is not a unique numerical tuple.

It represents an equivalence class of nonzero vectors.

The relation is:

$$
 \boxed{ \mathbf p\sim\lambda\mathbf p } 
$$

for:

$$
 \lambda\ne0. 
$$

In projective 2D:

$$
 (2,4,2)\sim(1,2,1). 
$$

Both represent Cartesian point:

$$
 (1,2). 
$$

Similarly:

$$
 (4,8,0)\sim(1,2,0). 
$$

Both represent the same point at infinity.

### Why zero scaling is excluded

If we permitted:

$$
 \lambda=0, 
$$

every homogeneous vector could become the zero vector.

Then all projective points would collapse into a single meaningless representation.

The all-zero vector is therefore excluded.

### Derive invariance under projective transformation

Let $H$ be an invertible projective transformation matrix.

Suppose two homogeneous vectors represent the same projective point:

$$
 \mathbf q=\lambda\mathbf p. 
$$

Then:

$$
 H\mathbf q=H(\lambda\mathbf p). 
$$

By linearity:

$$
 H\mathbf q=\lambda H\mathbf p. 
$$

Therefore:

$$
 \boxed{ H\mathbf q\sim H\mathbf p }. 
$$

The transformation is well-defined on projective equivalence classes.

This is one of the reasons matrix algebra fits projective geometry so naturally.

### Points versus planes under transformation

A projective point transforms as:

$$
 \boxed{ \mathbf p'=H\mathbf p }. 
$$

A plane is represented by a covector $\boldsymbol\pi$ satisfying:

$$
 \boldsymbol\pi^T\mathbf p=0. 
$$

We want incidence to remain valid after transformation:

$$
 \boldsymbol\pi'^T\mathbf p'=0. 
$$

Substitute:

$$
 \boldsymbol\pi'^TH\mathbf p=0. 
$$

To preserve the original plane equation for all points on the plane, choose:

$$
 \boldsymbol\pi'^TH=\boldsymbol\pi^T. 
$$

Therefore:

$$
 \boxed{ \boldsymbol\pi'=H^{-T}\boldsymbol\pi }. 
$$

This is the same inverse-transpose rule used to transform geometric planes.

It is not an arbitrary graphics convention.

It follows from preserving the point-plane incidence relation.

### Numerical cautions

Because projective coordinates are scale-equivalent, an implementation may normalize a homogeneous vector for numerical convenience.

But dividing by the fourth coordinate is valid only when that coordinate is nonzero and the point is finite in the chosen affine chart.

For points at infinity, dividing by $W=0$ is invalid.

For very small $W$, the resulting Cartesian coordinates can be extremely sensitive to error.

A robust projective geometry API should distinguish finite points from ideal points rather than silently dividing every vector.

**Lines in projective 2D**

A line in the projective plane has homogeneous coefficients:

$$
 \boxed{ \boldsymbol\ell= \begin{bmatrix} a\\b\\c \end{bmatrix} }. 
$$

A homogeneous point:

$$
 \mathbf p= \begin{bmatrix} x\\y\\w \end{bmatrix} 
$$

lies on the line if:

$$
 \boxed{ \boldsymbol\ell^T\mathbf p=0 }. 
$$

Expanded:

$$
 \boxed{ ax+by+cw=0 }. 
$$

For finite points with $w=1$, this becomes the ordinary Cartesian line:

$$
 ax+by+c=0. 
$$

### Construct a line from two points

Take two homogeneous points:

$$
 \mathbf p,\mathbf q. 
$$

We want a line whose coefficients are perpendicular to both homogeneous vectors:

$$
 \boldsymbol\ell^T\mathbf p=0, 
$$

$$
 \boldsymbol\ell^T\mathbf q=0. 
$$

In three dimensions, the cross product produces a vector orthogonal to both inputs.

Therefore:

$$
 \boxed{ \boldsymbol\ell=\mathbf p\times\mathbf q }. 
$$

This is the join operation in the projective plane.

### Worked line construction

Take Cartesian points:

$$
 P=(1,2) 
$$

and:

$$
 Q=(3,6). 
$$

Their homogeneous representations are:

$$
 \mathbf p=(1,2,1), 
$$

$$
 \mathbf q=(3,6,1). 
$$

Compute:

$$
 \mathbf p\times\mathbf q= \begin{bmatrix} 2(1)-1(6)\\ 1(3)-1(1)\\ 1(6)-2(3) \end{bmatrix}. 
$$

Thus:

$$
 \boxed{ \boldsymbol\ell=(-4,2,0) }. 
$$

The line equation is:

$$
 -4x+2y=0. 
$$

Simplify:

$$
 \boxed{y=2x}. 
$$

Both points satisfy it.

### Intersect two lines

Suppose two projective lines have coefficient vectors:

$$
 \boldsymbol\ell_1,\boldsymbol\ell_2. 
$$

Their intersection is a point orthogonal to both line vectors.

Therefore:

$$
 \boxed{ \mathbf p=\boldsymbol\ell_1\times\boldsymbol\ell_2 }. 
$$

This is the meet operation.

### Worked intersection

Consider:

$$
 x=1 
$$

and:

$$
 y=2. 
$$

In homogeneous form:

$$
 \boldsymbol\ell_1=(1,0,-1), 
$$

$$
 \boldsymbol\ell_2=(0,1,-2). 
$$

Their cross product is:

$$
 \boldsymbol\ell_1\times\boldsymbol\ell_2 = \begin{bmatrix} 1\\2\\1 \end{bmatrix}. 
$$

Thus the intersection is:

$$
 \boxed{(1,2)}. 
$$

The exact same operation also handles parallel lines, producing a homogeneous point at infinity when appropriate.

### Line transformation

Let image points transform by a homography:

$$
 \mathbf p'=H\mathbf p. 
$$

The corresponding line transforms as:

$$
 \boxed{ \boldsymbol\ell'=H^{-T}\boldsymbol\ell }. 
$$

This follows from the same incidence-preservation derivation used for planes.

The point and line representations are dual.

### Applications

Projective lines appear in epipolar geometry, image rectification, vanishing-point estimation, homography calculations, geometric feature matching, and camera calibration.

For example, a homography can map all points on an image line to another image line while preserving their incidence relationships.

A Euclidean distance formula is not needed for the basic intersection operation.

**Planes in projective 3D**

A plane in projective three-dimensional space is represented by four homogeneous coefficients:

$$
 \boxed{ \boldsymbol\pi= \begin{bmatrix} a\\b\\c\\d \end{bmatrix} }. 
$$

A homogeneous 3D point:

$$
 \mathbf p= \begin{bmatrix} X\\Y\\Z\\W \end{bmatrix} 
$$

lies on the plane when:

$$
 \boxed{ \boldsymbol\pi^T\mathbf p=0 }. 
$$

Expanded:

$$
 \boxed{ aX+bY+cZ+dW=0 }. 
$$

For an ordinary Cartesian point represented with $W=1$:

$$
 aX+bY+cZ+d=0. 
$$

### Plane through three points

Suppose three noncollinear finite points are:

$$
 P_0,\quad P_1,\quad P_2. 
$$

Construct edges:

$$
 \mathbf e_1=P_1-P_0, 
$$

$$
 \mathbf e_2=P_2-P_0. 
$$

The Euclidean normal is:

$$
 \boxed{ \mathbf n=\mathbf e_1\times\mathbf e_2 }. 
$$

The plane equation must satisfy:

$$
 \mathbf n\cdot(P-P_0)=0. 
$$

Expand:

$$
 \mathbf n\cdot P-\mathbf n\cdot P_0=0. 
$$

Thus the plane offset is:

$$
 \boxed{ d=-\mathbf n\cdot P_0 }. 
$$

The homogeneous plane is:

$$
 \boxed{ \boldsymbol\pi= (n_x,n_y,n_z,d) }. 
$$

### Example

Take:

$$
 P_0=(0,0,2), 
$$

$$
 P_1=(1,0,2), 
$$

$$
 P_2=(0,1,2). 
$$

The edges are:

$$
 \mathbf e_1=(1,0,0), 
$$

$$
 \mathbf e_2=(0,1,0). 
$$

Their cross product is:

$$
 \mathbf n=(0,0,1). 
$$

The offset is:

$$
 d=-\mathbf n\cdot P_0=-2. 
$$

Thus:

$$
 \boxed{ \boldsymbol\pi=(0,0,1,-2) }. 
$$

Its Cartesian equation is:

$$
 \boxed{z=2}. 
$$

### Three planes can determine a point

For three independent planes:

$$
 \boldsymbol\pi_1,\boldsymbol\pi_2,\boldsymbol\pi_3, 
$$

a point at their intersection satisfies:

$$
 \boldsymbol\pi_i^T\mathbf p=0 
$$

for all three planes.

Together, the equations form a $3\times4$ homogeneous linear system.

If its nullspace is one-dimensional, the corresponding nonzero nullspace vector represents the intersection point.

The nullspace can be computed through Gaussian elimination, SVD, or a generalized cross-product construction using signed $3\times3$ minors.

This is a projective version of solving three simultaneous plane equations.

### Plane transformation revisited

If points transform as:

$$
 \mathbf p'=H\mathbf p, 
$$

then planes transform as:

$$
 \boxed{ \boldsymbol\pi'=H^{-T}\boldsymbol\pi }. 
$$

This preserves the incidence relationship:

$$
 \boldsymbol\pi^T\mathbf p=0. 
$$

A crucial practical consequence is that you should not generally transform a plane by multiplying its coefficient vector directly by the point transformation matrix.

The plane lives in the dual space.

### Near planes, frustum planes, and projective planes

Frustum plane extraction is a concrete application of projective duality.

The projection matrix maps world points into clip space.

Canonical clip-space boundary planes are pulled back through the transformation.

Their coefficients become world-space frustum planes.

Thus culling and projective-plane theory are not separate tricks.

They are two views of the same mathematics.


<!-- end V11.09 -->

<!-- begin V11.03 -->
<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/01 - Computing Foundations/00 - Reading Map|Reading map]] · [[Chapter Edition/01 - Computing Foundations/31 - Implement Probability with Numerical Contracts|Previous chapter]] · [[Chapter Edition/01 - Computing Foundations/33 - Projection, Clipping, and Raster Sampling|Next chapter]]
