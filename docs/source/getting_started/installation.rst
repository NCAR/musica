Installation
============

MUSICA source code may be cloned from its public GitHub repository
and configured and built with the cmake utility.
In brief:

.. code-block:: console
 
    $ git clone https://github.com/NCAR/musica.git
    $ cd musica
    $ mkdir build
    $ cd build
    $ cmake ..
    $ make


.. code-block:: console

 BUILD_DOCS                       OFF
 BUILD_GMOCK                      OFF
 BUILD_TESTING                    ON
 CMAKE_BUILD_TYPE                 Release
 CMAKE_INSTALL_PREFIX             /usr/local
 CREATE_ENVIRONMENT_MODULE        OFF
 ENABLE_COVERAGE                  OFF
 ENABLE_MEMCHECK                  OFF
 ENABLE_MICM                      ON
 ENABLE_MPI                       OFF
 ENABLE_NC_CONFIG                 OFF
 ENABLE_TESTS                     ON
 ENABLE_TUVX                      ON
 FETCHCONTENT_BASE_DIR            <MUSICA_DIR>/MUSICA/build/_deps
 FETCHCONTENT_FULLY_DISCONNECTE   OFF
 FETCHCONTENT_QUIET               OFF
 FETCHCONTENT_SOURCE_DIR_GOOGLE   
 FETCHCONTENT_SOURCE_DIR_JSON     
 FETCHCONTENT_SOURCE_DIR_MICM     
 FETCHCONTENT_SOURCE_DIR_PYBIND   
 FETCHCONTENT_SOURCE_DIR_TUVX     
 FETCHCONTENT_SOURCE_DIR_YAML-C   
 FETCHCONTENT_UPDATES_DISCONNEC   OFF
 GIT_SUBMODULE                    ON
 GTEST_HAS_ABSL                   OFF
 INSTALL_GTEST                    OFF
 JSON_BuildTests                  OFF
 JSON_CI                          OFF
 JSON_Diagnostics                 OFF
 JSON_DisableEnumSerialization    OFF
 JSON_GlobalUDLs                  ON
 JSON_INCLUDE_DIR                 JSON_INCLUDE_DIR-NOTFOUND
 JSON_ImplicitConversions         ON
 JSON_Install                     OFF
 JSON_LIB                         JSON_LIB-NOTFOUND
 JSON_LegacyDiscardedValueCompa   OFF
 JSON_MultipleHeaders             ON
 JSON_SystemInclude               OFF
 MAKE_MUSICA_FORTRAN_INSTALLABL   ON
 MICM_BUILD_DOCS                  OFF
 MICM_DEFAULT_VECTOR_MATRIX_SIZ   4
 MICM_ENABLE_CLANG_TIDY           OFF
 MICM_ENABLE_COVERAGE             OFF
 MICM_ENABLE_CUDA                 OFF
 MICM_ENABLE_EXAMPLES             ON
 MICM_ENABLE_JSON                 ON
 MICM_ENABLE_LLVM                 OFF
 MICM_ENABLE_MEMCHECK             OFF
 MICM_ENABLE_MPI                  OFF
 MICM_ENABLE_OPENACC              OFF
 MICM_ENABLE_OPENMP               OFF
 MICM_ENABLE_TESTS                ON
 MICM_GPU_TYPE
 MUSICA_BUILD_C_CXX_INTERFACE     ON
 MUSICA_BUILD_DOCS                OFF
 MUSICA_BUILD_FORTRAN_INTERFACE   ON
 MUSICA_CREATE_ENVIRONMENT_MODU   OFF
 MUSICA_ENABLE_INSTALL            ON
 MUSICA_ENABLE_MEMCHECK           OFF
 MUSICA_ENABLE_MICM               ON
 MUSICA_ENABLE_MPI                OFF
 MUSICA_ENABLE_OPENMP             OFF
 MUSICA_ENABLE_PYTHON_LIBRARY     ON
 MUSICA_ENABLE_TESTS              ON
 MUSICA_ENABLE_TUVX               ON
 PYBIND11_DISABLE_HANDLE_TYPE_N   OFF
 PYBIND11_FINDPYTHON              OFF
 PYBIND11_INSTALL                 OFF
 PYBIND11_INTERNALS_VERSION
 PYBIND11_NOPYTHON                OFF
 PYBIND11_NUMPY_1_ONLY            OFF
 PYBIND11_PYTHONLIBS_OVERWRITE    ON
 PYBIND11_PYTHON_VERSION
 PYBIND11_SIMPLE_GIL_MANAGEMENT   OFF
 PYBIND11_TEST                    OFF
 USE_MUSICA                       ON
 USE_MUSICA_FORTRAN               ON
 YAML_BUILD_SHARED_LIBS           ON
 YAML_CPP_BUILD_CONTRIB           ON
 YAML_CPP_BUILD_TOOLS             ON
 YAML_CPP_CLANG_FORMAT_EXE        YAML_CPP_CLANG_FORMAT_EXE-NOTFOUND
 YAML_CPP_FORMAT_SOURCE           ON
 YAML_CPP_INSTALL                 OFF

