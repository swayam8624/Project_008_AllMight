# Companion laboratory guide {#laboratory-guide}

The companion lives in the repository under `Code Parts/tools/labs/book03_cpp23`. Its tests and demonstration executables are separate. The following commands configure and build the full project from the repository root, using a recent C++23-capable compiler and CMake:

~~~sh
cmake -S "Code Parts/tools/labs/book03_cpp23" \
      -B "Code Parts/build/book03-native" \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=23
cmake --build "Code Parts/build/book03-native" --parallel 4
ctest --test-dir "Code Parts/build/book03-native" --output-on-failure
~~~

A test executable needs no graphical interface. A workshop executable may require arguments or a new output directory: consult its own chapter and the companion README before invocation. For the independent Python/NumPy reconciliation checks, install NumPy in your chosen environment. No compiler command can certify your local machine's result before you run it.

| Chapter | Demonstration target | Source entry point |
|---|---|---|
| [01](#chapter-01) | `book03_canvas` | `main.cpp` |
| [02](#chapter-02) | `book03_frames` | `frame_playground.cpp` |
| [03](#chapter-03) | `book03_triangles` | `triangle_studio.cpp` |
| [04](#chapter-04) | `book03_transforms` | `transform_studio.cpp` |
| [05](#chapter-05) | `book03_precision` | `precision_studio.cpp` |
| [06](#chapter-06) | `book03_space` | `spatial_studio.cpp` |
| [07](#chapter-07) | `book03_mesh` | `mesh_studio.cpp` |
| [08](#chapter-08) | `book03_curves` | `curve_studio.cpp` |
| [09](#chapter-09) | `book03_world` | `procedural_studio.cpp` |
| [10](#chapter-10) | `book03_assets` | `asset_studio.cpp` |
| [11](#chapter-11) | `book03_scene` | `scene_studio.cpp` |
| [12](#chapter-12) | `book03_rotation` | `rotation_studio.cpp` |
| [13](#chapter-13) | `book03_camera` | `pinhole_studio.cpp` |
| [14](#chapter-14) | `book03_projection` | `projection_studio.cpp` |
| [15](#chapter-15) | `book03_stereo` | `stereo_studio.cpp` |
| [16](#chapter-16) | `book03_vertex` | `vertex_studio.cpp` |
| [17](#chapter-17) | `book03_clip` | `clipping_studio.cpp` |
| [18](#chapter-18) | `book03_raster` | `raster_studio.cpp` |
| [19](#chapter-19) | `book03_visibility` | `visibility_studio.cpp` |
| [20](#chapter-20) | `book03_sampling` | `sampling_studio.cpp` |
| [21](#chapter-21) | `book03_renderer_gallery / book03_renderer` | `renderer_gallery / renderer_obj.cpp (both entries)` |
| [22](#chapter-22) | `book03_lens` | `lens_studio.cpp` |
| [23](#chapter-23) | `book03_distortion` | `distortion_studio.cpp` |
| [24](#chapter-24) | `book03_sensor` | `sensor_studio.cpp` |
| [25](#chapter-25) | `book03_color` | `color_studio.cpp` |
| [26](#chapter-26) | `book03_calibration` | `calibration_studio.cpp` |
| [27](#chapter-27) | `book03_epipolar` | `epipolar_studio.cpp` |
| [28](#chapter-28) | `book03_matching` | `matching_studio.cpp` |
| [29](#chapter-29) | `book03_sfm` | `sfm_studio.cpp` |
| [30](#chapter-30) | `book03_surface` | `surface_studio.cpp` |
| [31](#chapter-31) | `book03_studio` | `studio_main.cpp` |
| [32](#chapter-32) | `book03_capstone` | `capstone_main.cpp` |

## A debugging route that survives new projects

Start with a single sample that has an exact answer. Write the frame, units, reference and expected value on paper. Trace that sample through each stage, recording each intermediate representation. If the last stage is wrong, find the earliest stage where the expected and actual values disagree. Do not change a downstream convention merely to hide an upstream error.

Next, build a family of cases using invariants: common rigid translations, valid coordinate changes, sign conventions, equivalent representations, subdivision, order reversal, and known scale transformations. A correct invariant is conditioned on the model, not on the appearance of the picture. Deliberately include invalid inputs and require an explicit rejection policy. When floating-point arithmetic is involved, record the supported range, the scale of each tolerance and whether your claim is about an exact represented input or an approximate physical measurement.

Finally, record cost only after output equivalence is established. Keep raw inputs and output, failures, seeds, units, compiler, hardware and repeat counts. See [Chapter 32](#chapter-32) for the complete evidence dossier.

# Notation and representation reference {#notation-reference}

The same letter can denote different quantities in different local derivations. This table names the common conventions; a chapter's explicitly stated domain takes precedence. Never infer storage layout or units merely from a symbol.

| Quantity | Meaning and contract |
|---|---|
| Point vs displacement | A point is a location; a vector is a difference or direction. Translation applies to points, not free displacements. |
| Matrix action | Vectors are columns by default. Composition applies the rightmost matrix first. Row-major memory storage is a separate choice. |
| Optical coordinates | In the calibrated-camera chapters, +X right, +Y down, +Z forward. Positive Z is axial depth, not necessarily Euclidean ray distance. |
| Graphics view coordinates | Some projection chapters use a right-handed view looking down local -Z; convert explicitly. |
| Image index (i,j) | Integer storage address; continuous sample center is (i+0.5,j+0.5) under the top-left edge convention. |
| K, R, t | Intrinsics and a world-to-optical proper rotation/translation, unless stated otherwise. Camera center is -R^T t. |
| w, q, Z | Homogeneous clip coordinate, divided normalized depth, and optical axial depth respectively. They are not interchangeable. |
| E, F | Essential and fundamental matrices; zero homogeneous incidence constraints with nonzero scale ambiguity. |
| RGB | Three coordinates in a named color space. Linear-light RGB and transfer-encoded RGB have different arithmetic meanings. |
| RMSE | Root mean square under a specified residual definition and units. Pixel RMSE does not measure metric surface accuracy. |
| TSDF weight | Support/confidence in a stored signed-distance contribution. An unknown cell is not an observed zero. |

## Where the series continues

This book gives the geometry and measurement foundations. *From Light to Realism* develops light transport and materials; *From Code to Living Worlds* develops spatial simulation and engine state; *From Frames to Real Time* develops hardware-aware real-time rendering; *From Perception to Discovery* develops vision, XR and research-oriented systems. The scope boundaries do not diminish the need to understand geometry: they are how we avoid confusing a small teaching implementation with an entire production engine.
