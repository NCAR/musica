from typing import Optional, Any, Dict, List
from .. import backend
from .species import Species
from .utils import _add_other_properties, _remove_empty_keys

_backend = backend.get_backend()
_Phase = _backend._mechanism_configuration._Phase


class Phase(_Phase):
    """
    A class representing a phase in a chemical mechanism.

    Attributes:
        name (str): The name of the phase.
        species (List[Species]): A list of species in the phase.
        other_properties (Dict[str, Any]): A dictionary of other properties of the phase.
    """

    def __init__(
        self,
        name: Optional[str] = None,
        species: Optional[List[Species]] = None,
        other_properties: Optional[Dict[str, Any]] = None,
    ):
        """
        Initializes the Phase object with the given parameters.

        Args:
            name (str): The name of the phase.
            species (List[Species]): A list of species in the phase.
            other_properties (Dict[str, Any]): A dictionary of other properties of the phase.
        """
        super().__init__()
        self.name = name if name is not None else self.name
        self.species = [
            s.name for s in species] if species is not None else self.species
        self.other_properties = other_properties if other_properties is not None else self.other_properties

    @staticmethod
    def serialize(instance):
        serialize_dict = {
            "name": instance.name,
            "species": instance.species,
        }
        _add_other_properties(serialize_dict, instance.other_properties)
        return _remove_empty_keys(serialize_dict)
