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

# NES-02T links the same parser and bus through a staged transactional runtime.
add_executable(nes_cartridge_transaction_cpp23_lab
    ../../../tools/labs/nes_cartridge_transaction_cpp23_lab.cpp
)
target_compile_features(nes_cartridge_transaction_cpp23_lab PRIVATE cxx_std_23)

# Pass 04: independent object-lifetime and transactional ownership witnesses.
add_executable(nes_ownership_proofs_cpp23_lab
    ../../../tools/labs/nes_ownership_proofs_cpp23_lab.cpp
)
target_compile_features(nes_ownership_proofs_cpp23_lab PRIVATE cxx_std_23)
set_target_properties(nes_ownership_proofs_cpp23_lab PROPERTIES CXX_EXTENSIONS OFF)

# Pass05: cartridge/texture/mesh through one typed publication protocol.
add_executable(typed_asset_pipeline_cpp23_lab
    ../../../tools/labs/typed_asset_pipeline_cpp23_lab.cpp
)
target_compile_features(typed_asset_pipeline_cpp23_lab PRIVATE cxx_std_23)
set_target_properties(typed_asset_pipeline_cpp23_lab PROPERTIES CXX_EXTENSIONS OFF)

# Pass06 builds on the SAME typed asset decoders and owned NROM records,
# adding generational reuse and owner-aware logical asset handles.
add_executable(generational_asset_registry_cpp23_lab
    ../../../tools/labs/generational_asset_registry_cpp23_lab.cpp
)
target_compile_features(generational_asset_registry_cpp23_lab PRIVATE cxx_std_23)
set_target_properties(generational_asset_registry_cpp23_lab PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(nes_cartridge_transaction_cpp23_lab PROPERTIES
    CXX_EXTENSIONS OFF
)
set_target_properties(nes_bus_nrom_cpp23_lab PROPERTIES
    CXX_EXTENSIONS OFF
)
set_target_properties(ines_header_cpp23_lab PROPERTIES
    CXX_EXTENSIONS OFF
)

if(MSVC)
    target_compile_options(ines_header_cpp23_lab PRIVATE /W4)
    target_compile_options(nes_bus_nrom_cpp23_lab PRIVATE /W4)
    target_compile_options(nes_cartridge_transaction_cpp23_lab PRIVATE /W4)
    target_compile_options(nes_ownership_proofs_cpp23_lab PRIVATE /W4)
    target_compile_options(typed_asset_pipeline_cpp23_lab PRIVATE /W4)
    target_compile_options(generational_asset_registry_cpp23_lab PRIVATE /W4)
else()
    foreach(lab IN ITEMS ines_header_cpp23_lab nes_bus_nrom_cpp23_lab nes_cartridge_transaction_cpp23_lab nes_ownership_proofs_cpp23_lab typed_asset_pipeline_cpp23_lab generational_asset_registry_cpp23_lab)
        target_compile_options(${lab} PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
    endforeach()
endif()

enable_testing()
add_test(NAME nes_01_synthetic_cartridge_contract
         COMMAND ines_header_cpp23_lab)

add_test(NAME nes_02_owned_cartridge_and_cpu_bus
         COMMAND nes_bus_nrom_cpp23_lab)

add_test(NAME nes_02t_transactional_cartridge_replacement
         COMMAND nes_cartridge_transaction_cpp23_lab)

add_test(NAME nes_02p_cpp23_ownership_and_failure_proofs
         COMMAND nes_ownership_proofs_cpp23_lab)

add_test(NAME nes_05a_typed_asset_pipeline_and_failure_boundaries
         COMMAND typed_asset_pipeline_cpp23_lab)

add_test(NAME nes_06g_generational_asset_registry_identity_and_reuse
         COMMAND generational_asset_registry_cpp23_lab)
