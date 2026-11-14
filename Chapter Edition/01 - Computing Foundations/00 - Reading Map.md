# Computing Foundations — Bits, C++, Mathematics and Systems — Reading Map

Begin with representations and object lifetime, then follow the mathematical and systems contracts needed to build visual software. The major chapters retain the full continuous lessons while making each reading session manageable.

[[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs|Continuous master]] · [[Chapter Edition/README|All three books]]

| Chapter | Estimated pages | What you will learn |
|---|---:|---|

| [[Chapter Edition/01 - Computing Foundations/01 - From Bits to Running Systems|01 — From Bits to Running Systems]] | 45.5 | Explain how state moves from external bytes through interpretation and representation into valid storage, and distinguish a value’s meaning from its encoding. |

| [[Chapter Edition/01 - Computing Foundations/02 - Values at Addresses Layout, Lifetimes, and Representation|02 — Values at Addresses: Layout, Lifetimes, and Representation]] | 51.9 | Choose and inspect a representation while respecting address, alignment, object lifetime, and validation boundaries. |

| [[Chapter Edition/01 - Computing Foundations/03 - Records as Resource Transactions|03 — Records as Resource Transactions]] | 50.1 | Distinguish aggregate and constructor initialization, then design copy/move operations as state transitions with explicit failure guarantees. |

| [[Chapter Edition/01 - Computing Foundations/04 - Prove C++ Ownership with Runnable Experiments|04 — Prove C++ Ownership with Runnable Experiments]] | 55.8 | Use executable probes to verify construction, cleanup, borrowing, optimization behavior, and failure guarantees without mistaking one run for a language rule. |

| [[Chapter Edition/01 - Computing Foundations/05 - Generic Asset Pipelines without Hidden Contracts|05 — Generic Asset Pipelines without Hidden Contracts]] | 45.8 | Build shared loader structure around type-specific validation, resource ownership, capacity, and copy/transfer policy. |

| [[Chapter Edition/01 - Computing Foundations/06 - Grow Registries without Invalidating Identity|06 — Grow Registries without Invalidating Identity]] | 60.4 | Design a growing asset registry that tolerates relocation and removal while preventing stale or mis-typed handles from silently naming another object. |

| [[Chapter Edition/01 - Computing Foundations/07 - Typed Interfaces, Identity, and Data Structures|07 — Typed Interfaces, Identity, and Data Structures]] | 43.5 | Choose an interface boundary and data structure from the operations and identity guarantees clients actually need. |

| [[Chapter Edition/01 - Computing Foundations/08 - Graphs, Search, and GPU Scheduling|08 — Graphs, Search, and GPU Scheduling]] | 51.4 | Select graph algorithms from edge assumptions, preserve dependency order, and connect spatial or render problems to their graph representation. |

| [[Chapter Edition/01 - Computing Foundations/09 - GPU Memory, Divergence, Atomics, and Ordering|09 — GPU Memory, Divergence, Atomics, and Ordering]] | 52.9 | Predict how memory placement, branch divergence, synchronization, and atomic ordering affect a GPU kernel. |

| [[Chapter Edition/01 - Computing Foundations/10 - Reproducible Builds, Profiling, and Repository Contracts|10 — Reproducible Builds, Profiling, and Repository Contracts]] | 56.3 | Create build and measurement workflows whose dependencies, configuration, test evidence, and platform boundaries can be reconstructed. |

| [[Chapter Edition/01 - Computing Foundations/11 - C++ Const, Borrowing, and CPU–GPU Buffering|11 — C++ Const, Borrowing, and CPU–GPU Buffering]] | 56.5 | Reason about qualification, lifetime-bounded views, and buffer reuse when CPU and GPU operations overlap. |

| [[Chapter Edition/01 - Computing Foundations/12 - Commands, Async Work, and Resource Lifetimes|12 — Commands, Async Work, and Resource Lifetimes]] | 47.0 | Model when commands are recorded, submitted, executed, and retired, and use synchronization only at real dependency boundaries. |

