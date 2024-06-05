Chapter 2
=========

An ABC MICM Fortran Example
---------------------------

In this next MUSICA Fortran example,
we will setup a MICM solver, starting with a set of MICM configuration files,
and run the solver for a single integration time step.

The MICM configuration is specified in a top-level ``config.json`` file,
which simply lists the chemical species configuration file followed by
the reactions configuration file.

  .. literalinclude:: ../../../configs/analytical/config.json
    :language: json

For this example, we will have a system of three chemical species
`A`, `B`, and `C`, defined in the JSON file ``species.json`` as follows:

  .. literalinclude:: ../../../configs/analytical/species.json
    :language: json

  .. literalinclude:: ../../../configs/analytical/reactions.json
    :language: json

  .. literalinclude:: ../../../fortran/test/fetch_content_integration/test_micm_box_model.F90
    :language: f90
