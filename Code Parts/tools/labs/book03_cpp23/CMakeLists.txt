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

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    foreach(target book03_canvas book03_canvas_tests)
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endforeach()
endif()

include(CTest)
if(BUILD_TESTING)
    add_test(NAME book03_pixel_canvas_contracts COMMAND book03_canvas_tests)
endif()
