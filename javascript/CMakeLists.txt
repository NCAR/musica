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
file(GLOB_RECURSE SOURCE_FILES "src/*.cpp")

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

# Memory leak detection with sanitizers
# Enable with: cmake -DENABLE_SANITIZERS=ON
option(ENABLE_SANITIZERS "Enable AddressSanitizer and LeakSanitizer for memory leak detection" OFF)
if(ENABLE_SANITIZERS)
    message(STATUS "MEMORY SANITIZERS ENABLED")
    message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
    message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")

    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        # Common sanitizer flags for all platforms
        set(SANITIZER_FLAGS
            -fsanitize=address
            -fsanitize=undefined
            -fno-omit-frame-pointer
            -g
            -O1
        )

        # Platform-specific configuration
        if(WIN32)
            # Windows with Clang
            message(STATUS "  - AddressSanitizer (ASan): Detects memory errors")
            message(STATUS "  - UndefinedBehaviorSanitizer (UBSan): Detects undefined behavior")
            message(STATUS "")
            message(STATUS "Note: LeakSanitizer not available on Windows")
        elseif(APPLE)
            # macOS - ASan includes leak detection
            message(STATUS "  - AddressSanitizer (ASan): Detects memory errors & leaks")
            message(STATUS "  - UndefinedBehaviorSanitizer (UBSan): Detects undefined behavior")
            message(STATUS "")
            message(STATUS "Note: On macOS, leak detection is included in AddressSanitizer")
        else()
            # Linux - full support for all sanitizers
            message(STATUS "  - AddressSanitizer (ASan): Detects memory errors")
            message(STATUS "  - LeakSanitizer (LSan): Detects memory leaks")
            message(STATUS "  - UndefinedBehaviorSanitizer (UBSan): Detects undefined behavior")
            list(APPEND SANITIZER_FLAGS -fsanitize=leak)
        endif()

        target_compile_options(${PROJECT_NAME} PRIVATE ${SANITIZER_FLAGS})
        target_link_options(${PROJECT_NAME} PRIVATE ${SANITIZER_FLAGS})

        # Add compile-time definition for pragma message
        target_compile_definitions(${PROJECT_NAME} PRIVATE SANITIZERS_ENABLED=1)

    elseif(MSVC)
        # MSVC has limited sanitizer support
        message(STATUS "  - AddressSanitizer (ASan): Detects memory errors")
        message(STATUS "")
        message(STATUS "Note: MSVC supports AddressSanitizer only")
        message(STATUS "      Use /fsanitize=address for MSVC")
        message(STATUS "")
        target_compile_options(${PROJECT_NAME} PRIVATE /fsanitize=address)
        target_compile_definitions(${PROJECT_NAME} PRIVATE SANITIZERS_ENABLED=1)
    else()
        message(WARNING "Sanitizers not supported with ${CMAKE_CXX_COMPILER_ID}")
        message(STATUS "")
    endif()
endif()

# macOS specific settings
if(APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        LINK_FLAGS "-undefined dynamic_lookup"
    )
endif()
