# Pass 35 — Graphics, Vision and XR source-unit routing

Pinned derived source revision `0afe1daf1efb5b1952b78a6476b3a37edde912cb`. The fourteen primary files named in the source ledger were retrieved in full through the repository connector. The inventory is generated from Markdown headings and declared source markers; it is not a complete line-by-line technical audit. Routes are proposed review destinations, including explicit later-book boundaries, not proof of integration. Master SHA-256 spans are reported as declared, not recomputed. Lines below refer to derived files, and include any source markers in the body.

## 01 - From Worlds to Images and Inference.md

Git blob `c727c2f278bd852fc194dabba9ab77fa30940b37`; 143494 characters; 2203 routed lines. Derived from Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography.md; source lines 1–2203; source-span SHA256 4e96f00f81fcedaba2f588a213edbd5f40d03c1303f3520719c1711b56329822.

| Derived line span | Destination and review responsibility |
|---|---|
| 14–164 | 01, 31–32: imaging model, project ladder, evidence |
| 165–478 | Books 04, 06: transport, ray tracing, hybrid execution; inspect geometry portions |
| 479–1500 | 01–21; Books 04–06: graphics lecture mechanisms, mixed scope |
| 1501–1548 | 01, 31–32: learning scope, task and evidence |
| 1549–1926 | 01–06, 11–12: sampled image, vectors and frames |
| 1927–2216 | 08; Book 05: curves, interpolation and animation |

Heading inventory (mechanical, including headings in fenced examples if present):

- 14: Worlds, Images and Inference
- 16: The question that holds the whole story together
- 61: How to use this volume without losing the thread
- 83: A project ladder that turns the image pipeline into evidence
- 96: Worked checkpoints for the six-stage route
- 149: A scene description becomes an image
- 151: From a triangle to a measured image
- 158: Graphics is a chain of representations
- 165: Ray tracing becomes difficult at the boundaries: precision, footprints, variance, reuse and hybrid execution
- 177: A secondary ray should not immediately hit the surface that spawned it
- 208: A ray query includes an interval
- 230: A mathematical ray has zero width; an image sample does not
- 284: Importance sampling changes variance only if the weight stays correct
- 314: Clamping and regularization trade bias for variance
- 344: Denoising is inference from a noisy measurement
- 373: Temporal reuse changes the statistics
- 391: Hybrid rendering is a deliberate decomposition, not an embarrassing halfway state
- 417: Ray tracing performance is scheduling and memory behavior too
- 435: How the MIT lectures and Ray Tracing Gems fit together
- 479: MIT graphics gap closure: the renderer is one continuous numerical model from shape to motion to light to pixels
- 497: Parametric curves are functions whose derivatives are geometric information
- 638: de Casteljau is both an evaluator and a geometric proof
- 692: Continuity between curve segments has levels
- 722: Hierarchical modeling is repeated change of basis
- 783: A scene graph and a transform-update graph are related but not identical
- 823: Skinning is weighted coordinate transformation
- 863: Normals cannot always use the same matrix as positions
- 905: Color values are not all in the same mathematical space
- 937: The rendering equation and BRDF are only meaningful under consistent units and spaces
- 966: Animation turns the world into an ODE problem
- 992: Explicit Euler
- 1008: Semi-implicit Euler
- 1030: Backward Euler
- 1048: Verlet-style position integration
- 1076: Time step is a numerical parameter, not merely frame duration
- 1107: A mass-spring system exposes stiffness
- 1140: Collision detection and collision response are different problems
- 1203: Restitution is not a generic bounce multiplier
- 1219: Friction is constrained tangential impulse
- 1250: Position correction and velocity correction solve different errors
- 1268: Sampling unifies rasterization and ray tracing
- 1289: Mipmapping is prefiltered integration over scale
- 1311: Rasterization is an analytic visibility shortcut plus sampling
- 1359: Depth is also a convention-sensitive quantity
- 1371: Shadow mapping is another sampling problem
- 1408: Image-based lighting turns the environment into an integral
- 1439: Output devices complete the numerical pipeline
- 1464: The MIT sequence is a dependency graph, not a list of disconnected lectures
- 1501: Algorithmic scope and debugging the image pipeline
- 1507: The first lecture's scope: algorithms behind pictures
- 1517: Learning path and conventions
- 1521: What an image can and cannot tell us
- 1525: Lecture 1’s demonstrations: vision is judged by the task it enables
- 1541: A photograph is a model of a view
- 1549: From sampled images to the mathematics that makes them controllable
- 1553: Mathematical language: points, vectors, and derivatives
- 1557: Points, vectors, bases, and linear maps
- 1583: Inner products and vector calculus
- 1597: Coordinate-free products and the meaning of second derivatives
- 1603: A compact glossary of recurring terms
- 1625: Additional geometric and rendering mechanisms from the graphics course
- 1627: Points and vectors obey different operations
- 1641: Coordinate frames, transformations, and pose
- 1645: Coordinate frames and affine transforms
- 1659: What different transforms preserve
- 1665: Rotation representations compared
- 1684: Coordinates make composition possible
- 1720: Figure U1 - Linear maps separate directions stretches and output placement
- 1726: Matrix composition as a worked graphics pipeline
- 1728: Matrix transforms build reusable models
- 1744: Why homogeneous coordinates are useful
- 1759: Rotation from a circle and angle addition
- 1769: Shear, reflection, and affine structure
- 1773: Composing operations: order and a numerical trace
- 1788: From one point to a hierarchy of objects
- 1794: Additional geometry notes for coordinate frames
- 1798: A fixed matrix separates parameter, basis, and geometry
- 1830: Coordinate frames make object motion composable
- 1871: Homogeneous coordinates turn perspective into a projection
- 1898: Rotation matrices, gimbal lock, and quaternions
- 1902: Quaternions: a compact way to compose orientation
- 1927: Curves as controllable functions
- 1931: Interpolating animation curves
- 1937: One cubic polynomial has several design languages
- 1949: Why endpoint data needs a cubic, and how the conversion works
- 2006: The blossom explains both control points and exact subdivision
- 2062: A curve is a controllable function, not a row of dots
- 2122: Choosing where to sample a curve
- 2128: Additional geometry notes for curves
- 2132: Bernstein polynomials turn affine combinations into curves
- 2156: Design curves: smooth without passing through every control point
- 2160: Chaikin corner cutting
- 2173: de Casteljau: evaluate the Bezier curve by repeated interpolation
- 2187: Joining curve segments and extending to surfaces
- 2193: Smooth curve and patch families

