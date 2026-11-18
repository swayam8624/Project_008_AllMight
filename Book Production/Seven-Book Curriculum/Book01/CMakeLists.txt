cmake_minimum_required(VERSION 3.23)

# This module deliberately builds without the platform-dependent KairoNES GUI.
# A C++23 compiler and standard library are sufficient for NES-01.
project(AllMight_NES_01 LANGUAGES CXX)

add_executable(ines_header_cpp23_lab
    ../../../tools/labs/ines_header_cpp23_lab.cpp
)
target_compile_features(ines_header_cpp23_lab PRIVATE cxx_std_23)

# NES-02 grows the same learning system: it consumes the NES-01 shared
# header-layout implementation and adds owned NROM storage plus CPU bus tests.
add_executable(nes_bus_nrom_cpp23_lab
    ../../../tools/labs/nes_bus_nrom_cpp23_lab.cpp
)
target_compile_features(nes_bus_nrom_cpp23_lab PRIVATE cxx_std_23)
set_target_properties(nes_bus_nrom_cpp23_lab PROPERTIES
    CXX_EXTENSIONS OFF
)
set_target_properties(ines_header_cpp23_lab PROPERTIES
    CXX_EXTENSIONS OFF
)

if(MSVC)
    target_compile_options(ines_header_cpp23_lab PRIVATE /W4)
    target_compile_options(nes_bus_nrom_cpp23_lab PRIVATE /W4)
else()
    foreach(lab IN ITEMS ines_header_cpp23_lab nes_bus_nrom_cpp23_lab)
        target_compile_options(${lab} PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
    endforeach()
endif()

enable_testing()
add_test(NAME nes_01_synthetic_cartridge_contract
         COMMAND ines_header_cpp23_lab)

add_test(NAME nes_02_owned_cartridge_and_cpu_bus
         COMMAND nes_bus_nrom_cpp23_lab)
