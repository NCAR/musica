from typing import Optional, Any, Dict, List, Union, Tuple
from .. import backend
from .._base import CppWrapper, CppField, _unwrap_list, _wrap_list
from .phase import Phase
from .species import Species
from .utils import _add_other_properties, _remove_empty_keys, _convert_components
from ..constants import BOLTZMANN
from .reaction_component import ReactionComponent
from .ancillary import ReactionType

_backend = backend.get_backend()
_Arrhenius = _backend._mechanism_configuration._Arrhenius


class Arrhenius(CppWrapper):
    """An Arrhenius rate constant.

    k = A * exp( C / T ) * ( T / D )^B * exp( 1 - E * P )

    where:
        k = rate constant
        A = pre-exponential factor [(mol m-3)^(n-1)s-1]
        B = temperature exponent [unitless]
        C = exponential term [K-1]
        D = reference temperature [K]
        E = pressure scaling term [Pa-1]
        T = temperature [K]
        P = pressure [Pa]
        n = number of reactants

    Attributes:
        name: The name of the Arrhenius rate constant.
        A: Pre-exponential factor [(mol m-3)^(n-1)s-1].
        B: Temperature exponent [unitless].
        C: Exponential term [K-1].
        D: Reference Temperature [K].
        E: Pressure scaling term [Pa-1].
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
    gas_phase = CppField()
    other_properties = CppField()

    def __init__(
        self,
        name: Optional[str] = None,
        A: Optional[float] = None,
        B: Optional[float] = None,
        C: Optional[float] = None,
        Ea: Optional[float] = None,
        D: Optional[float] = None,
        E: Optional[float] = None,
        reactants: Optional[List[Union[Species, Tuple[float, Species]]]] = None,
        products: Optional[List[Union[Species, Tuple[float, Species]]]] = None,
        gas_phase: Optional[Phase] = None,
        other_properties: Optional[Dict[str, Any]] = None,
    ):
        """Initialize the Arrhenius reaction.

        Args:
            name: The name of the Arrhenius rate constant.
            A: Pre-exponential factor [(mol m-3)^(n-1)s-1].
            B: Temperature exponent [unitless].
            C: Exponential term [K-1].
            Ea: Activation energy [J molecule-1]. Mutually exclusive with C.
            D: Reference Temperature [K].
            E: Pressure scaling term [Pa-1].
            reactants: A list of reactants involved in the reaction.
            products: A list of products formed in the reaction.
            gas_phase: The gas phase in which the reaction occurs.
            other_properties: A dictionary of other properties.
        """
        self._cpp = _Arrhenius()

        if C is not None and Ea is not None:
            raise ValueError("Cannot specify both C and Ea.")
        if Ea is not None:
            C = -Ea / BOLTZMANN

        self.name = name if name is not None else self.name
        self.A = A if A is not None else self.A
        self.B = B if B is not None else self.B
        self.C = C if C is not None else self.C
        self.D = D if D is not None else self.D
        self.E = E if E is not None else self.E
        self.gas_phase = gas_phase.name if gas_phase is not None else self.gas_phase
        self.other_properties = other_properties if other_properties is not None else self.other_properties
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

    @property
    def type(self):
        """Get the reaction type."""
        return ReactionType.Arrhenius

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
            "type": "ARRHENIUS",
            "name": self.name,
            "A": self.A,
            "B": self.B,
            "C": self.C,
            "D": self.D,
            "E": self.E,
            "reactants": [r.serialize() for r in self.reactants],
            "products": [r.serialize() for r in self.products],
            "gas phase": self.gas_phase,
        }
        _add_other_properties(serialize_dict, self.other_properties)
        return _remove_empty_keys(serialize_dict)
