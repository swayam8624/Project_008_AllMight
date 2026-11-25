# Pass 34 — Computing Foundations source-unit routing

This is a complete heading-and-line inventory of the seven primary derived Computing Foundations files 32–38 at the pinned source revision `a54c4a1cf9041b9bca505a78fce6208ac711b305`. Each full file was retrieved; headings and counts were extracted mechanically. Routes below are proposed destinations, not a claim that every equation, code listing or example is already integrated. Selected passages on frusta, pixel centers and camera-relative arithmetic were read in detail for this pass. Derived source-span hashes are recorded as declared in the files, not independently recomputed against the continuous master. All source remains read-only. Line numbers are one-based in the derived file, not the continuous master.

## 32 - Geometry Becomes an Image and an Inverse Problem.md

- Git blob: `2bb93ac0b57df60f2b7e7655ad196373215a30bf`
- Body routing extent: lines 14–2449; 53,297 characters in retrieved file.
- Declared master span: Derived from Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md; source lines 78505–80940; source-span SHA256 340c3f3eab22b702f8d250fbf0fdb8bd0a1e0b2e848681aa23ee77df8dacef80.

| Derived lines | Destination and responsibility |
|---|---|
| 14–379 | 02, 04 — frames, matrices, basis, parent chains |
| 380–619 | 13–14 — view, clip and raster contracts |
| 620–891 | 10, 16, 20 — UV, tangents, filtering and normal transform |
| 892–1651 | 04, 12–13 — homogeneous transforms and rigid camera poses |
| 1652–2449 | 14, 27 — projective points, lines, incidence and planes |

Subsection navigation (every heading in the source body):

- 14: Geometry becomes an image and an inverse problem
- 19: From Coordinate Frames to Camera Images: Projective Geometry and Cameras
- 37: a vector is a measured displacement, a matrix is a frame rule
- 58: A coordinate is a claim about a frame
- 112: Deriving matrix notation
- 136: Numerical example
- 176: General versus orthonormal bases
- 248: Transformation order changes the result
- 296: Parent-child transformations
- 336: World space is a chosen convention
- 360: World origin and large scenes
- 380: A camera is an inverse pose
- 415: Why use view space?
- 475: Why clip before perspective division?
- 522: NDC does not mean raster pixels
- 582: Continuous raster positions versus discrete pixel indices
- 620: Texture coordinates are attached to geometry
- 640: Texture origin conventions
- 652: Texture filtering
- 692: Deriving surface tangents from UV coordinates
- 752: Orthogonalization
- 774: Nonuniform scaling and normals
- 846: Numerical example · V11.02.12
- 892: Homogeneous point representation
- 948: Why it matters for camera rays
- 1012: Volume scaling
- 1056: Why camera transforms should generally be rigid
- 1093: Camera-to-world transform
- 1109: View transform
- 1164: Orthonormality requirements
- 1212: Why the basis must be recomputed carefully
- 1242: Step 1: Construct forward
- 1266: Step 2: Construct right
- 1280: Step 3: Recompute up
- 1292: Step 4: Construct camera-to-world rotation
- 1308: Step 5: Construct the view matrix
- 1314: Degenerate up vectors
- 1332: Worked look-at example
- 1440: Expanded look-at view matrix
- 1462: Verify camera position maps to origin
- 1484: Verify a target point lies on the forward axis
- 1522: Numerical example · V11.02.31
- 1586: Rotation behaves similarly
- 1602: Passive versus active transformations
- 1616: Connection to robotics and reconstruction
- 1652: Perspective belongs to projective geometry
- 1672: An ordinary affine transformation
- 1698: Perspective does not preserve parallelism
- 1748: Why a new geometry is useful
- 1790: Directional interpretation
- 1822: Parallel lines meet at an ideal point
- 1872: The line at infinity
- 1896: Three-dimensional projective points at infinity
- 1960: Why zero scaling is excluded
- 1974: Derive invariance under projective transformation
- 2006: Points versus planes under transformation
- 2050: Numerical cautions
- 2094: Construct a line from two points
- 2122: Worked line construction
- 2172: Intersect two lines
- 2190: Worked intersection
- 2228: Line transformation
- 2246: Applications
- 2286: Plane through three points
- 2334: Example
- 2384: Three planes can determine a point
- 2408: Plane transformation revisited
- 2432: Near planes, frustum planes, and projective planes

## 33 - Projection, Clipping, and Raster Sampling.md

