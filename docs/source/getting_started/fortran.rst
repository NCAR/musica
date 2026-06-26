Fortran
=======

Installing MUSICA
-----------------

The MUSICA library and the MUSICA-Fortran interface can be installed using the CMake build system.

Local installation
~~~~~~~~~~~~~~~~~~

Install the required dependencies. On Fedora/RHEL:

.. code-block:: bash

   sudo dnf install -y cmake git gcc-c++ gcc-gfortran netcdf-devel netcdf-fortran-devel

On other distributions, install equivalent packages via your system package manager.

Clone and build:

.. code-block:: bash

   git clone https://github.com/NCAR/musica.git
   mkdir build && cd build
   cmake -D CMAKE_INSTALL_PREFIX=<INSTALL_DIR> \
         -D MUSICA_BUILD_FORTRAN_INTERFACE=ON \
         ../musica
   make
   make install

where ``<INSTALL_DIR>`` is the directory where you want to install MUSICA.

Docker
~~~~~~

Alternatively, use the provided Dockerfile:

.. code-block:: bash

   git clone https://github.com/NCAR/musica.git
   cd musica
   docker build -t musica-fortran -f docker/Dockerfile.fortran-gcc .
   docker run -it musica-fortran bash

The ``<MUSICA_DIR>`` inside the container is located at ``/musica/build``.

Next steps
----------

- :ref:`Fortran User Guide <fortran-tutorials>` — first program, box model, and multi-grid-cell examples
- :ref:`Development Setup <development-setup>` — set up for contributing to MUSICA