## 02 - Surfaces, Topology and Parameterization.md

Git blob `41a7b270a4ee80ec12cdd57c8421351565f25ec4`; 117744 characters; 915 routed lines. Derived from Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography.md; source lines 2204–3118; source-span SHA256 124ad7fa099f4d4b38f41d50bfa82f53d8cab0b2d962cd14b22be2cb1f69f270.

| Derived line span | Destination and review responsibility |
|---|---|
| 14–62 | 07–10: surfaces, boundaries, parameterization |
| 63–426 | 07–10, 30; Book 02: editing, sparse operators, singular-value distortion |
| 427–928 | 07, 10, 30; Book 02: chains, topology, persistence, discrete Laplacian, repair |

Heading inventory (mechanical, including headings in fenced examples if present):

- 14: Surfaces, patches, topology, and parameterization
- 18: Curves, surfaces, and local coordinates
- 24: Orientability, boundaries, and local neighborhoods
- 30: Parameterization distortions
- 34: Surfaces: parameter domains, patches, and meshes
- 63: Meshes, connectivity, and geometry processing
- 65: Reconstruction checkpoint — topology before surgery
- 76: Mesh topology and data structures
- 84: Geometry processing and mesh edits
- 98: Local editing and quality measures
- 104: A local connectivity structure makes edits explicit
- 116: A mesh processing failure as a query problem
- 120: Why a mesh can be treated like a signal
- 127: Differential coordinates preserve local shape while solving for new global positions
- 195: Parameterization distortion is a singular-value statement about the surface map
- 235: Signed distance as a geometric signal
- 239: Subdivision masks and quadric error become executable rules
- 241: Subdivision masks and quadric error are computable rules, not labels
- 243: Loop subdivision: old vertices and new edge points
- 283: Catmull–Clark: face, edge, and vertex masks
- 303: Quadric error metric: turn nearby planes into a collapse cost
- 339: Figure U2 - A half-edge separates face traversal from edge traversal
- 345: Additional geometry notes for surfaces and parameterization
- 347: Subdivision meshes and the topology they need
- 363: Catmull–Clark and Loop subdivision
- 377: As-rigid-as-possible local shape preservation
- 403: Field-aligned remeshing without a global parameterization
- 427: From a scanned triangle soup to a mesh we can trust
- 444: A mesh is a neighborhood rule as well as a drawing
- 460: The boundary of a boundary cancels
- 501: Euler characteristic is a structural checksum, not a shape score
- 541: Homology: ask which loops actually have a filling
- 554: Alpha complexes turn a union of balls into a finite topological object
- 604: Persistence asks which features survive scale changes
- 619: Persistence stability says small function perturbations cannot move the diagram arbitrarily far
- 656: Morse functions and Reeb graphs compress how level-set components split and merge
- 680: The discrete Laplacian turns adjacency into a motion rule
- 704: Nonnegative energy and nonnegative weights are different promises
- 755: A stable heat step can still create a negative sample
- 784: Complete single-triangle energy witness
- 870: Smoothing is a flow with side effects
- 878: Remeshing improves elements while preserving the intended surface
- 888: Simplification trades face count against approximation error
- 901: Surface repair and volumetric repair preserve different kinds of evidence
- 919: A repeatable mesh-processing checkpoint

