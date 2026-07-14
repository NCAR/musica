"""
The aerosol configuration (representations, processes, constraints) is defined on
a ``mechanism_configuration.Mechanism`` via its ``aerosol`` section, using the
aerosol types in ``musica.mechanism_configuration``. This module provides the
``MIAM`` external-model handle: attach it to a solver with
``MICM(mechanism=..., external_models=[musica.MIAM()])`` to realize that
aerosol section with the MIAM aerosol model.
"""

from .. import backend

_backend = backend.get_backend()

__version__ = _backend._miam._get_miam_version() if backend.miam_available() else None


class MIAM:
    """External-model handle that attaches the MIAM aerosol model to a MICM solver.

    The aerosol configuration is read from the mechanism's ``aerosol`` section;
    this handle carries no configuration of its own. Attach it via::

        solver = musica.MICM(mechanism=mech, external_models=[musica.MIAM()])
    """

    def _create_solver(self, mechanism_cpp, solver_type):
        """Build a MICM solver with the MIAM aerosol model attached.

        Called by ``MICM`` through the generic external-model hook, so ``MICM``
        never needs to know that MIAM is the model being attached.

        Args:
            mechanism_cpp: The unwrapped C++ mechanism (``mechanism._cpp``), carrying
                the ``aerosol`` section plus the species and phase.
            solver_type: The MICM solver type to build.
        """
        return _backend._miam._create_solver_with_miam(mechanism_cpp, solver_type)


__all__ = ["MIAM"]
