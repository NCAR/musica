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
    target_compile_definitions(${target} PUBLIC -DMUSICA_USE_MPI)
  endif()

  if (MUSICA_ENABLE_OPENMP)
    target_compile_definitions(${target} PUBLIC -DMUSICA_USE_OPENMP)
  endif()

  if (MUSICA_ENABLE_MICM) 
    target_compile_definitions(${target} PUBLIC -DMUSICA_USE_MICM)
    target_link_libraries(${target}
      PUBLIC
        musica::micm
    )

    if(ARG_MODE STREQUAL "GPU")
        set_target_properties(micm_cuda PROPERTIES POSITION_INDEPENDENT_CODE ON)
      target_compile_definitions(${target} PUBLIC MUSICA_ENABLE_CUDA)
      target_link_libraries(${target} PUBLIC musica::micm_cuda)
    endif()
  endif()

  if (MUSICA_ENABLE_TUVX)
    target_sources(${target}
      PUBLIC
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

  silence_warnings(${target})
endfunction()

