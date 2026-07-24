.. _chapter5:

Chapter 5
=========

A TS1 Box Model Driven by TUV-x Photolysis Rates
-------------------------------------------------

The previous chapters advanced chemistry with fixed or user-supplied rate
constants. In this chapter we couple two MUSICA components in a single Fortran
program: **TUV-x** computes photolysis rate constants from the radiative
transfer environment, and **MICM** integrates the resulting chemistry. This is
the Fortran counterpart of the Python example in
``python/musica/examples/ts1_box_model.py``.

The mechanism is **TS1** (``MZ327_TS1.2``), the gas-phase chemistry used in
CAM-chem. It contains several hundred reactions, of which 123 are photolysis
reactions. We run it as a vertical column of nine independent box-model cells
spanning 1–9 km.

Setting up TUV-x for TS1/TSMLT
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

TUV-x needs grids, profiles, and radiators before it can compute rates. The
``tuvx_ts1_setup`` module builds a calculator configured for the TS1/TSMLT
photolysis setup. It reuses the height grid, profile loader, and aerosol
radiator loader from the ``tuvx_v54_setup`` module, and only overrides the
wavelength grid, the configuration file, and the TS1-specific data files:

.. literalinclude:: ../../../../fortran/examples/tuvx_ts1_setup.F90
  :language: f90

The Box Model
^^^^^^^^^^^^^

The main program ties everything together. Save the following to
``ts1_box_model.F90``:

.. literalinclude:: ../../../../fortran/examples/ts1_box_model.F90
  :language: f90

Key concepts illustrated by this example:

**Running TUV-x**

After the calculator is built, we query the ordering of the photolysis
reactions it reports, size the output arrays to the number of height-grid
edges, and call ``run`` for a single solar zenith angle:

.. code-block:: fortran

  photo_mappings => tuvx%get_photolysis_rate_constants_ordering(error)
  num_photo = photo_mappings%size()
  allocate(photo_rates(NUM_LAYERS, num_photo))
  call tuvx%run(SZA, EARTH_SUN_DISTANCE, photo_rates, heating_rates, error)

``photo_rates`` is indexed ``(layer, reaction)``. Layer 1 is the ground edge, so
the 1–9 km cells correspond to layers 2–10.

**Mapping TUV-x rates onto MICM reactions**

TUV-x reports only 73 photolysis reactions, but TS1 has 123. The remaining
reactions are derived from the reported ones using the
``__CAM options -> aliasing -> pairs`` section of the TUV-x configuration file.
Each pair names a target MICM reaction (``to``), a source TUV-x reaction
(``from``), and an optional ``scale by`` factor. We read this section with the
``config_t`` YAML/JSON reader and write each rate into the MICM state:

.. code-block:: fortran

  call config%from_file(config_file_path())
  call config%get("__CAM options", cam_options, "ts1_box_model")
  call cam_options%get("aliasing", aliasing, "ts1_box_model")
  call aliasing%get("pairs", pairs, "ts1_box_model")

  do p = 1, size(pairs)
    call pairs(p)%get("to", to_name, "ts1_box_model")
    call pairs(p)%get("from", from_name, "ts1_box_model")
    call pairs(p)%get("scale by", scale, "ts1_box_model", default=1.0_dk)
    ...
  end do

The MICM photolysis rate parameters carry a ``PHOTO.`` prefix (user-defined
reactions carry ``USER.`` and surface reactions ``SURF.``). One photolysis
reaction, ``jno``, is not computed by TUV-x and keeps the value from the initial
conditions file.

**Reading initial conditions from a CSV file**

The initial concentrations, user-defined and photolysis rate parameters, and
surface-reaction parameters are read from
``configs/v1/ts1/initial_conditions.csv``. Each row is keyed by a prefix
(``CONC.``, ``USER.``, ``PHOTO.``, ``SURF.``, ``ENV.``) that determines how the
value is applied. Names that are not present in this mechanism are skipped with
a warning, which keeps the reader robust to mechanism changes.

**Stepping the chemistry**

Each cell is an independent box (no transport). Photolysis rates are held fixed
at the ``SZA = 0`` values, and MICM integrates the chemistry over the requested
simulation length, sub-stepping until each output interval is filled:

.. code-block:: fortran

  do while (current_time < SIM_LENGTH)
    elapsed = 0.0_dk
    do while (elapsed < TIME_STEP)
      remaining = TIME_STEP - elapsed
      call micm%solve(remaining, state, solver_state, solver_stats, error)
      elapsed = elapsed + real(solver_stats%final_time(), dk)
      current_time = current_time + real(solver_stats%final_time(), dk)
    end do
    call write_output(...)
  end do

Building and Running
^^^^^^^^^^^^^^^^^^^^

This example is wired into the MUSICA build. Configure with the Fortran
interface enabled and build the ``test_ts1_box_model`` target:

.. code-block:: bash

  cmake -S . -B build -D MUSICA_BUILD_FORTRAN_INTERFACE=ON
  cmake --build build --target test_ts1_box_model

TUV-x resolves its data files relative to the working directory, and the MICM
configuration is referenced relative to it, so run the program from the
``configs/tuvx`` directory:

.. code-block:: bash

  cd build/configs/tuvx
  ../../test_ts1_box_model

You can also run it through CTest:

.. code-block:: bash

  ctest --test-dir build -R ts1_box_model --output-on-failure

The program writes ``ts1_box_model_fortran.csv`` with the concentrations of a
few tracked species (``BEPOMUC``, ``C6H5OOH``, ``BR``, ``CL``, ``O3``) for every
cell and output time. Expected console output:

.. code-block:: bash

   ======================================
   TS1 box model driven by TUV-x
   ======================================

   Setting up TUV-x (TS1/TSMLT)...
   Running TUV-x for 73 photolysis reactions...
   Setting up MICM (TS1)...
   Applying initial conditions...
   Mapping TUV-x photolysis rates (CAM aliasing)...
    Mapped 122 of 122 photolysis reactions from TUV-x
   Integrating for  8640. s in  30. s steps...

   Simulation progress:    5.6%
   ...
   Simulation progress:   99.3%

   Results written to ts1_box_model_fortran.csv
   Done.
