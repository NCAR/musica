Chapter 0
=========

The MUSICA CMake Package
------------------------

The MUSICA library installs with `CMake` ``musica`` and ``musica_fortran``
packages to facilitate linking
to higher level libraries and host models that have CMake build systems.

A minimal ``CMakeLists.txt`` file designed to link the ``musica_fortran`` library
to a Fortran program ``demo_f.f90`` is exhibited below

  .. literalinclude:: ../../../fortran/test/tutorial/CMakeLists.txt
    :language: cmake

These `CMake` directives are essentially equivalent to compilation on the command line via

.. code-block:: bash

  gfortran -o demo_f demo_f.f90 -I<MUSICA_DIR>/include -L<MUSICA_DIR>/lib -lmusica-fortran -lmusica -lstdc++

``<MUSICA_DIR>`` is the full path of the MUSICA installation directory,
specified by the option ``CMAKE_INSTALL_PREFIX``
during the `cmake` configuration process.

Common practice is to create a ``build`` subdir (relative to the top level ``CMakeLists.txt`` file, say).

.. code-block:: bash

  mkdir build
  cd build

The ``cmake`` could then be invoked with:

.. code-block:: bash

  cmake -DMUSICA_INSTALL_DIR <MUSICA_DIR> ..
  cmake --build .

