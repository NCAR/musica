JavaScript
==========

Installing MUSICA
-----------------

The JavaScript API is available as an npm package:

.. code-block:: bash

   npm install @ncar/musica

To build from source with WebAssembly support:

.. code-block:: bash

   git clone https://github.com/NCAR/musica.git
   cd musica
   npm install
   npm run build

.. note::

   The JavaScript API is designed specifically for use in web applications
   (particularly the MUSICA box model web app) and exposes only MICM
   gas-phase chemistry.

Verifying the installation
---------------------------

.. code-block:: javascript

   import { getVersion } from '@ncar/musica';
   console.log(getVersion());

Next steps
----------

- :ref:`JavaScript User Guide <javascript-user-guide>` — initialization, mechanism loading, and solver usage
- :doc:`Live Demo <../user_guide/javascript/demo>` — try MUSICA running in WebAssembly in your browser
- :doc:`JavaScript API Reference <../api/javascript>` — full API documentation
