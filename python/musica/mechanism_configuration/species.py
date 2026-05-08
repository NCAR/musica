from typing import Optional, Any, Dict
from .. import backend
from .._base import CppWrapper, CppField
from .utils import _add_other_properties, _remove_empty_keys

_backend = backend.get_backend()
_Species = _backend._mechanism_configuration._Species


class Species(CppWrapper):
    """A species in a chemical mechanism.

    Attributes:
        name: The name of the species.
        molecular_weight_kg_mol: Molecular weight [kg mol-1].
        density_kg_m3: Density [kg m-3].
        constant_concentration_mol_m3: Constant concentration of the species (mol m-3).
        constant_mixing_ratio_mol_mol: Constant mixing ratio of the species (mol mol-1).
        is_third_body: Whether the species is a third body.
        other_properties: A dictionary of other properties of the species.
    """

    name = CppField()
    molecular_weight_kg_mol = CppField()
    constant_concentration_mol_m3 = CppField()
    constant_mixing_ratio_mol_mol = CppField()
    is_third_body = CppField()
    other_properties = CppField()

    _DENSITY_KEY = "density [kg m-3]"

    def __init__(
        self,
        name: Optional[str] = None,
        molecular_weight_kg_mol: Optional[float] = None,
        density_kg_m3: Optional[float] = None,
        constant_concentration_mol_m3: Optional[float] = None,
        constant_mixing_ratio_mol_mol: Optional[float] = None,
        is_third_body: Optional[bool] = False,
        other_properties: Optional[Dict[str, Any]] = None,
    ):
        """Initialize the Species.

        Args:
            name: The name of the species.
            molecular_weight_kg_mol: Molecular weight [kg mol-1].
            density_kg_m3: Density [kg m-3].
            constant_concentration_mol_m3: Constant concentration of the species (mol m-3).
            constant_mixing_ratio_mol_mol: Constant mixing ratio of the species (mol mol-1).
            is_third_body: Whether the species is a third body.
            other_properties: A dictionary of other properties of the species.
        """
        self._cpp = _Species()
        self.name = name if name is not None else self.name
        self.molecular_weight_kg_mol = molecular_weight_kg_mol if molecular_weight_kg_mol is not None else self.molecular_weight_kg_mol
        self.constant_concentration_mol_m3 = constant_concentration_mol_m3 if constant_concentration_mol_m3 is not None else self.constant_concentration_mol_m3
        self.constant_mixing_ratio_mol_mol = constant_mixing_ratio_mol_mol if constant_mixing_ratio_mol_mol is not None else self.constant_mixing_ratio_mol_mol
        self.is_third_body = is_third_body
        self.other_properties = other_properties if other_properties is not None else self.other_properties
        if density_kg_m3 is not None:
            self.density_kg_m3 = density_kg_m3

    @property
    def density_kg_m3(self) -> Optional[float]:
        """Density [kg m-3]."""
        props = self.other_properties
        if props and self._DENSITY_KEY in props:
            return float(props[self._DENSITY_KEY])
        return None

    @density_kg_m3.setter
    def density_kg_m3(self, value: Optional[float]):
        """Set density [kg m-3]."""
        props = self.other_properties
        if props is None:
            props = {}
        if value is not None:
            props[self._DENSITY_KEY] = str(value)
        elif self._DENSITY_KEY in props:
            del props[self._DENSITY_KEY]
        self.other_properties = props

    def serialize(self) -> Dict:
        serialize_dict = {
            "name": self.name,
            "molecular weight [kg mol-1]": self.molecular_weight_kg_mol,
            "density [kg m-3]": self.density_kg_m3,
            "constant concentration [mol m-3]": self.constant_concentration_mol_m3,
            "constant mixing ratio [mol mol-1]": self.constant_mixing_ratio_mol_mol,
            "is third body": self.is_third_body,
        }
        _add_other_properties(serialize_dict, self.other_properties)
        return _remove_empty_keys(serialize_dict)
