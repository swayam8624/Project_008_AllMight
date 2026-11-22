if(NOT TARGET Vendor::Math OR NOT TARGET Project::Math)
  message(FATAL_ERROR "Imported target and alias must be visible below deps/")
endif()
add_executable(imported_consumer main.cpp)
# Linking the alias carries its location and include/definition usage requirements.
target_link_libraries(imported_consumer PRIVATE Project::Math)
