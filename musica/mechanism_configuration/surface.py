from typing import Optional, Any, Dict, List, Union, Tuple
from .. import backend
from .phase import Phase
from .species import Species
from .reactions import ReactionComponentSerializer
from .utils import _add_other_properties

_backend = backend.get_backend()
_Surface = _backend._mechanism_configuration._Surface
_ReactionComponent = _backend._mechanism_configuration._ReactionComponent


class Surface(_Surface):
    """
    A class representing a surface in a chemical mechanism.

    (TODO: get details from MusicBox)

    Attributes:
        name (str): The name of the surface.
        reaction_probability (float): The probability of a reaction occurring on the surface.
        gas_phase_species (Union[Species, Tuple[float, Species]]): The gas phase species involved in the reaction.
        gas_phase_products (List[Union[Species, Tuple[float, Species]]]): The gas phase products formed in the reaction.
        gas_phase (Phase): The gas phase in which the reaction occurs.
        condensed_phase (Phase): The condensed phase in which the reaction occurs.
        other_properties (Dict[str, Any]): A dictionary of other properties of the surface.
    """

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
        condensed_phase: Optional[Phase] = None,
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
            condensed_phase (Phase): The condensed phase in which the reaction occurs.
            other_properties (Dict[str, Any]): A dictionary of other properties of the surface.
        """
        super().__init__()
        self.name = name if name is not None else self.name
        self.reaction_probability = reaction_probability if reaction_probability is not None else self.reaction_probability
        self.gas_phase_species = (
            (
                _ReactionComponent(gas_phase_species.name)
                if isinstance(gas_phase_species, Species)
                else _ReactionComponent(gas_phase_species[1].name, gas_phase_species[0])
            )
            if gas_phase_species is not None
            else self.gas_phase_species
        )
        self.gas_phase_products = (
            [
                (
                    _ReactionComponent(p.name)
                    if isinstance(p, Species)
                    else _ReactionComponent(p[1].name, p[0])
                )
                for p in gas_phase_products
            ]
            if gas_phase_products is not None
            else self.gas_phase_products
        )
        self.gas_phase = gas_phase.name if gas_phase is not None else self.gas_phase
        self.condensed_phase = condensed_phase.name if condensed_phase is not None else self.condensed_phase
        self.other_properties = other_properties if other_properties is not None else self.other_properties

    @staticmethod
    def serialize(instance) -> Dict:
        serialize_dict = {
            "type": "SURFACE",
            "name": instance.name,
            "reaction probability": instance.reaction_probability,
            "gas-phase species": instance.gas_phase_species.species_name,
            "gas-phase products": ReactionComponentSerializer.serialize_list_reaction_components(instance.gas_phase_products),
            "gas phase": instance.gas_phase,
            "condensed phase": instance.condensed_phase,
        }
        _add_other_properties(serialize_dict, instance.other_properties)
        return serialize_dict
