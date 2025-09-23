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

# # Remove quotes and clean the path
string(REPLACE "\"" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR_RAW})
string(STRIP ${NODE_ADDON_API_DIR} NODE_ADDON_API_DIR)

# # Include directories
# include_directories(${CMAKE_JS_INC})
# include_directories(${NODE_ADDON_API_DIR})

# Include MUSICA and component headers
#include_directories(/Users/jason.blvck/Documents/Courses/Fall25/CSCE482/musica/include)
#include_directories(/Users/jason.blvck/Documents/Courses/Fall25/CSCE482/musica/musica-js-addon/musica-deps/build/_deps/micm-src/include)
#include_directories(/Users/jason.blvck/Documents/Courses/Fall25/CSCE482/musica/musica-js-addon/musica-deps/build/_deps/yaml-cpp-src/include)
#include_directories(/Users/jason.blvck/Documents/Courses/Fall25/CSCE482/musica/musica-js-addon/musica-deps/build/_deps/mechanism_configuration-src/include)


# Source files - using our MUSICA integrated addon
set(SOURCE_FILES
    src/musica_addon.cpp
    src/musica_wrapper.cpp
)

# Create the addon library
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${CMAKE_JS_SRC})

# Set target properties
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")

# Link MUSICA libraries and dependencies
#set(MUSICA_BUILD_DIR "/Users/jason.blvck/Documents/Courses/Fall25/CSCE482/musica/build")

target_link_libraries(${PROJECT_NAME}
    ${CMAKE_JS_LIB}
    musica::musica
)

# Compiler definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE NAPI_VERSION=8)
target_compile_definitions(${PROJECT_NAME} PRIVATE NAPI_DISABLE_CPP_EXCEPTIONS)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_JS_INC} ${NODE_ADDON_API_DIR})

# macOS specific settings
if(APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        LINK_FLAGS "-undefined dynamic_lookup"
    )
endif()

# Debug output
# message(STATUS "Raw node-addon-api path: ${NODE_ADDON_API_DIR_RAW}")
# message(STATUS "Clean node-addon-api path: ${NODE_ADDON_API_DIR}")
# message(STATUS "Node.js include: ${CMAKE_JS_INC}")
# message(STATUS "Source files: ${SOURCE_FILES}")

# Verify that napi.h exists
# if(EXISTS "${NODE_ADDON_API_DIR}/napi.h")
#     message(STATUS "Found napi.h at: ${NODE_ADDON_API_DIR}/napi.h")
# else()
#     message(WARNING "napi.h not found at: ${NODE_ADDON_API_DIR}/napi.h")
    
#     # Try alternative approach - find it manually
#     find_path(NAPI_INCLUDE_DIR
#         NAMES napi.h
#         PATHS 
#             ${CMAKE_SOURCE_DIR}/node_modules/node-addon-api
#             ${CMAKE_SOURCE_DIR}/../node_modules/node-addon-api
#             /usr/local/include/node-addon-api
#         NO_DEFAULT_PATH
#     )
    
#     if(NAPI_INCLUDE_DIR)
#         message(STATUS "Found napi.h via find_path at: ${NAPI_INCLUDE_DIR}")
#         include_directories(${NAPI_INCLUDE_DIR})
#     else()
#         message(FATAL_ERROR "Could not find napi.h anywhere")
#     endif()
#endif()