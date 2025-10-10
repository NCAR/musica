from typing import Optional, Any, Dict
from .. import backend
from .utils import _add_other_properties, _remove_empty_keys

_backend = backend.get_backend()
PhaseSpecies = _backend._mechanism_configuration._PhaseSpecies
original_init = PhaseSpecies.__init__


def __init__(
    self,
    name: Optional[str] = None,
    diffusion_coefficient_m2_s: Optional[float] = None,
    other_properties: Optional[Dict[str, Any]] = None,
):
    """
    Initializes the PhaseSpecies object with the given parameters.

    Args:
        name (str): The name of the species.
        diffusion_coefficient_m2_s (float): Diffusion coefficient [m2 s-1]
        other_properties (Dict[str, Any]): A dictionary of other properties of the species.
    """
    original_init(self)
    self.name = name if name is not None else self.name
    self.diffusion_coefficient_m2_s = diffusion_coefficient_m2_s if diffusion_coefficient_m2_s is not None else self.diffusion_coefficient_m2_s
    self.other_properties = other_properties if other_properties is not None else self.other_properties


def serialize(self) -> Dict:
    serialize_dict = {
        "name": self.name,
        "diffusion coefficient [m2 s-1]": self.diffusion_coefficient_m2_s,
    }
    _add_other_properties(serialize_dict, self.other_properties)
    return _remove_empty_keys(serialize_dict)


def equals(self, other):
    return self.name == other.name and self.diffusion_coefficient_m2_s == other.diffusion_coefficient_m2_s


PhaseSpecies.__doc__ = """
    Represents a chemical species within a specific phase of a mechanism,
    including phase-specific properties such as diffusion coefficients.

    This class is distinct from a regular Species class in that it models
    properties relevant to the species' behavior in a particular phase
    (e.g., gas, liquid, or solid), such as the diffusion coefficient.

    Attributes:
        name (str): The name of the species.
        diffusion_coefficient_m2_s (float): Diffusion coefficient in the phase [m2 s-1].
        other_properties (Dict[str, Any]): A dictionary of other phase-specific properties of the species.
    """
PhaseSpecies.__init__ = __init__
PhaseSpecies.serialize = serialize
PhaseSpecies.__eq__ = equals
