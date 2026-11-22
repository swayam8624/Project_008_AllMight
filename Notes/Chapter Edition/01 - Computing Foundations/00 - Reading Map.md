# Computing Foundations — Bits, C++, Mathematics and Systems — Reading Map

Begin with representations and object lifetime, then follow the mathematical and systems contracts needed to build visual software. The major chapters retain the full continuous lessons while making each reading session manageable.

[[Notes/Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs|Continuous master]] · [[Notes/Chapter Edition/README|All three books]]

| Chapter | Estimated pages | What you will learn |
|---|---:|---|

| [[Notes/Chapter Edition/01 - Computing Foundations/01 - From Bits to Running Systems|01 — From Bits to Running Systems]] | 45.5 | Explain how state moves from external bytes through interpretation and representation into valid storage, and distinguish a value’s meaning from its encoding. |

| [[Notes/Chapter Edition/01 - Computing Foundations/02 - Values at Addresses Layout, Lifetimes, and Representation|02 — Values at Addresses: Layout, Lifetimes, and Representation]] | 51.9 | Choose and inspect a representation while respecting address, alignment, object lifetime, and validation boundaries. |

| [[Notes/Chapter Edition/01 - Computing Foundations/03 - Records as Resource Transactions|03 — Records as Resource Transactions]] | 50.1 | Distinguish aggregate and constructor initialization, then design copy/move operations as state transitions with explicit failure guarantees. |

| [[Notes/Chapter Edition/01 - Computing Foundations/04 - Prove C++ Ownership with Runnable Experiments|04 — Prove C++ Ownership with Runnable Experiments]] | 55.8 | Use executable probes to verify construction, cleanup, borrowing, optimization behavior, and failure guarantees without mistaking one run for a language rule. |

| [[Notes/Chapter Edition/01 - Computing Foundations/05 - Generic Asset Pipelines without Hidden Contracts|05 — Generic Asset Pipelines without Hidden Contracts]] | 49.0 | Build shared loader structure around type-specific validation, resource ownership, capacity, and copy/transfer policy. |

| [[Notes/Chapter Edition/01 - Computing Foundations/06 - Grow Registries without Invalidating Identity|06 — Grow Registries without Invalidating Identity]] | 61.0 | Design a growing asset registry that tolerates relocation and removal while preventing stale or mis-typed handles from silently naming another object. |

| [[Notes/Chapter Edition/01 - Computing Foundations/07 - Typed Interfaces, Identity, and Data Structures|07 — Typed Interfaces, Identity, and Data Structures]] | 49.9 | Choose an interface boundary and data structure from the operations and identity guarantees clients actually need. |

| [[Notes/Chapter Edition/01 - Computing Foundations/08 - Graphs, Search, and GPU Scheduling|08 — Graphs, Search, and GPU Scheduling]] | 51.7 | Select graph algorithms from edge assumptions, preserve dependency order, and connect spatial or render problems to their graph representation. |

| [[Notes/Chapter Edition/01 - Computing Foundations/09 - GPU Memory, Divergence, Atomics, and Ordering|09 — GPU Memory, Divergence, Atomics, and Ordering]] | 52.9 | Predict how memory placement, branch divergence, synchronization, and atomic ordering affect a GPU kernel. |

| [[Notes/Chapter Edition/01 - Computing Foundations/10 - Reproducible Builds, Profiling, and Repository Contracts|10 — Reproducible Builds, Profiling, and Repository Contracts]] | 56.3 | Create build and measurement workflows whose dependencies, configuration, test evidence, and platform boundaries can be reconstructed. |

| [[Notes/Chapter Edition/01 - Computing Foundations/11 - C++ Const, Borrowing, and CPU–GPU Buffering|11 — C++ Const, Borrowing, and CPU–GPU Buffering]] | 56.5 | Reason about qualification, lifetime-bounded views, and buffer reuse when CPU and GPU operations overlap. |

| [[Notes/Chapter Edition/01 - Computing Foundations/12 - Commands, Async Work, and Resource Lifetimes|12 — Commands, Async Work, and Resource Lifetimes]] | 47.0 | Model when commands are recorded, submitted, executed, and retired, and use synchronization only at real dependency boundaries. |