## 03 - Lens Optics and Image Formation.md

Git blob `c10b45e9e8fd072c5a8595698a6a4de11c140b15`; 114951 characters; 546 routed lines. Derived from Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography.md; source lines 3119–3664; source-span SHA256 234e0e39434b3adbc828b221fd90c4ba777b0e9626abb705fe182486666fc470.

| Derived line span | Destination and review responsibility |
|---|---|
| 14–127 | 22–23: focus, aberrations and output tolerances |
| 128–242 | 24–25; Book 04: exposure, sensor, HDR |
| 243–466 | 13–15, 26–29: camera projection, calibration, orientation, panoramas |
| 467–513 | 07–10: mesh notes |
| 514–559 | 01, 20: image sampling |

Heading inventory (mechanical, including headings in fenced examples if present):

- 14: Lens optics, focus, and image formation
- 18: How a lens forms an image
- 36: Focus is determined by a tolerance and an output
- 44: Deriving the acceptable focus interval
- 103: Figure U6 - Principal rays explain the thin lens
- 115: The ideal pinhole and a real lens answer different questions
- 121: Aberrations, field falloff, and correction
- 128: Exposure, sensors, and dynamic range
- 132: From scene radiance to pixel brightness
- 148: Exposure: choosing what the sensor can collect
- 166: Sensors turn photons into measurements
- 200: Equivalent exposures preserve brightness but not the photograph
- 210: Choosing an HDR method by what moves
- 218: Stops, EV, and scene ratios
- 232: Meter readings are estimates, not scene knowledge
- 238: A blur threshold depends on the intended view
- 243: Cameras, projection, and calibration
- 247: Rotation and projective cameras
- 265: Perspective is linear before the divide
- 279: Perspective projection links 3-D velocity to image flow
- 309: Camera calibration from straight lines and vanishing points
- 333: Orthographic projection as an engineered measurement choice
- 341: Interior calibration from known scene points
- 359: Exterior orientation: where the camera stands and where it points
- 375: Panoramas: geometry before stitching
- 393: Field of view, image scale, and formats
- 405: From a camera view transform to viewport pixels
- 407: A 2D world window mapped to a screen viewport
- 428: The 3D view transform moves the world around a fixed camera
- 467: Additional geometry notes for meshes
- 471: From a camera frustum to the image-space cube
- 496: Clipping as signed distance and interpolation
- 514: Sampling continuous signals and image formation
- 518: Frequency, alias, and reconstruction
- 526: Raster sampling of a blurred signal
- 530: Sampling and aliasing: pixels measure areas, not mathematical points
- 538: Sampling and reconstruction
- 546: Figure U3 - Two signals can share the same samples
- 552: Pixel count, storage, and bandwidth
- 554: A raster is stored samples, not tiny squares

## 04 - Sampling, Filtering and Reconstruction.md

Git blob `20daeec94d157e754902725797c0d64091031eb7`; 127386 characters; 1859 routed lines. Derived from Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography.md; source lines 3665–5523; source-span SHA256 a5e8c6ab058880d462b98ce41d402c3c9d234b2758b0fab61efaabf5a5e4874b.

| Derived line span | Destination and review responsibility |
|---|---|
| 14–255 | 01, 20, 21; Book 06: frame buffers, sampling, convolution, integral images |
| 256–1774 | 24–25; Book 04: color, radiometric quantities, encoding and output |
| 1775–1872 | Book 04: light measurement, BRDF, illumination |

Heading inventory (mechanical, including headings in fenced examples if present):

