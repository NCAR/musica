project(musica-julia)

get_target_property(JlCxx_location JlCxx::cxxwrap_julia LOCATION)
get_filename_component(JlCxx_location ${JlCxx_location} DIRECTORY)

# Create the Julia wrapper library
add_library(musica_julia SHARED
    bindings/musica_julia.cpp
)

target_compile_features(musica_julia PUBLIC cxx_std_20)

target_link_libraries(musica_julia
    musica::musica
    JlCxx::cxxwrap_julia
)

# Set output properties
set_target_properties(musica_julia PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/julia/deps/lib"
)

# Install the library and Julia package files
if(MUSICA_ENABLE_INSTALL)
    install(TARGETS musica_julia
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib
            RUNTIME DESTINATION lib
    )
endif()
