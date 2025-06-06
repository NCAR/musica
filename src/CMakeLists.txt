################################################################################
# Preamble

project(
  musica
  VERSION ${PROJECT_VERSION}
  LANGUAGES C CXX
)

message (STATUS "CMake build configuration for ${PROJECT_NAME} (${CMAKE_BUILD_TYPE}) ${PROJECT_VERSION}")

if(MUSICA_ENABLE_TUVX)
  enable_language(Fortran)
endif()

################################################################################
# MUSICA object libraries

include(silence_warnings)

function(setup_object_target target_name)
  set(musica_libraries
  )
  set(musica_compile_definitions)

  if(MUSICA_ENABLE_MPI)
    list(APPEND musica_compile_definitions -DMUSICA_USE_MPI)
  endif()

  if(MUSICA_ENABLE_OPENMP)
    list(APPEND musica_compile_definitions -DMUSICA_USE_OPENMP)
  endif()

  if (MUSICA_ENABLE_PYTHON_LIBRARY OR MUSICA_ENABLE_PIC)
    set_target_properties(${target_name} PROPERTIES POSITION_INDEPENDENT_CODE ON)
  endif()

  ####################
  # MICM
  if(MUSICA_ENABLE_MICM)
    list(APPEND musica_compile_definitions -DMUSICA_USE_MICM)

    target_link_libraries(${target_name}
      PUBLIC
        musica::micm
    )

    target_sources(${target_name}
      PRIVATE
        micm/micm.cpp
        micm/micm_c_interface.cpp
        micm/state.cpp
        micm/state_c_interface.cpp
        micm/parse.cpp
        micm/v0_parse.cpp
        micm/v1_parse.cpp
        micm/cuda_availability.cpp
    )

  endif()

  ####################
  # TUV-x

  if(MUSICA_ENABLE_TUVX)
    target_sources(${target_name}
      PUBLIC 
        $<TARGET_OBJECTS:tuvx_object>
      PRIVATE
        tuvx/interface.F90
        tuvx/interface_grid.F90
        tuvx/interface_grid_map.F90
        tuvx/interface_profile.F90
        tuvx/interface_profile_map.F90
        tuvx/interface_radiator.F90
        tuvx/interface_radiator_map.F90
        tuvx/interface_util.F90
        tuvx/grid.cpp
        tuvx/grid_map.cpp
        tuvx/profile.cpp
        tuvx/profile_map.cpp
        tuvx/radiator.cpp
        tuvx/radiator_map.cpp
        tuvx/tuvx.cpp
    )

    target_link_libraries(${target_name}
      PUBLIC
        tuvx_object
    )

    target_include_directories(${target_name}
      PUBLIC
        $<BUILD_INTERFACE:${MUSICA_MOD_DIR}>
        $<INSTALL_INTERFACE:${MUSICA_INSTALL_INCLUDE_DIR}>
    )
  endif()

  target_sources(${target_name}
    PRIVATE
      util.cpp
      component_versions.cpp
      ${CMAKE_BINARY_DIR}/version.cpp
  )

  target_include_directories(${target_name}
    PUBLIC
      $<BUILD_INTERFACE:${MUSICA_PROJECT_SRC_DIR}/include>
      $<INSTALL_INTERFACE:${MUSICA_INSTALL_INCLUDE_DIR}>
  )

  target_compile_features(${target_name} PUBLIC cxx_std_20)

  target_link_libraries(${target_name}
    PUBLIC
      musica::mechanism_configuration
  )

  target_compile_definitions(${target_name} PUBLIC ${musica_compile_definitions})

  silence_warnings(${target_name})
endfunction()

# version
configure_file(version.cpp.in ${CMAKE_BINARY_DIR}/version.cpp @ONLY)

add_library(musica_object OBJECT)
setup_object_target(musica_object)

if (NOT ${MUSICA_GPU_TYPE} STREQUAL "None")
  add_library(musica_object_cuda OBJECT)
  setup_object_target(musica_object_cuda)
    if (MUSICA_ENABLE_MICM)
      ####################
      # CUDA
      set_target_properties(micm_cuda PROPERTIES POSITION_INDEPENDENT_CODE ON)
      target_compile_definitions(musica_object_cuda PUBLIC MUSICA_ENABLE_CUDA)
      target_link_libraries(musica_object_cuda
        PUBLIC
          musica::micm_cuda
      )
    endif()
endif()

################################################################################
# MUSICA

add_library(musica)
add_library(musica::musica ALIAS musica)

if (NOT ${MUSICA_GPU_TYPE} STREQUAL "None" AND NOT MUSICA_ENABLE_PYTHON_LIBRARY)
  target_sources(musica
    PUBLIC
      $<TARGET_OBJECTS:musica_object_cuda>
  )
  target_link_libraries(musica
    PUBLIC
      musica_object_cuda
  )
else()
  target_sources(musica
    PUBLIC
      $<TARGET_OBJECTS:musica_object>
  )
  target_link_libraries(musica
    PUBLIC
      musica_object
  )
endif()

set_target_properties(musica PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${MUSICA_LIB_DIR}
  LIBRARY_OUTPUT_DIRECTORY ${MUSICA_LIB_DIR}
  Fortran_MODULE_DIRECTORY ${MUSICA_MOD_DIR}
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR}
)

# Set the rpath for the shared library
if(APPLE)
  set_target_properties(musica PROPERTIES
    INSTALL_RPATH "@loader_path"
  )
elseif(UNIX)
  set_target_properties(musica PROPERTIES
    INSTALL_RPATH "$ORIGIN"
    BUILD_WITH_INSTALL_RPATH TRUE
  )
endif()

################################################################################
# testing
if(MUSICA_ENABLE_TESTS)
  add_subdirectory(test)
endif()

################################################################################
# Packaging
if(MUSICA_ENABLE_INSTALL AND NOT MUSICA_ENABLE_PYTHON_LIBRARY)
  add_subdirectory(packaging)
endif()