- 14: Image sampling, filtering, and efficient reconstruction
- 18: Animate a pixel by respecting the buffer’s coordinates and lifetime
- 45: Treat a render-order fix as evidence about a frame schedule
- 62: Sampling, integral images, and the limits of a box average
- 76: Texture mapping is filtering over a changing footprint
- 82: Pixel footprints and texture derivatives
- 88: Antialiasing is controlled filtering before and after sampling
- 104: Resolution, blur, and display
- 110: Pyramids trade pixel count against aliasing and feature visibility
- 114: The processing pipeline reconstructs a usable image
- 136: Aligning repeated frames
- 142: Image processing: local rules over an image grid
- 160: Image processing changes pixels, neighborhoods, and exposure
- 182: Fast image operations: sampling, convolution, and integral images
- 194: Sparse differences make wide spline filters cheap
- 216: Worked example: four-corner rectangle sum
- 226: An integral image makes every rectangular sum constant-time
- 242: Derivatives and integrals are convolution operators
- 256: Color perception, encoding, and display
- 258: Make every color number carry a meaning
- 368: Radiant energy
- 386: Radiant flux
- 1655: Color as spectral measurement
- 1671: Additive and subtractive mixtures
- 1675: Chromaticity, gamut, and color conversion
- 1681: Color: device values are not perceived color
- 1689: Output devices close the loop
- 1697: Color is a three-number perceptual response to a spectrum
- 1711: Output devices turn samples into emitted or reflected light
- 1725: Color is a measurement and a convention
- 1741: From scene spectrum to display color
- 1751: Color matrices and nonlinear encodings
- 1764: Color photographs are a sequence of measurements and reconstructions
- 1770: Additive and subtractive mixing are different operations
- 1775: Light measurement and radiometry
- 1779: Radiometry and BRDF
- 1797: Measure, units, and conservation
- 1805: Directional, hemispherical, and area lights
- 1809: Light in a scene: source size, reflection, and shape
- 1823: Lighting for a photograph
- 1835: Read the shadows to estimate a light source
- 1843: Environment illumination and differential compositing
- 1871: Reflectance, BRDFs, and shading

## 13 - From View Space to Clip Space.md

Git blob `c99505f5c26da93b53ffb68c0ac8c38261eddd2a`; 61634 characters; 3452 routed lines. Derived from Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography.md; source lines 28351–31800; source-span SHA256 cfb093b9ae71f87be0582e488bddba421a963189da090b4c068283090e5435cd.

| Derived line span | Destination and review responsibility |
|---|---|
| 14–2560 | 13–19; Book 06: view-to-clip, depth, implementation passages |
| 2561–2597 | Book 06: M113/M114 API-specific material |
| 2598–3465 | 18–21; Book 06: stencil, scissor, attachments, multisampling |

Heading inventory (mechanical, including headings in fenced examples if present):

- 14: View → clip
- 2561: + M113 + M114
- 2598: Stencil, culling, scissor/viewports, attachments, load/store, MSAA

## 15 - Rasterization Foundations and Renderer Evidence.md

Git blob `4ff530f607fe3ae9cd72c9da09794fb3c74b84e1`; 73295 characters; 2032 routed lines. Derived from Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography.md; source lines 35331–37362; source-span SHA256 a78b04e715d3b81b09dbc46ac29d254d8f057bb0a3210dcc2c4a1812faa8adc1.

| Derived line span | Destination and review responsibility |
|---|---|
| 14–150 | 18–21, 32: coverage, depth, renderer verification |
| 151–2045 | Book 06: shader data, parallel GPU execution; retain source-reported observations |

Heading inventory (mechanical, including headings in fenced examples if present):

- 14: What Kairo specifically taught us in this pass
- 20: Mastery gate
- 26: Next — Pass 03
- 33: Triangle coverage and sampling
- 49: Barycentric coordinates, depth, and alpha
- 63: How a triangle reaches the framebuffer
- 73: A depth-buffer table trace
- 77: Coverage samples and a triangle rasterizer
- 87: Modeling a cube by hand
- 91: A cube drawing pipeline
- 97: Opaque sample update
- 111: A final numerical checkpoint: coverage, color, and accumulation
- 117: Rasterization and depth
- 125: Rasterization converts projected triangles into depth-tested fragments
- 139: Perspective changes the interpolation weights inside a rasterized triangle
- 151: From measured samples to parallel GPU execution
- 155: Make a shader record an explicit binary contract
- 165: Let many invocations cooperate without inventing a global clock
- 175: Scan turns local counts into nonoverlapping output regions
- 192: Load a partial matrix tile with two different bounds
- 1088: + M129
- 1109: GPU EXECUTION ARCHITECTURE

## 19 - Textures as Sampled Surface Signals.md

Git blob `8ae1f05287729ebb8141adb9d0b3a880861c8371`; 51330 characters; 2734 routed lines. Derived from Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography.md; source lines 46448–49181; source-span SHA256 7652991984c323a7432aee7bf45394b9029c86219ca693a6cea9960a1be03bec.

| Derived line span | Destination and review responsibility |
|---|---|
| 14–2702 | 10, 16, 20, 25; Books 04, 06: texture signals, wrapping, filtering, mip levels and API implementation |
| 2703–2747 | 16, 20: surface coordinates and interpolation |

Heading inventory (mechanical, including headings in fenced examples if present):

- 14: Textures and surface detail
- 16: Match a texture footprint to the surface signal
- 126: Repeat
- 140: Clamp-to-edge
- 155: Mirrored repeat
- 2703: Texture is a function over a surface
- 2717: Texture coordinates make one surface address many kinds of detail
- 2741: Figure U9 - Homogeneous depth changes interpolation weights

