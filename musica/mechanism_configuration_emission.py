from typing import Optional, Any, Dict, List, Union, Tuple
from musica import _Emission, _ReactionComponent
from musica.mechanism_configuration_phase import Phase
from musica.mechanism_configuration_species import Species
from musica.mechanism_configuration_reactions import ReactionComponentSerializer
from musica.mechanism_configuration_utils import add_other_properties, remove_empty_keys


class Emission(_Emission):
    """
    A class representing an emission reaction rate constant.

    Attributes:
        name (str): The name of the emission reaction rate constant.
        scaling_factor (float): The scaling factor for the emission rate constant.
        products (List[Union[Species, Tuple[float, Species]]]): A list of products formed in the reaction.
        gas_phase (Phase): The gas phase in which the reaction occurs.
        other_properties (Dict[str, Any]): A dictionary of other properties of the emission reaction rate constant.
    """

    def __init__(
        self,
        name: Optional[str] = None,
        scaling_factor: Optional[float] = None,
        products: Optional[List[Union[Species, Tuple[float, Species]]]] = None,
        gas_phase: Optional[Phase] = None,
        other_properties: Optional[Dict[str, Any]] = None,
    ):
        """
        Initializes the Emission object with the given parameters.

        Args:
            name (str): The name of the emission reaction rate constant.
            scaling_factor (float): The scaling factor for the emission rate constant.
            products (List[Union[Species, Tuple[float, Species]]]): A list of products formed in the reaction.
            gas_phase (Phase): The gas phase in which the reaction occurs.
            other_properties (Dict[str, Any]): A dictionary of other properties of the emission reaction rate constant.
        """
        super().__init__()
        self.name = name if name is not None else self.name
        self.scaling_factor = scaling_factor if scaling_factor is not None else self.scaling_factor
        self.products = (
            [
                (
                    _ReactionComponent(p.name)
                    if isinstance(p, Species)
                    else _ReactionComponent(p[1].name, p[0])
                )
                for p in products
            ]
            if products is not None
            else self.products
        )
        self.gas_phase = gas_phase.name if gas_phase is not None else self.gas_phase
        self.other_properties = other_properties if other_properties is not None else self.other_properties

    @staticmethod
    def serialize(cls) -> Dict:
        serialize_dict = {
            "type": "EMISSION",
            "name": cls.name,
            "scaling factor": cls.scaling_factor,
            "products": ReactionComponentSerializer.serialize_list_reaction_components(cls.products),
            "gas phase": cls.gas_phase,
        }
        add_other_properties(serialize_dict, cls.other_properties)
        return remove_empty_keys(serialize_dict)
