cmake_minimum_required(VERSION 3.23)

# This module deliberately builds without the platform-dependent KairoNES GUI.
# A C++23 compiler and standard library are sufficient for NES-01.
project(AllMight_NES_01 LANGUAGES CXX)

# Public C++23 usage requirements for all independent NES course clients.
add_library(nes_course_public_api INTERFACE)
target_include_directories(nes_course_public_api INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../tools/labs"
)
target_compile_features(nes_course_public_api INTERFACE cxx_std_23)

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

# Pass07: real multi-translation-unit CPU debugger with public PIMPL.
add_executable(nes_cpu_debugger_cpp23_lab
    ../../../tools/labs/nes_cpu_debugger_cpp23_lab.cpp
    ../../../tools/labs/nes_course/cpu_debug_session.cpp
)
target_compile_features(nes_cpu_debugger_cpp23_lab PRIVATE cxx_std_23)
set_target_properties(nes_cpu_debugger_cpp23_lab PROPERTIES CXX_EXTENSIONS OFF)

# Pass08: real CPU host tasks; GPU operations are planning-only.
add_executable(nes_work_dependency_graph_cpp23_lab
    ../../../tools/labs/nes_work_dependency_graph_cpp23_lab.cpp
    ../../../tools/labs/nes_course/cpu_debug_session.cpp
)
target_compile_features(nes_work_dependency_graph_cpp23_lab PRIVATE cxx_std_23)
set_target_properties(nes_work_dependency_graph_cpp23_lab PROPERTIES CXX_EXTENSIONS OFF)

# Pass09: CPU-only device-memory oracle and host atomic release/acquire.
add_executable(nes_gpu_memory_contract_cpp23_lab
    ../../../tools/labs/nes_gpu_memory_contract_cpp23_lab.cpp
)
target_compile_features(nes_gpu_memory_contract_cpp23_lab PRIVATE cxx_std_23)
set_target_properties(nes_gpu_memory_contract_cpp23_lab PROPERTIES CXX_EXTENSIONS OFF)

# The Pass10 correctness self-test has no speed threshold; timing is opt-in.
add_executable(nes_reproducible_benchmark_cpp23_lab
    ../../../tools/labs/nes_reproducible_benchmark_cpp23_lab.cpp
)
target_compile_features(nes_reproducible_benchmark_cpp23_lab PRIVATE cxx_std_23)
set_target_properties(nes_reproducible_benchmark_cpp23_lab PROPERTIES CXX_EXTENSIONS OFF)

# Pass11: single-threaded lifetime-guarded CPU frame ring, with an actual
# separately compiled NES CPU debugger. No Vulkan/Metal device is created.
add_executable(nes_scoped_frame_upload_cpp23_lab
    ../../../tools/labs/nes_scoped_frame_upload_cpp23_lab.cpp
    ../../../tools/labs/nes_course/cpu_debug_session.cpp
)
target_compile_features(nes_scoped_frame_upload_cpp23_lab PRIVATE cxx_std_23)
set_target_properties(nes_scoped_frame_upload_cpp23_lab PROPERTIES CXX_EXTENSIONS OFF)
# Pass12: command lifecycle, CPU-only, reuses real PIMPL debugger.
add_executable(nes_command_lifecycle_cpp23_lab
 ../../../tools/labs/nes_command_lifecycle_cpp23_lab.cpp
 ../../../tools/labs/nes_course/cpu_debug_session.cpp
)
target_compile_features(nes_command_lifecycle_cpp23_lab PRIVATE cxx_std_23)
set_target_properties(nes_command_lifecycle_cpp23_lab PROPERTIES CXX_EXTENSIONS OFF)

# Pass13 cross-project GPU contracts, optional real Vulkan probe.
add_executable(gpu_device_resource_cpp23_lab
 ../../../tools/labs/gpu_device_resource_cpp23_lab.cpp
 ../../../tools/labs/gpu_course/native_vulkan_probe.cpp
)
target_compile_features(gpu_device_resource_cpp23_lab PRIVATE cxx_std_23)
set_target_properties(gpu_device_resource_cpp23_lab PROPERTIES CXX_EXTENSIONS OFF)
find_package(Vulkan QUIET)
if(Vulkan_FOUND AND TARGET Vulkan::Vulkan)
 target_link_libraries(gpu_device_resource_cpp23_lab PRIVATE Vulkan::Vulkan)
 target_compile_definitions(gpu_device_resource_cpp23_lab PRIVATE ALLMIGHT_COURSE_HAS_VULKAN=1)
endif()

