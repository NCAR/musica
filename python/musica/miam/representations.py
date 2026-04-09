"""Aerosol/cloud representations for MIAM."""

from dataclasses import dataclass, field
from typing import List


@dataclass
class UniformSection:
    """Uniform cloud/aerosol section representation.

    Assumes a uniform number distribution across the size range.
    Uses ``state.rate_parameters`` to set radius bounds at runtime.

    Args:
        name: Unique name for this representation (e.g. "CLOUD").
        phase_names: Names of condensed phases in this section.
        min_radius: Minimum droplet/particle radius [m].
        max_radius: Maximum droplet/particle radius [m].
    """
    name: str
    phase_names: List[str]
    min_radius: float
    max_radius: float

    def set_default_parameters(self, state):
        """Set the default radius parameters on a MICM state.

        Args:
            state: A ``musica.micm.State`` object.
        """
        state.set_user_defined_rate_parameters({
            f"{self.name}.MIN_RADIUS": self.min_radius,
            f"{self.name}.MAX_RADIUS": self.max_radius,
        })


@dataclass
class SingleMomentMode:
    """Single-moment aerosol mode representation.

    Fixed number concentration; size distribution described by a
    log-normal with fixed geometric standard deviation.

    Args:
        name: Unique name for this representation.
        phase_names: Names of condensed phases in this mode.
        geometric_mean_radius: Geometric mean radius [m].
        geometric_standard_deviation: Geometric standard deviation (dimensionless, > 1).
    """
    name: str
    phase_names: List[str]
    geometric_mean_radius: float
    geometric_standard_deviation: float

    def set_default_parameters(self, state):
        """Set the default parameters on a MICM state.

        Args:
            state: A ``musica.micm.State`` object.
        """
        state.set_user_defined_rate_parameters({
            f"{self.name}.GEOMETRIC_MEAN_RADIUS": self.geometric_mean_radius,
            f"{self.name}.GEOMETRIC_STANDARD_DEVIATION": self.geometric_standard_deviation,
        })


@dataclass
class TwoMomentMode:
    """Two-moment aerosol mode representation.

    Tracks both number concentration and mass; size distribution
    described by a log-normal with fixed geometric standard deviation.

    Args:
        name: Unique name for this representation.
        phase_names: Names of condensed phases in this mode.
        geometric_standard_deviation: Geometric standard deviation (dimensionless, > 1).
    """
    name: str
    phase_names: List[str]
    geometric_standard_deviation: float

    def set_default_parameters(self, state):
        """Set the default parameters on a MICM state.

        Args:
            state: A ``musica.micm.State`` object.
        """
        state.set_user_defined_rate_parameters({
            f"{self.name}.GEOMETRIC_STANDARD_DEVIATION": self.geometric_standard_deviation,
        })