## 23 - Mesh Topology and Connectivity.md

Git blob `991dfa264100be10e1df3efc801966ae1d383da1`; 83618 characters; 2673 routed lines. Derived from Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography.md; source lines 57090–59762; source-span SHA256 527e67cd9bc0edb8249c4bdc2df4b7befbdafe310af89d9dd833bbcf50cac8c8.

| Derived line span | Destination and review responsibility |
|---|---|
| 14–645 | 07, 10: topology and connectivity |
| 646–942 | 07, 10: boundary geometry and adjacency |
| 943–1372 | 05, 07, 32: robustness contracts |
| 1373–1645 | 31–32; Books 01, 06: library engineering and source-reported evidence |
| 1646–1769 | 32; Book 07: research bridge and challenge |
| 1770–1965 | 06; Book 05: primitive query foundations |
| 1966–2686 | 06; Book 04: ray-plane and sphere queries |

Heading inventory (mechanical, including headings in fenced examples if present):

- 14: PART IV — M071: Mesh Topology and Connectivity
- 646: PART V — M072: Boundary Geometry and Adjacency
- 943: PART VI — Numerical Robustness and the Geometry Contract
- 1373: PART VII — Building a Production Geometry Library
- 1570: PART VIII — Reference Implementation and Repository Verification
- 1646: PART IX — Research Bridge
- 1674: PART X — Reconstruction Challenge
- 1732: Phase 12 completion status
- 1770: PART I — M073: Intersection and Collision Query Fundamentals
- 1966: PART II — Ray Intersections with Planes and Spheres

## 24 - Ray–Triangle Intersection.md

Git blob `b6bf1b44ec478c09621ba1d5af7a6b2523d80cc5`; 68744 characters; 3210 routed lines. Derived from Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography.md; source lines 59763–62972; source-span SHA256 3ffa6980cdf14c6007237c740d88381cfe177214e26ca62f815836135e6fde94.

| Derived line span | Destination and review responsibility |
|---|---|
| 14–462 | 06, 32; Book 04: triangle ray query and shared-edge robustness |
| 463–1330 | 06; Books 04–05: ray-box/capsule |
| 1331–2262 | Book 05: overlap and contacts |
| 2263–3223 | Book 05: separating axes |

Heading inventory (mechanical, including headings in fenced examples if present):

- 14: PART III — Ray-Triangle Intersection
- 463: PART IV — Ray-Box and Ray-Capsule Intersections
- 1331: PART V — M074: Primitive Overlap and Contact Geometry
- 2263: PART VI — The Separating Axis Theorem

## 27 - Mesh Queries, Classification and Clipping.md

Git blob `aca655f43e39cff8d7bcbe1c9a03f8e994f6602f`; 110478 characters; 3069 routed lines. Derived from Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography.md; source lines 68892–71960; source-span SHA256 e7a6dba6ac80dd93ac1fc24c080cf25f2d60526c6a27b90ac043ee0164bb3394.

| Derived line span | Destination and review responsibility |
|---|---|
| 14–1752 | 06–07, 17, 30; Book 05: mesh queries, containment, clipping and classification |
| 1753–3082 | 05, 32; Books 05–06: numerical policy, collision API, SIMD and fuzzing |

Heading inventory (mechanical, including headings in fenced examples if present):

- 14: Mesh Ray, Overlap, Containment, Classification and Clipping
- 1753: Numerical Robustness, Collision API Design, SIMD, Fuzzing and Library Engineering

## 30 - Mesh Operators, Boundaries and Diffusion.md

Git blob `39386dac8d324737d1cd0690406cb060803f975b`; 135694 characters; 855 routed lines. Derived from Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography.md; source lines 75458–76312; source-span SHA256 03e8e6cd9c8e929e53857de31d071f980160748a88bdd059caae657f3729b92b.

| Derived line span | Destination and review responsibility |
|---|---|
| 14–295 | 07, 30; Book 02: diffusion, mesh operators, boundaries and stability |
| 296–549 | Book 02: PDE discretization and solvers |
| 550–678 | 28; Book 07: motion and optical flow |
| 679–772 | 28; Book 07: edges, subpixel localization and voting |
| 773–868 | 28; Book 07: matching and recognition |

Heading inventory (mechanical, including headings in fenced examples if present):

