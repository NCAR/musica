from typing import Optional, Any, Dict
from ... import backend
from ..._base import CppWrapper, CppField
from ..utils import _add_other_properties, _remove_empty_keys
from .species import Species

_backend = backend.get_backend()
_mc = _backend._mechanism_configuration


class PhaseSpecies(CppWrapper):
    """A chemical species within a specific phase of a mechanism.

    This class is distinct from a regular Species class in that it models
    properties relevant to the species' behavior in a particular phase
    (e.g., gas, liquid, or solid), such as the diffusion coefficient.

    Attributes:
        name: The name of the species.
        diffusion_coefficient_m2_s: Diffusion coefficient in the phase [m2 s-1].
        density_kg_m3: Density in the phase [kg m-3].
        other_properties: A dictionary of other phase-specific properties of the species.
    """

    name = CppField()
    diffusion_coefficient_m2_s = CppField()
    density_kg_m3 = CppField()
    other_properties = CppField()

    def __init__(
        self,
        species: Optional[Species] = None,
        name: Optional[str] = None,
        *,
        diffusion_coefficient_m2_s: Optional[float] = None,
        density_kg_m3: Optional[float] = None,
        other_properties: Optional[Dict[str, Any]] = None,
    ):
        """Initialize the PhaseSpecies.

        Args:
            name: The name of the species.
            diffusion_coefficient_m2_s: Diffusion coefficient [m2 s-1].
            density_kg_m3: Density in the phase [kg m-3].
            other_properties: A dictionary of other properties of the species.
        """
        if not ((name is None) ^ (species is None)):
            msg = f"PhaseSpecies requires exactly one of name or species. Got name={name}; species={species}"
            raise ValueError(msg)
        self._cpp = _mc._PhaseSpecies()
        self.name = name if name is not None else species.name
        self.diffusion_coefficient_m2_s = diffusion_coefficient_m2_s if diffusion_coefficient_m2_s is not None else self.diffusion_coefficient_m2_s
        if density_kg_m3 is not None:
            self.density_kg_m3 = density_kg_m3
        self.other_properties = other_properties if other_properties is not None else self.other_properties

    def serialize(self) -> Dict:
        serialize_dict = {
            "name": self.name,
            "diffusion coefficient [m2 s-1]": self.diffusion_coefficient_m2_s,
            "density [kg m-3]": self.density_kg_m3,
        }
        _add_other_properties(serialize_dict, self.other_properties)
        return _remove_empty_keys(serialize_dict)

    def __eq__(self, other):
        return (
            self.name == other.name
            and self.diffusion_coefficient_m2_s == other.diffusion_coefficient_m2_s
            and self.density_kg_m3 == other.density_kg_m3
        )
