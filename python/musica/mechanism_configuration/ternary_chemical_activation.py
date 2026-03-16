from typing import Optional, Any, Dict, List, Union, Tuple
from .. import backend
from .._base import CppWrapper, CppField, _unwrap_list, _wrap_list
from .phase import Phase
from .species import Species
from .utils import _add_other_properties, _remove_empty_keys, _convert_components
from .reaction_component import ReactionComponent
from .ancillary import ReactionType

_backend = backend.get_backend()
_TernaryChemicalActivation = _backend._mechanism_configuration._TernaryChemicalActivation


class TernaryChemicalActivation(CppWrapper):
    """A Ternary Chemical Activation rate constant.

    Attributes:
        name: The name of the rate constant.
        k0_A: Pre-exponential factor for the low-pressure limit.
        k0_B: Temperature exponent for the low-pressure limit.
        k0_C: Exponential term for the low-pressure limit.
        kinf_A: Pre-exponential factor for the high-pressure limit.
        kinf_B: Temperature exponent for the high-pressure limit.
        kinf_C: Exponential term for the high-pressure limit.
        Fc: Ternary Chemical Activation parameter [unitless].
        N: Ternary Chemical Activation parameter [unitless].
        reactants: A list of reactants involved in the reaction.
        products: A list of products formed in the reaction.
        gas_phase: The gas phase in which the reaction occurs.
        other_properties: A dictionary of other properties.
    """

    name = CppField()
    k0_A = CppField()
    k0_B = CppField()
    k0_C = CppField()
    kinf_A = CppField()
    kinf_B = CppField()
    kinf_C = CppField()
    Fc = CppField()
    N = CppField()
    gas_phase = CppField()
    other_properties = CppField()

    def __init__(
        self,
        name: Optional[str] = None,
        k0_A: Optional[float] = None,
        k0_B: Optional[float] = None,
        k0_C: Optional[float] = None,
        kinf_A: Optional[float] = None,
        kinf_B: Optional[float] = None,
        kinf_C: Optional[float] = None,
        Fc: Optional[float] = None,
        N: Optional[float] = None,
        reactants: Optional[List[Union[Species, Tuple[float, Species]]]] = None,
        products: Optional[List[Union[Species, Tuple[float, Species]]]] = None,
        gas_phase: Optional[Phase] = None,
        other_properties: Optional[Dict[str, Any]] = None,
    ):
        """Initialize the Ternary Chemical Activation reaction.

        Args:
            name: The name of the rate constant.
            k0_A: Pre-exponential factor for the low-pressure limit.
            k0_B: Temperature exponent for the low-pressure limit.
            k0_C: Exponential term for the low-pressure limit.
            kinf_A: Pre-exponential factor for the high-pressure limit.
            kinf_B: Temperature exponent for the high-pressure limit.
            kinf_C: Exponential term for the high-pressure limit.
            Fc: Ternary Chemical Activation parameter [unitless].
            N: Ternary Chemical Activation parameter [unitless].
            reactants: A list of reactants involved in the reaction.
            products: A list of products formed in the reaction.
            gas_phase: The gas phase in which the reaction occurs.
            other_properties: A dictionary of other properties.
        """
        self._cpp = _TernaryChemicalActivation()

        self.name = name if name is not None else self.name
        self.k0_A = k0_A if k0_A is not None else self.k0_A
        self.k0_B = k0_B if k0_B is not None else self.k0_B
        self.k0_C = k0_C if k0_C is not None else self.k0_C
        self.kinf_A = kinf_A if kinf_A is not None else self.kinf_A
        self.kinf_B = kinf_B if kinf_B is not None else self.kinf_B
        self.kinf_C = kinf_C if kinf_C is not None else self.kinf_C
        self.Fc = Fc if Fc is not None else self.Fc
        self.N = N if N is not None else self.N
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
        return ReactionType.TernaryChemicalActivation

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
            "type": "TERNARY_CHEMICAL_ACTIVATION",
            "name": self.name,
            "k0_A": self.k0_A,
            "k0_B": self.k0_B,
            "k0_C": self.k0_C,
            "kinf_A": self.kinf_A,
            "kinf_B": self.kinf_B,
            "kinf_C": self.kinf_C,
            "Fc": self.Fc,
            "N": self.N,
            "reactants": [r.serialize() for r in self.reactants],
            "products": [r.serialize() for r in self.products],
            "gas phase": self.gas_phase,
        }
        _add_other_properties(serialize_dict, self.other_properties)
        return _remove_empty_keys(serialize_dict)