- 14: Heat on a rod: why boundaries, modes, and mesh operators belong together
- 37: Boundary conditions choose the modes
- 92: Derivatives act on modes by multiplying their frequency
- 108: From a mesh Laplacian to a heat operator
- 142: Explicit steps have a mesh-dependent stability limit
- 180: A Green function is an impulse response
- 206: Sampling, truncation, and ringing
- 214: Manufactured fields make the solver testable
- 242: What the boundary means to the discrete solver
- 269: Spatial and temporal error have different signatures
- 296: PDEs, fields, and discrete solvers
- 300: PDE discretization and variational structure
- 314: The same field equation can describe very different behavior
- 332: Discretize the space with a representation that fits
- 358: Boundary data is part of the equation
- 372: Turn the discretized equations into a solver
- 392: Discrete field equations and their stability conditions
- 394: From a PDE to a sparse system and a stable update
- 398: Derive the five-point grid stencil
- 421: Poisson and Laplace equations become sparse linear systems
- 451: Explicit heat flow and its stability limit
- 487: The cotangent mesh Laplacian and what “positive” means
- 520: One spatial operator, different simulations
- 524: A wave needs two initial conditions and a different time update
- 550: Optical flow and motion from images
- 554: Brightness constancy is a measurement equation, not a universal law
- 580: Noise gain and the shape of a good measurement
- 584: The brightness equation has one constraint per pixel
- 604: Eigenvectors expose directional noise gain
- 622: B. Is the optical-flow estimate constrained?
- 626: Flow: why one bright edge cannot reveal full motion
- 632: A compact focus-of-expansion calculation
- 636: Lecture 3’s practical TTC estimator and its failure near impact
- 672: Figure U7 - Independent gradients resolve the aperture ambiguity
- 679: Edges, contours, and subpixel localization
- 683: Edges and blobs: image geometry without a full 3-D model
- 697: Worked blob example from a rectangular contour
- 703: Subpixel edge localization as a complete industrial pipeline
- 729: What defocus does to an edge, and where interpolation bias comes from
- 745: Hough voting turns weak local evidence into a global hypothesis
- 751: Orthogonal line fitting is the right geometry for edge fragments
- 773: Recognition, templates, and industrial matching
- 777: Industrial recognition: templates, transforms, and tolerances
- 807: From correlation to sparse, multidimensional pattern matching
- 835: The score is a design choice, and affine transforms act on normals
- 853: PatMax: continuous pose refinement after coarse localization
- 865: C. How should an edge alignment score behave?

## 32 - Pose Estimation and Robust Fitting.md

Git blob `138401d26904a38e03c89cea7dd1da9af213473c`; 133399 characters; 594 routed lines. Derived from Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography.md; source lines 77249–77842; source-span SHA256 2b26cc68a32efa9ed410fab065ef8ec5d22188d487162a50501d59794a738d97.

| Derived line span | Destination and review responsibility |
|---|---|
| 14–157 | 12, 26, 28–29: pose, rigid orientation and robust fitting |
| 158–254 | 15, 27, 29, 32: stereo, gauge, critical configurations and BA |
| 255–311 | Book 04/07: photometric stereo |
| 312–455 | Book 04/07: shape from shading and reflectance |
| 456–535 | 30; Book 07: normals and directional shape descriptors |
| 536–607 | Book 02/07: finite-field projection theory, not ordinary camera projection |

Heading inventory (mechanical, including headings in fenced examples if present):

- 14: Pose estimation, transforms, and robust fitting
- 18: A planar image warp is a constrained camera pose
- 30: Photogrammetry: orientation is a family of related unknowns
- 44: Rotations and the geometry of their parameter space
- 68: Absolute orientation in closed form and RANSAC
- 80: Making the quaternion fit stable, and knowing when it is ambiguous
- 96: Proper rotation from centered point clouds
- 110: RANSAC threshold and trial count are part of the model
- 114: CORDIC detail: why shift-add can replace a square root and arctangent
- 124: The explicit quaternion matrix makes absolute orientation computable
- 158: Stereo, relative orientation, and reconstruction
- 162: Stereo and relative orientation
- 184: Exterior orientation and bundle adjustment
- 194: Relative orientation: recover the two rays’ shared geometry
- 214: Critical surfaces: geometry can make stereo ill-conditioned
- 220: Why critical surfaces destroy relative orientation
- 226: D. How do scale and pose ambiguities enter reconstruction?
- 230: Stereo, essential geometry, and metric scale
- 236: Bundle adjustment objective and gauge
- 248: Figure U8 - Stereo disparity uses local image coordinates
- 255: Surface orientation from illumination
- 259: Recovering surface orientation from controlled illumination
- 285: A. Is a lighting setup informative enough?
- 296: Photometric stereo: uniqueness from albedo absorption
- 302: A second photometric-stereo check with orthogonal lights
- 306: Figure U4 - Lighting rank determines which normal components can be measured
- 312: Shape from shading and reflectance
- 316: Gradient space, reflectance maps, and the irradiance equation
- 322: Reflectance models matter: lunar soil and electron microscopy
- 344: Measuring a reflectance map without assuming Lambertian material
- 354: A reflectance function includes viewing direction
- 360: Shape from shading as a PDE, then a numerical surface
- 376: Why the Moon’s full phase can look flat, and what profiles still reveal
- 384: A real non-Lambertian example: scanning electron microscope shape cues
- 390: From a noisy needle field to one surface
- 416: Shape from one shading image when isophotes are straight
- 426: Characteristics: tracing the full nonlinear shading equation
- 444: Shape from shading: a local equation is not a global surface
- 456: Surface descriptions and Gaussian images
- 460: Shape descriptions, surface normals, and curvature distributions
- 470: Gaussian images: representing shape by normal direction
- 478: From surface area to a directional mass distribution
- 490: Corners, closed surfaces, and the zero-centroid condition
- 502: Solids of revolution and the torus counterexample
- 520: From directional masses to matching and sampling
- 528: Gaussian image refinements and the meaning of a directional histogram
- 536: Finite-field projections turn image size into collision counts
- 540: Projection as a collision problem
- 552: Finite fields: exact tests and arithmetic obstruction
- 558: Exact finite-field collision and orthogonality bounds
- 578: Fourier slice identity and the low-frequency local-constancy step
- 602: Figure U11 - Finite-grid projections expose collision counts

