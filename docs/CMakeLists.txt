# Based of off this blog post
# https://devblogs.microsoft.com/cppblog/clear-functional-c-documentation-with-sphinx-breathe-doxygen-cmake/
# 

################################################################################
# Doxygen

# Find all the public headers
get_target_property(PUBLIC_HEADER_DIR musica INTERFACE_INCLUDE_DIRECTORIES)
file(GLOB_RECURSE PUBLIC_HEADERS ${PUBLIC_HEADER_DIR}/*.h)

#This will be the main output of our command
set(DOXYGEN_INPUT_DIR "${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/src")
set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/doxygen)
set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIR}/xml/index.xml)
set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
set(DOXY_INCLUDE_STRIP ${PROJECT_SOURCE_DIR}/include)

#Replace variables inside @@ with the current values
configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

file(MAKE_DIRECTORY ${DOXYGEN_OUTPUT_DIR}) #Doxygen won't create this for us

add_custom_command(
  OUTPUT ${DOXYGEN_INDEX_FILE} __fake_file_to_ensure_this_always_run
  DEPENDS ${PUBLIC_HEADERS}
  COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
  MAIN_DEPENDENCY ${DOXYFILE_OUT} ${DOXYFILE_IN}
  COMMENT "Generating docs"
  VERBATIM
)

add_custom_target(Doxygen ALL DEPENDS ${DOXYGEN_INDEX_FILE})

################################################################################
# Sphinx

set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/source)
set(SPHINX_BUILD ${CMAKE_CURRENT_BINARY_DIR}/sphinx)
set(SPHINX_INDEX_FILE ${SPHINX_BUILD}/index.html)

add_custom_command(
  OUTPUT ${SPHINX_INDEX_FILE} __fake_file_to_ensure_this_always_run
  COMMAND 
    ${SPHINX_EXECUTABLE} -b html
    # Tell Breathe where to find the Doxygen output
    -Dbreathe_projects.musica=${DOXYGEN_OUTPUT_DIR}/xml
  ${SPHINX_SOURCE} ${SPHINX_BUILD}
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  MAIN_DEPENDENCY ${SPHINX_SOURCE}/conf.py
  COMMENT "Generating documentation with Sphinx"
)

add_custom_target(docs ALL DEPENDS ${SPHINX_INDEX_FILE} Doxygen)

