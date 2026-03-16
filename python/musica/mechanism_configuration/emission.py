from typing import Optional, Any, Dict, List, Union, Tuple
from .. import backend
from .._base import CppWrapper, CppField, _unwrap_list, _wrap_list
from .utils import _add_other_properties, _remove_empty_keys, _convert_components
from .species import Species
from .phase import Phase
from .reaction_component import ReactionComponent
from .ancillary import ReactionType

_backend = backend.get_backend()
_Emission = _backend._mechanism_configuration._Emission


class Emission(CppWrapper):
    """An emission reaction rate constant.

    Attributes:
        name: The name of the emission reaction rate constant.
        scaling_factor: The scaling factor for the emission rate constant.
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
        products: Optional[List[Union[Species, Tuple[float, Species]]]] = None,
        gas_phase: Optional[Phase] = None,
        other_properties: Optional[Dict[str, Any]] = None,
    ):
        """Initialize the Emission reaction.

        Args:
            name: The name of the emission reaction rate constant.
            scaling_factor: The scaling factor for the emission rate constant.
            products: A list of products formed in the reaction.
            gas_phase: The gas phase in which the reaction occurs.
            other_properties: A dictionary of other properties.
        """
        self._cpp = _Emission()
        self.name = name if name is not None else self.name
        self.scaling_factor = scaling_factor if scaling_factor is not None else self.scaling_factor
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
        return ReactionType.Emission

    @property
    def products(self) -> list:
        return _wrap_list(ReactionComponent, self._cpp.products)

    @products.setter
    def products(self, value):
        self._cpp.products = _unwrap_list(value)

    def serialize(self) -> Dict:
        serialize_dict = {
            "type": "EMISSION",
            "name": self.name,
            "scaling factor": self.scaling_factor,
            "products": [r.serialize() for r in self.products],
            "gas phase": self.gas_phase,
        }
        _add_other_properties(serialize_dict, self.other_properties)
        return serialize_dict