- Git blob: `ef481955be60941713582f1e5600191498f4bd8d`
- Body routing extent: lines 14–3411; 65,324 characters in retrieved file.
- Declared master span: Derived from Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md; source lines 80941–84338; source-span SHA256 fcf61fdb48daa14bb38e7cbb8d87cc82df71979648d4c3d5d7fa16fca6ec1b8b.

| Derived lines | Destination and responsibility |
|---|---|
| 14–1177 | 14 — orthographic and perspective projection |
| 1178–2262 | 14, 19 — depth, FOV, reversed-Z and precision |
| 2263–3411 | 14, 18, 20; Book 06 — conventions, viewport and API-specific bridge |

Subsection navigation (every heading in the source body):

- 14: Projection turns rays into clip coordinates
- 56: Why parallel rays matter
- 84: When orthographic projection is useful
- 158: Derive the depth mapping
- 212: Assemble the matrix
- 236: OpenGL default depth variant
- 264: Reversed-Z orthographic variant
- 290: Numerical example · V11.03.08
- 378: Projected size
- 398: Perspective is a rational mapping
- 442: Field of view from the viewing triangle
- 468: Numerical example · V11.03.12
- 550: Worked example · V11.03.13
- 566: What happens when $w_c=0$?
- 592: What happens for negative $w_c$?
- 662: Why projective equivalence matters
- 676: Affine transformations versus projective transformations
- 700: Geometric significance of homogeneous $w=0$
- 732: Why $w_c=-z_v$ is natural
- 764: $w$ is not always depth
- 786: Perspective-correct interpolation preview
- 830: Derive the vertical scale
- 868: Derive the horizontal scale
- 900: Choose homogeneous $w$
- 920: Derive the depth row
- 1022: Assemble the matrix · V11.03.26
- 1056: Verify the near plane
- 1088: Verify the far plane
- 1120: Numerical matrix example
- 1178: Depth is a numerical design choice
- 1206: Field of view controls perspective scale
- 1222: Numerical comparison
- 1262: Units: degrees versus radians
- 1334: Numerical example · V11.04.05
- 1356: Why FOV angles are not proportional to aspect ratio
- 1402: Horizontal versus vertical FOV locking
- 1416: Example · V11.04.08
- 1444: Pixel aspect ratio
- 1478: What does the near plane actually do?
- 1486: Near clipping is not the same as ray-intersection epsilon
- 1502: Why a very small near plane can be dangerous
- 1540: What the far plane controls
- 1554: Numerical example · V11.04.14
- 1576: Can the far plane be infinite?
- 1638: Why depth precision concentrates near the camera
- 1664: Numerical depth mapping
- 1700: Approximate physical depth resolution
- 1728: A 24-bit fixed-point example
- 1758: How the near plane affects precision
- 1790: Fixed-point versus floating-point depth
- 1811: A basic example
- 1833: Other causes that resemble Z-fighting
- 1843: Remedies
- 1879: Deriving reversed-Z
- 1981: Why floating-point reversed-Z works
- 1995: Required pipeline changes
- 2017: Hierarchical Z interpretation
- 2029: Is reversed-Z equally useful for integer depth?
- 2050: Conventional zero-to-one infinite perspective
- 2098: Reversed-Z infinite perspective
- 2154: Recover positive view depth
- 2180: Numerical example · V11.04.33
- 2238: Infinite far does not eliminate every depth issue
- 2253: Figure C11-1 — Depth mapping and floating-point spacing work together
- 2263: A projected point still needs a raster convention
- 2291: Right-handed projection
- 2311: Conventional left-handed perspective
- 2391: Handedness does not determine NDC depth range
- 2425: Handedness and face orientation
- 2459: Derive the right-handed OpenGL perspective depth row
- 2539: Depth-buffer mapping
- 2553: Modern OpenGL clip control
- 2578: Reversed-Z in OpenGL
- 2605: What Vulkan does not prescribe
- 2615: Viewport depth transform
- 2641: Negative viewport height
- 2668: Winding and rasterization
- 2690: Metal projection matrices
- 2709: Metal matrix memory conventions
- 2729: Metal depth states
- 2750: Maveb's existing projection
- 2792: The mathematical NDC-to-raster choice
- 2830: Vulkan's positive-height viewport
- 2868: Alternative: negative Vulkan viewport height
- 2874: OpenGL window origin
- 2882: Metal viewport interpretation
- 2890: Texture coordinates are yet another layer
- 2922: Deriving X mapping
- 2974: Deriving Y mapping
- 3026: Viewport offsets
- 3066: Viewport depth mapping
- 3084: Viewport versus scissor
- 3132: Pixel-centre coordinates in NDC
- 3186: Example: top-left pixel at 1920×1080
- 3220: A subtle result for even-resolution images
- 3264: Picking and ray generation
- 3302: Direct3D 9 pixel centres
- 3314: Direct3D 10 and later convention
- 3325: Why blindly adding half a pixel is wrong
- 3351: Texture texel centres
- 3381: Fullscreen triangles and post-processing
- 3393: Historical compatibility

