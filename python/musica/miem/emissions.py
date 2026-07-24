# Copyright (C) 2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0
from .. import backend
from .._base import CppWrapper, _unwrap

_backend = backend.get_backend()


class Emissions(CppWrapper):
    """Python wrapper around musica::EmissionsModel.

    Builds and runs an emissions module from a Mechanism's emissions section
    (see ``musica.mechanism_configuration.EmissionsConfig``), driving surface
    flux for a host grid.

    Attributes:
        num_species: Number of aggregated mechanism species across all sources.
        species_names: Aggregated mechanism species names across all sources.
    """

    def __init__(self, mechanism, n_cells: int, n_vert_levels: int):
        """Build an emissions module from a Mechanism's emissions section.

        Args:
            mechanism: A parsed or in-code-built Mechanism with an emissions section.
            n_cells: Number of horizontal grid cells.
            n_vert_levels: Number of vertical levels.
        """
        self._cpp = _backend._miem._create_emissions_from_mechanism(_unwrap(mechanism), n_cells, n_vert_levels)

    def run(self, epoch_seconds: float, dt_seconds: float):
        """Advance one time step.

        Args:
            epoch_seconds: Simulation time as seconds since epoch.
            dt_seconds: Time step [s].

        Returns:
            numpy.ndarray: Surface flux, shape (num_species, n_cells) [kg m-2 s-1].
        """
        return self._cpp.run(epoch_seconds, dt_seconds)

    @property
    def num_species(self) -> int:
        return self._cpp.num_species

    @property
    def species_names(self):
        return self._cpp.species_names
