from typing import Optional, Any, Dict, List, Union, Tuple
from .. import backend
from .._base import CppWrapper, CppField, _unwrap_list, _wrap_list
from .phase import Phase
from .species import Species
from .utils import _add_other_properties, _remove_empty_keys, _convert_components
from .reaction_component import ReactionComponent
from .ancillary import ReactionType

_backend = backend.get_backend()
_UserDefined = _backend._mechanism_configuration._UserDefined


class UserDefined(CppWrapper):
    """A user-defined reaction rate constant.

    Attributes:
        name: The name of the user-defined reaction rate constant.
        scaling_factor: The scaling factor for the rate constant.
        reactants: A list of reactants involved in the reaction.
        products: A list of products formed in the reaction.
        gas_phase: The gas phase in which the reaction occurs.
        other_properties: A dictionary of other properties.
    """

    name = CppField()
    scaling_factor = CppField()
    gas_phase = CppField()
    other_properties = CppField()

    def __init__(
        self,
        name: Optional[str] = None,
        scaling_factor: Optional[float] = None,
        reactants: Optional[List[Union[Species, Tuple[float, Species]]]] = None,
        products: Optional[List[Union[Species, Tuple[float, Species]]]] = None,
        gas_phase: Optional[Phase] = None,
        other_properties: Optional[Dict[str, Any]] = None,
    ):
        """Initialize the UserDefined reaction.

        Args:
            name: The name of the user-defined reaction rate constant.
            scaling_factor: The scaling factor for the rate constant.
            reactants: A list of reactants involved in the reaction.
            products: A list of products formed in the reaction.
            gas_phase: The gas phase in which the reaction occurs.
            other_properties: A dictionary of other properties.
        """
        self._cpp = _UserDefined()
        self.name = name if name is not None else self.name
        self.scaling_factor = scaling_factor if scaling_factor is not None else self.scaling_factor
        self.reactants = (
            _convert_components(reactants)
            if reactants is not None
            else self.reactants
        )
        self.products = (
            _convert_components(products)
            if products is not None
            else self.products
        )
        self.gas_phase = gas_phase.name if gas_phase is not None else self.gas_phase
        self.other_properties = other_properties if other_properties is not None else self.other_properties

    @property
    def type(self):
        """Get the reaction type."""
        return ReactionType.UserDefined

    @property
    def reactants(self) -> list:
        return _wrap_list(ReactionComponent, self._cpp.reactants)

    @reactants.setter
    def reactants(self, value):
        self._cpp.reactants = _unwrap_list(value)

    @property
    def products(self) -> list:
        return _wrap_list(ReactionComponent, self._cpp.products)

    @products.setter
    def products(self, value):
        self._cpp.products = _unwrap_list(value)

    def serialize(self) -> Dict:
        serialize_dict = {
            "type": "USER_DEFINED",
            "name": self.name,
            "scaling factor": self.scaling_factor,
            "reactants": [r.serialize() for r in self.reactants],
            "products": [r.serialize() for r in self.products],
            "gas phase": self.gas_phase,
        }
        _add_other_properties(serialize_dict, self.other_properties)
        return _remove_empty_keys(serialize_dict)
