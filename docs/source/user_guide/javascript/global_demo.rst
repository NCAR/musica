.. _javascript-global-demo:

Global Surface Demo
===================

This demo runs a global chemistry transport model in the browser. MICM solves a
NO\ :sub:`x`\ --O\ :sub:`3` mechanism independently in every cell of a lat-lon
surface grid. A prescribed wind field then transports the species between the
cells. All of the work happens in WebAssembly in the browser tab.

.. note::

   The demo may fail to load if the dependencies can't be loaded.
   If you encounter issues, please try refreshing the page.

.. raw:: html
   :file: global_widget.html

How It Works
------------

The example shows the pattern for a host model that drives MICM over many grid
cells:

#. One MICM grid cell holds each lat-lon box. A 5-degree grid gives 2592 cells.
#. The photolysis rate of NO\ :sub:`2` comes from the solar zenith angle in each
   cell. A day and night contrast therefore moves across the map.
#. Prescribed emissions of NO occur in the cells that contain 10 cities.
#. ``micm.solve`` advances the chemistry in all of the cells in one call.
#. Transport runs in JavaScript between the chemistry steps. The code reads the
   concentrations with ``state.getConcentrations()``, advects them, and writes
   them back with ``state.setConcentrations()``.

Flow Patterns
-------------

Every pattern comes from a streamfunction defined on the cell corners. The face
fluxes are differences of that streamfunction, so the discrete wind field is
divergence free. The advection is then conservative and positive definite. The
page reports the mass drift of the advection operator as a check.

Rossby--Haurwitz wave
   An exact solution of the barotropic vorticity equation on the sphere, and a
   standard dynamical core test case. The wave keeps its shape and travels at an
   analytic phase speed. With :math:`R = 4` it makes one circuit in 29.5 days,
   eastward.

Zonal jets with a travelling wave
   Tropical easterlies with a westerly jet in each middle latitude band, plus a
   travelling wave that makes the flow meander.

Blocking high over a low
   A westerly jet meets a stationary high sitting over a low. The two centres
   drive easterly flow between them, which splits the jet and diverts the plumes
   around the block.

Polar vortex
   A strong circumpolar jet near 65 degrees north with a wave on its edge. The jet
   acts as a partial transport barrier, so air inside the vortex mixes only slowly
   with middle latitude air.

Deformational flow
   The non-divergent test case of Nair and Lauritzen (2010). A solid body rotation
   combines with a deformation that reverses at half the period, so an exact scheme
   returns every tracer to its starting point after one full period.

Advection Scheme
----------------

The scheme is selectable. The default is second order with a van Leer limiter.
The alternative is first-order upwind, which is much more diffusive. In a solid
body rotation test at 5 degrees, the second-order scheme keeps 77 % of the peak
of a cosine bell after one full revolution, against 32 % for first-order upwind.
Both schemes conserve mass, keep the field non-negative, and preserve a uniform
field exactly. Switch between them to see how the numerical method changes the
sharpness of the plumes.

Performance
-----------

The page reports the wall time for each step. On a 5-degree grid, MICM solves
all 2592 cells in a few milliseconds, so the model animates in real time. The
resolution selector goes to 2.5 degrees, which is 10368 cells.

Running Locally
---------------

To run the demo from your local build:

.. code-block:: bash

   npm run example

Then open http://localhost:8000/javascript/wasm/global.html in your browser.

See :ref:`Development Setup <development-setup>` for instructions on building
the WASM module from source.
