from .utils import _add_other_properties
from .reactions import ReactionComponentSerializer
from .species import Species
from .phase import Phase
from typing import Optional, Any, Dict, List, Union, Tuple
from .. import backend

_backend = backend.get_backend()
_Emission = _backend._mechanism_configuration._Emission
_ReactionComponent = _backend._mechanism_configuration._ReactionComponent


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
    def serialize(instance) -> Dict:
        serialize_dict = {
            "type": "EMISSION",
            "name": instance.name,
            "scaling factor": instance.scaling_factor,
            "products": ReactionComponentSerializer.serialize_list_reaction_components(instance.products),
            "gas phase": instance.gas_phase,
        }
        _add_other_properties(serialize_dict, instance.other_properties)
        return serialize_dict
