from typing import Optional, Any, Dict
from ... import backend
from ..._base import CppWrapper, CppField
from ..utils import _add_other_properties, _remove_empty_keys

_backend = backend.get_backend()
_mc = _backend._mechanism_configuration


class Species(CppWrapper):
    """A species in a chemical mechanism.

    Attributes:
        absolute_tolerance: Absolute tolerance for the species in the solver (mol m-3).
        constant_concentration_mol_m3: Constant concentration of the species (mol m-3).
        constant_mixing_ratio_mol_mol: Constant mixing ratio of the species (mol mol-1).
        is_third_body: Whether the species is a third body.
        molecular_weight_kg_mol: Molecular weight [kg mol-1].
        name: The name of the species.
        other_properties: A dictionary of other properties of the species.
    """

    absolute_tolerance = CppField()
    constant_concentration_mol_m3 = CppField()
    constant_mixing_ratio_mol_mol = CppField()
    is_third_body = CppField()
    molecular_weight_kg_mol = CppField()
    name = CppField()
    other_properties = CppField()

    def __init__(
        self,
        absolute_tolerance: Optional[float] = None,
        constant_concentration_mol_m3: Optional[float] = None,
        constant_mixing_ratio_mol_mol: Optional[float] = None,
        is_third_body: Optional[bool] = False,
        molecular_weight_kg_mol: Optional[float] = None,
        name: Optional[str] = None,
        other_properties: Optional[Dict[str, Any]] = None,
    ):
        """Initialize the Species.

        Args:
            absolute_tolerance: Absolute tolerance for the species in the solver [dimensionless].
            constant_concentration_mol_m3: Constant concentration of the species (mol m-3).
            constant_mixing_ratio_mol_mol: Constant mixing ratio of the species (mol mol-1).
            is_third_body: Whether the species is a third body.
            molecular_weight_kg_mol: Molecular weight [kg mol-1].
            name: The name of the species.
            other_properties: A dictionary of other properties of the species.
        """
        self._cpp = _mc._Species()
        self.absolute_tolerance = absolute_tolerance if absolute_tolerance is not None else self.absolute_tolerance
        self.constant_concentration_mol_m3 = constant_concentration_mol_m3 if constant_concentration_mol_m3 is not None else self.constant_concentration_mol_m3
        self.constant_mixing_ratio_mol_mol = constant_mixing_ratio_mol_mol if constant_mixing_ratio_mol_mol is not None else self.constant_mixing_ratio_mol_mol
        self.is_third_body = is_third_body
        self.molecular_weight_kg_mol = molecular_weight_kg_mol if molecular_weight_kg_mol is not None else self.molecular_weight_kg_mol
        self.name = name if name is not None else self.name
        self.other_properties = other_properties if other_properties is not None else self.other_properties

    def serialize(self) -> Dict:
        serialize_dict = {
            "absolute tolerance": self.absolute_tolerance,
            "constant concentration [mol m-3]": self.constant_concentration_mol_m3,
            "constant mixing ratio [mol mol-1]": self.constant_mixing_ratio_mol_mol,
            "is third body": self.is_third_body,
            "molecular weight [kg mol-1]": self.molecular_weight_kg_mol,
            "name": self.name,
        }
        _add_other_properties(serialize_dict, self.other_properties)
        return _remove_empty_keys(serialize_dict)
