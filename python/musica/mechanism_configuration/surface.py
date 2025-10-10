from typing import Optional, Any, Dict, List, Union, Tuple
from .. import backend
from .phase import Phase
from .species import Species
from .utils import _add_other_properties, _remove_empty_keys
from .reaction_component import ReactionComponent
from .ancillary import ReactionType

_backend = backend.get_backend()
Surface = _backend._mechanism_configuration._Surface

original_init = Surface.__init__


@property
def type(self):
    return ReactionType.Surface


def __init__(
    self,
    name: Optional[str] = None,
    reaction_probability: Optional[float] = None,
    gas_phase_species: Optional[Union[Species,
                                      Tuple[float, Species]]] = None,
    gas_phase_products: Optional[
        List[Union[Species, Tuple[float, Species]]]
    ] = None,
    gas_phase: Optional[Phase] = None,
    other_properties: Optional[Dict[str, Any]] = None,
):
    """
    Initializes the Surface object with the given parameters.

    Args:
        name (str): The name of the surface.
        reaction_probability (float): The probability of a reaction occurring on the surface.
        gas_phase_species (Union[Species, Tuple[float, Species]]): The gas phase species involved in the reaction.
        gas_phase_products (List[Union[Species, Tuple[float, Species]]]): The gas phase products formed in the reaction.
        gas_phase (Phase): The gas phase in which the reaction occurs.
        other_properties (Dict[str, Any]): A dictionary of other properties of the surface.
    """
    original_init(self)
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
        [
            (
                ReactionComponent(p.name)
                if isinstance(p, Species)
                else ReactionComponent(p[1].name, p[0])
            )
            for p in gas_phase_products
        ]
        if gas_phase_products is not None
        else self.gas_phase_products
    )
    self.gas_phase = gas_phase.name if gas_phase is not None else self.gas_phase
    self.other_properties = other_properties if other_properties is not None else self.other_properties


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


Surface.__doc__ = """
A class representing a surface in a chemical mechanism.

Attributes:
    name (str): The name of the surface.
    reaction_probability (float): The probability of a reaction occurring on the surface.
    gas_phase_species (Union[Species, Tuple[float, Species]]): The gas phase species involved in the reaction.
    gas_phase_products (List[Union[Species, Tuple[float, Species]]]): The gas phase products formed in the reaction.
    gas_phase (Phase): The gas phase in which the reaction occurs.
    other_properties (Dict[str, Any]): A dictionary of other properties of the surface.
"""

Surface.__init__ = __init__
Surface.serialize = serialize
Surface.type = type
