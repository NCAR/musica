MUSICA Julia API
================

The Julia API provides access to MUSICA functionality through the ``Musica.jl`` package, which uses `CxxWrap.jl <https://github.com/JuliaInterop/CxxWrap.jl>`_ to interface with the underlying C++ library.

Installation
------------

Prerequisites
^^^^^^^^^^^^^

- Julia 1.6 through 1.10 (Note: Julia 1.11+ has compatibility issues with current CxxWrap versions)
- CMake 3.24 or later
- A C++ compiler with C++20 support

Building from Source
^^^^^^^^^^^^^^^^^^^^

1. Clone and build MUSICA with Julia support:

.. code-block:: bash

   git clone https://github.com/NCAR/musica.git
   cd musica
   cmake -S . -B build -D MUSICA_ENABLE_JULIA=ON -D CMAKE_BUILD_TYPE=Release
   cmake --build build

2. Install the Julia package dependencies:

.. code-block:: bash

   cd julia
   julia --project=. -e 'using Pkg; Pkg.instantiate()'

Quick Start
-----------

.. code-block:: julia

   using Musica

   # Get the MUSICA version
   version = Musica.get_version()
   println("MUSICA version: ", version)

API Reference
-------------

Core Functions
^^^^^^^^^^^^^^

.. function:: get_version() -> String

   Returns the version string of the MUSICA library.

   :returns: The MUSICA version as a string (e.g., "0.14.4")
   :rtype: String

   Example:

   .. code-block:: julia

      version = Musica.get_version()
      println("MUSICA version: ", version)

Testing
-------

To run the Julia test suite:

.. code-block:: bash

   cd julia
   julia --project=. test/runtests.jl

Additional Resources
--------------------

- `Julia Documentation <https://docs.julialang.org/>`_
- `CxxWrap.jl Documentation <https://github.com/JuliaInterop/CxxWrap.jl>`_
- `MUSICA GitHub Repository <https://github.com/NCAR/musica>`_
