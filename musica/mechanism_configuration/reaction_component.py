from typing import Optional, Any, Dict
from .. import backend
from .utils import _add_other_properties, _remove_empty_keys

_backend = backend.get_backend()
ReactionComponent = _backend._mechanism_configuration._ReactionComponent
original_init = ReactionComponent.__init__


def __init__(
    self,
    name: Optional[str] = None,
    coefficient: Optional[float] = 1.0,
    other_properties: Optional[Dict[str, Any]] = None,
):
    """
    Initializes the ReactionComponent object with the given parameters.

    Args:
        name (str): The name of the species.
        coefficient (float): The stichiometric coefficient
        other_properties (Dict[str, Any]): A dictionary of other properties of the species.
    """
    original_init(self)
    self.species_name = name if name is not None else self.name
    self.coefficient = coefficient if coefficient is not None else self.coefficient
    self.other_properties = other_properties if other_properties is not None else self.other_properties


def serialize(self) -> Dict:
    serialize_dict = {
        "species name": self.species_name,
        "coefficient": self.coefficient,
        "other_properties": self.other_properties,
    }
    _add_other_properties(serialize_dict, self.other_properties)
    return _remove_empty_keys(serialize_dict)


ReactionComponent.__doc__ = """
    A class representing a species in a chemical mechanism.

    Attributes:
        name (str): The name of the species.
        molecular_weight_kg_mol (float): Molecular weight [kg mol-1]
        constant_concentration_mol_m3 (float): Constant concentration of the species (mol m-3)
        constant_mixing_ratio_mol_mol (float): Constant mixing ratio of the species (mol mol-1)
        is_third_body (bool): Whether the species is a third body.
        other_properties (Dict[str, Any]): A dictionary of other properties of the species.
    """


ReactionComponent.__init__ = __init__
ReactionComponent.serialize = serialize
