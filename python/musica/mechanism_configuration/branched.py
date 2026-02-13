from typing import Optional, Any, Dict, List, Union, Tuple
from .. import backend
from .._base import CppWrapper, CppField, _unwrap_list, _wrap_list
from .utils import _add_other_properties, _remove_empty_keys, _convert_components
from .species import Species
from .phase import Phase
from .reaction_component import ReactionComponent
from .ancillary import ReactionType

_backend = backend.get_backend()
_Branched = _backend._mechanism_configuration._Branched


class Branched(CppWrapper):
    """A branched reaction rate constant.

    Attributes:
        name: The name of the branched reaction rate constant.
        X: Pre-exponential branching factor [(mol m-3)^-(n-1)s-1].
        Y: Exponential branching factor [K-1].
        a0: Z parameter [unitless].
        n: A parameter [unitless].
        reactants: A list of reactants involved in the reaction.
        nitrate_products: A list of products formed in the nitrate branch.
        alkoxy_products: A list of products formed in the alkoxy branch.
        gas_phase: The gas phase in which the reaction occurs.
        other_properties: A dictionary of other properties.
    """

    name = CppField()
    X = CppField()
    Y = CppField()
    a0 = CppField()
    n = CppField()
    gas_phase = CppField()
    other_properties = CppField()

    def __init__(
        self,
        name: Optional[str] = None,
        X: Optional[float] = None,
        Y: Optional[float] = None,
        a0: Optional[float] = None,
        n: Optional[float] = None,
        reactants: Optional[List[Union[Species, Tuple[float, Species]]]] = None,
        nitrate_products: Optional[List[Union[Species, Tuple[float, Species]]]] = None,
        alkoxy_products: Optional[List[Union[Species, Tuple[float, Species]]]] = None,
        gas_phase: Optional[Phase] = None,
        other_properties: Optional[Dict[str, Any]] = None,
    ):
        """Initialize the Branched reaction.

        Args:
            name: The name of the branched reaction rate constant.
            X: Pre-exponential branching factor.
            Y: Exponential branching factor [K-1].
            a0: Z parameter [unitless].
            n: A parameter [unitless].
            reactants: A list of reactants involved in the reaction.
            nitrate_products: A list of products formed in the nitrate branch.
            alkoxy_products: A list of products formed in the alkoxy branch.
            gas_phase: The gas phase in which the reaction occurs.
            other_properties: A dictionary of other properties.
        """
        self._cpp = _Branched()

        self.name = name if name is not None else self.name
        self.X = X if X is not None else self.X
        self.Y = Y if Y is not None else self.Y
        self.a0 = a0 if a0 is not None else self.a0
        self.n = n if n is not None else self.n
        self.gas_phase = gas_phase.name if gas_phase is not None else self.gas_phase
        self.other_properties = other_properties if other_properties is not None else self.other_properties
        self.reactants = (
            _convert_components(reactants)
            if reactants is not None
            else self.reactants
        )
        self.nitrate_products = (
            _convert_components(nitrate_products)
            if nitrate_products is not None
            else self.nitrate_products
        )
        self.alkoxy_products = (
            _convert_components(alkoxy_products)
            if alkoxy_products is not None
            else self.alkoxy_products
        )

    @property
    def type(self):
        """Get the reaction type."""
        return ReactionType.Branched

    @property
    def reactants(self) -> list:
        return _wrap_list(ReactionComponent, self._cpp.reactants)

    @reactants.setter
    def reactants(self, value):
        self._cpp.reactants = _unwrap_list(value)

    @property
    def nitrate_products(self) -> list:
        return _wrap_list(ReactionComponent, self._cpp.nitrate_products)

    @nitrate_products.setter
    def nitrate_products(self, value):
        self._cpp.nitrate_products = _unwrap_list(value)

    @property
    def alkoxy_products(self) -> list:
        return _wrap_list(ReactionComponent, self._cpp.alkoxy_products)

    @alkoxy_products.setter
    def alkoxy_products(self, value):
        self._cpp.alkoxy_products = _unwrap_list(value)

    def serialize(self) -> Dict:
        serialize_dict = {
            "type": "BRANCHED_NO_RO2",
            "name": self.name,
            "X": self.X,
            "Y": self.Y,
            "a0": self.a0,
            "n": self.n,
            "reactants": [r.serialize() for r in self.reactants],
            "nitrate products": [r.serialize() for r in self.nitrate_products],
            "alkoxy products": [r.serialize() for r in self.alkoxy_products],
            "gas phase": self.gas_phase,
        }
        _add_other_properties(serialize_dict, self.other_properties)
        return _remove_empty_keys(serialize_dict)
