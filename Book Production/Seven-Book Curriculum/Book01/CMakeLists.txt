cmake_minimum_required(VERSION 3.23)

# This module deliberately builds without the platform-dependent KairoNES GUI.
# A C++23 compiler and standard library are sufficient for NES-01.
project(AllMight_NES_01 LANGUAGES CXX)

add_executable(ines_header_cpp23_lab
    ../../../tools/labs/ines_header_cpp23_lab.cpp
)
target_compile_features(ines_header_cpp23_lab PRIVATE cxx_std_23)
set_target_properties(ines_header_cpp23_lab PROPERTIES
    CXX_EXTENSIONS OFF
)

if(MSVC)
    target_compile_options(ines_header_cpp23_lab PRIVATE /W4)
else()
    target_compile_options(ines_header_cpp23_lab PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion
    )
endif()

enable_testing()
add_test(NAME nes_01_synthetic_cartridge_contract
         COMMAND ines_header_cpp23_lab)