| [[Notes/Chapter Edition/01 - Computing Foundations/13 - Open Vulkan Devices and Describe Resources|13 — Open Vulkan Devices and Describe Resources]] | 65.3 | Trace Vulkan ownership from instance and device selection through swapchain, memory, descriptors, and draw-ready resource state. |

| [[Notes/Chapter Edition/01 - Computing Foundations/14 - Submit, Synchronize, and Retire GPU Work|14 — Submit, Synchronize, and Retire GPU Work]] | 63.7 | Design submission and retirement paths that account for frame-in-flight state, presentation, errors, and multiple queues. |

| [[Notes/Chapter Edition/01 - Computing Foundations/15 - Execute and Measure a Metal Frame|15 — Execute and Measure a Metal Frame]] | 44.3 | Map Metal device, command, storage, drawable, and synchronization concepts onto a measured frame without importing Vulkan assumptions blindly. |

| [[Notes/Chapter Edition/01 - Computing Foundations/16 - Profile GPUs and Respect Apple Ownership Boundaries|16 — Profile GPUs and Respect Apple Ownership Boundaries]] | 52.5 | Interpret GPU profiling evidence and define ownership, threading, and file/network boundaries across Apple platform APIs. |

| [[Notes/Chapter Edition/01 - Computing Foundations/17 - Python and Numerical Tools for Graphics Systems|17 — Python and Numerical Tools for Graphics Systems]] | 52.0 | Choose where Python is an effective control plane and how to keep data movement, profiling, and numerical assumptions visible. |

| [[Notes/Chapter Edition/01 - Computing Foundations/18 - Vectors for Light and Frames|18 — Vectors for Light and Frames]] | 50.5 | Derive reflection and refraction from vector components, and apply named frames to points and displacements. |

| [[Notes/Chapter Edition/01 - Computing Foundations/19 - Coordinate Frames and Linear Systems|19 — Coordinate Frames and Linear Systems]] | 52.2 | Transform coordinates between named frames and select a stable linear solve from the structure of the system. |

| [[Notes/Chapter Edition/01 - Computing Foundations/20 - Choose Matrix Factorizations from Structure|20 — Choose Matrix Factorizations from Structure]] | 49.6 | Select a factorization based on the matrix’s verified properties and explain when its assumptions fail. |

| [[Notes/Chapter Edition/01 - Computing Foundations/21 - Multivariable Derivatives and Sensitivity|21 — Multivariable Derivatives and Sensitivity]] | 56.9 | Compute and interpret derivatives of vector functions, normalization, and implicit constraints as local sensitivity information. |

| [[Notes/Chapter Edition/01 - Computing Foundations/22 - Accumulation, Quadrature, and the Bridge to Probability|22 — Accumulation, Quadrature, and the Bridge to Probability]] | 56.3 | Relate integrals to accumulation, derive simple antiderivatives, and compare exact quantities with numerical approximations. |

| [[Notes/Chapter Edition/01 - Computing Foundations/23 - Probability Models and Measurement Error|23 — Probability Models and Measurement Error]] | 57.2 | Define a probability model, distinguish discrete outcomes from continuous densities, and interpret uncertainty and standardized residuals. |

| [[Notes/Chapter Edition/01 - Computing Foundations/24 - Waiting Times, Covariance, and Matrix Decomposition|24 — Waiting Times, Covariance, and Matrix Decomposition]] | 51.0 | Derive waiting-time quantities and interpret covariance and matrix decompositions without confusing geometry with statistical variation. |

| [[Notes/Chapter Edition/01 - Computing Foundations/25 - Continuous Models and Numerical Error|25 — Continuous Models and Numerical Error]] | 71.3 | Connect boundary conditions and spatial discretization to a numerical operator, then identify truncation and roundoff limits. |