## 34 - Robust Predicates and Camera Sensitivity.md

- Git blob: `c75a9727e22b771187f941ddfebe0042bb8ee9ba`
- Body routing extent: lines 14–2786; 62,402 characters in retrieved file.
- Declared master span: Derived from Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md; source lines 84339–87111; source-span SHA256 f824915709a11c867778d38638510a33752e0269c1d2d5233cf5caca04f05d7c.

| Derived lines | Destination and responsibility |
|---|---|
| 14–69 | 05 — robust predicates and constructions |
| 70–1477 | 14, 17 — frustum geometry, plane extraction and conservative culling |
| 1478–2135 | 03, 16, 18 — interpolation, reciprocal w, barycentrics, edge functions |
| 2136–2786 | 13, 26, 32 — intrinsics and uncertainty |

Subsection navigation (every heading in the source body):

- 14: A sign can matter more than a coordinate: robust predicates
- 30: Filtering before exact work
- 44: Predicates and constructions have different contracts
- 50: Worked scale check
- 60: Where this belongs in a renderer or vision system
- 70: The viewing volume is six half-spaces
- 88: Derive the geometry from field of view
- 142: Numerical example · V11.06.03
- 206: Why a frustum is a convex volume
- 220: What frustum culling can and cannot prove
- 274: Worked construction
- 324: Transform corners into world space
- 342: Frustum corners from inverse view-projection
- 384: Reversed-Z changes corner assignments
- 402: Infinite-far geometry
- 446: Signed distance
- 472: Choose inward-facing normals
- 486: Derive the near and far planes
- 544: Derive the side planes
- 614: Normalize the side planes
- 684: Derive the left plane
- 710: Right plane
- 730: Bottom and top planes
- 756: Near and far for zero-to-one clip depth
- 796: OpenGL-style negative-one-to-one depth
- 818: Summary extraction table
- 840: Normalize and validate
- 877: Model-space frustum planes
- 935: Derive the three cases
- 965: Culling against all planes
- 992: Numerical example · V11.06.26
- 1058: Why this test is conservative
- 1070: Transforming a local sphere into world space
- 1140: Derive the projected radius
- 1180: Culling condition
- 1235: Numerical example · V11.06.31
- 1305: Why AABB tests can outperform sphere tests
- 1333: A robust culling pipeline
- 1363: Why ordinary near clipping is insufficient
- 1377: Plane transformation between spaces
- 1425: Clipping as a half-space inequality
- 1447: Oblique matrix construction is convention-dependent
- 1457: Practical safeguards
- 1468: Figure C11-2 — Culling is a conservative half-space test
- 1478: Raster interpolation remembers homogeneous depth
- 1504: A simple segment example
- 1542: Derive the corrected midpoint
- 1590: What visual error does naive interpolation cause?
- 1602: Depth is a special case
- 1663: Homogeneous projected vertices
- 1715: Solve for the original weights
- 1763: Recover the attribute
- 1787: Numerical triangle example
- 1839: Derive reciprocal-depth interpolation
- 1869: Why this is a rational expression
- 1885: Important implementation choices
- 1933: Derive using two edge vectors
- 1969: Derive the determinant form
- 2009: Numerical example · V11.10.15
- 2079: Point-in-triangle testing
- 2091: Triangle degeneracy
- 2107: Edge functions
- 2126: Figure C11-3 — Attributes follow reciprocal clip w
- 2136: Intrinsics map optical rays to pixels
- 2155: Start from the physical pinhole model
- 2187: From normalized image coordinates to pixels
- 2231: Intrinsics are not just sensor metadata
- 2280: Focal length versus field of view
- 2312: Numerical example · V11.11.06
- 2340: Why focal length controls perspective framing but not perspective existence
- 2364: Why it may be off-centre
- 2392: Principal-point offset changes ray directions
- 2404: Numerical example · V11.11.10
- 2462: Sensor dimensions
- 2484: Numerical example · V11.11.12
- 2526: How intrinsics change when resizing an image
- 2570: Cropping
- 2598: From camera coordinates to homogeneous pixels
- 2634: Numerical example · V11.11.16
- 2690: Inverse intrinsics
- 2738: Why $K$ is not the same as a GPU projection matrix
- 2752: derivative means local image sensitivity
- 2772: covariance carries both size and direction of uncertainty

