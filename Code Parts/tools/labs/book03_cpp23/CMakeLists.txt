cmake_minimum_required(VERSION 3.22)
project(Book03PixelCanvas VERSION 0.1.0 LANGUAGES CXX)

# Standard C++23 is the intended learner implementation language. No Vulkan,
# Metal, OpenGL, Python, GUI SDK or image-encoding dependency is required.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(book03_pixel_canvas INTERFACE)
target_include_directories(book03_pixel_canvas INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_features(book03_pixel_canvas INTERFACE cxx_std_23)

add_executable(book03_canvas src/main.cpp)
target_link_libraries(book03_canvas PRIVATE book03_pixel_canvas)

add_executable(book03_canvas_tests tests/pixel_canvas_tests.cpp)
target_link_libraries(book03_canvas_tests PRIVATE book03_pixel_canvas)

# Chapter 02 builds on Chapter 01 but keeps its numeric contracts separate.
add_library(book03_frame_math INTERFACE)
target_include_directories(book03_frame_math INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_features(book03_frame_math INTERFACE cxx_std_23)

add_executable(book03_frames src/frame_playground.cpp)
target_link_libraries(book03_frames PRIVATE book03_frame_math)

add_executable(book03_frame_tests tests/frame_math_tests.cpp)
target_link_libraries(book03_frame_tests PRIVATE book03_frame_math)

# Chapter 03: reuse the first two chapters' canvas and frame types.
add_library(book03_triangle_geometry INTERFACE)
target_include_directories(book03_triangle_geometry INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_features(book03_triangle_geometry INTERFACE cxx_std_23)

add_executable(book03_triangles src/triangle_studio.cpp)
target_link_libraries(book03_triangles PRIVATE
    book03_triangle_geometry book03_pixel_canvas)

add_executable(book03_triangle_tests tests/triangle_geometry_tests.cpp)
target_link_libraries(book03_triangle_tests PRIVATE book03_triangle_geometry)

# Chapter 04: the C++23 homogeneous-affine layer reuses the earlier
# Point2 / Vector2 / Frame2 / Triangle2 / Canvas contracts.
add_library(book03_affine2 INTERFACE)
target_include_directories(book03_affine2 INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_features(book03_affine2 INTERFACE cxx_std_23)

add_executable(book03_transforms src/transform_studio.cpp)
target_link_libraries(book03_transforms PRIVATE
    book03_affine2 book03_pixel_canvas book03_triangle_geometry)

add_executable(book03_affine_tests tests/affine2_tests.cpp)
target_link_libraries(book03_affine_tests PRIVATE
    book03_affine2 book03_triangle_geometry)

# Chapter 05: the exact represented-binary64 orientation oracle is
# fully standard C++23. Its proof-carrying slow path uses integer limbs.
add_library(book03_exact_orient2d INTERFACE)
target_include_directories(book03_exact_orient2d INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_features(book03_exact_orient2d INTERFACE cxx_std_23)

add_executable(book03_precision src/precision_studio.cpp)
target_link_libraries(book03_precision PRIVATE
    book03_exact_orient2d book03_triangle_geometry)

add_executable(book03_exact_tests tests/exact_orient2d_tests.cpp)
target_link_libraries(book03_exact_tests PRIVATE
    book03_exact_orient2d book03_triangle_geometry)

# Chapter 06: actual 3D points, displacements, planes, rays, triangles and
# spheres. Pure C++23, no camera, GUI, GPU or external geometry SDK.
add_library(book03_geometry3d INTERFACE)
target_include_directories(book03_geometry3d INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_features(book03_geometry3d INTERFACE cxx_std_23)

add_executable(book03_space src/spatial_studio.cpp)
target_link_libraries(book03_space PRIVATE book03_geometry3d)

add_executable(book03_space_tests tests/geometry3d_tests.cpp)
target_link_libraries(book03_space_tests PRIVATE
    book03_geometry3d book03_triangle_geometry)

# Chapter 07: indexed mesh identity, shared-edge incidence, orientability
# diagnostics, nonmanifold vertices and the source-generated mesh inspector.
add_library(book03_mesh_topology INTERFACE)
target_include_directories(book03_mesh_topology INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_features(book03_mesh_topology INTERFACE cxx_std_23)

add_executable(book03_mesh src/mesh_studio.cpp)
target_link_libraries(book03_mesh PRIVATE
    book03_mesh_topology book03_geometry3d)

add_executable(book03_mesh_tests tests/mesh_topology_tests.cpp)
target_link_libraries(book03_mesh_tests PRIVATE
    book03_mesh_topology book03_geometry3d)

# Same Chapter 07, second independent test target: build/traverse half-edge
# face cycles and only assign twins when incidence is exactly two and opposite.
add_executable(book03_halfedge_tests tests/halfedge_mesh_tests.cpp)
target_link_libraries(book03_halfedge_tests PRIVATE
    book03_mesh_topology book03_geometry3d)

# Chapter 08: de Casteljau/Bernstein/Hermite/uniform cubic spline curves,
# derivative joins, adaptive chord tessellation, and indexed 3D patches.
# It reuses the Point3 and Mesh3 sources rather than inventing new types.
add_library(book03_curve_math INTERFACE)
target_include_directories(book03_curve_math INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_features(book03_curve_math INTERFACE cxx_std_23)

add_executable(book03_curves src/curve_studio.cpp)
target_link_libraries(book03_curves PRIVATE
    book03_curve_math book03_mesh_topology)

add_executable(book03_curves_tests tests/curves3d_tests.cpp)
target_link_libraries(book03_curves_tests PRIVATE
    book03_curve_math book03_mesh_topology)

# Chapter 09: analytic terrain, capped cylinder, pole-safe UV sphere and
# grid-edge-cached implicit spherical level-set triangulation. All C++23.
add_library(book03_procedural_geometry INTERFACE)
target_include_directories(book03_procedural_geometry INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_features(book03_procedural_geometry INTERFACE cxx_std_23)

add_executable(book03_world src/procedural_studio.cpp)
target_link_libraries(book03_world PRIVATE
    book03_procedural_geometry book03_mesh_topology)

add_executable(book03_world_tests tests/procedural_geometry_tests.cpp)
target_link_libraries(book03_world_tests PRIVATE
    book03_procedural_geometry book03_mesh_topology)

# Chapter 10: bounded OBJ independent-stream parser, topological-to-GPU
# corner-tuple conversion, source asset auditor and glTF binary layout kernel.
add_library(book03_asset_import INTERFACE)
target_include_directories(book03_asset_import INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_features(book03_asset_import INTERFACE cxx_std_23)

add_executable(book03_assets src/asset_studio.cpp)
target_link_libraries(book03_assets PRIVATE
    book03_asset_import book03_mesh_topology)

add_executable(book03_asset_tests tests/asset_import_tests.cpp)
target_link_libraries(book03_asset_tests PRIVATE
    book03_asset_import book03_mesh_topology)

# Chapter 11: shared mesh instances and affine scene forest.
add_library(book03_scene_forest INTERFACE)
target_include_directories(book03_scene_forest INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_features(book03_scene_forest INTERFACE cxx_std_23)
add_executable(book03_scene src/scene_studio.cpp)
target_link_libraries(book03_scene PRIVATE book03_scene_forest book03_mesh_topology)
add_executable(book03_scene_tests tests/scene_forest_tests.cpp)
target_link_libraries(book03_scene_tests PRIVATE book03_scene_forest book03_mesh_topology)

# Chapter 12: proper rotations, unit quaternion and rigid pose contracts.
add_library(book03_rotation3 INTERFACE)
target_include_directories(book03_rotation3 INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_features(book03_rotation3 INTERFACE cxx_std_23)
add_executable(book03_rotation src/rotation_studio.cpp)
target_link_libraries(book03_rotation PRIVATE book03_rotation3 book03_scene_forest)
add_executable(book03_rotation_tests tests/rotation3_tests.cpp)
target_link_libraries(book03_rotation_tests PRIVATE book03_rotation3 book03_scene_forest)

# Chapter 13: calibrated pinhole and rigid camera pose.
add_library(book03_pinhole INTERFACE)
target_include_directories(book03_pinhole INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_features(book03_pinhole INTERFACE cxx_std_23)
add_executable(book03_camera src/pinhole_studio.cpp)
target_link_libraries(book03_camera PRIVATE book03_pinhole book03_rotation3)
add_executable(book03_camera_tests tests/pinhole_camera_tests.cpp)
target_link_libraries(book03_camera_tests PRIVATE book03_pinhole book03_rotation3)

# Chapter 14: homogeneous clip projection and depth storage reference.
add_library(book03_projection_pipeline INTERFACE)
target_include_directories(book03_projection_pipeline INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_features(book03_projection_pipeline INTERFACE cxx_std_23)
add_executable(book03_projection src/projection_studio.cpp)
target_link_libraries(book03_projection PRIVATE book03_projection_pipeline book03_pinhole)
add_executable(book03_projection_tests tests/projection_pipeline_tests.cpp)
target_link_libraries(book03_projection_tests PRIVATE book03_projection_pipeline book03_pinhole)

# Chapter 15: calibrated stereo and triangulation.
add_library(book03_stereo_geometry INTERFACE)
target_include_directories(book03_stereo_geometry INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_features(book03_stereo_geometry INTERFACE cxx_std_23)
add_executable(book03_stereo src/stereo_studio.cpp)
target_link_libraries(book03_stereo PRIVATE book03_stereo_geometry book03_pinhole)
add_executable(book03_stereo_tests tests/stereo_geometry_tests.cpp)
target_link_libraries(book03_stereo_tests PRIVATE book03_stereo_geometry book03_pinhole)

# Chapter 16: indexed vertex processing and perspective interpolation.
add_library(book03_vertex_pipeline INTERFACE)
target_include_directories(book03_vertex_pipeline INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_features(book03_vertex_pipeline INTERFACE cxx_std_23)
add_executable(book03_vertex src/vertex_studio.cpp)
target_link_libraries(book03_vertex PRIVATE book03_vertex_pipeline book03_projection_pipeline)
add_executable(book03_vertex_tests tests/vertex_pipeline_tests.cpp)
target_link_libraries(book03_vertex_tests PRIVATE book03_vertex_pipeline book03_projection_pipeline)

# Chapter 17: rectangular and homogeneous primitive clipping.
add_library(book03_primitive_clipping INTERFACE)
target_include_directories(book03_primitive_clipping INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_features(book03_primitive_clipping INTERFACE cxx_std_23)
add_executable(book03_clip src/clipping_studio.cpp)
target_link_libraries(book03_clip PRIVATE book03_primitive_clipping book03_vertex_pipeline)
add_executable(book03_clip_tests tests/primitive_clipping_tests.cpp)
target_link_libraries(book03_clip_tests PRIVATE book03_primitive_clipping book03_vertex_pipeline)

# Chapter 18: bounded exact integer coverage on a snapped sample grid.
add_library(book03_raster_coverage INTERFACE)
target_include_directories(book03_raster_coverage INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_features(book03_raster_coverage INTERFACE cxx_std_23)
add_executable(book03_raster src/raster_studio.cpp)
target_link_libraries(book03_raster PRIVATE book03_raster_coverage book03_primitive_clipping)
add_executable(book03_raster_tests tests/raster_coverage_tests.cpp)
target_link_libraries(book03_raster_tests PRIVATE book03_raster_coverage)

# Chapter 19: opaque per-sample visibility and depth storage.
add_library(book03_visibility_lib INTERFACE)
target_include_directories(book03_visibility_lib INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_features(book03_visibility_lib INTERFACE cxx_std_23)
add_executable(book03_visibility src/visibility_studio.cpp)
target_link_libraries(book03_visibility PRIVATE book03_visibility_lib book03_raster_coverage)
add_executable(book03_visibility_tests tests/visibility_tests.cpp)
target_link_libraries(book03_visibility_tests PRIVATE book03_visibility_lib book03_raster_coverage)

# Chapter 20: analytic sampling and numerical quadrature.
add_library(book03_sampling_lib INTERFACE)
target_include_directories(book03_sampling_lib INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_features(book03_sampling_lib INTERFACE cxx_std_23)
add_executable(book03_sampling src/sampling_studio.cpp)
target_link_libraries(book03_sampling PRIVATE book03_sampling_lib)
add_executable(book03_sampling_tests tests/sampling_tests.cpp)
target_link_libraries(book03_sampling_tests PRIVATE book03_sampling_lib)

# Chapter 21: native-validated backend and separately gated front end.
add_library(book03_software_renderer INTERFACE)
target_include_directories(book03_software_renderer INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_features(book03_software_renderer INTERFACE cxx_std_23)
add_executable(book03_renderer_gallery src/renderer_gallery.cpp)
target_link_libraries(book03_renderer_gallery PRIVATE book03_software_renderer book03_visibility_lib book03_sampling_lib)
add_executable(book03_renderer_tests tests/renderer_tests.cpp)
target_link_libraries(book03_renderer_tests PRIVATE book03_software_renderer)
add_executable(book03_renderer src/renderer_obj.cpp)
target_link_libraries(book03_renderer PRIVATE book03_software_renderer book03_primitive_clipping)
add_executable(book03_renderer_frontend_tests tests/renderer_frontend_tests.cpp)
target_link_libraries(book03_renderer_frontend_tests PRIVATE book03_software_renderer book03_primitive_clipping)

# Chapter 22: ideal paraxial thin lens in millimetres.
add_library(book03_thin_lens INTERFACE)
target_include_directories(book03_thin_lens INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_features(book03_thin_lens INTERFACE cxx_std_23)
add_executable(book03_lens src/lens_studio.cpp)
target_link_libraries(book03_lens PRIVATE book03_thin_lens)
add_executable(book03_lens_tests tests/lens_tests.cpp)
target_link_libraries(book03_lens_tests PRIVATE book03_thin_lens)

# Chapter 23: normalized lens distortion and bounded numerical inversion.
add_library(book03_distortion_lib INTERFACE)
target_include_directories(book03_distortion_lib INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_features(book03_distortion_lib INTERFACE cxx_std_23)
add_executable(book03_distortion src/distortion_studio.cpp)
target_link_libraries(book03_distortion PRIVATE book03_distortion_lib)
add_executable(book03_distortion_tests tests/distortion_tests.cpp)
target_link_libraries(book03_distortion_tests PRIVATE book03_distortion_lib)

# Chapter 24: physical exposure, stochastic charge and digitization.
add_library(book03_sensor_lib INTERFACE)
target_include_directories(book03_sensor_lib INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_features(book03_sensor_lib INTERFACE cxx_std_23)
add_executable(book03_sensor src/sensor_studio.cpp)
target_link_libraries(book03_sensor PRIVATE book03_sensor_lib)
add_executable(book03_sensor_tests tests/sensor_tests.cpp)
target_link_libraries(book03_sensor_tests PRIVATE book03_sensor_lib)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    foreach(target book03_canvas book03_canvas_tests
                   book03_frames book03_frame_tests
                   book03_triangles book03_triangle_tests
                   book03_transforms book03_affine_tests
                   book03_precision book03_exact_tests
                   book03_space book03_space_tests
                   book03_mesh book03_mesh_tests
                   book03_halfedge_tests
                   book03_curves book03_curves_tests
                   book03_world book03_world_tests
                   book03_assets book03_asset_tests
                   book03_scene book03_scene_tests
                   book03_rotation book03_rotation_tests
                   book03_camera book03_camera_tests
                   book03_projection book03_projection_tests
                   book03_stereo book03_stereo_tests
                   book03_vertex book03_vertex_tests
                   book03_clip book03_clip_tests
                   book03_raster book03_raster_tests
                   book03_visibility book03_visibility_tests
                   book03_sampling book03_sampling_tests
                   book03_renderer_gallery book03_renderer_tests
                   book03_renderer book03_renderer_frontend_tests
                   book03_lens book03_lens_tests
                   book03_distortion book03_distortion_tests
                   book03_sensor book03_sensor_tests)
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endforeach()
endif()

include(CTest)
if(BUILD_TESTING)
    add_test(NAME book03_pixel_canvas_contracts COMMAND book03_canvas_tests)
    add_test(NAME book03_affine_frame_contracts COMMAND book03_frame_tests)
    add_test(NAME book03_triangle_geometry_contracts COMMAND book03_triangle_tests)
    add_test(NAME book03_affine_transform_contracts COMMAND book03_affine_tests)
    add_test(NAME book03_binary64_exact_orientation_contracts COMMAND book03_exact_tests)
    add_test(NAME book03_3d_geometry_ray_plane_sphere_contracts COMMAND book03_space_tests)
    add_test(NAME book03_mesh_topology_incidence_and_manifold_contracts COMMAND book03_mesh_tests)
    add_test(NAME book03_halfedge_cycle_and_twin_contracts COMMAND book03_halfedge_tests)
    add_test(NAME book03_curve_spline_tessellation_and_patch_contracts COMMAND book03_curves_tests)
    add_test(NAME book03_procedural_heightfield_sphere_cylinder_isosurface_contracts COMMAND book03_world_tests)
    add_test(NAME book03_obj_corner_domains_and_gltf_bounds_contracts COMMAND book03_asset_tests)
    add_test(NAME book03_affine_scene_forest_contracts COMMAND book03_scene_tests)
    add_test(NAME book03_rotation_and_rigid_pose_contracts COMMAND book03_rotation_tests)
    add_test(NAME book03_calibrated_pinhole_contracts COMMAND book03_camera_tests)
    add_test(NAME book03_projection_and_depth_contracts COMMAND book03_projection_tests)
    add_test(NAME book03_calibrated_stereo_contracts COMMAND book03_stereo_tests)
    add_test(NAME book03_indexed_vertex_and_varying_contracts COMMAND book03_vertex_tests)
    add_test(NAME book03_primitive_clipping_contracts COMMAND book03_clip_tests)
    add_test(NAME book03_integer_raster_coverage_contracts COMMAND book03_raster_tests)
    add_test(NAME book03_opaque_visibility_contracts COMMAND book03_visibility_tests)
    add_test(NAME book03_sampling_and_quadrature_contracts COMMAND book03_sampling_tests)
    add_test(NAME book03_renderer_backend_contracts COMMAND book03_renderer_tests)
    add_test(NAME book03_renderer_frontend_contracts COMMAND book03_renderer_frontend_tests)
    add_test(NAME book03_thin_lens_contracts COMMAND book03_lens_tests)
    add_test(NAME book03_distortion_contracts COMMAND book03_distortion_tests)
    add_test(NAME book03_sensor_contracts COMMAND book03_sensor_tests)
endif()