| [[Notes/Chapter Edition/01 - Computing Foundations/26 - Matrix Functions and Nonlinear Least Squares|26 — Matrix Functions and Nonlinear Least Squares]] | 47.2 | Interpret matrix exponential and related functions, and express nonlinear fitting as minimizing a residual-based objective. |

| [[Notes/Chapter Edition/01 - Computing Foundations/27 - Gauss–Newton for Geometry and Pose|27 — Gauss–Newton for Geometry and Pose]] | 47.3 | Derive a Gauss–Newton update from residual linearization and diagnose rank deficiency or an ambiguous pose estimate. |

| [[Notes/Chapter Edition/01 - Computing Foundations/28 - Reliable Optimization and Root Finding|28 — Reliable Optimization and Root Finding]] | 53.0 | Implement and assess optimization and root-finding procedures with finite checks, stopping criteria, failure statuses, and independent numerical witnesses. |

| [[Notes/Chapter Edition/01 - Computing Foundations/29 - Discretization, Convergence, and Experimental Design|29 — Discretization, Convergence, and Experimental Design]] | 52.9 | Separate discretization, truncation, and roundoff, and design experiments that measure the error relevant to the application. |

| [[Notes/Chapter Edition/01 - Computing Foundations/30 - Monte Carlo Estimates and Uncertainty Metrics|30 — Monte Carlo Estimates and Uncertainty Metrics]] | 67.4 | Estimate expectations from random samples, quantify uncertainty, and select an error metric whose weighting reflects the intended comparison. |

| [[Notes/Chapter Edition/01 - Computing Foundations/31 - Implement Probability with Numerical Contracts|31 — Implement Probability with Numerical Contracts]] | 44.6 | Implement probability and covariance routines while preventing overflow, preserving tail accuracy, and validating random algorithms deterministically. |

| [[Notes/Chapter Edition/01 - Computing Foundations/32 - Geometry Becomes an Image and an Inverse Problem|32 — Geometry Becomes an Image and an Inverse Problem]] | 40.7 | Carry points and directions through camera transforms and projection, then identify what depth, multiple views, or calibration can recover. |

| [[Notes/Chapter Edition/01 - Computing Foundations/33 - Projection, Clipping, and Raster Sampling|33 — Projection, Clipping, and Raster Sampling]] | 55.5 | Derive projection mappings and explain clipping, perspective division, raster coordinates, and sample-center conventions. |

| [[Notes/Chapter Edition/01 - Computing Foundations/34 - Robust Predicates and Camera Sensitivity|34 — Robust Predicates and Camera Sensitivity]] | 49.0 | Distinguish predicates from constructions and analyze how geometric and camera parameters affect a result near a decision boundary. |

| [[Notes/Chapter Edition/01 - Computing Foundations/35 - Camera Extrinsics and Projection Geometry|35 — Camera Extrinsics and Projection Geometry]] | 48.8 | Convert between camera-to-world and world-to-camera conventions and derive rays from image measurements for non-pinhole cameras. |

| [[Notes/Chapter Edition/01 - Computing Foundations/36 - Exposure, Physical Optics, and Depth of Field|36 — Exposure, Physical Optics, and Depth of Field]] | 56.5 | Relate exposure and optical parameters to image formation, and understand how multi-view consistency constrains camera and scene estimates. |

| [[Notes/Chapter Edition/01 - Computing Foundations/37 - Camera Contracts and Transform-Chain Debugging|37 — Camera Contracts and Transform-Chain Debugging]] | 54.5 | Design camera interfaces with explicit conventions and debug a point through every transform stage using inspectable values. |

| [[Notes/Chapter Edition/01 - Computing Foundations/38 - Reconstruct the Complete Geometry Pipeline|38 — Reconstruct the Complete Geometry Pipeline]] | 56.0 | Reconstruct the full geometry and numerical pipeline while keeping storage, coordinate, interpolation, and ownership assumptions explicit. |

| [[Notes/Chapter Edition/01 - Computing Foundations/39 - Research Navigation and Source Boundaries|39 — Research Navigation and Source Boundaries]] | 52.8 | Use source records and research navigation to verify a claim, identify its limits, and choose a next reading or experiment. |
