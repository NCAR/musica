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


ReactionComponent.__doc__ = """
    A class representing a reaction component in a chemical reaction.

    A reaction component typically consists of a chemical species, its stoichiometric coefficient in the reaction,
    and any additional properties relevant to its role in the reaction.

    Attributes:
        species_name (str): The name of the chemical species involved in the reaction.
        coefficient (float): The stoichiometric coefficient of the species in the reaction.
        other_properties (Dict[str, Any]): A dictionary of other properties relevant to the reaction component.
    """


ReactionComponent.__init__ = __init__
ReactionComponent.serialize = serialize
