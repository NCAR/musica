project(musica-addon)

# Set C++ standard (required for MUSICA headers)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Check if we're building with Emscripten for WASM
if(EMSCRIPTEN)
    # Building WASM module
    add_executable(musica-wasm src/musica_wasm.cpp)
    
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
        "SHELL:-s EXPORT_NAME='createMusicaModule'"
        "SHELL:-s EXPORTED_RUNTIME_METHODS=['ccall','cwrap']"
        "SHELL:-s ALLOW_MEMORY_GROWTH=1"
        "SHELL:--bind"
    )
else()
    # Building Node.js native addon
    # Get the node-addon-api include directory and clean it
    execute_process(
         COMMAND node -p "require('node-addon-api').include"
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
         OUTPUT_VARIABLE NODE_ADDON_API_DIR_RAW
         OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    
    # Remove quotes and clean the path
    string(REPLACE "\"" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR_RAW})
    string(STRIP ${NODE_ADDON_API_DIR} NODE_ADDON_API_DIR)
    
    # Create the addon library
    add_library(${PROJECT_NAME} SHARED)
    
    target_sources(
        ${PROJECT_NAME}
        PRIVATE
            ${CMAKE_JS_SRC}
            src/musica_addon.cpp
            src/micm/state_wrapper.cpp
            src/micm/micm_wrapper.cpp
            src/micm/solver_result_wrapper.cpp
            src/micm/state.cpp
            src/micm/micm.cpp
    )
    
    # Set target properties
    set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
    
    target_link_libraries(${PROJECT_NAME}
        ${CMAKE_JS_LIB}
        musica::musica
    )
    
    # Compiler definitions
    target_compile_definitions(${PROJECT_NAME} PRIVATE NAPI_VERSION=8)
    target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_JS_INC} ${NODE_ADDON_API_DIR})
    
    # macOS specific settings
    if(APPLE)
        set_target_properties(${PROJECT_NAME} PROPERTIES
            LINK_FLAGS "-undefined dynamic_lookup"
        )
    endif()
endif()
