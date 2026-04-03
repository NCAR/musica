from typing import Optional, Any, Dict, List, Union
from .. import backend
from .._base import CppWrapper, CppField, _unwrap_list, _wrap_list
from .species import Species
from .phase_species import PhaseSpecies
from .utils import _add_other_properties, _remove_empty_keys

_backend = backend.get_backend()
_Phase = _backend._mechanism_configuration._Phase


class Phase(CppWrapper):
    """A phase in a chemical mechanism.

    Attributes:
        name: The name of the phase.
        species: A list of species in the phase.
        other_properties: A dictionary of other properties of the phase.
    """

    name = CppField()
    other_properties = CppField()

    def __init__(
        self,
        name: Optional[str] = None,
        species: Optional[Union[List[Species], List[PhaseSpecies]]] = None,
        other_properties: Optional[Dict[str, Any]] = None,
    ):
        """Initialize the Phase.

        Args:
            name: The name of the phase.
            species: A list of species in the phase. Species objects are
                     automatically converted to PhaseSpecies.
            other_properties: A dictionary of other properties of the phase.
        """
        self._cpp = _Phase()
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

    @property
    def species(self) -> List[PhaseSpecies]:
        """List of species in the phase."""
        return _wrap_list(PhaseSpecies, self._cpp.species)

    @species.setter
    def species(self, value: List[PhaseSpecies]):
        self._cpp.species = _unwrap_list(value)

    def serialize(self):
        serialize_dict = {
            "name": self.name,
            "species": [s.serialize() for s in self.species],
        }
        _add_other_properties(serialize_dict, self.other_properties)
        return _remove_empty_keys(serialize_dict)
