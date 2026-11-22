cmake_minimum_required(VERSION 3.23)
project(AllMightAcademicReferencePrograms LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
enable_testing()

add_executable(book01_tiny_cpu tiny_cpu.cpp)
add_executable(book01_expected_record expected_record.cpp)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  foreach(target IN ITEMS book01_tiny_cpu book01_expected_record)
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Werror)
  endforeach()
endif()

add_test(NAME book01_cpu_isa_and_invalid_opcodes COMMAND book01_tiny_cpu)
add_test(NAME book01_expected_and_error_channel COMMAND book01_expected_record)
