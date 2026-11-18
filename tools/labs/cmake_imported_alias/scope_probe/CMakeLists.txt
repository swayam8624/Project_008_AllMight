# The imported target was not GLOBAL; sibling directories cannot see it.
if(TARGET Vendor::Math OR TARGET Project::Math)
  message(FATAL_ERROR "non-GLOBAL imported target leaked outside deps/")
endif()
message(STATUS "PASS: non-GLOBAL imported target and alias are directory-scoped")
