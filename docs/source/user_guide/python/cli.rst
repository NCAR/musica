######################
Command Line Interface
######################

MUSICA provides a command-line interface (``musica-cli``) for working with examples and configuration conversion.
The CLI is installed automatically when you install MUSICA via pip.

Installation
============

To use all features of the examples, install MUSICA with the tutorial dependencies:

.. code-block:: console

    $ pip install 'musica[tutorial]'

Basic Usage
===========

Check the installed version:

.. code-block:: console

    $ musica-cli --version

View available options:

.. code-block:: console

    $ musica-cli -h

Available Options
=================

.. list-table::
   :widths: 25 75
   :header-rows: 1

   * - Option
     - Description
   * - ``-h``, ``--help``
     - Show help message and exit
   * - ``-e``, ``--example``
     - Name of the example to copy out
   * - ``-o``, ``--output``
     - Path to save the output to
   * - ``-v``, ``--verbose``
     - Increase logging verbosity. Use ``-v`` for info, ``-vv`` for debug
   * - ``--version``
     - Show the installed MUSICA version
   * - ``--convert``
     - Path to a MUSICA v0 configuration to convert to v1 format

Available Examples
==================

The following examples are available through the CLI:

.. list-table::
   :widths: 25 75
   :header-rows: 1

   * - Example Name
     - Description
   * - ``CARMA_Aluminum``
     - A CARMA example for simulating aluminum aerosol particles
   * - ``CARMA_Sulfate``
     - A CARMA example for simulating sulfate aerosol particles
   * - ``Sulfate_Box_Model``
     - A box model example for simulating sulfate aerosol particles
   * - ``TS1LatinHyperCube``
     - A Latin hypercube sampling example for the TS1 mechanism

Example Workflow
================

Copy an Example
---------------

Copy an example to your current directory:

.. code-block:: console

    $ musica-cli -e TS1LatinHyperCube

Copy an example to a specific directory:

.. code-block:: console

    $ musica-cli -e TS1LatinHyperCube -o /path/to/output/

Convert Configuration
---------------------

Convert a MUSICA v0 configuration to v1 format:

.. code-block:: console

    $ musica-cli --convert /path/to/v0/config.json -o /path/to/output/

Verbose Output
--------------

For more detailed logging information:

.. code-block:: console

    $ musica-cli -e TS1LatinHyperCube -v

For debug-level logging:

.. code-block:: console

    $ musica-cli -e TS1LatinHyperCube -vv
