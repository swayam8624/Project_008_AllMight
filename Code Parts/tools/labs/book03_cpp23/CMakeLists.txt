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

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    foreach(target book03_canvas book03_canvas_tests
                   book03_frames book03_frame_tests
                   book03_triangles book03_triangle_tests)
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endforeach()
endif()

include(CTest)
if(BUILD_TESTING)
    add_test(NAME book03_pixel_canvas_contracts COMMAND book03_canvas_tests)
    add_test(NAME book03_affine_frame_contracts COMMAND book03_frame_tests)
    add_test(NAME book03_triangle_geometry_contracts COMMAND book03_triangle_tests)
endif()
