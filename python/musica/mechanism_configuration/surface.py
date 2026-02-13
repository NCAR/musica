from typing import Optional, Any, Dict, List, Union, Tuple
from .. import backend
from .._base import CppWrapper, CppField, _unwrap, _unwrap_list, _wrap_list
from .phase import Phase
from .species import Species
from .utils import _add_other_properties, _remove_empty_keys, _convert_components
from .reaction_component import ReactionComponent
from .ancillary import ReactionType

_backend = backend.get_backend()
_Surface = _backend._mechanism_configuration._Surface


class Surface(CppWrapper):
    """A surface reaction in a chemical mechanism.

    Attributes:
        name: The name of the surface.
        reaction_probability: The probability of a reaction occurring on the surface.
        gas_phase_species: The gas phase species involved in the reaction.
        gas_phase_products: The gas phase products formed in the reaction.
        gas_phase: The gas phase in which the reaction occurs.
        other_properties: A dictionary of other properties of the surface.
    """

    name = CppField()
    reaction_probability = CppField()
    gas_phase = CppField()
    other_properties = CppField()

    def __init__(
        self,
        name: Optional[str] = None,
        reaction_probability: Optional[float] = None,
        gas_phase_species: Optional[Union[Species, Tuple[float, Species]]] = None,
        gas_phase_products: Optional[List[Union[Species, Tuple[float, Species]]]] = None,
        gas_phase: Optional[Phase] = None,
        other_properties: Optional[Dict[str, Any]] = None,
    ):
        """Initialize the Surface reaction.

        Args:
            name: The name of the surface.
            reaction_probability: The probability of a reaction occurring on the surface.
            gas_phase_species: The gas phase species involved in the reaction.
            gas_phase_products: The gas phase products formed in the reaction.
            gas_phase: The gas phase in which the reaction occurs.
            other_properties: A dictionary of other properties of the surface.
        """
        self._cpp = _Surface()
        self.name = name if name is not None else self.name
        self.reaction_probability = reaction_probability if reaction_probability is not None else self.reaction_probability
        self.gas_phase_species = (
            (
                ReactionComponent(gas_phase_species.name)
                if isinstance(gas_phase_species, Species)
                else ReactionComponent(gas_phase_species[1].name, gas_phase_species[0])
            )
            if gas_phase_species is not None
            else self.gas_phase_species
        )
        self.gas_phase_products = (
            _convert_components(gas_phase_products)
            if gas_phase_products is not None
            else self.gas_phase_products
        )
        self.gas_phase = gas_phase.name if gas_phase is not None else self.gas_phase
        self.other_properties = other_properties if other_properties is not None else self.other_properties

    @property
    def type(self):
        """Get the reaction type."""
        return ReactionType.Surface

    @property
    def gas_phase_species(self) -> ReactionComponent:
        return ReactionComponent._from_cpp(self._cpp.gas_phase_species)

    @gas_phase_species.setter
    def gas_phase_species(self, value):
        self._cpp.gas_phase_species = _unwrap(value)

    @property
    def gas_phase_products(self) -> list:
        return _wrap_list(ReactionComponent, self._cpp.gas_phase_products)

    @gas_phase_products.setter
    def gas_phase_products(self, value):
        self._cpp.gas_phase_products = _unwrap_list(value)

    def serialize(self) -> Dict:
        serialize_dict = {
            "type": "SURFACE",
            "name": self.name,
            "reaction probability": self.reaction_probability,
            "gas-phase species": self.gas_phase_species.species_name,
            "gas-phase products": [r.serialize() for r in self.gas_phase_products],
            "gas phase": self.gas_phase,
        }
        _add_other_properties(serialize_dict, self.other_properties)
        return _remove_empty_keys(serialize_dict)
