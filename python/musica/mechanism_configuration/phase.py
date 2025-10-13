from typing import Optional, Any, Dict, List, Union
from .. import backend
from .species import Species
from .phase_species import PhaseSpecies
from .utils import _add_other_properties, _remove_empty_keys

_backend = backend.get_backend()
Phase = _backend._mechanism_configuration._Phase
Phase.__doc__ = """
    A class representing a phase in a chemical mechanism.

    Attributes:
        name (str): The name of the phase.
        species (List[Species]): A list of species in the phase.
        other_properties (Dict[str, Any]): A dictionary of other properties of the phase.
    """

original_init = Phase.__init__


def init(
    self,
    name: Optional[str] = None,
    species: Optional[Union[List[Species], List[PhaseSpecies]]] = None,
    other_properties: Optional[Dict[str, Any]] = None,
):
    """
    Initializes the Phase object with the given parameters.

    Args:
        name (str): The name of the phase.
        species (List[Species]): A list of species in the phase.
        other_properties (Dict[str, Any]): A dictionary of other properties of the phase.
    """
    original_init(self)
    self.name = name if name is not None else self.name
    converted_species = []
    if species is not None:
        for s in species:
            if isinstance(s, PhaseSpecies):
                converted_species.append(s)
            elif isinstance(s, Species):
                converted_species.append(PhaseSpecies(name=s.name))
    self.species = converted_species
    self.other_properties = other_properties if other_properties is not None else self.other_properties


def serialize(instance):
    serialize_dict = {
        "name": instance.name,
        "species": [s.serialize() for s in instance.species],
    }
    _add_other_properties(serialize_dict, instance.other_properties)
    return _remove_empty_keys(serialize_dict)


Phase.__init__ = init
Phase.serialize = serialize
