# 37 — Camera Contracts and Transform-Chain Debugging

[[Chapter Edition/01 - Computing Foundations/00 - Reading Map|Reading map]] · [[Chapter Edition/01 - Computing Foundations/36 - Exposure, Physical Optics, and Depth of Field|Previous chapter]] · [[Chapter Edition/01 - Computing Foundations/38 - Reconstruct the Complete Geometry Pipeline|Next chapter]]

A safe camera API separates pose, projection, depth, and raster conventions rather than hiding them in one matrix. The chapter turns those distinctions into semantic types, complete scalar laboratories, and checks that identify which transform boundary introduced an error.

**By the end:** Design camera interfaces with explicit conventions and debug a point through every transform stage using inspectable values.

**Recall question:** If the final pixel is wrong, what intermediate coordinate should you inspect first, and which contract does it test?

<!-- Derived from Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md; source lines 92790–95543; source-span SHA256 775dc2899b9313850b9315786fc230dfde9c711696c577f50b6691de99817ebe. -->

<!-- CHAPTER-SOURCE-BEGIN -->
## Make camera conventions explicit and debug the full transform chain


By this point, the camera is no longer one matrix and one FOV value. It is a collection of related but independently configurable systems.

A robust API should distinguish between:

1. Pose: where the camera is and how it is oriented.
2. Projection: how 3D directions become image-plane coordinates.
3. Depth convention: how near and far map into clip space and the depth buffer.
4. Raster convention: how NDC maps to framebuffer coordinates.
5. Optics: how aperture, focus, distortion, and shutter behavior modify ray generation.
6. Temporal state: how current and previous camera configurations relate.
7. Calibration: how real measurements map between 3D rays and pixel coordinates.

These are separate concerns. Encoding all of them in an ambiguous `Matrix4 projection` can make it easy to accidentally combine incompatible conventions.

### Why a generic `Camera` class can become unsafe

Consider this interface:

```
Mat4 projection(float fov, float near, float far);
```

It leaves numerous questions unanswered.

Does the function expect degrees or radians? Is the camera right-handed or left-handed? Is FOV vertical or horizontal? Does `far=0` mean infinity? Does the function use zero-to-one depth or negative-one-to-one depth? Does it use conventional or reversed-Z? Is Y flipped for Vulkan? Does the resulting matrix include temporal jitter?

A correct implementation cannot resolve those questions from the signature.

### Use semantic types

A more precise interface can define:

```
enum class Handedness {
    RightHanded,
    LeftHanded
};

enum class ClipDepthRange {
    ZeroToOne,
    MinusOneToOne
};

enum class DepthDirection {
    Conventional,
    Reversed
};

enum class RasterOrigin {
    TopLeft,
    BottomLeft
};

enum class ProjectionKind {
    Perspective,
    Orthographic
};
```

These enums do not, by themselves, solve every convention problem. They make the choices explicit so that construction and validation can enforce compatible combinations.

### Separate the camera pose

A world-space camera pose could use double-precision position and a normalized quaternion:

```
struct CameraPose {
    Vec3d positionWorld;
    Quatd orientationCameraToWorld;
};
```

The orientation field should say which direction it transforms.

For example, `orientationCameraToWorld` converts a camera-local direction into a world-space direction.

Then the inverse orientation is used for world-to-camera transformation.

### Separate projection specifications

```
struct PerspectiveSpec {
    double verticalFovRadians;
    double aspectRatio;
    double nearPlane;
    std::optional<double> farPlane;

    Handedness handedness;
    ClipDepthRange clipDepthRange;
    DepthDirection depthDirection;
};
```

An absent far plane can represent an explicit infinite-far perspective request.

This is clearer than overloading a value such as zero, infinity, or a negative number without documenting its meaning.

For stereo and calibrated cameras, an off-axis specification is also useful:

```
struct OffAxisPerspectiveSpec {
    double nearLeft;
    double nearRight;
    double nearBottom;
    double nearTop;

    double nearPlane;
    std::optional<double> farPlane;

    ClipDepthRange clipDepthRange;
    DepthDirection depthDirection;
};
```

The bounds are defined on the near plane and must be validated.

For OpenXR, the runtime-provided four FOV angles can be converted into these bounds.

### Define the viewport independently

```
struct ViewportSpec {
    double x;
    double y;
    double width;
    double height;

    RasterOrigin origin;
    double minDepth;
    double maxDepth;
};
```

Note that an API's viewport transform, Y direction, and depth range may have constraints that depend on the graphics API and enabled features.

The goal is not to pretend every combination is valid on every device.

The goal is to represent the intended semantics and convert them explicitly into each backend's supported state.

### Define optics independently

```
struct PhysicalCameraSpec {
    double focalLengthMillimeters;
    double fNumber;
    double focusDistanceMeters;
    double shutterOpenSeconds;
    double shutterCloseSeconds;
    double iso;
};
```

Separate physical units in names or use strong quantity types.

A field called `focalLength` can become ambiguous if one code path expects metres, another expects millimetres, and a third expects pixels.

Similarly, a `focusDistance` field should identify whether it is measured along the camera optical axis, to a focus plane, or as an actual range to a target point.

### Define temporal camera state

```
struct TemporalCameraState {
    Mat4d currentView;
    Mat4d previousView;

    Mat4d currentProjectionUnjittered;
    Mat4d previousProjectionUnjittered;

    Mat4d currentProjectionJittered;
    Mat4d previousProjectionJittered;

    Vec2d currentJitterPixels;
    Vec2d previousJitterPixels;

    bool historyValid;
};
```

This allows consumers to explicitly choose the representation they need.

For example, a motion-vector pass may calculate current and previous unjittered projected positions, then apply jitter corrections in a known place.

A renderer should not assume all passes want the same projection matrix.

### Frame-state update ordering

Temporal camera state must be updated in a carefully defined order.

One possible frame contract is:

```
void beginFrame(
    CameraFrameState& frame,
    const CameraPose& newPose,
    const ProjectionSpec& newProjection,
    Vec2d newJitter
) {
    // Preserve the completed previous frame before
    // constructing the new current-frame state.
    frame.previous = frame.current;

    frame.current.pose = newPose;
    frame.current.projection = newProjection;
    frame.current.jitter = newJitter;

    // Construct matrices once and retain a coherent
    // snapshot for all passes within this frame.
    frame.current.rebuildMatrices();

    // Invalidate history when the projection/viewport
    // or pose changes discontinuously.
    frame.historyValid =
        frame.historyValid &&
        frame.previous.compatibleWith(frame.current);
}
```

This is an architectural illustration; `CameraFrameState`, `ProjectionSpec`, `rebuildMatrices`, and `compatibleWith` represent types and functions the engine would define.

The key invariant is that all passes in one frame should consume a coherent camera snapshot.

If a camera pose changes halfway through command-buffer recording, different passes may use incompatible view matrices.

### A camera API should expose operations, not just matrices

Useful operations include:

```
Ray generateRayFromPixel(...);

Vec3d reconstructViewPosition(...);

Vec3d reconstructWorldPosition(...);

ProjectedPixel projectWorldPoint(...);

std::optional<ProjectedPixel> tryProjectVisiblePoint(...);

DepthResult linearizeDepth(...);

Mat4d viewMatrix(...);

Mat4d projectionMatrix(...);
```

Each operation should clearly declare coordinate spaces and depth semantics.

A `projectWorldPoint` function should not silently accept view-space positions.

A `generateRayFromPixel` function should not silently treat logical UI points as framebuffer pixels.

### Make coordinate-space mistakes difficult to compile

For a more advanced C++23 math library, use distinct types for positions and directions in different spaces.

For example:

```
struct LocalSpace {};
struct WorldSpace {};
struct ViewSpace {};
struct ClipSpace {};

template<class Space>
struct Point3 {
    double x, y, z;
};

template<class Space>
struct Direction3 {
    double x, y, z;
};
```

Then define transformations with explicit source and target spaces:

```
template<class From, class To>
struct Transform3 {
    Mat4d matrix;
};

Point3<WorldSpace> transform(
    const Transform3<LocalSpace, WorldSpace>& T,
    Point3<LocalSpace> point
);
```

A transformation accepting `Point3<LocalSpace>` cannot accidentally be called with `Point3<ViewSpace>` without an explicit conversion.

This is the same motivation behind strong units for radians, degrees, metres, millimetres, and pixels.

The types carry meaning that a generic `Vec3` cannot express.

### Camera invariants

At construction, verify that requested dimensions and focal parameters are finite and physically or mathematically meaningful.

For a rigid pose:

$$ R^TR\approx I $$

and:

$$ \det R\approx1. $$

For a finite perspective camera:

$$ 0\<n\<f. $$

For vertical FOV:

$$ 0<\theta_y<\pi. $$

For a physical lens:

$$ f>0,\quad N>0,\quad S>f $$

under the ordinary real-image thin-lens setup.

For a valid pixel:

$$ 0\le i\<W,\qquad0\le j\<H. $$

For a reconstructed finite perspective point, reject invalid homogeneous division and nonfinite results.

For stereo cameras, validate per-eye poses, frustum bounds, and expected runtime conventions.

The class should return meaningful errors rather than silently producing NaNs that propagate through the render graph.

### Cross-API matrix upload

The mathematical convention and memory layout must be separated.

A CPU library might store matrices row-major but multiply column vectors:

$$ \mathbf p'=M\mathbf p. $$

A Metal shader may represent a matrix as four column vectors.

A Vulkan shader might use a buffer layout and multiplication convention determined by its shader source and decorations.

The correct upload conversion is determined by the combination of storage layout, matrix multiplication semantics, and shader interface.

Do not transpose a matrix merely because the destination API is different. Verify that the resulting transformed basis vectors and test points agree on CPU and GPU.

### Reversed-Z must be implemented as a complete pipeline contract

A reversed-Z camera requires more than swapping two projection coefficients.

The depth clear value, depth compare function, clip convention, depth reconstruction, hierarchical Z logic, screen-space effects, and any custom depth-writing shaders must agree.

For a common zero-to-one reversed-Z configuration:

```
depthClear = 0.0f;
depthCompare = Greater;
```

For conventional depth:

```
depthClear = 1.0f;
depthCompare = Less;
```

An engine can support both, but must not accidentally pair the reversed projection with conventional depth state.

### Physical cameras and rendering APIs are different layers

The physical camera model determines rays and exposure behavior.

The graphics API determines raster clip conventions, viewport mapping, and depth testing.

These should not be coupled unnecessarily.

A ray tracer can use the same camera pose, FOV, and calibration as a rasterizer while generating rays directly rather than constructing a hardware depth buffer.

A stereo renderer can share a headset-derived eye pose with a path tracer or rasterizer while choosing different output pipelines.

That separation makes the camera architecture more reusable.


This final section tests the complete chain, from coordinate-frame definitions through the rendered camera image.

When a camera produces a blank screen, inverted scene, incorrect depth, broken picking ray, or warped geometry, the fastest reliable diagnosis is often to select one known geometric point and trace it through the entire pipeline.

The goal is to identify the first stage whose output violates a known invariant.

### The reference scene

Choose a mesh vertex in local space:

$$ \boxed{ \mathbf p_L=(1,2,0). } $$

Let the model transform be a translation:

$$ \boxed{ \mathbf t_M=(2,0,-6). } $$

Thus the model matrix is:

$$ M= \begin{bmatrix} 1&0&0&2\\ 0&1&0&0\\ 0&0&1&-6\\ 0&0&0&1 \end{bmatrix}. $$

The camera is at:

$$ \boxed{ \mathbf c=(10,2,5). } $$

Its target is:

$$ (10,2,4), $$

so it looks along world $-Z$ with canonical orientation.

Its vertical FOV is:

$$ \theta_y=90^\circ, $$

and aspect is:

$$ a=\frac{16}{9}. $$

Use:

$$ n=0.1,\qquad f=1000, $$

zero-to-one reversed-Z, and a top-left raster mapping at 1920×1080.

### Step 1: Local to world

Apply the model:

$$ \mathbf p_W=M\mathbf p_L. $$

Therefore:

$$ \boxed{ \mathbf p_W=(3,2,-6). } $$

This should match the authoritative world-space vertex position.

If it does not, the fault is in model transformation, hierarchy composition, coordinate units, or the vertex data.

### Step 2: World to view

Because the camera orientation is canonical, view-space coordinates are the world point minus camera position:

$$ \mathbf p_V=\mathbf p_W-\mathbf c. $$

Thus:

$$ (3,2,-6)-(10,2,5) = \boxed{(-7,0,-11)}. $$

The positive axial depth is:

$$ \boxed{d=11}. $$

