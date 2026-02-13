CMake Configuration Options
===========================

This section describes the CMake configuration options available when building MUSICA from source.
These options can be set when running CMake using the ``-D`` flag, for example:

.. code-block:: bash

    cmake -D MUSICA_CREATE_ENVIRONMENT_MODULE=ON ..

Environment Module Generation
-----------------------------

MUSICA supports generating an `Lmod <https://lmod.readthedocs.io/>`_ environment module file,
which is useful for HPC environments where module systems are commonly used to manage software.

.. list-table:: Environment Module Options
   :widths: 40 15 45
   :header-rows: 1

   * - Option
     - Default
     - Description
   * - ``MUSICA_CREATE_ENVIRONMENT_MODULE``
     - OFF
     - When enabled, generates an Lmod module file during the build process.
   * - ``MUSICA_INSTALL_MODULE_FILE_PATH``
     - (not set)
     - Specifies the directory where the module file should be installed.

Usage
^^^^^

To enable module file generation, configure MUSICA with:

.. code-block:: bash

    cmake -D MUSICA_CREATE_ENVIRONMENT_MODULE=ON ..

To also specify where the module file should be installed (typically your site's module path):

.. code-block:: bash

    cmake -D MUSICA_CREATE_ENVIRONMENT_MODULE=ON \
          -D MUSICA_INSTALL_MODULE_FILE_PATH=/path/to/modulefiles \
          -D CMAKE_INSTALL_PREFIX=/path/to/install \
          ..

After building and installing, the module file will be installed to
``<MUSICA_INSTALL_MODULE_FILE_PATH>/musica/<version>.lua``.

The generated module file sets the following environment variables:

- ``musica_ROOT`` / ``MUSICA_ROOT``: Base installation directory
- ``MUSICA_INC``: Path to include files
- ``MUSICA_LIB``: Path to library files
- ``MUSICA_FORTRAN_INC``: Path to Fortran module files
- ``PKG_CONFIG_PATH``: Updated to include MUSICA's pkgconfig directory

Example: Loading the Module
^^^^^^^^^^^^^^^^^^^^^^^^^^^

Once installed, users can load the MUSICA module using:

.. code-block:: bash

    module load musica/<version>

This makes MUSICA available for use in CMake projects via ``find_package``:

.. code-block:: cmake

    find_package(musica REQUIRED)
    target_link_libraries(my_target musica::musica)