## 35 - Camera Extrinsics and Projection Geometry.md

- Git blob: `ae269463e966c7c8e19dfa18954d2a4552438833`
- Body routing extent: lines 14–3068; 79,026 characters in retrieved file.
- Declared master span: Derived from Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md; source lines 87112–90166; source-span SHA256 d05c86caf710d5dae5ba53d28b3e1e882519751530a0c533621a24830008f64c.

| Derived lines | Destination and responsibility |
|---|---|
| 14–520 | 12–13, 26, 29 — extrinsics, projection and observability |
| 521–1444 | 23 — distortion and inverse mapping |
| 1445–2097 | 13–15, 21 — depth recovery and picking |
| 2098–2660 | 15; Book 07 — stereo, off-axis views, XR |
| 2661–3068 | 23; Books 04, 07 — spherical sampling, fisheye and environment mapping |

Subsection navigation (every heading in the source body):

- 14: Extrinsics place the optical camera in the world
- 36: Camera centre versus extrinsic translation
- 66: Numerical example · V11.12.03
- 108: Camera-to-world versus world-to-camera
- 172: The camera projection matrix in computer vision
- 194: Example · V11.12.06
- 256: Coordinate convention mismatch: graphics versus CV
- 316: Complete projection equation
- 366: Why a single image cannot determine unique 3D depth
- 406: Projective scale ambiguity
- 424: Reprojection error
- 456: Projection Jacobian
- 511: Figure C11-8 — Residual count does not guarantee observable motion
- 521: distortion is a forward map; undistortion solves its inverse
- 528: Real lenses bend the pinhole image
- 544: Two different coordinate pairs
- 590: Why distortion must be modeled separately from $K$
- 602: Different lens families require different models
- 617: Distortion and calibration residuals
- 659: Why powers of $r^2$ appear
- 679: Barrel distortion
- 709: Pincushion distortion
- 731: Numerical example · V11.13.09
- 797: Why radial distortion can become non-invertible
- 840: Rational radial distortion
- 880: Full radial-plus-tangential model
- 912: Numerical example with tangential distortion only
- 1002: Units and interpretation
- 1022: Complete projection pipeline with distortion
- 1080: Convert observed pixels to distorted normalized coordinates
- 1110: Fixed-point inversion
- 1154: Newton inversion
- 1194: Derive the distortion Jacobian
- 1310: Solve the Newton system
- 1366: What can go wrong?
- 1380: Undistorting an image is not just undistorting its corner points
- 1399: Distortion and reprojection uncertainty
- 1435: Figure C11-6 — Distortion is a map with a domain, not just a coefficient list
- 1445: Recover geometry from depth and construct screen-space rays
- 1464: Derive the geometric relationship
- 1480: Numerical example · V11.14.03
- 1500: Converting a range image to axial depth
- 1529: Start from the perspective matrix
- 1555: Conventional finite zero-to-one depth
- 1583: Finite reversed-Z
- 1595: Infinite reversed-Z
- 1611: Traditional OpenGL NDC depth
- 1631: A generalized endpoint derivation
- 1659: Numerical example · V11.14.11
- 1689: Numerical stability matters when linearizing
- 1714: Method A: reconstruct from NDC and FOV
- 1750: Numerical example · V11.14.14
- 1780: Method B: inverse projection
- 1806: Why this works
- 1822: Important implementation details
- 1851: A concrete worked example
- 1877: Why large-world systems need special care
- 1891: Why inverse VP sometimes fails in practice
- 1906: Perspective camera: derive the ray from NDC
- 1926: Pixel centre to NDC
- 1940: Example: 90-degree square camera
- 1970: Orthographic cameras are different
- 1988: Ray generation through inverse projection
- 2025: Worked viewport example
- 2057: The rest of the picking pipeline
- 2071: Picking from an object-ID buffer
- 2081: High-DPI displays
- 2098: stereo turns a calibrated angle into a distance
- 2107: Build projection and stereo geometry around real camera baselines
- 2118: Derive the matrix
- 2174: Worked example · V11.15.03
- 2208: What off-axis projection is not
- 2241: Derive the two image coordinates
- 2277: Recover depth
- 2287: Numerical example · V11.15.07
- 2307: Why stereo depth accuracy worsens at distance
- 2331: Depth-uncertainty example
- 2353: Rectification and epipolar constraints
- 2384: Why IPD matters
- 2400: Eye centres, lens centres, and display centres are different
- 2416: Practical XR architecture
- 2436: Why asymmetry appears
- 2448: Derive shifted near-plane bounds
- 2482: The zero-parallax plane
- 2490: Why toe-in stereo is problematic
- 2500: OpenXR angular frusta
- 2543: Derive translational parallax
- 2579: Numerical example · V11.15.20
- 2613: Relative parallax
- 2621: Rotation versus translation
- 2639: Parallax is not identical to perceived depth
- 2651: Figure C11-4 — Stereo turns pixel uncertainty into depth uncertainty
- 2661: Represent directions over the full sphere
- 2674: Construct six camera directions
- 2684: Derive a cubemap direction mapping
- 2721: Why the mapping is not uniform in solid angle
- 2735: Cubemaps and real environment reconstruction
- 2745: Seam debugging
- 2766: Define spherical coordinates
- 2786: Convert image UV to angles
- 2816: Inverse mapping
- 2834: Equirectangular is not equal-area
- 2864: Importance sampling an HDR panorama
- 2882: Why this matters to Maveb
- 2909: Four common fisheye mappings
- 2928: Derive the equidistant mapping
- 2944: Derive the equisolid-angle form
- 2976: Numerical comparison · V11.16.16
- 3000: Generate a fisheye ray from a pixel
- 3036: Why fisheye matters for XR and capture
- 3059: Figure C11-7 — Image coordinates need a sampling-measure conversion

