from typing import Optional, Any, Dict
from .. import backend
from .._base import CppWrapper, CppField
from .utils import _add_other_properties, _remove_empty_keys

_backend = backend.get_backend()
_ReactionComponent = _backend._mechanism_configuration._ReactionComponent


class ReactionComponent(CppWrapper):
    """A reaction component in a chemical reaction.

    A reaction component typically consists of a chemical species, its
    stoichiometric coefficient in the reaction, and any additional properties
    relevant to its role in the reaction.

    Attributes:
        species_name: The name of the chemical species involved in the reaction.
        coefficient: The stoichiometric coefficient of the species in the reaction.
        other_properties: A dictionary of other properties relevant to the reaction component.
    """

    species_name = CppField()
    coefficient = CppField()
    other_properties = CppField()

    def __init__(
        self,
        name: Optional[str] = None,
        coefficient: Optional[float] = 1.0,
        other_properties: Optional[Dict[str, Any]] = None,
    ):
        """Initialize the ReactionComponent.

        Args:
            name: The name of the species.
            coefficient: The stoichiometric coefficient.
            other_properties: A dictionary of other properties of the species.
        """
        self._cpp = _ReactionComponent()
        self.species_name = name if name is not None else self.species_name
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