# Pass14 is Kairo Vulkan/Maveb Metal/physics/XR oriented, not NES-only.
# Portable CTest models frame/fence/WSI/resource retirement. The separate
# --probe-native path actually submits vkCmdFillBuffer and checks fence/readback
# only if Vulkan headers/library are available and explicitly requested.
add_executable(gpu_submit_retire_cpp23_lab
    ../../../tools/labs/gpu_submit_retire_cpp23_lab.cpp
    ../../../tools/labs/gpu_course/native_vulkan_submit_probe.cpp
)
target_compile_features(gpu_submit_retire_cpp23_lab PRIVATE cxx_std_23)
set_target_properties(gpu_submit_retire_cpp23_lab PROPERTIES CXX_EXTENSIONS OFF)
if(Vulkan_FOUND AND TARGET Vulkan::Vulkan)
    target_link_libraries(gpu_submit_retire_cpp23_lab PRIVATE Vulkan::Vulkan)
    target_compile_definitions(gpu_submit_retire_cpp23_lab PRIVATE ALLMIGHT_COURSE_HAS_VULKAN=1)
endif()
find_package(Threads REQUIRED)
target_link_libraries(nes_gpu_memory_contract_cpp23_lab PRIVATE Threads::Threads)
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
    target_compile_options(nes_cpu_debugger_cpp23_lab PRIVATE /W4)
    target_compile_options(nes_work_dependency_graph_cpp23_lab PRIVATE /W4)
    target_compile_options(nes_gpu_memory_contract_cpp23_lab PRIVATE /W4)
    target_compile_options(nes_reproducible_benchmark_cpp23_lab PRIVATE /W4)
    target_compile_options(nes_scoped_frame_upload_cpp23_lab PRIVATE /W4)
    target_compile_options(nes_command_lifecycle_cpp23_lab PRIVATE /W4)
    target_compile_options(gpu_device_resource_cpp23_lab PRIVATE /W4)
    target_compile_options(gpu_submit_retire_cpp23_lab PRIVATE /W4)
else()
    foreach(lab IN ITEMS ines_header_cpp23_lab nes_bus_nrom_cpp23_lab nes_cartridge_transaction_cpp23_lab nes_ownership_proofs_cpp23_lab typed_asset_pipeline_cpp23_lab generational_asset_registry_cpp23_lab nes_cpu_debugger_cpp23_lab nes_work_dependency_graph_cpp23_lab nes_gpu_memory_contract_cpp23_lab nes_reproducible_benchmark_cpp23_lab nes_scoped_frame_upload_cpp23_lab nes_command_lifecycle_cpp23_lab gpu_device_resource_cpp23_lab gpu_submit_retire_cpp23_lab)
        target_compile_options(${lab} PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
    endforeach()
endif()


# All labs consume one explicit public include/language contract.
foreach(course_lab IN ITEMS
 ines_header_cpp23_lab nes_bus_nrom_cpp23_lab
 nes_cartridge_transaction_cpp23_lab nes_ownership_proofs_cpp23_lab
 typed_asset_pipeline_cpp23_lab generational_asset_registry_cpp23_lab
 nes_cpu_debugger_cpp23_lab nes_work_dependency_graph_cpp23_lab
 nes_gpu_memory_contract_cpp23_lab nes_reproducible_benchmark_cpp23_lab
 nes_scoped_frame_upload_cpp23_lab nes_command_lifecycle_cpp23_lab
 gpu_device_resource_cpp23_lab gpu_submit_retire_cpp23_lab)
    target_link_libraries(${course_lab} PRIVATE nes_course_public_api)
endforeach()

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

add_test(NAME nes_07c_6502_subset_debugger_and_data_structures
         COMMAND nes_cpu_debugger_cpp23_lab)

add_test(NAME nes_08g_dependency_hazards_and_conceptual_gpu_work
         COMMAND nes_work_dependency_graph_cpp23_lab)

add_test(NAME nes_09m_memory_visibility_and_host_atomic_ordering
         COMMAND nes_gpu_memory_contract_cpp23_lab)

add_test(NAME nes_10r_reproducible_correctness_and_measurement_protocol
         COMMAND nes_reproducible_benchmark_cpp23_lab)

add_test(NAME nes_11b_const_borrow_guarded_frame_upload_and_retirement
         COMMAND nes_scoped_frame_upload_cpp23_lab)

add_test(NAME nes_12q_command_submission_uncertainty_and_lifetimes
 COMMAND nes_command_lifecycle_cpp23_lab)

# Native Vulkan probing is separately opt-in, never a default CTest pass.
add_test(NAME gpu_13d_cross_project_device_memory_contracts_cpu_only
 COMMAND gpu_device_resource_cpp23_lab)

# Never execute native Vulkan in the general CTest matrix: hardware
# success is a separately documented and explicitly requested experiment.
add_test(NAME gpu_14f_kairo_maveb_submission_retirement_cpu_only
         COMMAND gpu_submit_retire_cpp23_lab)
