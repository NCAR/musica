Chapter 1
=========

First Fortran MUSICA Program
----------------------------
The MUSICA library can be used within a fortran program.
To get started, let us create a simple program that links
to MUSICA and prints the version of MICM.

Here are the contents of the program `demo.f90`:

  .. literalinclude:: ../../../fortran/test/fetch_content_integration/test_get_micm_version.F90
    :language: f90

From the ``musica_micm`` module, we only need the function ``get_micm_version``,
which returns a derived string type from the ``musica_util`` module, ``string_t``.
The ``string_t`` type will be discussed in more detail in later chapters.
To print the version string we just want the fortran character array,
accessed by ``get_char_array``.

Now, to build this simple program,
invoke the `gfortran` compiler and link to ``libmusica-fortran``, ``libmusica``,
and the standard C++ library ``libstdc++``.
The full command is

.. code-block:: bash

  gfortran -o demo demo.f90 -I<MUSICA_DIR>/include -L<MUSICA_DIR>/lib -lmusica-fortran -lmusica -lstdc++

``<MUSICA_DIR>`` is the full path of the MUSICA installation directory,
specified by the option ``CMAKE_INSTALL_PREFIX``
during the `cmake` configuration process.
Note the include path allows the linker to find the ``musica_micm.mod`` and ``musica_util.mod``
module definition files.

When the `demo` program is run it should display the MICM version: 

.. code-block:: bash

  $ ./demo
   MICM version 3.5.0
  $

