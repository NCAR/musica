project(musica-wasm)

# Set C++ standard (required for MUSICA headers)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Building WASM module
add_executable(musica-wasm 
    src/musica_wasm.cpp
    src/micm/state_wrapper.cpp
    src/micm/micm_wrapper.cpp
)

# Link against MUSICA
target_link_libraries(musica-wasm musica::musica)

# Set output name and location
# NOTE: Writing to source directory is intentional for ease of use and
# to keep the WASM files alongside the JavaScript wrapper that uses them.
# The generated files (musica.js, musica.wasm) are excluded via .gitignore.
set_target_properties(musica-wasm PROPERTIES
    OUTPUT_NAME "musica"
    SUFFIX ".js"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/wasm"
)

# Emscripten linker flags for generating WASM
target_link_options(musica-wasm PRIVATE
    "SHELL:-s WASM=1"
    "SHELL:-s MODULARIZE=1"
    "SHELL:-s EXPORT_ES6=1"
    "SHELL:-s EXPORT_NAME='createMusicaModule'"
    "SHELL:-s DISABLE_EXCEPTION_CATCHING=0"
    "SHELL:-s ALLOW_MEMORY_GROWTH=1"
    "SHELL:-s FORCE_FILESYSTEM=1"
    "SHELL:-lnodefs.js"
    "SHELL:-s EXPORTED_RUNTIME_METHODS=['FS','NODEFS']"
    "SHELL:--bind"
)
