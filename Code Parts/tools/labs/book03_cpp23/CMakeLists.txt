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
                   book03_rotation book03_rotation_tests)
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
endif()
