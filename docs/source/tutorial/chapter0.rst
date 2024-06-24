Chapter 0
=========

The MUSICA CMake Package
------------------------

The MUSICA library installs with CMake ``musica`` and ``musica_fortran``
packages to facilitate linking
to higher level libraries and host models that have CMake build systems.

A minimal ``CMakeLists.txt`` file designed to link the ``musica_fortran`` library
to a Fortran program ``demo_f.f90`` is exhibited below

  .. literalinclude:: ../../../tutorial/CMakeLists.txt
    :language: cmake
