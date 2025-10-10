from typing import Optional, Any, Dict
from .. import backend
from .utils import _add_other_properties, _remove_empty_keys

_backend = backend.get_backend()
Species = _backend._mechanism_configuration._Species
original_init = Species.__init__


def __init__(
    self,
    name: Optional[str] = None,
    molecular_weight_kg_mol: Optional[float] = None,
    constant_concentration_mol_m3: Optional[float] = None,
    constant_mixing_ratio_mol_mol: Optional[float] = None,
    is_third_body: Optional[bool] = False,
    other_properties: Optional[Dict[str, Any]] = None,
):
    """
    Initializes the Species object with the given parameters.

    Args:
        name (str): The name of the species.
        molecular_weight_kg_mol (float): Molecular weight [kg mol-1]
        constant_concentration_mol_m3 (float): Constant concentration of the species (mol m-3)
        constant_mixing_ratio_mol_mol (float): Constant mixing ratio of the species (mol mol-1)
        is_third_body (bool): Whether the species is a third body.
        other_properties (Dict[str, Any]): A dictionary of other properties of the species.
    """
    original_init(self)
    self.name = name if name is not None else self.name
    self.molecular_weight_kg_mol = molecular_weight_kg_mol if molecular_weight_kg_mol is not None else self.molecular_weight_kg_mol
    self.constant_concentration_mol_m3 = constant_concentration_mol_m3 if constant_concentration_mol_m3 is not None else self.constant_concentration_mol_m3
    self.constant_mixing_ratio_mol_mol = constant_mixing_ratio_mol_mol if constant_mixing_ratio_mol_mol is not None else self.constant_mixing_ratio_mol_mol
    self.is_third_body = is_third_body
    self.other_properties = other_properties if other_properties is not None else self.other_properties


def serialize(self) -> Dict:
    serialize_dict = {
        "name": self.name,
        "molecular weight [kg mol-1]": self.molecular_weight_kg_mol,
        "constant concentration [mol m-3]": self.constant_concentration_mol_m3,
        "constant mixing ratio [mol mol-1]": self.constant_mixing_ratio_mol_mol,
        "is third body": self.is_third_body,
    }
    _add_other_properties(serialize_dict, self.other_properties)
    return _remove_empty_keys(serialize_dict)


Species.__doc__ = """
    A class representing a species in a chemical mechanism.

    Attributes:
        name (str): The name of the species.
        molecular_weight_kg_mol (float): Molecular weight [kg mol-1]
        constant_concentration_mol_m3 (float): Constant concentration of the species (mol m-3)
        constant_mixing_ratio_mol_mol (float): Constant mixing ratio of the species (mol mol-1)
        is_third_body (bool): Whether the species is a third body.
        other_properties (Dict[str, Any]): A dictionary of other properties of the species.
    """


Species.__init__ = __init__
Species.serialize = serialize
