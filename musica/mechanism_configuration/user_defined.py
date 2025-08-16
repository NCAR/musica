from typing import Optional, Any, Dict, List, Union, Tuple
from .. import backend
from .phase import Phase
from .species import Species
from .reactions import ReactionComponentSerializer
from .utils import _add_other_properties

_backend = backend.get_backend()
_UserDefined = _backend._mechanism_configuration._UserDefined
_ReactionComponent = _backend._mechanism_configuration._ReactionComponent


class UserDefined(_UserDefined):
    """
    A class representing a user-defined reaction rate constant.

    Attributes:
        name (str): The name of the photolysis reaction rate constant.
        scaling_factor (float): The scaling factor for the photolysis rate constant.
        reactants (List[Union[Species, Tuple[float, Species]]]): A list of reactants involved in the reaction.
        products (List[Union[Species, Tuple[float, Species]]]): A list of products formed in the reaction.
        gas_phase (Phase): The gas phase in which the reaction occurs.
        other_properties (Dict[str, Any]): A dictionary of other properties of the photolysis reaction rate constant.
    """

    def __init__(
        self,
        name: Optional[str] = None,
        scaling_factor: Optional[float] = None,
        reactants: Optional[List[Union[Species,
                                       Tuple[float, Species]]]] = None,
        products: Optional[List[Union[Species, Tuple[float, Species]]]] = None,
        gas_phase: Optional[Phase] = None,
        other_properties: Optional[Dict[str, Any]] = None,
    ):
        """
        Initializes the UserDefined object with the given parameters.

        Args:
            name (str): The name of the photolysis reaction rate constant.
            scaling_factor (float): The scaling factor for the photolysis rate constant.
            reactants (List[Union[Species, Tuple[float, Species]]]): A list of reactants involved in the reaction.
            products (List[Union[Species, Tuple[float, Species]]]): A list of products formed in the reaction.
            gas_phase (Phase): The gas phase in which the reaction occurs.
            other_properties (Dict[str, Any]): A dictionary of other properties of the photolysis reaction rate constant.
        """
        super().__init__()
        self.name = name if name is not None else self.name
        self.scaling_factor = scaling_factor if scaling_factor is not None else self.scaling_factor
        self.reactants = (
            [
                (
                    _ReactionComponent(r.name)
                    if isinstance(r, Species)
                    else _ReactionComponent(r[1].name, r[0])
                )
                for r in reactants
            ]
            if reactants is not None
            else self.reactants
        )
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
            "type": "USER_DEFINED",
            "name": instance.name,
            "scaling factor": instance.scaling_factor,
            "reactants": ReactionComponentSerializer.serialize_list_reaction_components(instance.reactants),
            "products": ReactionComponentSerializer.serialize_list_reaction_components(instance.products),
            "gas phase": instance.gas_phase,
        }
        _add_other_properties(serialize_dict, instance.other_properties)
        return serialize_dict
