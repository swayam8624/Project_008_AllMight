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

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    foreach(target book03_canvas book03_canvas_tests
                   book03_frames book03_frame_tests
                   book03_triangles book03_triangle_tests
                   book03_transforms book03_affine_tests
                   book03_precision book03_exact_tests
                   book03_space book03_space_tests
                   book03_mesh book03_mesh_tests
                   book03_halfedge_tests)
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
endif()
