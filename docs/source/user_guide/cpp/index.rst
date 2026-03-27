.. _cpp-user-guide:

Overview
========

This section covers using MUSICA as a C++ library. The C++ library is the
foundation that all other language bindings are built on, and gives direct
access to MICM, TUV-x, and CARMA.

Building from Source
--------------------

Prerequisites
~~~~~~~~~~~~~

Install the required system dependencies. On Fedora/RHEL:

.. code-block:: bash

   sudo dnf install -y cmake git gcc-c++ netcdf-devel

On Ubuntu/Debian:

.. code-block:: bash

   sudo apt-get install -y cmake git g++ libnetcdf-dev

Clone and Build
~~~~~~~~~~~~~~~

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

See :doc:`../cmake_options/index` for the full list of CMake configuration
options, including enabling CARMA and GPU support.

Using MUSICA in a CMake Project
--------------------------------

Once installed, link MUSICA into your own CMake project with ``find_package``:

.. code-block:: cmake

   cmake_minimum_required(VERSION 3.21)
   project(my_chemistry_model)

   find_package(musica REQUIRED)

   add_executable(my_model main.cpp)
   target_link_libraries(my_model musica::musica)

If you installed to a non-standard prefix, set ``musica_ROOT`` or add the
install prefix to ``CMAKE_PREFIX_PATH``:

.. code-block:: bash

   cmake -D CMAKE_PREFIX_PATH=<INSTALL_DIR> ..

Basic Usage
-----------

The C++ API loads a mechanism from a
:doc:`mechanism configuration file <mc:index>` directory and exposes a
solver, state, and conditions object. A minimal box-model example:

.. code-block:: cpp

   #include <musica/micm.hpp>
   #include <iostream>

   int main()
   {
     // Load mechanism from JSON config directory
     auto micm = musica::MICM("configs/analytical",
                               musica::MICMSolver::RosenbrockStandardOrder);

     auto state = micm.CreateState();

     // Set initial conditions
     state.SetConcentrations({ {"A", {1.0}}, {"B", {3.0}}, {"C", {5.0}} });
     state.SetConditions(300.0 /*K*/, 101000.0 /*Pa*/);

     // Integrate over time
     double time_step  = 4.0;   // s
     double sim_length = 20.0;  // s

     for (double t = time_step; t <= sim_length; t += time_step) {
       micm.Solve(state, time_step);
       auto conc = state.GetConcentrations();
       std::cout << "t=" << t << "s  A=" << conc["A"][0] << "\n";
     }

     return 0;
   }

Further Reading
---------------

- :doc:`C++ API Reference <../api/C++>`
- :doc:`MICM Documentation <micm:index>`
- :doc:`CMake Options <../cmake_options/index>`
