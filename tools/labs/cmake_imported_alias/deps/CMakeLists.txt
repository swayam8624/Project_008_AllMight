# STATIC describes the external archive; IMPORTED means CMake will not build it.
add_library(Vendor::Math STATIC IMPORTED)
set_target_properties(Vendor::Math PROPERTIES
  IMPORTED_LOCATION "${VENDOR_MATH_LIBRARY}"
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/include"
  INTERFACE_COMPILE_DEFINITIONS "VENDOR_MATH_IMPORTED=1"
)

# Alias supplies this project with a stable name. It remains the same target.
add_library(Project::Math ALIAS Vendor::Math)
# An alias does not copy properties. Later changes to Vendor::Math are
# observed through Project::Math too; modify the real target, not the alias.

# Both imported target and its alias are visible in this child directory.
add_subdirectory(consumer)
