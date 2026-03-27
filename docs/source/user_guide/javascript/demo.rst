.. _javascript-demo:

Live Demo
=========

A live interactive demo of MUSICA running in WebAssembly is deployed at
`https://ncar.github.io/musica/ <https://ncar.github.io/musica/>`_.
It demonstrates version information, example mechanisms, and interactive
sliders for exploring atmospheric chemistry in the browser.

.. raw:: html

   <iframe
     src="https://ncar.github.io/musica/"
     width="100%"
     height="700px"
     style="border: 1px solid #ddd; border-radius: 4px;"
     title="MUSICA WebAssembly Live Demo">
   </iframe>

Running Locally
---------------

To run the same demo from your local build:

.. code-block:: bash

   npm run example

Then open http://localhost:8000/javascript/wasm/index.html in your browser.

See :ref:`Development Setup <development-setup>` for instructions on building
the WASM module from source.