## 36 - Exposure, Physical Optics, and Depth of Field.md

- Git blob: `1391fdcc5adbf5c320ff5bde278a76d912189564`
- Body routing extent: lines 14–2883; 106,031 characters in retrieved file.
- Declared master span: Derived from Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md; source lines 90167–93036; source-span SHA256 85da0b62a78b033757909481b5fd79089a16a18ac238ea0a3caf1745b6ce438f.

| Derived lines | Destination and responsibility |
|---|---|
| 14–1248 | 22, 24, 25; Book 04 — exposure, lens, aperture, time integration |
| 1249–1981 | 20; Books 05–06 — temporal jitter, history, motion |
| 1982–2542 | 04, 05, 12–14, 31; Book 06 — large-world precision |
| 2543–2673 | 26–29, 32 — calibration observability, gauge and multiview |
| 2674–2730 | 20, 22; Book 04 — ray footprint and measure |
| 2731–2883 | 26–29; Book 07 — photometric inference, flow and bundle adjustment |

Subsection navigation (every heading in the source body):

- 14: Model exposure, physical optics, and depth of field
- 23: Exposure versus scene radiance
- 39: Exposure stops
- 63: Photographic exposure value
- 85: Derive the EV relationship
- 113: Numerical example · V11.17.06
- 141: Exposure and tone mapping are not the same
- 157: Auto-exposure
- 190: F-number is inversely related to aperture size
- 210: Derive the aperture area
- 232: The f-stop sequence
- 254: Aperture affects more than brightness
- 276: Sampling a disk uniformly
- 321: Mathematical image formation over time
- 341: Motion blur as a temporal integral
- 363: A moving-point example
- 393: Global shutter versus rolling shutter
- 411: Shutter angle
- 429: A critical ray-tracing implementation detail
- 446: Simple exposure model
- 458: Numerical example · V11.17.21
- 476: ISO and noise
- 500: Why physically based renderers must distinguish exposure from gain
- 510: EV100 example
- 541: The thin-lens equation
- 568: Derive the CoC diameter
- 600: Numerical example · V11.17.27
- 634: CoC in pixels
- 658: Signed CoC
- 672: CoC is not a perfect physical blur disk
- 697: Depth of field depends on the output criterion
- 707: Derive the hyperfocal distance
- 743: Numerical hyperfocal example
- 773: Derive near and far DoF limits
- 867: What happens beyond the hyperfocal focus distance?
- 875: What changes DoF?
- 883: Depth of field as a rendering integral
- 914: Geometry of the camera
- 940: Step 1: Construct the pinhole viewing direction
- 956: Step 2: Intersect the pinhole ray with the focus plane
- 986: Step 3: Sample the aperture
- 1006: Step 4: Construct the new ray
- 1026: Why all aperture rays agree at the focus plane
- 1044: A simple numerical example
- 1088: Step 5: Transform lens rays into world space
- 1120: C++23 implementation
- 1183: Pinhole limit
- 1201: Focal length versus focus distance
- 1213: Lens and shutter sampling together
- 1239: Figure C11-5 — Focus distance, focal length and aperture are different controls
- 1249: A camera image changes over time
- 1281: Why different samples reveal different information
- 1299: A simple two-frame jitter example
- 1331: Jitter sequences
- 1359: Jitter amplitude
- 1371: The geometry of temporal error
- 1434: Matrix-row derivation
- 1482: Convert pixel jitter to NDC jitter
- 1520: Numerical example · V11.07.09
- 1560: How this changes a conventional perspective matrix
- 1592: Jittered versus unjittered projection
- 1668: Deriving reprojection for a static world point
- 1706: Camera-only reprojection from current clip space
- 1730: Previous model matrices
- 1758: Deformation is another case
- 1778: Camera cuts and history validity
- 1810: Derive camera-translation screen motion
- 1884: NDC motion versus pixel motion
- 1914: Jitter contribution to motion vectors
- 1948: Translation, rotation, and parallax
- 1964: Motion vectors are not optical flow
- 1982: Large worlds need local arithmetic
- 2022: Bad order of operations
- 2039: Better order
- 2055: Numerical demonstration
- 2101: Object-relative vertices
- 2135: Why camera-relative rendering matters for temporal history
- 2155: Derive float spacing
- 2213: Consequences in a graphics engine
- 2223: Difference between absolute and relative accuracy
- 2247: The limitation of merely switching matrix multiplication to double
- 2308: Numerical example · V11.08.11
- 2338: What must be rebased?
- 2348: Spatial acceleration structures
- 2358: Temporal history problem
- 2394: Camera-relative rendering versus origin rebasing
- 2412: A double-precision rigid view transform
- 2463: Why rotation also matters
- 2507: Precision must be end-to-end
- 2519: Repository connection: Maveb camera paths
- 2543: calibration needs observable parameters and a fixed reference
- 2552: Calibration needs informative observations and a fixed gauge
- 2570: Gauge freedom: the whole scene can move without changing its image
- 2576: Planar calibration and the danger of repetitive views
- 2582: Small worked observability example
- 2592: Design calibration as an experiment
- 2598: The solver does not erase ambiguity
- 2612: Calibrated two-view geometry: direction before distance
- 2630: Gauge freedom, singular values, and choosing a physical pose
- 2644: Planes change the model
- 2658: Why three views contribute information beyond pairs
- 2674: A camera ray carries a pixel footprint
- 2687: Why this prevents shimmer
- 2693: Sample spacing is part of the derivative
- 2697: The camera's probability measure must match its samples
- 2703: Worked perspective example
- 2721: Practical checks and limitations
- 2731: Four deeper links from the TUM lecture review
- 2735: Photometric identifiability: a direct method needs a brightness model
- 2761: Derivation: which pose changes are visible to a direct residual?
- 2783: A variational optical-flow model, derived from the brightness equation
- 2801: Worked Euler–Lagrange derivation
- 2827: From a six-parameter pose update to a bundle-adjustment Jacobian
- 2861: Pairwise epipolar agreement does not guarantee one shared 3D structure
- 2873: A compact evidence-to-claim map

