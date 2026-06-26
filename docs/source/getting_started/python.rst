Python
======

Installing MUSICA
-----------------

MUSICA is available on PyPI. Install it with pip:

.. code-block:: console

   $ pip install musica

We recommend installing inside a virtual environment:

.. code-block:: console

   $ conda create --name musica python=3.9 --yes
   $ conda activate musica
   $ pip install musica

Verifying the installation
---------------------------

From a Python shell or script:

.. code-block:: python

   import musica
   print(musica.__version__)

This should print the installed version. You can also verify from the command line:

.. code-block:: console

   $ pip show musica

Next steps
----------

- :ref:`Python User Guide <python-user-guide>` — learn how to define mechanisms, run solvers, and visualize results
- :ref:`Python Notebooks <python-notebooks>` — interactive Jupyter notebook tutorials
- :ref:`Development Setup <development-setup>` — set up for contributing to MUSICA
