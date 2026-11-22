# This minimum selects the documented imported-alias directory-scope behavior.
cmake_minimum_required(VERSION 3.18)
# Enable and identify the host C++ toolchain. No dependency is downloaded.
project(ImportedAliasExample LANGUAGES CXX)

if(NOT DEFINED VENDOR_MATH_LIBRARY OR NOT EXISTS "${VENDOR_MATH_LIBRARY}")
  message(FATAL_ERROR "Set VENDOR_MATH_LIBRARY to the prebuilt vendor_math archive")
endif()

# The target is a build-graph description of a file produced elsewhere.
# It contributes no compile or archive rule of its own.
add_subdirectory(deps)
add_subdirectory(scope_probe)