The point is left of the camera's optical axis, exactly at camera height, and in front of the camera.

This already predicts the qualitative image position: left of screen centre, vertically centred.

### Step 3: View to homogeneous clip

For 90° vertical FOV:

$$ s_y=\frac1{\tan45^\circ}=1. $$

Horizontal scale:

$$ s_x=\frac{s_y}{a}=\frac9{16}=0.5625. $$

Thus:

$$ x_c=s_xx_v=0.5625(-7). $$

Therefore:

$$ \boxed{x_c=-3.9375}. $$

Vertical clip coordinate:

$$ \boxed{y_c=0}. $$

Perspective W:

$$ \boxed{w_c=-z_v=11}. $$

For reversed-Z, the finite projection depth coefficients are:

$$ A=\frac n{f-n}, \qquad B=\frac{nf}{f-n}. $$

Thus:

$$ z_c=Az_v+B. $$

With:

$$ z_v=-11, $$

the resulting post-divide depth is approximately:

$$ \boxed{z_n=0.008991808}. $$

### Step 4: Perspective division

Horizontal NDC:

$$ x_n=\frac{x_c}{w_c}. $$

Thus:

$$ x_n=\frac{-3.9375}{11}. $$

Therefore:

$$ \boxed{ x_n=-0.357954545. } $$

Vertical NDC:

$$ \boxed{y_n=0}. $$

Depth NDC:

$$ \boxed{z_n\approx0.008991808}. $$

All three values are within the expected visible ranges.

### Step 5: Viewport mapping

For a 1920×1080 top-left viewport:

$$ x_p=\frac{x_n+1}{2}(1920). $$

Thus:

$$ x_p= \frac{1-0.357954545}{2}(1920). $$

Therefore:

$$ \boxed{x_p\approx616.363636}. $$

For Y:

$$ y_p=\frac{1-y_n}{2}(1080). $$

Since $y_n=0$:

$$ \boxed{y_p=540}. $$

The final continuous raster position is:

$$ \boxed{ (616.363636,\ 540). } $$

This is exactly what we expect: left of image centre, vertically centred.

### Step 6: Reverse the pipeline

Now take the resulting NDC position and depth:

$$ (-0.357954545,0,0.008991808). $$

Apply inverse projection.

The recovered view-space point should be:

$$ \boxed{(-7,0,-11)}. $$

Apply inverse view.

The recovered world-space point should be:

$$ \boxed{(3,2,-6)}. $$

Subtract model translation or apply the inverse model matrix.

The recovered local point should be:

$$ \boxed{(1,2,0)}. $$

If every stage round-trips within justified numerical tolerance, the mathematical camera transform chain is internally consistent.

It does not prove every renderer feature is correct, but it narrows the fault domain substantially.

### Step 7: Compare CPU and GPU

Use one known point as a shader debugging input.

Record or inspect its clip-space position from the actual vertex shader.

Compare it with a CPU reference constructed from the same transforms.

If CPU and GPU disagree, investigate shader multiplication order, row/column layout, buffer packing, transpose logic, stale constants, or world-coordinate precision.

If CPU and GPU clip positions agree but the pixel appears elsewhere, investigate viewport configuration, Y orientation, primitive clipping, rasterization, and sample locations.

If the point projects correctly but is invisible, investigate depth testing, face culling, scissor state, depth writes, attachment clears, colour writes, and occlusion.

### A systematic camera-debugging matrix

| Symptom                                      | First mathematical or pipeline checks                            |
| -------------------------------------------- | ---------------------------------------------------------------- |
| Entire image upside down                     | NDC-to-raster Y mapping, projection Y flip, viewport height sign |
| Objects behind camera appear                 | Clip $w$, near clipping, projection handedness               |
| Near objects disappear                       | Near plane, reverse-Z clip assignment, depth test                |
| Distant geometry flickers                    | Depth precision, near/far ratio, coplanar surfaces               |
| Picking offset from cursor                   | Viewport-local conversion, DPI scale, pixel centres              |
| Mouse ray moves opposite horizontally        | Camera right basis, NDC X mapping                                |
| Camera translation moves world the wrong way | View matrix inversion                                            |
| Scene becomes unstable far from origin       | Float absolute positions, premature precision conversion         |
| TAA ghosts after camera moves                | Previous transforms, jitter handling, history validity           |
| XR depth feels incorrect                     | Per-eye poses, IPD, asymmetric FOV, display calibration          |
| Calibration fits centre but not edges        | Distortion model, principal point, crop/resize intrinsics        |
| Defocus seems wrong                          | Focus distance, focal length, aperture units, lens-ray geometry  |
| Exposure changes blur but not brightness     | Normalized lens sampling versus physical throughput              |
| Panorama seams                               | Longitude wrapping, face orientation, filtering                  |
| Fisheye rays don't match observations        | Projection family, calibration, valid angle domain               |

### Instrument the camera instead of guessing

A robust camera implementation can offer a diagnostic function that returns a structured transform trace.

For example:

```
struct CameraTrace {
    Vec4d local;
    Vec4d world;
    Vec4d view;
    Vec4d clip;

    Vec3d ndc;
    Vec2d raster;

    double axialDepth;
    double euclideanRange;

    bool inFrontOfCamera;
    bool insideClipVolume;
    bool finite;
};
```

A function such as:

```
CameraTrace tracePoint(
    const CameraFrame& camera,
    const Mat4d& model,
    Point3<LocalSpace> localPoint
);
```

should compute every stage and validate each result.

The returned trace can be shown in an editor inspector or written to a deterministic test log.

For a hybrid graphics engine, this is particularly valuable because it allows a single selected vertex or world point to be followed through rasterization, ray tracing, scene reconstruction, and depth unprojection.

The important debugging habit is to ask where the first incorrect value appears, rather than immediately changing unrelated signs and matrix entries until an image looks approximately correct.


To verify the equations rather than leave them as symbolic derivations, the supplied source reports building a standalone C++23 reference that exercises the mathematical concepts in this chapter.

### Standalone camera-geometry reference laboratory (source-reported)

The source reports a 386-line C++23 standalone program, strict warning checks, ordinary execution, and sanitizer-enabled execution. The implementation and runs were not independently available in this staging pass.

Download the complete commented C++23 source

Compile it with:

```
clang++ -std=c++23 -O2 \
  -Wall -Wextra -Wpedantic -Werror \
  phase11_m065_m066_complete_camera_lab.cpp \
  -o phase11_camera_lab

./phase11_camera_lab
```

The source includes its own vector/matrix reference operations, projection specifications, inverse matrices, depth reconstruction, screen rays, off-axis projections, stereo calculations, cubemap and spherical directions, fisheye projection functions, physical aperture/exposure calculations, CoC, and thin-lens ray generation.

### Numerical results reported by the supplied source

> The supplied source reports these results; its complete standalone reference laboratory was not present in the scratch inputs, so the runs are not independently verified here.

SOURCE-REPORTED C++23 REFERENCE RESULTS

| Test                                                          | Observed value                 |
| ------------------------------------------------------------- | ------------------------------ |
| Standard depth, 10 units                                      | 0.990099010                    |
| Reversed depth, 10 units                                      | 0.009900990                    |
| Stereo disparity, $f_x=800$ px, baseline 64 mm, depth 2 m | 25.600000000 px                |
| Aperture diameter, 50 mm f/2                                  | 25.000000000 mm                |
| CoC, focus 3 m, object 6 m                                    | 0.211864407 mm                 |
| EV100, f/2, 1/125 s, ISO 100                                  | 8.965784285                    |
| World point from model transform                              | (3, 2, −6)                     |
| View-space point                                              | (−7, 0, −11)                   |
| Projected NDC point                                           | (−0.357954545, 0, 0.008991808) |
| Raster coordinates, 1920×1080                                 | (616.363636, 540)              |

The supplied source reports that all numerical assertions passed in ordinary execution and with AddressSanitizer and UndefinedBehaviorSanitizer enabled; those runs were not independently reproduced in this staging pass.

The tests include depth round-trips for standard and reversed projections, view/world unprojection, orthographic ray origins, asymmetric frustum boundaries, disparity inversion, all six cubemap face-centre directions, equirectangular round-trips, fisheye forward/inverse mappings, physical exposure relationships, and the requirement that different thin-lens aperture samples pass through the same focus point.

This is a standalone mathematical oracle rather than a change to your production repositories.

### Repository observations reported by the supplied source

> The following project inspections and implementation claims are reported by the supplied source. They have not been independently rechecked in this staging pass.


The actual code inspection provides several concrete starting points for applying the lesson.

### KairoRayTracer — currently a pinhole camera