## 37 - Camera Contracts and Transform-Chain Debugging.md

- Git blob: `957f1b0ddd3c620cf28e5289b606687c8f32d71c`
- Body routing extent: lines 14–2767; 102,521 characters in retrieved file.
- Declared master span: Derived from Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md; source lines 93037–95790; source-span SHA256 ff9fb23aab9ebc413a2ce9ee1acf22cb01bdfc4a2c0af26d14b3f670299495fb.

| Derived lines | Destination and responsibility |
|---|---|
| 14–811 | 13–14, 21, 31–32 — semantic camera contracts and reference checks |
| 812–1645 | 13–15, 21, 31–32 — complete forward trace, off-axis and implementation |
| 1646–2208 | 13, 17, 20, 23, 26, 31–32 — calibrated uncertainty and debugging |
| 2209–2301 | 31–32 — runnable camera oracle |
| 2302–2767 | 04–05, 31–32; Books 01–02, 06 — numeric contracts, storage, SIMD |

Subsection navigation (every heading in the source body):

- 14: Make camera conventions explicit and debug the full transform chain
- 31: Why a generic `Camera` class can become unsafe
- 45: Use semantic types
- 78: Separate the camera pose
- 95: Separate projection specifications
- 135: Define the viewport independently
- 156: Define optics independently
- 175: Define temporal camera state
- 201: Frame-state update ordering
- 240: A camera API should expose operations, not just matrices
- 268: Make coordinate-space mistakes difficult to compile
- 311: Camera invariants
- 347: Cross-API matrix upload
- 363: Reversed-Z must be implemented as a complete pipeline contract
- 385: Physical cameras and rendering APIs are different layers
- 406: The reference scene
- 444: Step 1: Local to world
- 458: Step 2: World to view
- 476: Step 3: View to homogeneous clip
- 518: Step 4: Perspective division
- 542: Step 5: Viewport mapping
- 570: Step 6: Reverse the pipeline
- 598: Step 7: Compare CPU and GPU
- 612: A systematic camera-debugging matrix
- 632: Instrument the camera instead of guessing
- 678: Standalone camera-geometry reference laboratory (source-reported)
- 697: Numerical results reported by the supplied source
- 722: Repository observations reported by the supplied source
- 729: KairoRayTracer — currently a pinhole camera
- 739: KairoRenderer — conventional depth and viewport-specific Y handling
- 749: Maveb — reversed-Z
- 763: Cross-project camera consistency
- 797: A consistent depth-of-field verification example
- 812: A camera model must close the loop
- 826: Trace, implementation, repository audit, and derivation bridge
- 828: Complete worked example: from a mesh vertex to a framebuffer pixel
- 876: Local to world
- 896: World to view
- 930: View to clip
- 984: Perspective divide
- 1014: NDC to raster
- 1069: Additional derivation: an off-centre perspective projection
- 1169: C++23 implementation and verification
- 1193: 45.1 The most important implementation: projection conventions as explicit parameters
- 1281: 45.2 Infinite-far generalization
- 1321: 45.3 Verified program output
- 1365: Source-reported repository observations: KairoMath, KairoRenderer, KairoRayTracer, and Maveb
- 1369: KairoMath: matrix and projection conventions
- 1401: Improvement: validation
- 1413: Improvement: explicit depth variants
- 1432: KairoRenderer: camera and viewport
- 1444: Improvement: look-at degeneracy
- 1466: Improvement: framebuffer extent
- 1488: KairoRenderer: Vulkan viewport target
- 1508: KairoRayTracer: camera-ray basis
- 1536: Maveb: reversed-Z Metal camera
- 1554: Mastery verification: what you should now be able to derive
- 1592: Final formula sheet
- 1646: Calibrated projection, uncertainty, reference implementation, and repository evidence
- 1648: End-to-end example: calibrated projection, optical distortion, and image uncertainty
- 1670: 36.1 World to graphics view
- 1688: 36.2 Graphics view to CV optical frame
- 1712: 36.3 Apply intrinsics
- 1752: 36.4 Apply simple radial distortion
- 1818: 36.5 Derive the image-space uncertainty
- 1926: Verified C++23 implementation
- 1948: 37.1 The numerical results
- 1972: 37.2 C++23: frustum plane extraction
- 2002: 37.3 C++23: stable temporal jitter
- 2035: 37.4 C++23: Newton undistortion
- 2068: What the actual repositories establish
- 2072: KairoRenderer — camera state
- 2103: Maveb — camera paths
- 2117: Maveb — calibration loading
- 2139: Vulkax — observation-space pinhole camera
- 2175: Research bridge: why these two modules matter
- 2209: A runnable camera oracle with declared conventions
- 2302: The mathematics becomes a reliable implementation
- 2307: Three stored numbers are not the complete contract
- 2313: Initialization and access should be unsurprising
- 2323: Operators should preserve the mathematical question
- 2331: Choose the component type separately from the geometric role
- 2345: Storage and computation can choose different layouts
- 2355: SIMD has two different opportunities
- 2375: A GPU buffer is a separate agreement
- 2387: A reliability checklist for moving from symbols to images
- 2396: Reliability begins at the boundary of the calculation
- 2400: Ask what “close” means before choosing a tolerance
- 2417: Locate the first invalid operation
- 2432: Large world coordinates can lose a small movement before geometry starts
- 2442: Types can remember the promises that three components forget
- 2475: A complete scalar laboratory makes the story executable
- 2481: Reference implementation and regression driver
- 2743: What each layer guarantees, and what it does not

