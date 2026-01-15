project(musica-julia)

# Find Julia executable
find_program(Julia_EXECUTABLE julia REQUIRED)

if(NOT Julia_EXECUTABLE)
    message(FATAL_ERROR "Julia executable not found. Please install Julia.")
endif()

# Get Julia version to check compatibility
execute_process(
    COMMAND ${Julia_EXECUTABLE} --version
    OUTPUT_VARIABLE Julia_VERSION_STRING
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
message(STATUS "Found Julia: ${Julia_EXECUTABLE} (version: ${Julia_VERSION_STRING})")

# Get Julia's include directory
execute_process(
    COMMAND ${Julia_EXECUTABLE} -e "print(joinpath(Sys.BINDIR, Base.DATAROOTDIR, \"julia\"))"
    OUTPUT_VARIABLE Julia_INCLUDE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE Julia_INCLUDE_RESULT
)

if(NOT Julia_INCLUDE_RESULT EQUAL 0 OR NOT EXISTS ${Julia_INCLUDE_DIR})
    message(FATAL_ERROR "Failed to determine Julia include directory")
endif()

message(STATUS "Julia include directory: ${Julia_INCLUDE_DIR}")

# Try to find JlCxx through CxxWrap
# First ensure the project dependencies are installed
execute_process(
    COMMAND ${Julia_EXECUTABLE} --project=${CMAKE_CURRENT_SOURCE_DIR} -e "using Pkg; Pkg.instantiate()"
    RESULT_VARIABLE PKG_INSTANTIATE_RESULT
    OUTPUT_VARIABLE PKG_INSTANTIATE_OUTPUT
    ERROR_VARIABLE PKG_INSTANTIATE_ERROR
)

if(NOT PKG_INSTANTIATE_RESULT EQUAL 0)
    message(WARNING "Failed to instantiate Julia project dependencies: ${PKG_INSTANTIATE_ERROR}")
endif()

# Try to get CxxWrap prefix path
execute_process(
    COMMAND ${Julia_EXECUTABLE} --project=${CMAKE_CURRENT_SOURCE_DIR} -e "using CxxWrap; print(CxxWrap.prefix_path())"
    OUTPUT_VARIABLE CxxWrap_PREFIX
    RESULT_VARIABLE CxxWrap_RESULT
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_VARIABLE CxxWrap_ERROR
)

if(CxxWrap_RESULT EQUAL 0 AND EXISTS ${CxxWrap_PREFIX})
    message(STATUS "CxxWrap prefix: ${CxxWrap_PREFIX}")
    list(APPEND CMAKE_PREFIX_PATH ${CxxWrap_PREFIX})
    find_package(JlCxx REQUIRED)
else()
    message(FATAL_ERROR 
        "Failed to find CxxWrap.jl. Error: ${CxxWrap_ERROR}\n"
        "Please ensure CxxWrap is properly installed:\n"
        "  cd ${CMAKE_CURRENT_SOURCE_DIR}\n"
        "  julia --project=. -e 'using Pkg; Pkg.instantiate()'\n"
        "  julia --project=. -e 'using CxxWrap'\n"
        "\nNote: Julia 1.6-1.10 are supported. Julia 1.11+ has compatibility issues with CxxWrap 0.16.x"
    )
endif()

# Create the Julia wrapper library
add_library(musica_julia SHARED
    bindings/musica_julia.cpp
)

target_compile_features(musica_julia PUBLIC cxx_std_20)

# Link against MUSICA and JlCxx
target_link_libraries(musica_julia
    musica::musica
    JlCxx::cxxwrap_julia
)

target_include_directories(musica_julia PRIVATE
    ${Julia_INCLUDE_DIR}
)

# Set output properties
set_target_properties(musica_julia PROPERTIES
    PREFIX ""
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib"
)

# Set RPATH for the library
if (APPLE)
  set_target_properties(musica_julia PROPERTIES
    INSTALL_RPATH "@loader_path"
    BUILD_WITH_INSTALL_RPATH TRUE
  )
elseif(UNIX)
  set_target_properties(musica_julia PROPERTIES
    INSTALL_RPATH "$ORIGIN"
    BUILD_WITH_INSTALL_RPATH TRUE
  )
endif()

# Create symlink or copy to source tree for development/testing
# This allows Julia to find the library without installation
add_custom_command(TARGET musica_julia POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/deps/lib"
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:musica_julia>" "${CMAKE_CURRENT_SOURCE_DIR}/deps/lib/"
    COMMENT "Copying musica_julia library to source tree for development"
)

# Install the library and Julia package files
if(MUSICA_ENABLE_INSTALL)
  install(TARGETS musica_julia
          LIBRARY DESTINATION julia/deps/lib
          ARCHIVE DESTINATION julia/deps/lib)
  
  # Install Julia source files
  install(DIRECTORY src/
          DESTINATION julia/src
          FILES_MATCHING PATTERN "*.jl")
  
  # Install Julia package files
  install(FILES Project.toml README.md
          DESTINATION julia)
  
  # Install test files
  install(DIRECTORY test/
          DESTINATION julia/test
          FILES_MATCHING PATTERN "*.jl")
endif()
