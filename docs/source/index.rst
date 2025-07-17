.. musica documentation HTML titles
..
.. # (over and under) for module headings
.. = for sections
.. - for subsections
.. ^ for subsubsections
.. ~ for subsubsubsections
.. " for paragraphs

==================================
Welcome to MUSICA's documentation!
==================================

The Multiscale Interface for Chemistry and Aerosols (MUSICA) aims to create a model-independent infrastructure to simulate chemistry and aerosols at a large number of different resolutions in a single, coherent fashion.
Initially integrated within the `NSF NCAR Community Earth system Model (CESM) <https://www.cesm.ucar.edu>`_, it enables full feedbacks between the atmosphere, ocean and land.
MUSICA unifies diverse chemical transport models including `CAM-Chem <https://wiki.ucar.edu/spaces/camchem/pages/59512396/Home>`_, `WACCM <https://www2.acom.ucar.edu/gcm/waccm>`_,
`WRF-Chem <https://www2.acom.ucar.edu/wrf-chem>`_, NSF NCAR `LES <https://ral.ucar.edu/solutions/products/rtfdda-les>`_, and a box model in a single modular framework.
The model infrastructure is open source, flexible and computationally efficient in order to facilitate community co-development and use for scientific and operational purposes. 

At the heart of MUSICA is the standalone `Model Independent Chemistry Model (MICM) <https://ncar.github.io/micm/index.html>`_, which is a gas-phase kinetic solver. MICM is made available by the MUSICA wrapper which satisfies the requirements of the Common Community Physics Package (CCPP)
and that can be connected to any CCPP compliant atmosphere model.


.. grid:: 1 1 2 2
    :gutter: 2

    .. grid-item-card:: Getting started
        :img-top: _static/index_getting_started.svg
        :link: getting_started/getting_started
        :link-type: doc

        Check out the getting started guide to build and install musica.

    .. grid-item-card::  User guide
        :img-top: _static/index_user_guide.svg
        :link: user_guide/index
        :link-type: doc

        Learn how to configure MUSICA for your mechanisms here!

    .. grid-item-card::  API reference
        :img-top: _static/index_api.svg
        :link: api/index
        :link-type: doc

        The source code for MUSICA is heavily documented. This reference will help you extend MUSICA for your needs.

    .. grid-item-card::  Contributors guide
        :img-top: _static/index_contribute.svg
        :link: contributing/index
        :link-type: doc

        If you'd like to contribute some new science code or update the docs, checkout the contributor's guide!


.. toctree::
   :maxdepth: 2
   :caption: Contents:

   getting_started/getting_started
   user_guide/index
   tutorials/tutorials
   api/index
   contributing/index
   citing/index

Indices and tables
==================

* :ref:`genindex`
* :ref:`search`