| [[Chapter Edition/01 - Computing Foundations/13 - Open Vulkan Devices and Describe Resources|13 — Open Vulkan Devices and Describe Resources]] | 65.3 | Trace Vulkan ownership from instance and device selection through swapchain, memory, descriptors, and draw-ready resource state. |

| [[Chapter Edition/01 - Computing Foundations/14 - Submit, Synchronize, and Retire GPU Work|14 — Submit, Synchronize, and Retire GPU Work]] | 63.7 | Design submission and retirement paths that account for frame-in-flight state, presentation, errors, and multiple queues. |

| [[Chapter Edition/01 - Computing Foundations/15 - Execute and Measure a Metal Frame|15 — Execute and Measure a Metal Frame]] | 44.3 | Map Metal device, command, storage, drawable, and synchronization concepts onto a measured frame without importing Vulkan assumptions blindly. |

| [[Chapter Edition/01 - Computing Foundations/16 - Profile GPUs and Respect Apple Ownership Boundaries|16 — Profile GPUs and Respect Apple Ownership Boundaries]] | 52.5 | Interpret GPU profiling evidence and define ownership, threading, and file/network boundaries across Apple platform APIs. |

| [[Chapter Edition/01 - Computing Foundations/17 - Python and Numerical Tools for Graphics Systems|17 — Python and Numerical Tools for Graphics Systems]] | 51.0 | Choose where Python is an effective control plane and how to keep data movement, profiling, and numerical assumptions visible. |

| [[Chapter Edition/01 - Computing Foundations/18 - Vectors for Light and Frames|18 — Vectors for Light and Frames]] | 50.5 | Derive reflection and refraction from vector components, and apply named frames to points and displacements. |

| [[Chapter Edition/01 - Computing Foundations/19 - Coordinate Frames and Linear Systems|19 — Coordinate Frames and Linear Systems]] | 52.2 | Transform coordinates between named frames and select a stable linear solve from the structure of the system. |

| [[Chapter Edition/01 - Computing Foundations/20 - Choose Matrix Factorizations from Structure|20 — Choose Matrix Factorizations from Structure]] | 49.6 | Select a factorization based on the matrix’s verified properties and explain when its assumptions fail. |

| [[Chapter Edition/01 - Computing Foundations/21 - Multivariable Derivatives and Sensitivity|21 — Multivariable Derivatives and Sensitivity]] | 56.9 | Compute and interpret derivatives of vector functions, normalization, and implicit constraints as local sensitivity information. |

| [[Chapter Edition/01 - Computing Foundations/22 - Accumulation, Quadrature, and the Bridge to Probability|22 — Accumulation, Quadrature, and the Bridge to Probability]] | 56.3 | Relate integrals to accumulation, derive simple antiderivatives, and compare exact quantities with numerical approximations. |

| [[Chapter Edition/01 - Computing Foundations/23 - Probability Models and Measurement Error|23 — Probability Models and Measurement Error]] | 57.3 | Define a probability model, distinguish discrete outcomes from continuous densities, and interpret uncertainty and standardized residuals. |

| [[Chapter Edition/01 - Computing Foundations/24 - Waiting Times, Covariance, and Matrix Decomposition|24 — Waiting Times, Covariance, and Matrix Decomposition]] | 51.0 | Derive waiting-time quantities and interpret covariance and matrix decompositions without confusing geometry with statistical variation. |

| [[Chapter Edition/01 - Computing Foundations/25 - Continuous Models and Numerical Error|25 — Continuous Models and Numerical Error]] | 59.0 | Connect boundary conditions and spatial discretization to a numerical operator, then identify truncation and roundoff limits. |

| [[Chapter Edition/01 - Computing Foundations/26 - Matrix Functions and Nonlinear Least Squares|26 — Matrix Functions and Nonlinear Least Squares]] | 45.9 | Interpret matrix exponential and related functions, and express nonlinear fitting as minimizing a residual-based objective. |

