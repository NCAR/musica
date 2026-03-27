.. _javascript-demo:

Live Demo
=========

An interactive demo of MUSICA running in WebAssembly is embedded below.
It demonstrates version information, example mechanisms, and interactive
sliders for exploring atmospheric chemistry in the browser.

.. note::

   The demo may fail to load if the dependencies can't be loaded. 
   If you encounter issues, please try refreshing the page or accessing
   `the deployed version <https://ncar.github.io/musica/>`_ directly which hosts just this demo.

.. raw:: html
   :file: widget.html

Running Locally
---------------

To run the demo from your local build:

.. code-block:: bash

   npm run example

Then open http://localhost:8000/javascript/wasm/index.html in your browser.

See :ref:`Development Setup <development-setup>` for instructions on building
the WASM module from source.
