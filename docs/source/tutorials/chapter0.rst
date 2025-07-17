Chapter 0
=========

.. _installing_musica:

Installing MUSICA
-----------------

This tutorial will guide you through the installation and use of MUSICA and
the MUSICA-Fortran interface. The MUSICA library is a C++ library that
provides a set of tools and solvers for the simulation of atmospheric
chemistry and aerosols. The MUSICA-Fortran interface provides a
Fortran API to the MUSICA library.

The MUSICA library and the MUSICA-Fortran interface can be installed
together or separately. The MUSICA library is required to build the
MUSICA-Fortran interface.

The MUSICA library and the MUSICA-Fortran interface can be installed
using the CMake build system.


Local Installation
~~~~~~~~~~~~~~~~~~

First, ensure that you have the required dependencies installed. On
Fedora, you can install the required dependencies with the following:

.. code-block:: bash

  sudo dnf install -y cmake git gcc-c++ gcc-gfortran netcdf-devel netcdf-fortran-devel

On other distributions or operating systems, you may need to install the dependencies
using the package manager for your system.

Next, clone the MUSICA repository from GitHub:

.. code-block:: bash

  git clone https://github.com/NCAR/musica.git

Next, create a build directory and run CMake:

.. code-block:: bash

  mkdir build
  cd build
  cmake -D CMAKE_INSTALL_PREFIX=<INSTALL_DIR>  -D MUSICA_BUILD_FORTRAN_INTERFACE=ON ../musica

where ``<INSTALL_DIR>`` is the directory where you want to install MUSICA.
We use the ``MUSICA_BUILD_FORTRAN_INTERFACE`` option to build the MUSICA-Fortran
interface, which is not built by default.

Finally, build and install MUSICA:

.. code-block:: bash

  make
  make install

Docker Installation
~~~~~~~~~~~~~~~~~~~

Alternatively, you can build and install MUSICA using Docker. First, ensure
that you have
`Docker Desktop <https://www.docker.com/products/docker-desktop/>`_
installed and running on your system.

Then, clone the MUSICA GitHub repository and use the provided Dockerfile to
build the MUSICA Docker image:

.. code-block:: bash

  git clone https://github.com/NCAR/musica.git
  cd musica
  docker build -t musica-fortran -f docker/Dockerfile.fortran-gcc .
  docker run -it musica-fortran bash

You can then perform the remainder of the tutorial inside the Docker container.
The ``<MUSICA_DIR>`` directory used throughout the tutorial will be located at ``/musica/build``

Once you are finished, you can exit the container by typing ``exit``. (Note that
the container will be deleted along with any files you created or modified when
you exit.)