The inspected [KairoRayTracer Camera.cppm](https://github.com/swayam8624/KairoRayTracer/blob/main/Camera.cppm?utm_source=chatgpt.com) explicitly states that its V1 implementation is pinhole-only. It generates primary rays from normalized raster coordinates using camera forward/right/up vectors, vertical FOV, and aspect ratio.

It does not currently define lens radius, focus distance, or shutter sampling in that camera class.

That is a coherent baseline for geometric ray tracing. To support depth of field and motion blur, the next architectural step would be to accept an explicit camera sample containing raster, lens, and time coordinates, then produce a ray whose origin and time depend on the selected sample.

The existing `GenerateRay(u,v)` function can remain useful as the pinhole path and as a reference against which the zero-aperture thin-lens limit is tested.

### KairoRenderer — conventional depth and viewport-specific Y handling

The inspected [KairoRenderer Camera.cppm](https://github.com/swayam8624/KairoRenderer/blob/main/Camera.cppm?utm_source=chatgpt.com) builds standard zero-to-one perspective or orthographic projection and applies a Y sign flip for its intended Vulkan viewport convention.

Its pose validation and projection construction form a useful renderer-neutral starting point. But a more complete camera contract should distinguish pixel coordinates, viewport offsets, current/previous frame state, projection convention, and depth reconstruction.

A picking-ray generator in the renderer should use the same effective projection and viewport conventions as the rendered scene.

Otherwise the camera image can look correct while clicking on objects produces offset or inverted rays.

### Maveb — reversed-Z

The inspected [Maveb Camera.cpp](https://github.com/swayam8624/Maveb/blob/main/engine/scene/src/Camera.cpp?utm_source=chatgpt.com) implements reversed-Z perspective projection, including an infinite-far option.

For an infinite reversed-Z camera, the simple axial-depth reconstruction is:

$$ \boxed{d=n/q} $$

when $q$ is the matching zero-to-one NDC depth value.

That is a useful implementation-specific optimization, provided the shader explicitly handles $q=0$ and the actual depth texture follows the expected mapping.

It would be incorrect to reuse KairoRenderer's conventional depth-linearization formula blindly in this Maveb path.

### Cross-project camera consistency

For Kairo, Maveb, and the XR/FoveaBench work, the most useful shared contract would cover:

- Explicit camera-to-world and world-to-camera transform directions.
- A declared view-axis and NDC depth convention.
- Pixel-centre and viewport mapping.
- Central and off-axis projection.
- Correct inverse projection and depth reconstruction.
- Pinhole and physical-lens ray sampling.
- Temporal jitter and previous-frame state.
- Calibration intrinsics and distortion as separate concepts.

The strongest benefit would be preventing subtle differences in conventions from appearing as unrelated rendering, picking, and reconstruction bugs.


The full camera-geometry sequence can now support a rigorous verification suite.

A useful research experiment would combine a synthetic scene with known geometry, two virtual cameras, and several image-formation models.

Start with a calibrated perspective camera and render known 3D points. Reconstruct them from pixel coordinates and depth. Measure the world-space round-trip error as a function of depth, image radius, NDC depth convention, and floating-point precision.

Then introduce a stereo baseline and compute disparity. Add controlled subpixel disparity errors and verify that the observed depth uncertainty grows consistently with:

$$ \boxed{ \sigma_Z\approx\frac{Z^2}{f_xB}\sigma_\Delta. } $$

Next, replace the rectilinear camera with panoramic or fisheye projection and test directional round-trips across the entire supported angular domain. The error metric should be angular error between unit vectors, not only a pixel difference.

For physical cameras, simulate aperture samples and verify that their rays converge at the focus plane. Vary focus distance, focal length, aperture, and shutter time independently. Test whether CoC follows the predicted geometry and whether brightness and blur respond according to the intended physical or artistic rendering contract.

Finally, compare each mathematical result with the actual renderer output, recording the CPU reference coordinate, GPU clip coordinate, NDC position, raster position, and reconstructed world point.

A clean research implementation would report not only average errors but their distributions, extrema, failure cases, and numerical conditioning.

### A consistent depth-of-field verification example

Using the exact thin-lens CoC formula derived above with $f=50$ mm, f/8, focus at 3 m, and acceptable sensor CoC of 0.03 mm gives a hyperfocal distance of approximately 10.467 m. Solving the same CoC equation without changing distance conventions gives a near acceptable-focus distance of approximately 2.338 m and a far distance of approximately 4.185 m.

These are threshold-defined acceptable sharpness limits, not physical planes where blur suddenly switches on or off. This distinction should be kept explicit when comparing a geometric CoC calculation against a rendered image.


The teaching and reference-code coverage is complete; that is distinct from independently implementing every concept in the production engines or producing the curriculum's V21/V22 video deliverables.

The next subject builds on these camera foundations: computational geometry primitives, robust predicates, distances, and closest points. These tools turn the camera’s rays and reconstructed points into reliable intersections, collision tests, and spatial queries.


<!-- end V11.18 -->

<!-- begin V11.19 -->
## A camera model must close the loop

```mermaid
flowchart LR
 O["Measured image points"] --> E["Predict and form residuals"] --> J["Jacobian and weights"] --> G["Gauge and rank checks"] --> S["Damped solve"] --> H["Validate on new views"]
 class O,E,J blue;class G amber;class S,H green;
classDef blue fill:#e5eff9,stroke:#2463a6,color:#18354c;
classDef green fill:#e3f3ed,stroke:#16806a,color:#174b3c;
classDef amber fill:#fff0dc,stroke:#c57820,color:#64400d;
```


A derivation becomes useful when a known world point can be traced to a raster sample and an observed image point can be mapped back to a ray or uncertainty region. The final worked traces, reference implementations, and repository observations connect those forward and inverse calculations.

### Trace, implementation, repository audit, and derivation bridge

### Complete worked example: from a mesh vertex to a framebuffer pixel

We have now studied all 42 original parts individually. Let's combine them into one concrete calculation.

Suppose a mesh contains local point:

$$
 \boxed{ \mathbf p_L=(1,0,0) }. 
$$

Its model matrix translates the object by:

$$
 \mathbf t=(2,1,0) 
$$

with no rotation or scale.

The camera is at:

$$
 \mathbf c=(0,0,5) 
$$

looking at the world origin with world up:

$$
 (0,1,0). 
$$

The camera parameters are:

$$
 \theta_y=60^\circ, \qquad a=\frac{16}{9}, 
$$

$$
 n=0.1,\qquad f=1000. 
$$

The framebuffer resolution is:

$$
 1920\times1080. 
$$

We use conventional zero-to-one perspective depth and an explicitly defined top-left raster mapping.

#### Local to world

The model matrix is:

$$
 M= \begin{bmatrix} 1&0&0&2\\ 0&1&0&1\\ 0&0&1&0\\ 0&0&0&1 \end{bmatrix}. 
$$

Transform the homogeneous local point:

$$
 \mathbf p_L= \begin{bmatrix} 1\\0\\0\\1 \end{bmatrix}. 
$$

Therefore:

$$
 \boxed{ \mathbf p_W= M\mathbf p_L= \begin{bmatrix} 3\\1\\0\\1 \end{bmatrix} }. 
$$

#### World to view

The camera is translated five units along $+Z$, looking toward $-Z$.

Its view matrix is:

$$
 V= \begin{bmatrix} 1&0&0&0\\ 0&1&0&0\\ 0&0&1&-5\\ 0&0&0&1 \end{bmatrix}. 
$$

Thus:

$$
 \boxed{ \mathbf p_V= V\mathbf p_W= \begin{bmatrix} 3\\1\\-5\\1 \end{bmatrix} }. 
$$

The point is three units right, one unit up, and five units forward from the camera.

Its positive camera depth is:

$$
 \boxed{d=5}. 
$$

Note that this is axial view depth, not Euclidean distance from the camera centre.

The Euclidean distance is:

$$
 \sqrt{3^2+1^2+5^2}=\sqrt{35}\approx5.9161. 
$$

Ordinary perspective depth mapping uses $d=-z_v=5$, not the Euclidean range.

#### View to clip

For the stated FOV and aspect ratio:

$$
 s_x\approx0.9742785793, 
$$

$$
 s_y\approx1.7320508076. 
$$

Our projection is:

$$
 P= \begin{bmatrix} s_x&0&0&0\\ 0&s_y&0&0\\ 0&0&-1.00010001&-0.100010001\\ 0&0&-1&0 \end{bmatrix}. 
$$

Apply to:

$$
 \mathbf p_V=(3,1,-5,1)^T. 
$$

The clip X coordinate is:

$$
 x_c=s_x(3)\approx2.922835738. 
$$

The clip Y coordinate is:

$$
 y_c=s_y(1)\approx1.732050808. 
$$

The clip W coordinate is:

$$
 \boxed{w_c=5}. 
$$

The clip Z coordinate follows from the depth row and is approximately:

$$
 z_c=4.900490049. 
$$

Thus:

$$
 \boxed{ \mathbf p_C\approx \begin{bmatrix} 2.922835738\\ 1.732050808\\ 4.900490049\\ 5 \end{bmatrix} }. 
$$

#### Perspective divide

Divide by $w_c=5$.

Therefore:

$$
 \boxed{ x_n\approx0.584567148 } 
$$

and:

$$
 \boxed{ y_n\approx0.346410162 }. 
$$

The depth is:

$$
 \boxed{ z_n\approx0.980098010 }. 
$$

The NDC point is:

$$
 \boxed{ \mathbf p_N\approx (0.584567148,\ 0.346410162,\ 0.980098010) }. 
$$

It lies inside the conventional zero-to-one visible depth interval and symmetric XY NDC bounds.

#### NDC to raster

Using a top-left raster origin:

$$
 x_p=\frac{x_n+1}{2}1920. 
$$

Substitute:

$$
 x_p= \frac{1.584567148}{2}(1920). 
$$

Therefore:

$$
 \boxed{x_p\approx1521.184462}. 
$$

For Y:

$$
 y_p=\frac{1-y_n}{2}1080. 
$$

Thus:

$$
 \boxed{y_p\approx352.938513}. 
$$

The final continuous raster position is:

$$
 \boxed{ \mathbf p_{\mathrm{raster}} \approx (1521.1845,\ 352.9385) }. 
$$

That is the complete mathematical journey from a mesh vertex to an image position.

WORLD-TO-SCREEN TRACE

| Space  | Coordinates                       |
| ------ | --------------------------------- |
| Local  | $(1,0,0)$                     |
| World  | $(3,1,0)$                     |
| View   | $(3,1,-5)$                    |
| Clip   | $(2.92284,1.73205,4.90049,5)$ |
| NDC    | $(0.58457,0.34641,0.98010)$   |
| Raster | $(1521.1845,352.9385)$        |

This table is one of the most useful debugging tools you can build.

For a suspicious vertex, log its coordinates after each transformation. If an object appears upside down, disappears, clips incorrectly, or has the wrong depth, the first incorrect stage identifies the subsystem to investigate.

### Additional derivation: an off-centre perspective projection

The earlier perspective derivation used a centred frustum; the more general form handles off-axis views too.

Suppose the near plane has horizontal coordinates:

$$
 l,\ r 
$$

and vertical coordinates:

$$
 b,\ t. 
$$

The near plane is at positive camera depth $n$.

We want near-plane X coordinate $l$ to map to NDC $-1$, and $r$ to map to $+1$.

Assume the clip X numerator is:

$$
 x_c=\alpha x+\beta z. 
$$

The perspective denominator is:

$$
 w_c=-z=d. 
$$

At the near plane:

$$
 z=-n. 
$$

Thus:

$$
 x_n=\frac{\alpha x-\beta n}{n} = \frac{\alpha x}{n}-\beta. 
$$

Enforce the left and right bounds:

$$
 \frac{\alpha l}{n}-\beta=-1, 
$$

$$
 \frac{\alpha r}{n}-\beta=1. 
$$

Subtract:

$$
 \frac{\alpha(r-l)}n=2. 
$$

Therefore:

$$
 \boxed{ \alpha=\frac{2n}{r-l} }. 
$$

Substitute:

$$
 \boxed{ \beta=\frac{r+l}{r-l} }. 
$$

The same reasoning applies vertically.

Therefore a general asymmetric right-handed zero-to-one perspective matrix is:

$$
 \boxed{ P= \begin{bmatrix} \dfrac{2n}{r-l}&0&\dfrac{r+l}{r-l}&0\\ 0&\dfrac{2n}{t-b}&\dfrac{t+b}{t-b}&0\\ 0&0&-\dfrac f{f-n}&-\dfrac{fn}{f-n}\\ 0&0&-1&0 \end{bmatrix} }. 
$$

When the frustum is symmetric:

$$
 l=-r,\qquad b=-t, 
$$

the off-axis terms vanish:

$$
 \frac{r+l}{r-l}=0, 
$$

$$
 \frac{t+b}{t-b}=0. 
$$

The matrix reduces to our ordinary centred perspective projection.

This general form is the foundation of off-axis projection, principal-point offsets, stereo-camera frusta, and certain XR displays. Those applications connect to calibrated intrinsics, stereo baselines, and XR displays.

### C++23 implementation and verification

I created a standalone camera-geometry reference using the exact mathematical conventions declared at the beginning of this lesson.

It does not depend on Vulkan, Metal, GLFW, or an existing engine math library. This makes it useful as an independent oracle when testing KairoMath or KairoRenderer transformations.

Tests passed

C++23 source · right-handed look-at · affine point/direction transforms · camera-to-world inversion · finite standard/reversed projection · OpenGL-style and zero-to-one depth · orthographic projection · infinite reversed-Z · viewport and pixel-centre tests.

Download the complete C++23 implementation

Compile with:

```
c++ -std=c++23 -O2 -Wall -Wextra -Wpedantic -Werror \
  phase11_m061_m062_camera_geometry.cpp \
  -o camera_geometry_lab

./camera_geometry_lab
```

The source reports successful compilation and execution under these warning settings, plus AddressSanitizer and UndefinedBehaviorSanitizer runs. Those results are **To verify** here because this staging task did not receive or execute the referenced standalone source file.

### 45.1 The most important implementation: projection conventions as explicit parameters

The laboratory defines:

```
enum class DepthRange {
    ZeroToOne,
    MinusOneToOne
};

enum class DepthDirection {
    Standard,
    Reversed
};

struct ProjectionSpec {
    double nearPlane;
    double farPlane;

    DepthRange range;
    DepthDirection direction;

    bool infiniteFar;
};
```

Rather than writing several superficially different projection functions, it derives the depth coefficients from desired near and far NDC values.

For our right-handed perspective convention, let:

$$
 z_n(d)=-A+\frac Bd. 
$$

Define desired endpoint depths:

$$
 d_n=z_n(n), 
$$

$$
 d_f=z_n(f). 
$$

We have:

$$
 -A+\frac Bn=d_n, 
$$

$$
 -A+\frac Bf=d_f. 
$$

Subtract:

$$
 B\left(\frac1n-\frac1f\right)=d_n-d_f. 
$$

Thus:

$$
 \boxed{ B=\frac{nf(d_n-d_f)}{f-n} }. 
$$

Use the near equation:

$$
 A=\frac Bn-d_n. 
$$

Substitute:

$$
 \boxed{ A=\frac{nd_n-fd_f}{f-n} }. 
$$

Therefore a general finite perspective depth row is:

$$
 \boxed{ \left( 0,\ 0,\ \frac{nd_n-fd_f}{f-n},\ \frac{nf(d_n-d_f)}{f-n} \right) }. 
$$

This formula handles conventional and reversed depth in both supported depth intervals.

It is more robust conceptually than memorizing four matrices separately.

### 45.2 Infinite-far generalization

Take:

$$
 f\to\infty. 
$$

Then:

$$
 A\to-d_f, 
$$

$$
 B\to n(d_n-d_f). 
$$

Thus:

$$
 \boxed{ A_{\infty}=-d_f, \qquad B_{\infty}=n(d_n-d_f) }. 
$$

For infinite reversed-Z with zero-to-one depth:

$$
 d_n=1,\qquad d_f=0. 
$$

Therefore:

$$
 A=0,\qquad B=n. 
$$

We recover the simple infinite reversed-Z matrix.

The reference code uses this generalized derivation.

### 45.3 Verified program output

ACTUAL C++23 REFERENCE OUTPUT

| Verification                            | Result              |
| --------------------------------------- | ------------------- |
| World origin from camera $(0,0,5)$  | View $(0,0,-5)$ |
| Horizontal FOV at vertical 60°, 16:9    | 91.492844520°       |
| Standard depth at $d=10$            | 0.990099009901      |
| Standard depth at $d=100$           | 0.999099909991      |
| Standard depth at $d=1000$          | 1.000000000000      |
| Infinite reversed depth at $d=10$   | 0.010000000000      |
| Infinite reversed depth at $d=100$  | 0.001000000000      |
| Infinite reversed depth at $d=1000$ | 0.000100000000      |
| World origin, 1920×1080 raster          | (960, 540)          |

All camera-geometry tests passed.

The tests verify several important properties rather than merely comparing printed numbers.

The camera-to-world and world-to-camera matrices compose to identity:

$$
 \boxed{VC=I}. 
$$

The camera's world position transforms to the view origin.

The camera forward direction transforms to local $-Z$.

Projection maps the near and far planes to the requested NDC boundaries.

A near-plane point at the appropriate horizontal and vertical frustum extent maps to NDC $(1,1)$.

Orthographic projection preserves XY position under depth changes.

Infinite reversed-Z maps near depth to one and approaches zero as depth increases.

Degenerate look-at inputs are rejected.

The pixel-centre conversion uses the half-integer convention consistently.

These are the invariants a production camera math library should preserve.

### Source-reported repository observations: KairoMath, KairoRenderer, KairoRayTracer, and Maveb

The source excerpt reports inspection of these files. Treat the following repository observations as **To verify** until the specific revisions and runtime paths are rechecked against primary evidence.

#### KairoMath: matrix and projection conventions

[KairoMath/Matrix.cppm](https://github.com/swayam8624/KairoMath/blob/master/Matrix.cppm?utm_source=chatgpt.com)

The module explicitly documents:

- Row-major CPU matrix storage.
- Column-vector multiplication.
- Translation in the final matrix column.
- Composition order in which $AB\mathbf v$ applies $B$ first.
- Projection helpers targeting zero-to-one depth.

These conventions are internally coherent and match the reference mathematics of this lesson.

The existing `Perspective()` function produces:

$$
 P_{22}=\frac f{n-f}, 
$$

$$
 P_{23}=\frac{fn}{n-f}, 
$$

$$
 P_{32}=-1. 
$$

Those are the conventional right-handed zero-to-one coefficients derived earlier.

The existing `Orthographic()` helper also matches the corresponding conventional zero-to-one matrix.

### Improvement: validation

The low-level projection helpers rely on `assert` checks for some invalid values such as zero aspect ratio or equal near/far planes.

Assertions are useful during development, but they are not necessarily active in release builds.

The functions also do not individually enforce every necessary finite-parameter or physically valid range condition.

For a low-level math library, it is worth separating a checked public constructor from a fast internal function whose preconditions are documented.

This avoids accepting malformed input silently in release builds.

### Improvement: explicit depth variants

The current general helpers implement conventional depth.

But your Maveb camera path already uses reversed-Z.

A useful shared math API could expose:

```
PerspectiveRH_ZO(...)
PerspectiveRH_ZO_Reverse(...)
PerspectiveRH_ZO_ReverseInfinite(...)
PerspectiveRH_NO(...)
```

or a strongly typed specification object.

This reduces the chance of mixing matrices with incompatible depth states.

#### KairoRenderer: camera and viewport

[KairoRenderer/Camera.cppm](https://github.com/swayam8624/KairoRenderer/blob/main/Camera.cppm?utm_source=chatgpt.com)

The current renderer has a `CameraPose` containing position, target, up vector, projection mode, field of view, and near/far parameters.

Its `View()` uses the math library's `LookAt()`.

Its `Projection()` constructs a perspective or orthographic matrix and negates the Y scale for its intended Vulkan positive-height viewport convention.

That is a deliberate convention choice.

### Improvement: look-at degeneracy

The pose validator checks that position and target differ and that the up vector is nonzero.

But those checks do not prove the up vector is nonparallel to the forward direction.

If:

$$
 \mathbf f\parallel\mathbf u_0, 
$$

then:

$$
 \mathbf f\times\mathbf u_0=\mathbf0. 
$$

A look-at implementation must handle this case explicitly.

The current low-level `LookAt` uses `SafeNormalize`, so behavior under degenerate inputs depends on that helper's fallback semantics. The pose validator should ideally enforce a nondegenerate basis rather than relying on fallback normalization to repair arbitrary geometry.

### Improvement: framebuffer extent

The camera computes aspect ratio using:

```
const float aspect =
    static_cast<float>(width) /
    static_cast<float>(std::max(height, 1u));
```

The height is protected against zero.

But a zero width still produces aspect ratio zero.

The lower-level `Perspective()` function asserts that aspect is nonzero.

Therefore a direct zero-width call could violate the low-level precondition.

A camera projection API should either reject zero width and height or explicitly define how minimized/zero-size viewport states are handled.

This matters in editors where panels can collapse or be temporarily resized to zero width.

#### KairoRenderer: Vulkan viewport target

[KairoRenderer/VulkanViewportTarget.cppm](https://github.com/swayam8624/KairoRenderer/blob/main/VulkanViewportTarget.cppm?utm_source=chatgpt.com)

The viewport target uses:

- `VK_FORMAT_R16G16B16A16_SFLOAT` for HDR colour.
- `VK_FORMAT_R32_UINT` for object identifiers.
- `VK_FORMAT_D32_SFLOAT` for depth.

The floating-point depth format is suitable for a reversed-Z design.

However, the depth attachment format alone does not establish that reversed-Z is active.

To verify actual depth behavior, inspect the selected projection, depth-clear value, depth-stencil compare function, and consuming shaders together.

The current camera helper inspected here produces conventional zero-to-one depth, not reversed-Z.

Thus it would be incorrect to claim this renderer already uses reversed-Z merely because its depth image uses `D32_SFLOAT`.

#### KairoRayTracer: camera-ray basis

[KairoRayTracer/Camera.cppm](https://github.com/swayam8624/KairoRayTracer/blob/main/Camera.cppm?utm_source=chatgpt.com)

The pinhole camera constructs a forward/right/up basis and generates rays using:

$$
 \boxed{ \mathbf d= \operatorname{normalize} \left[ \mathbf f+ x_s\mathbf r+ y_s\mathbf u \right] }. 
$$

The screen offsets use vertical FOV and aspect ratio.

Its normalized raster inputs are mapped using:

$$
 x_s=(2u-1)\tan(\theta_y/2)a, 
$$

$$
 y_s=(1-2v)\tan(\theta_y/2). 
$$

This is mathematically consistent with the top-left normalized raster convention and a right-handed camera looking along local $-Z$.

The code uses `SafeNormalize` fallbacks. For a general robust camera API, the final orthonormality and handedness invariants should still be verified.

We will derive the full pinhole-ray mapping and pixel-ray construction in the later camera modules.

#### Maveb: reversed-Z Metal camera

[Maveb/engine/scene/src/Camera.cpp](https://github.com/swayam8624/Maveb/blob/main/engine/scene/src/Camera.cpp?utm_source=chatgpt.com)

The inspected projection matrix already uses reversed-Z depth coefficients and supports infinite far projection.

That aligns with the mathematical recommendations for a floating-point depth buffer in a large-depth-range renderer.

The `viewMatrix()` implementation computes the inverse of the supplied world transform after checking that it is finite and has nonzero scale.

That is mathematically valid for an invertible affine transform, but it does not automatically mean the resulting view transform represents a rigid physical camera pose.

If a caller supplies a nonuniformly scaled camera transform, its inverse contains the corresponding scale.

Whether this is intended should be specified by the camera contract.

For a rigid camera, extracting translation and orthonormal orientation, or otherwise explicitly rejecting nonrigid input, can make the semantics clearer.

### Mastery verification: what you should now be able to derive

The lesson's essential conceptual achievement is not remembering four standard matrix layouts.

It is being able to reconstruct the complete rendering coordinate pipeline from a small number of geometric principles.

Start with the definition of a frame:

$$
 \mathbf p=\mathbf o+B\mathbf p_{\mathrm{local}}. 
$$

Derive its inverse, explain why points and directions transform differently, derive rigid camera inverse matrices, construct a look-at basis with correct cross-product order, and demonstrate why camera movement produces an inverse transformation of world coordinates.

You should also explain how tangent frames and normal matrices differ, why nonuniform scale creates complications, and why matrix memory layout is separate from multiplication convention.

Start from similar triangles:

$$
 x_s=\frac{sx}{d}, \qquad y_s=\frac{sy}{d}. 
$$

Derive the perspective divide through:

$$
 w_c=-z_v. 
$$

Derive the projection scale from FOV and aspect ratio.

Then impose near/far boundary conditions to derive the depth row, rather than memorizing its coefficients.

From there, derive conventional depth, reversed-Z, finite far, infinite far, and traditional OpenGL depth mappings.

Finally, convert a selected NDC point to a pixel centre using a declared viewport origin and verify the exact raster coordinate.

A complete implementation should test all of these against independently constructed reference cases.

### Final formula sheet

Object to clip

$$
 \boxed{ \mathbf p_C=PVM\mathbf p_L } 
$$

World to camera

$$
 \boxed{ \mathbf p_V=R^T(\mathbf p_W-\mathbf c) } 
$$

Perspective division

$$
 \boxed{ \mathbf p_N= \frac{\mathbf p_C.xyz}{\mathbf p_C.w} } 
$$

Vertical and horizontal FOV

$$
 \boxed{ \theta_x= 2\arctan \left( a\tan(\theta_y/2) \right) } 
$$

Standard zero-to-one perspective depth

$$
 \boxed{ z_n(d)= \frac f{f-n} - \frac{fn}{(f-n)d} } 
$$

Finite reversed-Z perspective depth

$$
 \boxed{ z_n(d)= \frac{nf}{(f-n)d} - \frac n{f-n} } 
$$

Infinite reversed-Z

$$
 \boxed{ z_n(d)=\frac nd } 
$$

Top-left pixel centre to NDC

$$
 \boxed{ x_n=\frac{2(i+0.5)}W-1 } 
$$

$$
 \boxed{ y_n=1-\frac{2(j+0.5)}H } 
$$

### Calibrated projection, uncertainty, reference implementation, and repository evidence

### End-to-end example: calibrated projection, optical distortion, and image uncertainty

Let's connect the two modules through one camera observation.

Assume a graphics camera located at world position:

$$
 \mathbf C_W=(0,0,5) 
$$

with its canonical orientation looking toward world $-Z$.

A world point is:

$$
 \mathbf X_W=(1,1,0). 
$$

We want to determine where it appears in a calibrated camera image, optionally apply lens distortion, and determine how small 3D position uncertainty affects the projected pixel.

This is the type of calculation needed in 3D reconstruction, camera calibration, and observation-space validation.

#### 36.1 World to graphics view

The graphics camera is translated five units along $+Z$.

Its world-to-view transform subtracts:

$$
 (0,0,5). 
$$

Therefore:

$$
 \boxed{ \mathbf X_G=(1,1,-5) }. 
$$

The point is one unit right, one unit up, and five units forward.

#### 36.2 Graphics view to CV optical frame

Apply the convention conversion:

$$
 S= \begin{bmatrix} 1&0&0\\ 0&-1&0\\ 0&0&-1 \end{bmatrix}. 
$$

Then:

$$
 \boxed{ \mathbf X_C=S\mathbf X_G=(1,-1,5) }. 
$$

The optical camera sees the point at normalized coordinates:

$$
 x=\frac15=0.2, 
$$

$$
 y=\frac{-1}{5}=-0.2. 
$$

#### 36.3 Apply intrinsics

Let:

$$
 K= \begin{bmatrix} 800&0&960\\ 0&820&540\\ 0&0&1 \end{bmatrix}. 
$$

For the undistorted image:

$$
 u=800(0.2)+960. 
$$

Thus:

$$
 \boxed{u=1120}. 
$$

And:

$$
 v=820(-0.2)+540. 
$$

Thus:

$$
 \boxed{v=376}. 
$$

The undistorted pixel is:

$$
 \boxed{(1120,376)}. 
$$

It lies to the right and above the principal point, as expected.

#### 36.4 Apply simple radial distortion

Let:

$$
 k_1=-0.1 
$$

with all other distortion coefficients zero.

The normalized squared radius is:

$$
 r^2=0.2^2+(-0.2)^2=0.08. 
$$

The radial factor is:

$$
 L=1-0.1(0.08)=0.992. 
$$

Therefore:

$$
 x_d=0.2(0.992)=0.1984, 
$$

$$
 y_d=-0.2(0.992)=-0.1984. 
$$

Map to pixels:

$$
 u_d=800(0.1984)+960. 
$$

Thus:

$$
 \boxed{u_d=1118.72}. 
$$

And:

$$
 v_d=820(-0.1984)+540. 
$$

Thus:

$$
 \boxed{v_d=377.312}. 
$$

The distorted pixel is:

$$
 \boxed{(1118.72,377.312)}. 
$$

The observed point moves inward toward the principal point.

That is consistent with the negative radial coefficient in this example.

#### 36.5 Derive the image-space uncertainty

Suppose the camera-space 3D point has covariance:

$$
 \Sigma_X= \begin{bmatrix} 0.0001&0&0\\ 0&0.0001&0\\ 0&0&0.0004 \end{bmatrix}. 
$$

These entries correspond to standard deviations of:

$$
 0.01,\quad0.01,\quad0.02 
$$

world units along the three camera-space axes.

For the undistorted pinhole mapping, the Jacobian is:

$$
 J_\pi= \begin{bmatrix} f_x/Z&0&-f_xX/Z^2\\ 0&f_y/Z&-f_yY/Z^2 \end{bmatrix}. 
$$

Substitute:

$$
 f_x=800,\quad f_y=820, 
$$

$$
 X=1,\quad Y=-1,\quad Z=5. 
$$

Then:

$$
 \boxed{ J_\pi= \begin{bmatrix} 160&0&-32\\ 0&164&32.8 \end{bmatrix} }. 
$$

The first-order projected pixel covariance is:

$$
 \Sigma_u\approx J_\pi\Sigma_XJ_\pi^T. 
$$

Calculate the horizontal variance:

$$
 \sigma_u^2= 160^2(0.0001)+(-32)^2(0.0004). 
$$

Therefore:

$$
 \sigma_u^2=2.56+0.4096 =\boxed{2.9696}. 
$$

The vertical variance is:

$$
 \sigma_v^2= 164^2(0.0001)+32.8^2(0.0004). 
$$

Thus:

$$
 \sigma_v^2= 2.6896+0.430336 =\boxed{3.119936}. 
$$

The covariance is:

$$
 \operatorname{Cov}(u,v) = (-32)(32.8)(0.0004). 
$$

Therefore:

$$
 \boxed{ \operatorname{Cov}(u,v)=-0.41984 }. 
$$

The full first-order pixel covariance is:

$$
 \boxed{ \Sigma_u\approx \begin{bmatrix} 2.9696&-0.41984\\ -0.41984&3.119936 \end{bmatrix} }. 
$$

Its off-diagonal terms arise because uncertainty in camera-space depth affects both pixel coordinates.

This is a concrete example of a 3D uncertainty ellipsoid projecting to a correlated 2D uncertainty ellipse.

With optical distortion, the full local Jacobian becomes:

$$
 \boxed{ J_{\mathrm{pixel}} = K_{2\times2}J_DJ_{\mathrm{normalize}} } 
$$

where $J_D$ is the distortion Jacobian and $J_{\mathrm{normalize}}$ is the normalized pinhole Jacobian.

Then:

$$
 \boxed{ \Sigma_{\mathrm{pixel}} \approx J_{\mathrm{pixel}} \Sigma_X J_{\mathrm{pixel}}^T }. 
$$

This is a useful bridge from the camera mathematics to uncertainty-aware Gaussian projection and reconstruction.

The approximation is first-order. Strongly nonlinear distortions or large uncertainty may require more sophisticated propagation, such as unscented transforms or Monte Carlo simulation.

### Verified C++23 implementation

The source reports a standalone advanced camera laboratory; its reported build and test results remain **To verify** here.

Tests passed

C++23 source · Frustum extraction and intersection · Infinite and reversed-Z projections · Temporal jitter · Camera-motion reprojection · Projective lines · Barycentric and perspective-correct interpolation · Intrinsics · Brown–Conrady distortion · Analytic-Jacobian undistortion · Large-world precision tests.

Download the full commented source

Compile it with:

```
c++ -std=c++23 -O2 -Wall -Wextra -Wpedantic -Werror \
  phase11_m063_m064_advanced_camera_lab.cpp \
  -o advanced_camera_lab

./advanced_camera_lab
```

I also compiled and ran it using AddressSanitizer and UndefinedBehaviorSanitizer. The supplied assertions passed under both the ordinary and sanitizer-enabled builds.

#### 37.1 The numerical results

ACTUAL REFERENCE EXECUTION

| Test                                    | Result                      |
| --------------------------------------- | --------------------------- |
| Frustum near/far corners                | −1 / −10                    |
| Requested jitter shift                  | (+0.25, −0.50) px           |
| Observed raster shift                   | (+0.25, −0.50) px           |
| Camera-motion displacement              | 38.4 px                     |
| Naive affine attribute interpolation    | 0.25                        |
| Perspective-correct interpolation       | 0.10                        |
| Calibrated ideal pixel                  | (1360, 740)                 |
| Distorted normalized coordinate         | (0.294899386, −0.196426258) |
| Recovered undistorted coordinate        | (0.300000000, −0.200000000) |
| Absolute float-before-subtraction error | 0.125 lost                  |
| Double-relative, then float             | 0.125 preserved             |

All numerical assertions passed.

The implementation tests more than the printed examples. It checks frustum corners, all six extracted plane constraints, sphere and AABB rejection, world-space plane extraction, conventional and reversed depth, infinite reversed-Z, homogeneous scale equivalence, projective line intersections, barycentric weights, calibration round trips, and a grid of distortion/undistortion coordinates.

This is an independent reference laboratory, not a claim that the same algorithms have been integrated into the production repositories.

### 37.2 C++23: frustum plane extraction

The central code is compact because the mathematics has already done the work:

```
std::array<Plane, 6> frustumPlanes(Mat4 vp) {
    // Column-vector convention: clip = VP * world.
    // Each row is a 4D plane-coefficient covector.
    const Vec4 r0 = vp.row(0);
    const Vec4 r1 = vp.row(1);
    const Vec4 r2 = vp.row(2);
    const Vec4 r3 = vp.row(3);

    return {
        normalizePlane(r3 + r0), // Left
        normalizePlane(r3 - r0), // Right
        normalizePlane(r3 + r1), // Bottom
        normalizePlane(r3 - r1), // Top

        // Zero-to-one clip depth only:
        normalizePlane(r2),      // z_clip >= 0
        normalizePlane(r3 - r2)  // w_clip - z_clip >= 0
    };
}
```

This is an explanatory form using `Mat4`, `Vec4`, and arithmetic operators. The downloadable standalone source contains its own concrete types and corresponding implementation.

If the projection uses infinite reversed-Z, one of the nominal depth boundaries can be degenerate. A production extractor should represent the absent far boundary explicitly rather than normalizing an invalid plane.

### 37.3 C++23: stable temporal jitter

```
Mat4 addJitter(
    Mat4 projection,
    double pixelOffsetX,
    double pixelOffsetY,
    int width,
    int height
) {
    if (width <= 0 || height <= 0)
        throw std::invalid_argument("Invalid viewport");

    // NDC +X points right; NDC +Y points up.
    // Raster +Y points down.
    const double jx = 2.0 * pixelOffsetX / width;
    const double jy = -2.0 * pixelOffsetY / height;

    // Homogeneous shift: x' = x + jx*w,
    //                    y' = y + jy*w.
    for (int col = 0; col < 4; ++col) {
        projection(0, col) += jx * projection(3, col);
        projection(1, col) += jy * projection(3, col);
    }

    return projection;
}
```

This implementation preserves the perspective divide's meaning and shifts the projected image by the requested pixel amount.

In production, retain the unjittered projection separately.

### 37.4 C++23: Newton undistortion

The important core is:

```
// At the current normalized undistorted guess (x, y):
const Vec2 predicted = distort({x, y}, coefficients);

const Vec2 residual = predicted - observedDistorted;

const Mat2 J = distortionJacobian(
    {x, y},
    coefficients
);

// Solve J * delta = residual.
// Use a safeguarded numerical solve in production.
const Vec2 delta = solve2x2(J, residual);

x -= delta.x;
y -= delta.y;
```

This is not just a procedural recipe. Every step follows from the local linearization:

$$
 D(\mathbf q+\Delta) \approx D(\mathbf q)+J_D(\mathbf q)\Delta. 
$$

A complete implementation must check the determinant or numerical conditioning, limit iterations, reject nonfinite results, and confirm the final residual.

The downloadable file contains the full analytic $2\times2$ Jacobian and the explicit Newton update.

### What the actual repositories establish

The source inspection is particularly useful because your graphics renderer, capture calibration, and experimental camera models use related mathematics but different abstractions.

#### KairoRenderer — camera state

The inspected [KairoRenderer Camera.cppm](https://github.com/swayam8624/KairoRenderer/blob/main/Camera.cppm?utm_source=chatgpt.com) provides a camera pose, view matrix, and projection matrix.

It uses a right-handed camera and conventional zero-to-one Vulkan depth. It flips projected Y for the chosen positive-height viewport interpretation.

The inspected camera module does not itself expose a temporal jitter sequence or previous-frame view/projection state.

For TAA, a useful interface expansion would separate stable camera pose from render-sample state:

```
struct TemporalCameraMatrices {
    Mat4f currentView;
    Mat4f currentProjectionJittered;
    Mat4f currentProjectionUnjittered;

    Mat4f previousView;
    Mat4f previousProjectionJittered;
    Mat4f previousProjectionUnjittered;

    Vec2f currentJitterPixels;
    Vec2f previousJitterPixels;

    bool historyValid;
};
```

Such a structure helps prevent accidental use of the wrong projection when generating motion vectors, culling objects, or reprojecting history.

The type is a proposed architectural contract, not an existing repository definition.

#### Maveb — camera paths

The inspected [Maveb CameraPath.cpp](https://github.com/swayam8624/Maveb/blob/main/engine/scene/src/CameraPath.cpp?utm_source=chatgpt.com) supports time-ordered keyframes and interpolates translation, rotation, FOV, and exposure.

It uses linear interpolation for translation, spherical interpolation for quaternion rotation, and linear interpolation for the scalar camera parameters.

Its stored transform translation is float-based even though keyframe position values are initially parsed from JSON as double.

For very large scenes, that conversion can become a precision boundary.

An improved high-precision architecture would retain an authoritative double-precision world pose and convert to camera-relative float only when constructing renderer data.

Also, a temporal renderer must distinguish sampled camera-path motion from TAA jitter. Jitter should not be baked into the authoritative cinematic camera trajectory.

#### Maveb — calibration loading

The inspected [Maveb CalibrationLoader.cpp](https://github.com/swayam8624/Maveb/blob/main/engine/capture/calibration/CalibrationLoader.cpp?utm_source=chatgpt.com) reads:

$$
 f_x,\ f_y,\ c_x,\ c_y 
$$

and supports distortion-model labels including `None`, `Radial`, and `Opencv`.

Its visible loader reads an eight-element distortion array when present.

That array's exact coefficient semantics must be established from the calibration type and downstream implementation before interpreting individual positions.

The loader also converts a parsed `uint64_t` image dimension to `uint32_t` using `static_cast` without a visible range check in that helper.

A hardened loader should verify that dimensions fit the destination type and satisfy the downstream camera model's valid range.

The loader's successful parsing should not be confused with proving that a calibration has physically valid focal lengths, monotonic distortion, or acceptable reprojection error.

Those require semantic validation.

#### Vulkax — observation-space pinhole camera

The inspected [Vulkax pinhole_camera.hpp](https://github.com/swayam8624/Vulkax/blob/main/include/vulkax/world/pinhole_camera.hpp?utm_source=chatgpt.com) implements a pinhole projection forward model with:

$$
 f_x,\ f_y,\ c_x,\ c_y, 
$$

camera position, target, up, and near plane.

It uses world-space basis vectors and computes positive forward depth as the dot product of the point's camera-relative displacement with the forward direction.

Its pixel mapping is:

$$
 \boxed{ u=f_x\frac{X}{d}+c_x } 
$$

and:

$$
 \boxed{ v=c_y-f_y\frac{Y}{d} } 
$$

where its camera $Y$ axis points upward while pixel Y points downward.

That is consistent with a top-left image coordinate system.

The inspected pinhole model does not apply a nonlinear lens-distortion function to the projected coordinates.

That may be completely appropriate for an idealized experimental observation model. But if comparing its predictions against real distorted camera images, the model boundary must be explicit.

The calibration and experimental observation pipelines should agree on whether observations are raw distorted pixels or rectified/undistorted pixels.

Mixing them creates systematic residuals that a parameter optimizer may attempt to explain using incorrect geometry.

### Research bridge: why these two modules matter

These 35 parts collectively explain several problems that otherwise look unrelated.

A rendering engine culls geometry using frustum half-spaces. A temporal renderer reconstructs history using current and previous projective transformations. A large-world system changes coordinate frames to preserve local precision. A rasterizer interpolates attributes using homogeneous $w$. A camera-calibration system estimates the projective mapping between physical 3D geometry and measured image pixels. A distortion solver uses nonlinear root finding to reverse lens effects.

The common mathematical structure is:

$$
 \boxed{ \text{Geometry} \rightarrow \text{Coordinate transformation} \rightarrow \text{Projection} \rightarrow \text{Observation} }. 
$$

The same structure supports both forward rendering and inverse reconstruction.

In forward rendering, scene geometry and camera parameters determine predicted pixels.

In reconstruction, observed pixels constrain unknown geometry and camera parameters.

A strong research system should be able to traverse the forward pipeline, differentiate it, quantify error, and validate it against measurements.

For an uncertainty-aware reconstruction pipeline, a particularly valuable test is to compare analytic projection Jacobians against finite-difference derivatives, then compare covariance propagation against repeated perturbed 3D samples.

For a TAA renderer, compare jittered versus unjittered motion conventions on a stationary scene, a translating camera, and a translating object. Each case should have a known expected displacement.

For a frustum culler, compare the conservative bounding-volume result against brute-force geometric visibility on randomized scenes. A false positive is permissible; a false negative indicates a correctness failure.

For a calibration pipeline, generate synthetic distorted observations from known parameters, recover their undistorted normalized coordinates, and measure the round-trip error over the entire image radius.

These tests are more persuasive than one visually correct screenshot because they isolate mathematical properties and failure modes.


<!-- end V11.19 -->

<!-- begin V11.20 -->
## A runnable camera oracle with declared conventions

This new, independent C++23 driver makes the convention choices explicit. It tests the finite and infinite perspective cases, both depth directions, homogeneous reconstruction, and clip-plane identities. The small pivoted inverse is a teaching oracle for these well-scaled matrices; its fixed pivot cutoff is not a general conditioning test. Original source-reported repository or laboratory results remain separate evidence.

```cpp
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>
using V4=std::array<double,4>;
using M4=std::array<V4,4>; // Row indexing is explicit; vectors are columns.
V4 mul(const M4& m,const V4& p) {
    V4 out{};for(int i=0;i<4;++i)for(int j=0;j<4;++j)out[i]+=m[i][j]*p[j];return out;
}
M4 inverse(M4 a) {
    M4 b{};for(int i=0;i<4;++i)b[i][i]=1;
    for(int j=0;j<4;++j) {
        int p=j;for(int i=j+1;i<4;++i)if(std::abs(a[i][j])>std::abs(a[p][j]))p=i;
        // Teaching oracle for these well-scaled matrices, not a general conditioning test.
        if(std::abs(a[p][j])<1e-14)throw std::invalid_argument("singular oracle matrix");
        std::swap(a[p],a[j]);std::swap(b[p],b[j]);double d=a[j][j];
        for(int k=0;k<4;++k){a[j][k]/=d;b[j][k]/=d;}
        for(int i=0;i<4;++i)if(i!=j){double q=a[i][j];for(int k=0;k<4;++k){a[i][k]-=q*a[j][k];b[i][k]-=q*b[j][k];}}
    }
    return b;
}
M4 perspective(double n,double f,bool reverse,bool infinite) {
    if(!(n>0)||!std::isfinite(n)||(!infinite&&(!(f>n)||!std::isfinite(f))))
        throw std::invalid_argument("invalid depth bounds");
    // RH, camera forward -Z, vertical FOV 90 degrees, aspect 2, depth [0,1].
    M4 p{};p[0][0]=.5;p[1][1]=1;p[3][2]=-1;
    if(infinite){p[2][2]=reverse?0:-1;p[2][3]=reverse?n:-n;}
    else{p[2][2]=reverse?n/(f-n):f/(n-f);p[2][3]=reverse?n*f/(f-n):n*f/(n-f);}
    return p;
}
V4 divide(V4 p) {
    if(!std::isfinite(p[3]) || p[3]==0)throw std::invalid_argument("invalid homogeneous divide");
    double w=p[3];for(double& x:p)x/=w;return p;
}
double dot(V4 a,V4 b){double s=0;for(int i=0;i<4;++i)s+=a[i]*b[i];return s;}
int main() {
    const double n=.1,f=100;
    for(bool reverse:{false,true})for(bool infinite:{false,true}) {
        auto p=perspective(n,f,reverse,infinite);auto ip=inverse(p);
        assert(std::abs(divide(mul(p,{0,0,-n,1}))[2]-(reverse?1:0))<1e-14);
        if(!infinite)assert(std::abs(divide(mul(p,{0,0,-f,1}))[2]-(reverse?0:1))<1e-14);
        for(V4 v:{V4{1,1,-5,1},V4{-2,.3,-10,1}}) {
            auto c=mul(p,v);auto ndc=divide(c);auto recovered=divide(mul(ip,ndc));
            for(int i=0;i<4;++i)assert(std::abs(recovered[i]-v[i])<3e-12);
            // Planes come directly from clip inequalities, without normalized-distance claims.
            for(int side:{-1,1}) {
                V4 plane{};for(int k=0;k<4;++k)plane[k]=p[3][k]+side*p[0][k];
                assert(std::abs(dot(plane,v)-(c[3]+side*c[0]))<1e-14);
            }
            const double q=ndc[2];
            const double s=infinite?(reverse?n/q:n/(1-q)):
                (reverse?n*f/(n+q*(f-n)):n*f/(f-q*(f-n)));
            assert(std::abs(s+v[2])<3e-12);
        }
    }
    auto p=perspective(n,f,false,false);auto q=divide(mul(p,{1,1,-5,1}));
    double u=(q[0]+1)*800/2,v=(1-q[1])*400/2;
    assert(std::abs(u-440)<1e-12&&std::abs(v-160)<1e-12);
    double corrected=(1./12+1./12)/(1./3+1./12+1./12);assert(std::abs(corrected-1./3)<1e-15);
    // Rectified stereo: metric baseline, pixel focal length, axial depth.
    const double fpx=800, baseline=.064,Z=2,disparity=fpx*baseline/Z;
    assert(std::abs(disparity-25.6)<1e-14&&std::abs(fpx*baseline/disparity-Z)<1e-15);
    const double sigmaZ=Z*Z/(fpx*baseline)*.25;
    // Exact paraxial CoC/DoF limits for the stated thin-lens convention.
    const double F=.05,N=8,c=.00003,S=3,H=F*F/(N*c)+F;
    const double near=S*(H-F)/(H+S-2*F),far=S*(H-F)/(H-S),D=F/N;
    auto coc=[=](double object){return D*F*std::abs(S-object)/(object*(S-F));};
    assert(std::abs(coc(near)-c)<1e-17 && std::abs(coc(far)-c)<1e-17);
    assert(coc(S)==0);
    // Infinite reverse far endpoint is a homogeneous direction, not a finite point.
    auto ideal=mul(inverse(perspective(n,f,true,true)),{0,0,0,1});assert(ideal[3]==0);
    bool guarded=false;try{(void)divide(ideal);}catch(const std::invalid_argument&){guarded=true;}assert(guarded);
    std::cout<<std::setprecision(15)<<"raster=("<<u<<","<<v<<") perspective_attribute="<<corrected
             <<" stereo_disparity="<<disparity<<" sigmaZ="<<sigmaZ
             <<" DoF=["<<near<<","<<far<<"]\n";
}
```

Expected witnesses include raster position $(440,160)$ in an $800\times400$ upper-left raster, corrected attribute $1/3$, disparity $25.6$ pixels at $2$ metres, first-order depth uncertainty $0.01953125$ metres, and thin-lens near/far limits approximately $2.337905$ and $4.185268$ metres for the declared optical parameters. The infinite reversed far endpoint is tested as an ideal direction with zero homogeneous $w$, and division is rejected.


<!-- end V11.20 -->


# The mathematics becomes a reliable implementation

Implementation contracts expose assumptions about representation, ownership, dimensions, arithmetic and failure. Keep each complete laboratory as an independent program; multiple main functions in this book are separate examples, not one translation unit.

<!-- begin V06.23 -->
## Three stored numbers are not the complete contract

The robot's computation must eventually use memory. A [[Chapter Edition/01 - Computing Foundations/37 - Camera Contracts and Transform-Chain Debugging#Three stored numbers are not the complete contract|vector value type]] stores components directly and can be copied without separate allocation. A tiny arithmetic primitive normally does not need a heap, reference counting, or a virtual interface.

An ordinary `Vec2` stores two components, `Vec3` three, and `Vec4` four. The last may hold homogeneous coordinates, a color with an opacity channel, or unrelated data. Four components do not automatically make an object a quaternion. A quaternion has a multiplication law and rotation invariants that a raw `Vec4` does not supply.

### Initialization and access should be unsurprising

Member initializers such as `double x{};` make default construction produce zero. The zero vector is a valid displacement, though not a valid unit direction. A deliberately uninitialized performance path needs explicit lifetime and initialization rules; evaluating an indeterminate component is not a legitimate way to create a diagnostic NaN.

Named fields are convenient, but adjacent members are not an array. Do not implement component access using `(&x)[i]`. Use explicit selection or store an actual array. C++ pointer arithmetic is tied to an array relationship, not merely neighboring addresses. [C++ working draft, Additive Operators](https://eel.is/c++draft/expr.add).

`constexpr` permits suitable expressions to be evaluated at compile time; it does not force every call to happen there. `noexcept` promises not to throw; it does not promise finite results, valid geometry, or freedom from undefined behavior. `[[nodiscard]]` encourages callers to notice important results, especially fallible construction.

Read swizzles select and reorder components: a `yx()` result from $(2,5)$ is $(5,2)$. Returning a value is simple. Writable swizzles require aliasing and duplicate-destination policies. For a swap-like assignment, save source values before writing destinations. A write to duplicated components such as `xx` needs explicit rejection or a documented ordering rule; do not hide it behind clever proxies.

### Operators should preserve the mathematical question

Addition, subtraction, unary negation, scalar multiplication, and scalar division are natural vector operations. If `Vec3 * Vec3` could mean a dot product, cross product, or componentwise product, the operator obscures its result type and meaning. Prefer named `dot`, `cross`, and `hadamard` operations.

The **Hadamard product** is componentwise multiplication. It is useful for channel scales and certain calculations, but is not the inner product. Do not add an ordering operator just because components can be lexicographically sorted; geometric vectors have no canonical “less than” meaning.

Keep exact component equality separately from geometric closeness. Even exact floating-point equality is not byte identity: positive and negative zero compare equal, and NaN is not equal to itself. Raw `memcmp` can also inspect irrelevant padding. [[Standard-layout and trivial copyability]] do not automatically define serialization, GPU layout, array traversal, or semantic equality.

### Choose the component type separately from the geometric role

A generic storage type may use a component parameter and provide concrete aliases for `float` and `double`. That parameter describes numeric storage; a space tag describes coordinate meaning. Neither replaces a point/vector or normal/direction distinction.

If the algorithm needs fractional division, finiteness checks and square roots, its numeric requirements are stronger than “supports addition.” An integer-coordinate record can be useful for grid cells, but ordinary integer normalization does not implement the real-vector formula. Constrain the algorithm or provide an explicitly different result type and policy.

The runnable reference below uses `double` to keep the examples and tolerances consistent. It is not a demonstration that every engine vector should use binary64. A `float` version must be tested with its own representable range, component spacing, tolerances and layout. Merely substituting a type name does not transfer all the numerical guarantees.

For an $n$-component generic representation, independent coordinate arithmetic takes work proportional to $n$, and a dot product uses $n$ products and a reduction. For fixed three-component types these are constant-size operations. Normalization adds a reduction, square root and scaling. Larger batches are where memory arrangement and throughput measurements become central.


<!-- end V06.23 -->

<!-- begin V06.24 -->
## Storage and computation can choose different layouts

[[Chapter Edition/01 - Computing Foundations/37 - Camera Contracts and Transform-Chain Debugging#Storage and computation can choose different layouts|Alignment]] is the required address spacing for an object. `alignof` reports that requirement; `sizeof` reports the object size, including padding. The array stride is `sizeof` its element type. Larger alignment may create tail padding so every array element remains correctly aligned.

On common binary32 platforms, three `float` members often occupy twelve bytes. A sixteen-byte-aligned computation type often occupies sixteen bytes. These are target facts to measure, not universal C++ guarantees. A million such vectors then occupy approximately twelve or sixteen million bytes of payload. The four-million-byte difference is about $33\%$ of the smaller payload.

Do not use a global packing pragma to manufacture the result. “Packed vector” here means a compact storage representation with measured layout, not unaligned arbitrary member access.

Separate a compact storage record from an aligned register-friendly record where the workload needs both. The fourth compute lane is a declared auxiliary lane, not automatically a homogeneous point coordinate or trustworthy zero. Dot products must not accidentally include uninitialized padding.

### SIMD has two different opportunities

**SIMD**, single instruction multiple data, operates on several lanes with one instruction. It may process the components of one vector, or the same component of several objects.

An **array of structures**, AoS, places complete objects beside one another: all components of particle one, then particle two. A **structure of arrays**, SoA, places all horizontal coordinates together, all vertical coordinates together, and so on. **AoSoA** groups small blocks of component arrays. The best layout follows which values the kernel consumes together.

For a batch of particles, across-object SIMD can update several horizontal positions independently. Inside a single-vector dot product, SIMD still needs a horizontal reduction combining lanes. A cross product needs component permutations and subtraction of products. One vector-looking expression is not necessarily one cheap hardware instruction.

Do not load sixteen bytes from an isolated twelve-byte object and justify it by discarding the fourth lane. The load itself may exceed valid accessible storage. Use a safe load/conversion contract or a type that really owns those bytes.

Fused multiply-add, compiler reassociation, approximate reciprocal square root, and reduction order can change floating-point results. An approximate normalization might refine an inverse-square-root estimate $y$ using

$$
y_{\mathrm{new}}=y\left(\frac32-\frac12xy^2\right),
$$

where $x$ is a positive squared length. This is Newton refinement for reciprocal square root. Its quality depends on the initial estimate, ranges, operation semantics, and iteration count. It is not a replacement for validating a zero or non-finite direction.

Optimize only after establishing a scalar reference, representative data, an error budget, and a performance measurement. This chapter does not claim SIMD speedups or GPU timing. Maximum throughput and cross-platform bitwise repeatability may require different policies.

### A GPU buffer is a separate agreement

A vertex stream format, a shader storage block, a uniform block, and a CPU object can all describe “three floats” yet have different offset and stride rules. Under Vulkan base alignment, a three-component vector can contain twelve bytes while requiring sixteen-byte alignment; relaxed or scalar layout changes some permitted placements. Check enabled features and shader decorations rather than transferring a universal `Vec3` layout rule. [Vulkan Guide, Shader Memory Layout](https://docs.vulkan.org/guide/latest/shader_memory_layout.html).

Write an explicit upload record when necessary and verify member offsets, array strides, component format, and matrix convention. The C++ object being trivially copyable only concerns certain language-level byte copies. It does not certify the receiver's interpretation contract. See [[Interpretation contract]] and [[SIMD and GPU data layouts]].

Serialization also needs a scalar format, byte order, version, validation, and treatment of padding/non-finite values. Encode semantic components, not incidental object bytes. Copying an object does not solve these questions.


<!-- end V06.24 -->


## A reliability checklist for moving from symbols to images

Before interpreting a result, ask which object is represented, which frames and bases its coordinates use, and what units each coordinate carries. Check whether the operation is linear, affine, or only locally approximated by a derivative. Confirm whether a matrix stores input directions as columns and whether vector perturbations are columns or rows. For a solve, inspect rank, residual, singular spectrum, and the metric defining “smallest.” For PCA, verify centering, covariance convention, feature scaling, and eigengap. For a derivative, state the point, input/output dimensions, convention, and validity neighborhood. For optimization, state noise assumptions, gauges, constraints, stopping conditions, and conditioning.

Use small witnesses. Apply a transform to each basis direction and reconstruct a sample vector. Check $A\hat x-b$ rather than trusting a solver status alone. Verify a claimed null vector by computing $Az$. Reconstruct $U\Sigma V^T$ and check the orthogonality of $U$ and $V$. Project centered sample points onto PCA axes and compare measured variance with eigenvalues. Compare a Jacobian-vector product against a carefully scaled finite difference at an interior point. Perturb the observations and see whether the answer moves in the predicted weak directions. These checks turn abstract contracts into observable evidence.

Finally, remember that the same image can be described with different coordinates and that the same data can support more than one model. Mathematics gives explicit rules for moving between representations and for measuring what the evidence determines. It does not remove the need to declare conventions, units, priors, thresholds, or uncertainty. A reliable graphics pipeline makes those assumptions visible from the first vector through the final pixel.

<!-- begin V06.25 -->
## Reliability begins at the boundary of the calculation

The mathematical model uses real vectors. The implementation can contain NaNs, infinities, and already-rounded components. [[Chapter Edition/01 - Computing Foundations/37 - Camera Contracts and Transform-Chain Debugging#Reliability begins at the boundary of the calculation|Finite-state checks]] belong at points where data enters a trusted subsystem and where operations can fail.

### Ask what “close” means before choosing a tolerance

For finite scalars, one common rule is

$$
|a-b|\le\max\left(\varepsilon_{\mathrm{abs}},
\varepsilon_{\mathrm{rel}}\max(|a|,|b|)\right).
$$

Absolute tolerance handles values near zero; relative tolerance scales with magnitude. Both tolerances must be finite and nonnegative. Very large opposite-sign operands can overflow their difference; a routine needs a range policy, not only a nice displayed equation.

For vectors, componentwise tolerance creates a box-shaped acceptable error region. Euclidean distance tolerance creates a ball. If each of two components differs by $0.09$, a $0.1$ component tolerance passes, but Euclidean error is approximately $0.127$, so a $0.1$ distance tolerance fails. Both answers can be correct because the questions differ.

For unit directions, an angular bound often communicates the actual requirement. For numerical-library tests, ULP distance may be appropriate. ULP means spacing between nearby representable values, not a fixed physical distance.

Approximate equality is generally not transitive. With absolute tolerance $1$, values $0$ and $0.75$ are close, and $0.75$ and $1.5$ are close, but $0$ and $1.5$ are not. Do not use such a relation as ordinary key equality in a hash container.

### Locate the first invalid operation

Suppose a vertex disappears. The renderer may only be where corruption became visible. A zero triangle edge may have produced an invalid normal two stages earlier. Trace the dataflow:

1. Were imported positions finite and in the intended units?
2. Did point subtraction preserve a usable displacement?
3. Did the triangle have nonzero area?
4. Did normalization report failure?
5. Was the transform invertible for its normal operation?
6. Did the shader receive the intended bytes?

On typical IEEE arithmetic, NaN fails ordinary equality and ordered comparisons. A NaN distance may cause a collision branch to be skipped rather than a crash. Clamping and min/max are not universal NaN sanitizers. Reject invalid geometry explicitly and log the first responsible stage.

Constructed diagnostic NaNs can help debugging. Reading uninitialized storage is not a substitute. GPU diagnostic colors or buffers can expose invalid regions, but interpreting them still requires a documented shader environment.

### Large world coordinates can lose a small movement before geometry starts

For binary32 values near $10^8$, adjacent representable values are spaced by about $8$. Converting the intended positions $10^8$ and $10^8+1$ can produce the same stored coordinate. Their computed difference is then zero. No later normalization, double-precision dot, or exact predicate can recover the metre that disappeared during input storage.

Useful architectures retain large global locations in a wider representation and calculate local differences before narrowing, or use camera-relative/local-origin coordinates. The choice must respect the required scene scale and resolution. “Use double everywhere” is not a substitute for a measured storage and bandwidth policy.


<!-- end V06.25 -->

<!-- begin V06.26 -->
## Types can remember the promises that three components forget

The robot's local direction and the camera's world direction may both have tuple $(1,0,0)$ while describing different physical arrows. [[Chapter Edition/01 - Computing Foundations/37 - Camera Contracts and Transform-Chain Debugging#Types can remember the promises that three components forget|Space tags]] can make their difference visible to the compiler.

A tagged point and vector can enforce same-space subtraction and the affine result type:

```cpp
template<class Space> struct VectorIn { double x{}, y{}, z{}; };
template<class Space> struct PointIn  { double x{}, y{}, z{}; };
template<class Space>
constexpr VectorIn<Space> operator-(PointIn<Space> q,
                                    PointIn<Space> p) noexcept {
    return {q.x - p.x, q.y - p.y, q.z - p.z};
}
template<class Space>
constexpr PointIn<Space> operator+(PointIn<Space> p,
                                   VectorIn<Space> d) noexcept {
    return {p.x + d.x, p.y + d.y, p.z + d.z};
}
```

The tag is a template argument, not a stored member. The implementation need not add runtime bytes; inspect sizes on the intended ABI. A single broad `LocalSpace` tag still does not distinguish two independently moving objects' local frames. Frame identity may need ownership context, transform APIs, or runtime checks.

Similarly a transform type can record its source and destination spaces. A local-to-world transform accepts local inputs and produces world outputs. Composition must match the intermediate space. Separate point, tangent-direction, and normal operations because their transformation laws differ.

A `UnitVec3` wrapper should be created only through successful validation/normalization. Its value accessor should not expose mutable components that can invalidate the promise. Negation preserves unit length. Multiplying by two, adding unit vectors, and ordinary linear interpolation do not. Return an ordinary vector when an operation drops the invariant.

Do not force every semantic combination into the primitive type. A coherent layering is: simple component storage, explicit geometric operations, validated wrappers, then subsystem-specific types. Finite, normalized, coordinate-space, units, and role checks are most valuable at expensive error boundaries.


<!-- end V06.26 -->

<!-- begin V06.27 -->
## A complete scalar laboratory makes the story executable

The following C++20 program is a teaching reference, not a claim about any existing Kairo library. It allocates no dynamic storage for its vectors. It exposes failures where the mathematics has no usable answer. Core arithmetic assumes a suitable result range; only the specifically range-aware routines make stronger intermediate guarantees.

The operator division has an asserted finite/nonzero divisor precondition. Assertions are diagnostics, not release-mode input validation. Callers handling untrusted input must use checked boundaries. This design deliberately separates trusted primitive arithmetic from fallible geometry.

### Reference implementation and regression driver

Copy this single block into a source file and compile it as shown below. The tests use ordinary floating-point semantics, not fast-math. A deterministic generator produces reproducible moderate-range samples; it is not a statistical certification of every input.

<!-- phase6-lab-start -->
```cpp
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <numbers>
#include <optional>
#include <stdexcept>
#include <type_traits>

namespace phase6 {
struct Vec3 {
    double x{}, y{}, z{};
    constexpr double& at(std::size_t i) {
        if (i == 0) return x;
        if (i == 1) return y;
        if (i == 2) return z;
        throw std::out_of_range("Vec3 component");
    }
    constexpr const double& at(std::size_t i) const {
        if (i == 0) return x;
        if (i == 1) return y;
        if (i == 2) return z;
        throw std::out_of_range("Vec3 component");
    }
    friend constexpr bool operator==(Vec3, Vec3) = default;
};
constexpr Vec3 operator+(Vec3 a, Vec3 b) noexcept {
    return {a.x+b.x, a.y+b.y, a.z+b.z};
}
constexpr Vec3 operator-(Vec3 a, Vec3 b) noexcept {
    return {a.x-b.x, a.y-b.y, a.z-b.z};
}
constexpr Vec3 operator-(Vec3 a) noexcept {
    return {-a.x,-a.y,-a.z};
}
constexpr Vec3 operator*(Vec3 a, double s) noexcept {
    return {a.x*s,a.y*s,a.z*s};
}
constexpr Vec3 operator*(double s, Vec3 a) noexcept { return a*s; }
Vec3 operator/(Vec3 a, double s) noexcept {
    assert(std::isfinite(s) && s != 0.0);
    return {a.x/s,a.y/s,a.z/s};
}
constexpr double dot(Vec3 a, Vec3 b) noexcept {
    return a.x*b.x+a.y*b.y+a.z*b.z;
}
constexpr Vec3 cross(Vec3 a, Vec3 b) noexcept {
    return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};
}
bool finite(Vec3 a) noexcept {
    return std::isfinite(a.x)&&std::isfinite(a.y)&&std::isfinite(a.z);
}
double length(Vec3 a) noexcept { return std::hypot(a.x,a.y,a.z); }
double maxAbs(Vec3 a) noexcept {
    return std::max({std::abs(a.x),std::abs(a.y),std::abs(a.z)});
}

class UnitVec3 {
    Vec3 v_;
    explicit UnitVec3(Vec3 v) noexcept : v_(v) {}
public:
    Vec3 value() const noexcept { return v_; }
    [[nodiscard]] static std::optional<UnitVec3> from(Vec3 v) noexcept {
        if (!finite(v)) return std::nullopt;
        const double m=maxAbs(v);
        if (m==0.0) return std::nullopt;
        const Vec3 s=v/m;
        const double n=std::sqrt(dot(s,s));
        if (!(n>0.0) || !std::isfinite(n)) return std::nullopt;
        return UnitVec3{s/n};
    }
};

Vec3 project(Vec3 a, UnitVec3 axis) noexcept {
    const Vec3 u=axis.value(); return u*dot(a,u);
}
Vec3 reject(Vec3 a, UnitVec3 axis) noexcept { return a-project(a,axis); }
Vec3 reflect(Vec3 a, UnitVec3 normal) noexcept {
    const Vec3 n=normal.value(); return a-n*(2.0*dot(a,n));
}
[[nodiscard]] std::optional<double> angle(Vec3 a, Vec3 b) noexcept {
    if (!finite(a)||!finite(b)) return std::nullopt;
    const double ma=maxAbs(a), mb=maxAbs(b);
    if (ma==0.0||mb==0.0) return std::nullopt;
    a=a/ma; b=b/mb;
    return std::atan2(length(cross(a,b)),dot(a,b));
}

enum class RefractionState { transmitted, totalInternalReflection, invalid };
struct Refraction {
    RefractionState state;
    std::optional<UnitVec3> direction;
};
[[nodiscard]] Refraction refract(UnitVec3 incident, UnitVec3 normal,
                                  double eta) noexcept {
    if (!std::isfinite(eta)||eta<=0.0)
        return {RefractionState::invalid,std::nullopt};
    const Vec3 i=incident.value(), n=normal.value();
    const double rawCos=-dot(i,n);
    if (rawCos<0.0) return {RefractionState::invalid,std::nullopt};
    const double ci=std::clamp(rawCos,0.0,1.0);
    const double tangentSine=eta*std::sqrt(std::max(0.0,1.0-ci*ci));
    if (tangentSine>1.0)
        return {RefractionState::totalInternalReflection,std::nullopt};
    const double ct=std::sqrt(std::max(0.0,1.0-tangentSine*tangentSine));
    // Decomposed form avoids a large cancelling normal coefficient.
    auto transmitted=UnitVec3::from((i+n*ci)*eta-n*ct);
    if (!transmitted) return {RefractionState::invalid,std::nullopt};
    return {RefractionState::transmitted,transmitted};
}

struct Frame { UnitVec3 tangent,bitangent,normal; };
[[nodiscard]] std::optional<Frame> frame(UnitVec3 normal) noexcept {
    const Vec3 n=normal.value();
    const double x=std::abs(n.x), y=std::abs(n.y), z=std::abs(n.z);
    const Vec3 helper=(x<=y&&x<=z)?Vec3{1,0,0}:
                      (y<=z?Vec3{0,1,0}:Vec3{0,0,1});
    auto t=UnitVec3::from(cross(helper,n));
    if (!t) return std::nullopt;
    auto b=UnitVec3::from(cross(n,t->value()));
    if (!b) return std::nullopt;
    return Frame{*t,*b,normal};
}

struct Point3 { double x{},y{},z{}; };
constexpr Vec3 operator-(Point3 q, Point3 p) noexcept {
    return {q.x-p.x,q.y-p.y,q.z-p.z};
}
constexpr Point3 operator+(Point3 p, Vec3 v) noexcept {
    return {p.x+v.x,p.y+v.y,p.z+v.z};
}
// Moderate-range, nondegenerate triangles only. Not a certified predicate.
[[nodiscard]] std::optional<std::array<double,3>> barycentric(
    Point3 p,Point3 a,Point3 b,Point3 c) noexcept {
    const Vec3 e=b-a,f=c-a,q=p-a;
    if (!finite(e)||!finite(f)||!finite(q)) return std::nullopt;
    const double ee=dot(e,e),ef=dot(e,f),ff=dot(f,f);
    const double eq=dot(e,q),fq=dot(f,q),d=ee*ff-ef*ef;
    if (!(d>0.0)||!std::isfinite(d)) return std::nullopt;
    const double beta=(ff*eq-ef*fq)/d, gamma=(ee*fq-ef*eq)/d;
    const std::array<double,3> weights{1.0-beta-gamma,beta,gamma};
    for(double w:weights) if(!std::isfinite(w)) return std::nullopt;
    return weights;
}

// Finite values only. Scaled difference avoids intermediate subtraction overflow.
bool nearScalar(double a,double b,double absolute,double relative) noexcept {
    if (!std::isfinite(a)||!std::isfinite(b)||!std::isfinite(absolute)||
        !std::isfinite(relative)||absolute<0.0||relative<0.0) return false;
    if (a==b) return true;
    const double m=std::max(std::abs(a),std::abs(b));
    const double scaledDifference=std::abs(a/m-b/m);
    return scaledDifference<=std::max(absolute/m,relative);
}
bool nearVector(Vec3 a,Vec3 b,double tolerance=1e-12) noexcept {
    return nearScalar(a.x,b.x,tolerance,tolerance)&&
           nearScalar(a.y,b.y,tolerance,tolerance)&&
           nearScalar(a.z,b.z,tolerance,tolerance);
}
static_assert(std::is_trivially_copyable_v<Vec3>);
static_assert(std::is_standard_layout_v<Vec3>);
static_assert(dot(Vec3{1,2,3},Vec3{4,5,6})==32);
static_assert(cross(Vec3{1,0,0},Vec3{0,1,0})==Vec3{0,0,1});
} // namespace phase6

int main() {
    using namespace phase6;
    const Point3 a{0,0,0},b{4,0,0},c{0,3,0},p{1,1,0};
    auto weights=barycentric(p,a,b,c);
    assert(weights);
    assert(nearScalar((*weights)[0],5.0/12.0,1e-12,1e-12));
    assert(nearScalar((*weights)[1],0.25,1e-12,1e-12));
    assert(nearScalar((*weights)[2],1.0/3.0,1e-12,1e-12));
    assert(nearVector((b-a)*(*weights)[1]+(c-a)*(*weights)[2],p-a));
    assert(!barycentric(p,a,b,b));
    auto normal=UnitVec3::from(cross(b-a,c-a)); assert(normal);
    assert(length(cross(b-a,c-a))/2.0==6.0);
    auto basis=frame(*normal); assert(basis);
    assert(nearScalar(dot(basis->tangent.value(),basis->bitangent.value()),
                      0.0,1e-12,1e-12));
    assert(dot(cross(basis->tangent.value(),basis->bitangent.value()),
               basis->normal.value())>0.0);
    auto axis=UnitVec3::from({2,0,0}); assert(axis);
    assert(nearVector(project({3,4,0},*axis),{3,0,0}));
    assert(nearVector(reject({3,4,0},*axis),{0,4,0}));
    assert(nearVector(reflect({0.6,0,-0.8},*normal),{0.6,0,0.8}));
    assert(!UnitVec3::from({0,0,0}));
    const double nan=std::numeric_limits<double>::quiet_NaN();
    const double inf=std::numeric_limits<double>::infinity();
    assert(!UnitVec3::from({nan,0,0}));
    assert(!UnitVec3::from({inf,0,0}));
    for(double scale:{1e-300,1.0,1e300}) {
        auto u=UnitVec3::from({scale,scale,0}); assert(u);
        assert(nearScalar(length(u->value()),1.0,1e-12,1e-12));
    }
    assert(!angle({0,0,0},{1,0,0}));
    assert(nearScalar(*angle({1,0,0},{-1,0,0}),
                      std::numbers::pi,1e-12,1e-12));
    assert(nearScalar(*angle({1e300,0,0},{0,1e300,0}),
                      std::numbers::pi/2.0,1e-12,1e-12));
    auto incident=UnitVec3::from({0.6,0,-0.8}); assert(incident);
    auto transmitted=refract(*incident,*normal,1.0/1.5);
    assert(transmitted.state==RefractionState::transmitted);
    assert(transmitted.direction);
    assert(nearScalar(transmitted.direction->value().x,0.4,1e-12,1e-12));
    auto grazing=UnitVec3::from({0.8,0,-0.6}); assert(grazing);
    assert(refract(*grazing,*normal,1.5).state==
           RefractionState::totalInternalReflection);
    assert(refract(*incident,*normal,-1.0).state==RefractionState::invalid);
    auto backward=UnitVec3::from({0,0,1}); assert(backward);
    assert(refract(*backward,*normal,1.0).state==RefractionState::invalid);
    assert(!nearScalar(nan,nan,1e-12,1e-12));
    assert(!nearScalar(inf,inf,1e-12,1e-12));
    assert(!nearScalar(1,1,-1,0));
    assert(!nearScalar(1e308,-1e308,1,1e-12));
    assert(nearScalar(0,0.75,1,0)&&nearScalar(0.75,1.5,1,0));
    assert(!nearScalar(0,1.5,1,0));
    Vec3 indexed{}; indexed.at(2)=7; assert(indexed.z==7);
    bool caught=false;
    try { (void)indexed.at(3); } catch(const std::out_of_range&) { caught=true; }
    assert(caught);
    const Vec3 tangent{1,1,0},n{1,-1,0};
    const Vec3 transformedTangent{2*tangent.x,tangent.y,tangent.z};
    const Vec3 transformedNormal{n.x/2.0,n.y,n.z};
    assert(dot(transformedTangent,transformedNormal)==0.0);

    std::uint32_t state=0x12345678u;
    auto sample=[&]() {
        state=1664525u*state+1013904223u;
        return (static_cast<double>(state)/4294967296.0-0.5)*20.0;
    };
    for(int k=0;k<2000;++k) {
        const Vec3 v{sample(),sample(),sample()};
        const Vec3 w{sample(),sample(),sample()};
        auto u=UnitVec3::from(w); assert(u);
        const Vec3 along=project(v,*u),across=reject(v,*u);
        assert(nearVector(along+across,v,1e-10));
        assert(nearScalar(dot(across,u->value()),0.0,1e-10,1e-10));
        assert(nearVector(project(along,*u),along,1e-10));
        assert(nearScalar(length(reflect(v,*u)),length(v),1e-10,1e-10));
        const Vec3 area=cross(v,w);
        assert(nearScalar(dot(v,area),0.0,1e-9,1e-9));
        assert(nearScalar(dot(w,area),0.0,1e-9,1e-9));
        assert(nearVector(cross(v,w),-cross(w,v),1e-10));
    }
    std::cout << "Phase 6 scalar laboratory: all assertions passed\n";
    std::cout << "Vec3 sizeof=" << sizeof(Vec3)
              << "; alignof=" << alignof(Vec3) << '\n';
}
```
<!-- phase6-lab-end -->

### What each layer guarantees, and what it does not

`Vec3` is component storage with exact component comparison. `UnitVec3::from` rejects non-finite and zero input and uses scaled normalization. Unit length is approximate under the tested floating-point environment, not an exact symbolic identity.

Projection and reflection accept a validated unit axis, so they do not repeat normalization. Their output can still overflow for sufficiently extreme arbitrary input vectors; the lab does not claim unbounded safe arithmetic.

`angle` scales inputs before computing bounded dot and cross values. `refract` distinguishes invalid configuration, total internal reflection, and transmission. Its critical-angle decision follows computed floating-point values and has no physical uncertainty band. Extreme index ratios and tiny tangential components still need domain-specific policies.

`barycentric` is intentionally a moderate-range educational solve. A positive computed denominator is not a certified condition estimate. A real geometry service needs scale-aware degeneracy and off-plane residual handling. The program's ordinary checks must not be presented as a robust-predicate implementation.

The approximate comparison rejects infinities as invalid geometric values, even equal ones. This is deliberate and differs from a scalar utility that supports infinite limits. Its scaled comparison changes rounding behavior slightly, so tolerance-boundary values are not guaranteed to match an alternative expression bit for bit.

The C++ source can be compiled with:

```sh
clang++ -std=c++20 -O2 -Wall -Wextra -Wpedantic phase6_lab.cpp -o phase6_lab
./phase6_lab
```

For a diagnostic build, add AddressSanitizer and UndefinedBehaviorSanitizer. These check certain execution errors, not the correctness of the underlying geometry or every language violation. The exact commands and observed results for this vault are recorded in [[Supplementary/Sources and Code Anchors#Phase 6 - Source analysis and verification|the Phase 6 verification record]].


<!-- end V06.27 -->

<!-- begin V06.28 -->
<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/01 - Computing Foundations/00 - Reading Map|Reading map]] · [[Chapter Edition/01 - Computing Foundations/36 - Exposure, Physical Optics, and Depth of Field|Previous chapter]] · [[Chapter Edition/01 - Computing Foundations/38 - Reconstruct the Complete Geometry Pipeline|Next chapter]]
