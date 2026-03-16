from typing import Optional, Any, Dict, List, Union, Tuple
from .. import backend
from .._base import CppWrapper, CppField, _unwrap_list, _wrap_list
from .utils import _add_other_properties, _remove_empty_keys, _convert_components
from .phase import Phase
from .species import Species
from .reaction_component import ReactionComponent
from .ancillary import ReactionType

_backend = backend.get_backend()
_TaylorSeries = _backend._mechanism_configuration._TaylorSeries


class TaylorSeries(CppWrapper):
    """A Taylor series rate constant.

    The rate constant k is represented as a Taylor series expansion in temperature:

        k = a0 + a1*T + a2*T^2 + a3*T^3 + ... + an*T^n

    Attributes:
        name: The name of the Taylor series rate constant.
        A: Optional parameter for extended forms.
        B: Optional parameter for extended forms.
        C: Optional parameter for extended forms.
        D: Optional parameter for extended forms.
        E: Optional parameter for extended forms.
        taylor_coefficients: Coefficients for the Taylor series expansion.
        reactants: A list of reactants involved in the reaction.
        products: A list of products formed in the reaction.
        gas_phase: The gas phase in which the reaction occurs.
        other_properties: A dictionary of other properties.
    """

    name = CppField()
    A = CppField()
    B = CppField()
    C = CppField()
    D = CppField()
    E = CppField()
    taylor_coefficients = CppField()
    gas_phase = CppField()
    other_properties = CppField()

    def __init__(
        self,
        name: Optional[str] = None,
        gas_phase: Optional[Phase] = None,
        A: Optional[float] = None,
        B: Optional[float] = None,
        C: Optional[float] = None,
        D: Optional[float] = None,
        E: Optional[float] = None,
        taylor_coefficients: Optional[List[float]] = None,
        reactants: Optional[List[Union[Species, Tuple[float, Species]]]] = None,
        products: Optional[List[Union[Species, Tuple[float, Species]]]] = None,
        other_properties: Optional[Dict[str, Any]] = None,
    ):
        """Initialize the TaylorSeries reaction.

        Args:
            name: The name of the TaylorSeries object.
            gas_phase: The gas phase associated with the reaction.
            A: The A coefficient for the Taylor series.
            B: The B coefficient for the Taylor series.
            C: The C coefficient for the Taylor series.
            D: The D coefficient for the Taylor series.
            E: The E coefficient for the Taylor series.
            taylor_coefficients: List of Taylor series coefficients.
            reactants: List of reactants.
            products: List of products.
            other_properties: Additional properties for the reaction.
        """
        self._cpp = _TaylorSeries()
        self.name = name if name is not None else self.name
        self.A = A if A is not None else self.A
        self.B = B if B is not None else self.B
        self.C = C if C is not None else self.C
        self.D = D if D is not None else self.D
        self.E = E if E is not None else self.E
        self.taylor_coefficients = taylor_coefficients if taylor_coefficients is not None else self.taylor_coefficients
        self.gas_phase = gas_phase.name if gas_phase is not None else self.gas_phase
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
        self.other_properties = other_properties if other_properties is not None else self.other_properties

    @property
    def type(self):
        """Get the reaction type."""
        return ReactionType.TaylorSeries

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
            "type": "TAYLOR_SERIES",
            "name": self.name,
            "A": self.A,
            "B": self.B,
            "C": self.C,
            "D": self.D,
            "E": self.E,
            "taylor coefficients": list(self.taylor_coefficients),
            "reactants": [r.serialize() for r in self.reactants],
            "products": [r.serialize() for r in self.products],
            "gas phase": self.gas_phase,
        }
        _add_other_properties(serialize_dict, self.other_properties)
        return _remove_empty_keys(serialize_dict)
