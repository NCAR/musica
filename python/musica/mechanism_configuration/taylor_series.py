from typing import Optional, Any, Dict, List, Union, Tuple
from .. import backend
from .utils import _add_other_properties, _remove_empty_keys
from .phase import Phase
from .species import Species
from .reaction_component import ReactionComponent
from .ancillary import ReactionType

_backend = backend.get_backend()
TaylorSeries = _backend._mechanism_configuration._TaylorSeries

original_init = TaylorSeries.__init__


@property
def type(self):
    return ReactionType.TaylorSeries


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
    reactants: Optional[List[Union[Species,
                                   Tuple[float, Species]]]] = None,
    products: Optional[List[Union[Species, Tuple[float, Species]]]] = None,
    other_properties: Optional[Dict[str, Any]] = None,
):
    """
    Initializes the TaylorSeries object with the given parameters.

    Args:
        name (Optional[str]): The name of the TaylorSeries object.
        gas_phase (Optional[Phase]): The gas phase associated with the reaction.
        A (Optional[float]): The A coefficient for the Taylor series.
        B (Optional[float]): The B coefficient for the Taylor series.
        C (Optional[float]): The C coefficient for the Taylor series.
        D (Optional[float]): The D coefficient for the Taylor series.
        E (Optional[float]): The E coefficient for the Taylor series.
        taylor_coefficients (Optional[List[float]]): List of Taylor series coefficients.
        reactants (Optional[List[Union[Species, Tuple[float, Species]]]]): List of reactants, either as Species or (stoichiometry, Species) tuples.
        products (Optional[List[Union[Species, Tuple[float, Species]]]]): List of products, either as Species or (stoichiometry, Species) tuples.
        other_properties (Optional[Dict[str, Any]]): Additional properties for the reaction.
    """
    original_init(self)
    self.name = name if name is not None else self.name
    self.A = A if A is not None else self.A
    self.B = B if B is not None else self.B
    self.C = C if C is not None else self.C
    self.D = D if D is not None else self.D
    self.E = E if E is not None else self.E
    self.taylor_coefficients = taylor_coefficients if taylor_coefficients is not None else self.taylor_coefficients
    self.gas_phase = gas_phase.name if gas_phase is not None else self.gas_phase
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
    self.other_properties = other_properties if other_properties is not None else self.other_properties


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


TaylorSeries.__doc__ = """
    A class representing a Taylor series rate constant.

    The rate constant k is represented as a Taylor series expansion in temperature (and optionally other variables):

        k = a0 + a1*T + a2*T^2 + a3*T^3 + ... + an*T^n

    where:
        k = rate constant
        T = temperature [K]
        a0, a1, ..., an = Taylor series coefficients

    Optionally, additional parameters (A, B, C, D, E) may be provided for compatibility or extended forms.

    Attributes:
        name (str): The name of the Taylor series rate constant.
        taylor_coefficients (List[float]): Coefficients [a0, a1, ..., an] for the Taylor series expansion.
        A (float, optional): Optional parameter for extended forms.
        B (float, optional): Optional parameter for extended forms.
        C (float, optional): Optional parameter for extended forms.
        D (float, optional): Optional parameter for extended forms.
        E (float, optional): Optional parameter for extended forms.
        reactants (List[Union[Species, Tuple[float, Species]]]): A list of reactants involved in the reaction.
        products (List[Union[Species, Tuple[float, Species]]]): A list of products formed in the reaction.
        gas_phase (Phase): The gas phase in which the reaction occurs.
        other_properties (Dict[str, Any]): A dictionary of other properties of the Taylor series rate constant.
    """


TaylorSeries.__init__ = __init__
TaylorSeries.serialize = serialize
TaylorSeries.type = type
