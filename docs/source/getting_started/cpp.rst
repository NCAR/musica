C++
===

Installing MUSICA
-----------------

Prerequisites
~~~~~~~~~~~~~

On Fedora/RHEL:

.. code-block:: bash

   sudo dnf install -y cmake git gcc-c++ netcdf-devel

On Ubuntu/Debian:

.. code-block:: bash

   sudo apt-get install -y cmake git g++ libnetcdf-dev

Build and install
~~~~~~~~~~~~~~~~~

.. code-block:: bash

   git clone https://github.com/NCAR/musica.git
   cd musica
   mkdir build && cd build
   cmake -D CMAKE_INSTALL_PREFIX=<INSTALL_DIR> \
         -D MUSICA_ENABLE_MICM=ON \
         -D MUSICA_ENABLE_TUVX=ON \
         ..
   make -j
   make install

Docker
~~~~~~

A pre-built Docker image is also available:

.. code-block:: bash

   docker build -t musica-cpp -f docker/Dockerfile.mpi .
   docker run -it musica-cpp bash

Using MUSICA in a CMake project
--------------------------------

Once installed, link MUSICA into your project:

.. code-block:: cmake

   find_package(musica REQUIRED)
   target_link_libraries(my_target musica::musica)

If installed to a non-standard prefix, pass it via ``CMAKE_PREFIX_PATH``:

.. code-block:: bash

   cmake -D CMAKE_PREFIX_PATH=<INSTALL_DIR> ..

Next steps
----------

- :ref:`C++ User Guide <cpp-user-guide>` — usage examples and embedding guide
- :doc:`C++ API Reference <../api/C++>` — full API documentation
- :ref:`Development Setup <development-setup>` — set up for contributing to MUSICA
