from typing import Optional, Any, Dict, List, Union, Tuple
from .. import backend
from .phase import Phase
from .species import Species
from .utils import _add_other_properties, _remove_empty_keys
from .reaction_component import ReactionComponent
from .ancillary import ReactionType

_backend = backend.get_backend()
TernaryChemicalActivation = _backend._mechanism_configuration._TernaryChemicalActivation

original_init = TernaryChemicalActivation.__init__


@property
def type(self):
    return ReactionType.TernaryChemicalActivation


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
    reactants: Optional[List[Union[Species,
                                   Tuple[float, Species]]]] = None,
    products: Optional[List[Union[Species, Tuple[float, Species]]]] = None,
    gas_phase: Optional[Phase] = None,
    other_properties: Optional[Dict[str, Any]] = None,
):
    """
    Initializes the Ternary Chemical Activation object with the given parameters.

    k0 = k0_A * exp( k0_C / T ) * ( T / 300.0 )^k0_B
    kinf = kinf_A * exp( kinf_C / T ) * ( T / 300.0 )^kinf_B
    k = k0[M] / ( 1 + k0[M] / kinf ) * Fc^(1 + 1/N*(log10(k0[M]/kinf))^2)^-1

    where:
        k = rate constant
        k0 = low-pressure limit rate constant
        kinf = high-pressure limit rate constant
        k0_A = pre-exponential factor for the low-pressure limit [(mol m-3)^(n-1)s-1]
        k0_B = temperature exponent for the low-pressure limit [unitless]
        k0_C = exponential term for the low-pressure limit [K-1]
        kinf_A = pre-exponential factor for the high-pressure limit [(mol m-3)^(n-1)s-1]
        kinf_B = temperature exponent for the high-pressure limit [unitless]
        kinf_C = exponential term for the high-pressure limit [K-1]
        Fc = Ternary Chemical Activation parameter [unitless]
        N = Ternary Chemical Activation parameter [unitless]
        T = temperature [K]
        M = concentration of the third body [mol m-3]

    Args:
        name (str): The name of the Ternary Chemical Activation rate constant.
        k0_A (float): Pre-exponential factor for the low-pressure limit [(mol m-3)^(n-1)s-1].
        k0_B (float): Temperature exponent for the low-pressure limit [unitless].
        k0_C (float): Exponential term for the low-pressure limit [K-1].
        kinf_A (float): Pre-exponential factor for the high-pressure limit [(mol m-3)^(n-1)s-1].
        kinf_B (float): Temperature exponent for the high-pressure limit [unitless].
        kinf_C (float): Exponential term for the high-pressure limit [K-1].
        Fc (float): Ternary Chemical Activation parameter [unitless].
        N (float): Ternary Chemical Activation parameter [unitless].
        reactants (List[Union[Species, Tuple[float, Species]]]): A list of reactants involved in the reaction.
        products (List[Union[Species, Tuple[float, Species]]]): A list of products formed in the reaction.
        gas_phase (Phase): The gas phase in which the reaction occurs.
        other_properties (Dict[str, Any]): A dictionary of other properties of the Ternary Chemical Activation rate constant.
    """
    original_init(self)

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
        [
            (
                ReactionComponent(r.name)
                if isinstance(r, Species)
                else ReactionComponent(r[1].name, r[0])
            )
            for r in reactants
        ]
        if reactants is not None
        else self.reactants
    )
    self.products = (
        [
            (
                ReactionComponent(p.name)
                if isinstance(p, Species)
                else ReactionComponent(p[1].name, p[0])
            )
            for p in products
        ]
        if products is not None
        else self.products
    )


def serialize(self) -> Dict:
    """
    Serialize the Ternary Chemical Activation object to a dictionary using only Python-visible data.

    Returns:
        Dict: A dictionary representation of the Ternary Chemical Activation object.
    """
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


TernaryChemicalActivation.__doc__ = """
A class representing a Ternary Chemical Activation rate constant.

Attributes:
    name (str): The name of the Ternary Chemical Activation rate constant.
    k0_A (float): Pre-exponential factor for the low-pressure limit [(mol m-3)^(n-1)s-1].
    k0_B (float): Temperature exponent for the low-pressure limit [unitless].
    k0_C (float): Exponential term for the low-pressure limit [K-1].
    kinf_A (float): Pre-exponential factor for the high-pressure limit [(mol m-3)^(n-1)s-1].
    kinf_B (float): Temperature exponent for the high-pressure limit [unitless].
    kinf_C (float): Exponential term for the high-pressure limit [K-1].
    Fc (float): Ternary Chemical Activation parameter [unitless].
    N (float): Ternary Chemical Activation parameter [unitless].
    reactants (List[Union[Species, Tuple[float, Species]]]): A list of reactants involved in the reaction.
    products (List[Union[Species, Tuple[float, Species]]]): A list of products formed in the reaction.
    gas_phase (Phase): The gas phase in which the reaction occurs.
    other_properties (Dict[str, Any]): A dictionary of other properties of the Ternary Chemical Activation rate constant.
"""

TernaryChemicalActivation.__init__ = __init__
TernaryChemicalActivation.serialize = serialize
TernaryChemicalActivation.type = type