## 33 - Projection Scale and Incidence Geometry.md

Git blob `cca53c1b1d80e6c24c7977a6cf81fef0c6cdf008`; 156686 characters; 718 routed lines. Derived from Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography.md; source lines 77843–78560; source-span SHA256 f5532dbf3c06d22c654e798ec804f292df845606be733657b4f428786dcd6f44.

| Derived line span | Destination and review responsibility |
|---|---|
| 14–235 | Book 02/07: geometric measure, incidence and multiscale projection; not a camera matrix chapter |
| 236–562 | Book 02/07: additive structure and projection growth |
| 563–731 | Book 02/07: dynamics and rich tubes |

Heading inventory (mechanical, including headings in fenced examples if present):

- 14: Real projections add scale, spacing, and incidence geometry
- 16: The real setting adds scales
- 27: Sieve and incidence geometry
- 35: The first eight lectures: from exact collisions to geometric scale
- 58: What the Szemerédi–Trotter proof does not reveal (Lecture 9)
- 74: Sumsets, energy, and robust structure (Lectures 10–12)
- 137: What the Szemerédi–Trotter proof does not reveal
- 153: Why the Euclidean projection theorem needs scale regularity
- 171: Uniform trees make multiscale size usable
- 187: The large sieve: why rational samples separate
- 199: Szemerédi–Trotter partitioning and the cross-cell limitation
- 209: Bourgain’s real theorem: what uniformization actually preserves
- 219: Frostman measures, energy, and the projection integral
- 236: Additive structure explains repeated coincidences
- 238: Additive structure and robust projection growth
- 244: Sumsets, energy, and robust structure
- 307: From many additive coincidences to a genuinely structured rectangle
- 329: Why the Euclidean projection theorem needs scale regularity
- 347: Uniform trees make multiscale size usable
- 363: Robust expansion is what makes the real argument iterate
- 373: Finite groups: when a walk is stuck, look for an approximate subgroup
- 411: Homogeneous dynamics: shear, diagonal renormalization, and projection geometry
- 435: Lattice-orbit spreading: expanding a segment is not enough
- 449: Furstenberg sets: rich tubes, three regimes, and Beck’s theorem
- 473: AD regularity turns the rich-tube count into a submultiplicative quantity
- 485: The last scale-combination argument
- 505: Proof and normalization supplement: finite fields through real discretization
- 507: From many additive coincidences to a genuinely structured rectangle
- 529: Robust expansion is what makes the real argument iterate
- 539: Ruzsa, Plünnecke, Petridis, and BSG: explicit variables and losses
- 563: Dynamics connect projection growth to rich tubes
- 565: Dynamics and the sharp rich-tube theorem
- 576: Finite groups: when a walk is stuck, look for an approximate subgroup
- 614: Homogeneous dynamics: shear, diagonal renormalization, and projection geometry
- 638: Lattice-orbit spreading: expanding a segment is not enough
- 652: Furstenberg sets: rich tubes, three regimes, and Beck’s theorem
- 676: AD regularity turns the rich-tube count into a submultiplicative quantity
- 688: The last scale-combination argument
- 708: Finite-group mixing: norms, gap, and expansion
- 724: Homogeneous dynamics: hypotheses behind the lattice picture

## 41 - Per-Vertex and Per-Corner Mesh Attributes.md

