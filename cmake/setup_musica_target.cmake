function(musica_setup_target target)
  cmake_parse_arguments(ARG "" "MODE" "" ${ARGN})

  if (MUSICA_ENABLE_PYTHON_LIBRARY OR MUSICA_ENABLE_PIC)
    set_target_properties(${target_name} PROPERTIES POSITION_INDEPENDENT_CODE ON)
  endif()

  target_include_directories(${target}
    PUBLIC
      $<BUILD_INTERFACE:${MUSICA_PROJECT_SRC_DIR}/include>
      $<INSTALL_INTERFACE:${MUSICA_INSTALL_INCLUDE_DIR}>
  )

  target_compile_features(${target} PUBLIC cxx_std_20)

  target_link_libraries(${target}
    PUBLIC
      musica::mechanism_configuration
  )

  if (MUSICA_ENABLE_MPI)
    list(APPEND musica_compile_definitions MUSICA_USE_MPI)
  endif()

  if (MUSICA_ENABLE_OPENMP)
    list(APPEND musica_compile_definitions MUSICA_USE_OPENMP)
  endif()

  if (MUSICA_ENABLE_MICM) 
    list(APPEND musica_compile_definitions MUSICA_USE_MICM)
    target_link_libraries(${target}
      PUBLIC
        musica::micm
    )

    if(ARG_MODE STREQUAL "GPU")
      set_target_properties(micm_cuda PROPERTIES POSITION_INDEPENDENT_CODE ON)
      list(APPEND musica_compile_definitions MUSICA_ENABLE_CUDA)
      target_link_libraries(${target} PUBLIC musica::micm_cuda)
    endif()
  endif()

  if (MUSICA_ENABLE_TUVX)
    list(APPEND musica_compile_definitions MUSICA_USE_TUVX)
    target_sources(${target}
      PRIVATE
        $<TARGET_OBJECTS:tuvx_object>
    )
    target_link_libraries(${target}
      PUBLIC
        tuvx_object
    )
    target_include_directories(${target}
      PUBLIC
        $<BUILD_INTERFACE:${MUSICA_MOD_DIR}>
        $<INSTALL_INTERFACE:${MUSICA_INSTALL_INCLUDE_DIR}>
    )
  endif()

  if (MUSICA_ENABLE_CARMA)
    list(APPEND musica_compile_definitions MUSICA_USE_CARMA)
    target_sources(${target}
      PRIVATE
        $<TARGET_OBJECTS:carma_object>
    )
    target_link_libraries(${target}
      PUBLIC
        carma_object
    )
    target_include_directories(${target}
      PUBLIC
        $<BUILD_INTERFACE:${MUSICA_MOD_DIR}>
        $<INSTALL_INTERFACE:${MUSICA_INSTALL_INCLUDE_DIR}>
    )
    if (CMAKE_Fortran_COMPILER_ID STREQUAL "GNU")
      target_compile_options(carma_object PUBLIC $<$<COMPILE_LANGUAGE:Fortran>:-ffree-line-length-none>)
    endif()
  endif()


  target_compile_definitions(${target} PUBLIC ${musica_compile_definitions})
endfunction()

