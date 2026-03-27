.. _api-ref:

MUSICA API
==========

Welcome to the MUSICA API documentation. Below, you can find links to the documentation for 
the C++, Python, Julia, and JavaScript APIs.


MUSICA is quite versatile. While we don't provide the exact same API set for all languages, the
core fucntionality needed to solve gas-phase chemistry given a valid
:doc:`mechanism configuration file <mc:index>` is available in all languages. 

The C++ is essentially a wrapper around MICM and TUVx to make it easy to expose the funtionality 
in other languages. The Python API is the most fully featured, and the Julia and 
JavaScript APIs are still in development. Additionally, musica-javascript is exclusively meant to
support our box model web application, so it is not expected to have the same level of functionality 
as the other APIs.


The full ecosystem of MUSICA software (as a dependency diagram) is show below with a couple of 
models listed. We hope the list of models where MUSICA is used will grow over time!

.. figure:: /_static/ecosystem.png
   :width: 80%
   :align: center

   A diagram showing (some) software dependencies with names of models we itegrate with listed at the top
   and core libraries at the bottom.

.. toctree::
   :maxdepth: 1
   :caption: Contents:

   C++
   python
   julia
   javascript
  