## 38 - Reconstruct the Complete Geometry Pipeline.md

- Git blob: `94e1114642639aef5914bc4c53952bb61d78f39b`
- Body routing extent: lines 14–1655; 127,547 characters in retrieved file.
- Declared master span: Derived from Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md; source lines 95791–97432; source-span SHA256 6248bbc6f44946b2e0faa9869bf7ac6e9ebc30a7945ed4cc75ec8c9ecd4db96e.

| Derived lines | Destination and responsibility |
|---|---|
| 14–593 | 31–32; Books 01–02 — reconstruction desk, matrices and layout |
| 594–651 | 31–32; Book 02 — implementation review |
| 652–931 | 26–30; Book 02 — SVD, factorization validation and solver architecture |
| 932–1220 | 29; Book 02 — sparse systems and solver selection |
| 1221–1587 | 32; Books 01–02 — numerical and probability reference laboratories |
| 1588–1655 | 31–32; Books 05–06 — mastery ladder and systems integration |

Subsection navigation (every heading in the source body):

- 14: Teach the complete journey, then break one assumption at a time
- 33: A reconstruction desk for active teaching
- 56: The mathematical table needs an explicit storage agreement
- 62: Own actual arrays rather than inventing array bounds between members
- 107: Figure M7 - A submatrix keeps the parent's physical stride
- 126: Runtime views borrow a layout as well as a lifetime
- 142: SIMD and loop order change the execution schedule, not the matrix question
- 178: A complete scalar laboratory makes the contracts observable
- 186: Complete runnable matrix and orientation reference
- 584: Read the assertions as experiments
- 594: Teach the journey by changing one promise
- 604: Where transforms lead next
- 613: Implementation kernels and shape reasoning
- 639: An implementation review checklist grounded in the map
- 652: Householder reductions turn a dense SVD into a bidiagonal problem
- 667: Jacobi SVD removes column coupling pair by pair
- 694: Scale-aware convergence and output conventions make results interpretable
- 709: Independent reconstruction and orthogonality checks test the SVD invariants
- 715: Figure L8 - SVD validation needs independent checks
- 739: A matrix object must describe shape, ownership, and workspace
- 753: Dynamic matrix storage makes factorization dimensions and lifetimes explicit
- 845: Zero singular directions require deliberate orthonormal completion
- 856: Matrix views need checked address arithmetic as well as a stride
- 867: Factorization is also a memory and lifetime decision
- 884: Workspace allocation is part of the solver contract
- 932: Sparse problems change the economics of solving
- 940: Figure L10 - Direct and iterative methods occupy different workloads
- 958: Sparse storage changes the cost model
- 981: Direct and iterative solvers suit different workloads
- 1012: Choose a factorization from structure and the requested answer
- 1034: Board workbench: follow the numbers before trusting the name
- 1036: One elimination step preserves the complete equation
- 1052: Cholesky exposes an energy structure
- 1064: QR fits noisy samples without changing the residual measure
- 1068: Power iteration needs a visible starting component
- 1072: An SVD separates input directions from output directions
- 1083: Pseudoinversion states what to do with an impossible target
- 1087: PCA tells geometry only after centering and choosing units
- 1091: Orthogonal factors support clearer error statements
- 1099: An SVD API begins with shapes and guarantees
- 1146: Numerical policies must share one rank decision
- 1159: A call lifecycle makes ownership reviewable
- 1170: Validate reconstruction, orthogonality, and triplets separately
- 1191: The library boundary is part of numerical correctness
- 1201: Close the journey with an algorithm-selection conversation
- 1221: Reconstructed C++23 laboratory
- 1225: Standalone C++23 probability and statistics laboratory
- 1483: A runnable laboratory for numerical contracts
- 1588: A mastery ladder for the whole foundations journey
- 1594: Rung 1 — Decode a bounded asset
- 1604: Rung 2 — Give the panel a stable identity and a safe borrow
- 1616: Rung 3 — Publish asynchronous work without racing ownership
- 1626: Rung 4 — Reconstruct a measured panel position
- 1648: Rung 5 — Integrate and make a bounded claim

## Coverage boundary

627 headings across 18928 routed body lines. Book 03 destinations are the reader's chapter numbers. Book 01/02/04/05/06/07 destinations are scope boundaries, not permission to remove or alter source; review original passages and current destination before composition. Remaining: the Graphics Vision and XR sources, continuous-master byte reconciliation, equation-by-equation review, original figure/code integration and full editorial acceptance.
