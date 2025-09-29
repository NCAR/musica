project(musica-addon)

# Set C++ standard (required for MUSICA headers)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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


# Source files - using our MUSICA integrated addon
set(SOURCE_FILES
    src/musica_addon.cpp
    src/musica_wrapper.cpp
)

# Create the addon library
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${CMAKE_JS_SRC})

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
