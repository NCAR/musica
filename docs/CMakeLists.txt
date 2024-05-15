################################################################################
# Sphinx

set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/source)
set(SPHINX_BUILD ${CMAKE_CURRENT_BINARY_DIR}/sphinx)
set(SPHINX_INDEX_FILE ${SPHINX_BUILD}/index.html)

add_custom_command(
  OUTPUT ${SPHINX_INDEX_FILE} __fake_file_to_ensure_this_always_run
  COMMAND 
    ${SPHINX_EXECUTABLE} -b html
  ${SPHINX_SOURCE} ${SPHINX_BUILD}
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  MAIN_DEPENDENCY ${SPHINX_SOURCE}/conf.py
  COMMENT "Generating documentation with Sphinx"
  # run this command each time. This way we don't have to keep track of each individual rst file
  BuildDocs
)

add_custom_target(docs ALL DEPENDS ${SPHINX_INDEX_FILE})

