################################################################################
# Add subdirectories containing tests

add_subdirectory(connections)
add_subdirectory(unit)

if(ENABLE_CCPP)
  add_subdirectory(ccpp)
endif()