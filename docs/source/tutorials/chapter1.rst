.. _chapter1:

Chapter 1
=========

First Fortran MUSICA Program
----------------------------
The MUSICA-Fortran API provides access to the MUSICA library within a fortran program.
To get started, let us create a simple program that links
to MUSICA and prints the version of MICM.

Save the following code to a file named `demo.F90`:

  .. literalinclude:: ../../../fortran/test/fetch_content_integration/test_get_micm_version.F90
    :language: F90

From the ``musica_micm`` module, we only need the function ``get_micm_version``,
which returns a derived string type from the ``musica_util`` module, ``string_t``.
(The ``string_t`` type will be discussed in more detail in later chapters.)
To print the version string we just want the fortran character array,
accessed by the ``get_char_array`` function.

Now, to build this simple program,
invoke the `gfortran` compiler and link to ``libmusica-fortran``, ``libmusica``, ``yaml-cpp``,
and the standard C++ library ``libstdc++``.
The full command is

.. code-block:: bash

  gfortran -o demo demo.F90 -I<MUSICA_DIR>/include -L<MUSICA_DIR>/lib64 -lmusica-fortran -lmusica -lstdc++ -lyaml-cpp

``<MUSICA_DIR>`` is the full path of the MUSICA installation directory,
specified by the option ``CMAKE_INSTALL_PREFIX``
during installation (see :ref:`installing_musica`).
Note that the include path allows the linker to find the ``musica_micm.mod`` and ``musica_util.mod``
module definition files.

When the `demo` program is run it should display the MICM version: 

.. code-block:: bash

  $ ./demo
   MICM version 3.6.0
  $

Building a MUSICA Fortran Program with CMake
--------------------------------------------

A minimal ``CMakeLists.txt`` file designed to link the ``musica_fortran`` library
to the ``demo_f.F90`` file described above is exhibited below

  .. literalinclude:: ../../../fortran/test/tutorial/CMakeLists.txt
    :language: cmake

Common practice is to create a ``build`` subdirectory (relative to the top level ``CMakeLists.txt`` file shown above).

.. code-block:: bash

  mkdir build
  cd build

Then, ``cmake`` can then be invoked with:

.. code-block:: bash

  cmake -DMUSICA_INSTALL_DIR=<MUSICA_DIR> ..
  make

Then, the ``demo_f`` executable can be run:

.. code-block:: bash

  $ ./demo_f
    MICM version 3.6.0
  $
