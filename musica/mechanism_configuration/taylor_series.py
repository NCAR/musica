from typing import Optional, Any, Dict, List, Union, Tuple
from .. import backend
from .utils import _add_other_properties, _remove_empty_keys
from .phase import Phase
from .species import Species
from .reactions import ReactionComponentSerializer

_backend = backend.get_backend()
TaylorSeries = _backend._mechanism_configuration._TaylorSeries
_ReactionComponent = _backend._mechanism_configuration._ReactionComponent
ReactionType = _backend._mechanism_configuration._ReactionType

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
        "taylor coefficients": self.taylor_coefficients,
        "reactants": ReactionComponentSerializer.serialize_list_reaction_components(self.reactants),
        "products": ReactionComponentSerializer.serialize_list_reaction_components(self.products),
        "gas phase": self.gas_phase,
    }
    _add_other_properties(serialize_dict, self.other_properties)
    return _remove_empty_keys(serialize_dict)

TaylorSeries.__doc__ = """
    A class representing an Arrhenius rate constant.

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
        name (str): The name of the Arrhenius rate constant.
        A (float): Pre-exponential factor [(mol m-3)^(n-1)s-1] where n is the number of reactants.
        B (float): Temperature exponent [unitless].
        C (float): Exponential term [K-1].
        D (float): Reference Temperature [K].
        E (float): Pressure scaling term [Pa-1].
        reactants (List[Union[Species, Tuple[float, Species]]]): A list of reactants involved in the reaction.
        products (List[Union[Species, Tuple[float, Species]]]): A list of products formed in the reaction.
        gas_phase (Phase): The gas phase in which the reaction occurs.
        other_properties (Dict[str, Any]): A dictionary of other properties of the Arrhenius rate constant.
    """


TaylorSeries.__init__ = __init__
TaylorSeries.serialize = serialize
TaylorSeries.type = type
