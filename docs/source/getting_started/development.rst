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

JavaScript
----------

Prerequisites:

- `Node.js <https://nodejs.org/>`_ 22 or later
- `Emscripten SDK <https://emscripten.org/docs/getting_started/downloads.html>`_ >= 4.0.2 (for compiling to WebAssembly)
- CMake >= 3.21

Install and activate the Emscripten SDK:

.. code-block:: bash

   git clone https://github.com/emscripten-core/emsdk.git
   cd emsdk
   ./emsdk install latest
   ./emsdk activate latest
   source ./emsdk_env.sh
   cd ..

Install Node.js dependencies and build the WASM module:

.. code-block:: bash

   npm install
   npm run build

The WASM files (``musica.js`` and ``musica.wasm``) are placed in ``javascript/wasm/``.

Run the test suite:

.. code-block:: bash

   npm run test             # all tests
   npm run test:unit        # unit tests only
   npm run test:integration # integration tests only
   npm run test:coverage    # with coverage report

To run the browser example locally:

.. code-block:: bash

   npm run example

Then open http://localhost:8000/javascript/wasm/index.html. See :doc:`../user_guide/javascript/demo` for the live deployed demo.

Documentation
-------------

Install the documentation dependencies and build locally:

.. code-block:: bash

   cd docs
   pip install -r requirements.txt
   make copy   # sync notebooks from tutorials/
   make html

The built docs are at ``docs/build/html/index.html``.
For C++ API changes, regenerate the Doxygen XML first:

.. code-block:: bash

   doxygen Doxyfile.in

Alternatively, build and serve the docs using Docker (no local dependencies required):

.. code-block:: bash

   docker build -t docs -f docker/Dockerfile.docs .
   docker run --rm -p 8123:8123 -it docs

Then navigate to http://localhost:8123.
