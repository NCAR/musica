function(checkout_submodules)
  find_package(Git)
  if(GIT_FOUND)
    message(STATUS "Source dir: ${CMAKE_CURRENT_LIST_DIR}")
    if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/.git")
      # Update submodules as needed
      option(GIT_SUBMODULE "Check submodules during build" ON)
      if(GIT_SUBMODULE)
          message(STATUS "Submodule update")
          execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                          WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
                          RESULT_VARIABLE GIT_SUBMOD_RESULT)
          if(NOT GIT_SUBMOD_RESULT EQUAL "0")
              message(WARNING "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
          endif()
      endif()
    else()
      message(WARNING "Unable to find .git directory and cannot checkout submodules")
    endif()
  else()
    message(WARNING "Git was not found")
  endif()
endfunction(checkout_submodules)

function(combine_archives output_archive list_of_input_archives)
    set(mri_file ${CMAKE_BINARY_DIR}/${output_archive}.mri)
    set(FULL_OUTPUT_PATH ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/lib${output_archive}.a)
    file(WRITE ${mri_file} "create ${FULL_OUTPUT_PATH}\n")
    FOREACH(in_archive ${list_of_input_archives})
        file(APPEND ${mri_file} "addlib ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/lib${in_archive}.a\n")
    ENDFOREACH()
    file(APPEND ${mri_file} "save\n")
    file(APPEND ${mri_file} "end\n")

    set(output_archive_dummy_file ${CMAKE_BINARY_DIR}/${output_archive}.dummy)
    add_custom_command(OUTPUT ${output_archive_dummy_file}
                       COMMAND touch ${output_archive_dummy_file}
                       DEPENDS ${list_of_input_archives})

    add_library(${output_archive} STATIC ${output_archive_dummy_file})
    set_target_properties(${output_archive} PROPERTIES LINKER_LANGUAGE Fortran)
    add_custom_command(TARGET ${output_archive}
                       POST_BUILD
                       COMMAND ar -M < ${mri_file})
endfunction(combine_archives)