Git blob `ca921d2747a6321fb9bebcde97edb086bb74edb7`; 43368 characters; 3258 routed lines. Derived from Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography.md; source lines 95949–99206; source-span SHA256 c8d8b33f6c039e15153ca041dae1dd9fdbdecdebf8e8bea1c5148b831d7a5ad9.

| Derived line span | Destination and review responsibility |
|---|---|
| 14–347 | 07, 10: mesh identity, adjacency, half-edges |
| 348–815 | 10, 16: normals, tangents, bounds and OBJ |
| 816–1176 | 10; Books 04–06: glTF bytes, attributes, nodes, materials and animation |
| 1177–1573 | 07–10, 30: simplification, subdivision, smoothing, parameterization |
| 1574–2260 | Book 05: skeletons and animation |
| 2261–2778 | Books 05–06: morphs, compression, animated bounds and complete trace |
| 2779–3253 | 31–32; Books 05–06: C++ model, reconstruction gates and source-reported status |
| 3254–3271 | Book 07: perceptual rendering and XR |

Heading inventory (mechanical, including headings in fenced examples if present):

- 14: Per-vertex vs per-corner
- 59: Index sharing
- 79: Topology vs geometry
- 107: Boundary edges
- 133: Manifold and non-manifold
- 159: Connected components
- 181: Face adjacency
- 213: Vertex adjacency
- 247: Half-edge
- 294: One-ring traversal
- 322: Current Kairo representation
- 348: NORMALS, TANGENTS, BOUNDS AND OBJ
- 352: Flat face normal
- 404: Smooth normals
- 425: Area-weighted normals
- 451: Angle-weighted normals
- 478: Tangent generation
- 553: Orthogonalize tangent
- 576: Handedness
- 613: Degenerate UVs
- 639: Bounds
- 679: OBJ
- 721: OBJ negative indices
- 763: Polygon triangulation
- 789: Tuple deduplication
- 816: glTF FROM BYTES TO PORTABLE SCENE
- 838: Buffers
- 850: bufferView
- 873: Accessor
- 919: Primitive attributes
- 941: Indices
- 963: Nodes
- 1010: Kairo's matrix conversion
- 1030: Materials
- 1054: Images/textures/samplers
- 1094: Animation structures
- 1133: Skins
- 1149: Morph targets
- 1177: SIMPLIFICATION, SUBDIVISION, SMOOTHING AND PARAMETERIZATION
- 1183: Edge collapse
- 1209: Quadric Error Metrics
- 1292: Solve optimal collapse position
- 1337: Topology constraints
- 1351: Subdivision
- 1388: Existing Loop vertex
- 1424: Laplacian smoothing
- 1463: Cotangent Laplacian
- 1506: Parameterization
- 1544: UV seams
- 1574: SKELETONS, SKINNING AND ANIMATION
- 1578: Skeleton hierarchy
- 1621: Local transforms
- 1642: Bind pose
- 1694: glTF inverse-bind matrices
- 1724: Linear Blend Skinning
- 1782: Weight normalization
- 1823: Skinning normals/tangents
- 1857: Candy-wrapper artifact
- 1877: Dual-quaternion skinning
- 1895: Animation clips/channels/samplers
- 1921: Translation interpolation
- 1948: Scale interpolation
- 1966: Quaternion interpolation
- 2034: Step interpolation
- 2048: Cubic spline
- 2102: Pose evaluation
- 2134: Animation blending
- 2167: Crossfade
- 2195: Additive animation
- 2236: Root motion
- 2261: MORPHS, ANIMATED BOUNDS, COMPRESSION AND COMPLETE TRACE
- 2265: Morph position deltas
- 2302: Morph normal deltas
- 2327: Morph weights
- 2349: Morph before or after skinning
- 2380: Animated bounds
- 2404: CPU vs GPU skinning
- 2434: Animation compression
- 2464: Coordinate-convention conversion
- 2514: Asset validation
- 2625: Skin influence
- 2679: Entity/world transform
- 2726: Previous-frame version for TAA
- 2779: A COHERENT END-TO-END C++ MODEL
- 2913: THE CONNECTION BETWEEN PASS 17 AND PASS 18
- 2977: RECONSTRUCTION GATE — PASS 17
- 3084: RECONSTRUCTION GATE — PASS 18
- 3222: STATUS
- 3235: Check which transmittance the stopping test actually uses
- 3254: From a rendered frame to a viewer's experience
- 3258: A correct image must survive the viewer — perceptual rendering and XR

## Scope and acceptance

14 derived files; 28118 routed body lines; 502 heading entries; 1421821 characters in retrieved files. Chapter 33's mathematical projection/incidence theory must not be mistaken for GPU projection matrices. Historical repository observations are not current code verification. Individual sections, original assets, proofs, code and exercises still need source-to-reader comparison. The three continuous masters, derived chapters and old generated LaTeX remain unchanged.