| [[Chapter Edition/01 - Computing Foundations/27 - Gauss–Newton for Geometry and Pose|27 — Gauss–Newton for Geometry and Pose]] | 47.4 | Derive a Gauss–Newton update from residual linearization and diagnose rank deficiency or an ambiguous pose estimate. |

| [[Chapter Edition/01 - Computing Foundations/28 - Reliable Optimization and Root Finding|28 — Reliable Optimization and Root Finding]] | 53.0 | Implement and assess optimization and root-finding procedures with finite checks, stopping criteria, failure statuses, and independent numerical witnesses. |

| [[Chapter Edition/01 - Computing Foundations/29 - Discretization, Convergence, and Experimental Design|29 — Discretization, Convergence, and Experimental Design]] | 51.2 | Separate discretization, truncation, and roundoff, and design experiments that measure the error relevant to the application. |

| [[Chapter Edition/01 - Computing Foundations/30 - Monte Carlo Estimates and Uncertainty Metrics|30 — Monte Carlo Estimates and Uncertainty Metrics]] | 60.2 | Estimate expectations from random samples, quantify uncertainty, and select an error metric whose weighting reflects the intended comparison. |

| [[Chapter Edition/01 - Computing Foundations/31 - Implement Probability with Numerical Contracts|31 — Implement Probability with Numerical Contracts]] | 44.6 | Implement probability and covariance routines while preventing overflow, preserving tail accuracy, and validating random algorithms deterministically. |

| [[Chapter Edition/01 - Computing Foundations/32 - Geometry Becomes an Image and an Inverse Problem|32 — Geometry Becomes an Image and an Inverse Problem]] | 40.7 | Carry points and directions through camera transforms and projection, then identify what depth, multiple views, or calibration can recover. |

| [[Chapter Edition/01 - Computing Foundations/33 - Projection, Clipping, and Raster Sampling|33 — Projection, Clipping, and Raster Sampling]] | 55.5 | Derive projection mappings and explain clipping, perspective division, raster coordinates, and sample-center conventions. |

| [[Chapter Edition/01 - Computing Foundations/34 - Robust Predicates and Camera Sensitivity|34 — Robust Predicates and Camera Sensitivity]] | 49.0 | Distinguish predicates from constructions and analyze how geometric and camera parameters affect a result near a decision boundary. |

| [[Chapter Edition/01 - Computing Foundations/35 - Camera Extrinsics and Projection Geometry|35 — Camera Extrinsics and Projection Geometry]] | 48.8 | Convert between camera-to-world and world-to-camera conventions and derive rays from image measurements for non-pinhole cameras. |

| [[Chapter Edition/01 - Computing Foundations/36 - Exposure, Physical Optics, and Depth of Field|36 — Exposure, Physical Optics, and Depth of Field]] | 56.5 | Relate exposure and optical parameters to image formation, and understand how multi-view consistency constrains camera and scene estimates. |

| [[Chapter Edition/01 - Computing Foundations/37 - Camera Contracts and Transform-Chain Debugging|37 — Camera Contracts and Transform-Chain Debugging]] | 54.5 | Design camera interfaces with explicit conventions and debug a point through every transform stage using inspectable values. |

| [[Chapter Edition/01 - Computing Foundations/38 - Reconstruct the Complete Geometry Pipeline|38 — Reconstruct the Complete Geometry Pipeline]] | 51.8 | Reconstruct the full geometry and numerical pipeline while keeping storage, coordinate, interpolation, and ownership assumptions explicit. |

| [[Chapter Edition/01 - Computing Foundations/39 - Research Navigation and Source Boundaries|39 — Research Navigation and Source Boundaries]] | 47.6 | Use source records and research navigation to verify a claim, identify its limits, and choose a next reading or experiment. |
