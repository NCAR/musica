.. _development-setup:

Development Setup
=================

This page covers setting up a local development environment for contributing to MUSICA.
For the contribution workflow (branching, PRs, code standards), see the :ref:`Contributing <contributing>` guide.

Python
------

Clone the repository and create a conda environment:

.. code-block:: bash

   git clone https://github.com/NCAR/musica.git
   cd musica
   conda create --name musica-dev python=3.9 --yes
   conda activate musica-dev

Install in editable mode so your local changes are reflected immediately:

.. code-block:: bash

   pip install -e ".[dev]"

Run the test suite:

.. code-block:: bash

   python -m pytest python/test/ -v

C++
---

Clone and configure with tests enabled:

.. code-block:: bash

   git clone https://github.com/NCAR/musica.git
   cd musica && mkdir build && cd build
   cmake -D MUSICA_ENABLE_MICM=ON \
         -D MUSICA_ENABLE_TUVX=ON \
         -D MUSICA_ENABLE_TESTS=ON \
         ..
   make -j

Run the test suite:

.. code-block:: bash

   ctest --output-on-failure

Julia
-----

To develop the Julia bindings, use ``Pkg.develop`` with the local path:

.. code-block:: julia

   using Pkg
   Pkg.develop(path="/path/to/musica")

Run the Julia tests from the REPL:

.. code-block:: julia

   using Pkg
   Pkg.test("MUSICA")

Fortran
-------

Clone and configure with the Fortran interface and tests enabled:

.. code-block:: bash

   git clone https://github.com/NCAR/musica.git
   cd musica && mkdir build && cd build
   cmake -D MUSICA_BUILD_FORTRAN_INTERFACE=ON \
         -D MUSICA_ENABLE_TESTS=ON \
         ..
   make -j
   ctest --output-on-failure

Documentation
-------------

Install the documentation dependencies and build locally:

.. code-block:: bash

   cd docs
   pip install -r requirements.txt
   make html

The built docs are at ``docs/build/html/index.html``.
For C++ API changes, regenerate the Doxygen XML first:

.. code-block:: bash

   doxygen Doxyfile.in
