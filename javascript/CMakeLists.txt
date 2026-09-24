# Building WASM module
add_executable(musica-wasm 
  bindings/musica_wasm.cpp
)

target_compile_features(musica-wasm PUBLIC cxx_std_20)

# Link against MUSICA
target_link_libraries(musica-wasm musica::musica)

# Use native WebAssembly exceptions. Unlike Emscripten's JavaScript-based exceptions, they add
# almost no cost when no exception is thrown. Each library in the module must use the same
# exception model, so the flag is set on MUSICA and on each dependency that it builds.
foreach(target IN ITEMS musica-wasm musica mechanism_configuration yaml-cpp fmt)
  if(TARGET ${target})
    target_compile_options(${target} PRIVATE -fwasm-exceptions)
  endif()
endforeach()
target_link_options(musica-wasm PRIVATE -fwasm-exceptions)

# Set output name and location
# NOTE: Writing to source directory is intentional for ease of use and
# to keep the WASM files alongside the JavaScript wrapper that uses them.
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
  "SHELL:-s ALLOW_MEMORY_GROWTH=1"
  "SHELL:-s FORCE_FILESYSTEM=1"
  "SHELL:-lnodefs.js"
  "SHELL:-s EXPORTED_RUNTIME_METHODS=['FS','NODEFS']"
  "SHELL:--bind"
)
