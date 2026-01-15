project(musica-julia)

# Find Julia and CxxWrap
find_package(Julia REQUIRED)
find_package(JlCxx REQUIRED)

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
    ${Julia_INCLUDE_DIRS}
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
