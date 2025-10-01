from typing import Optional, Any, Dict, List, Union, Tuple
from .. import backend
from .phase import Phase
from .species import Species
from .utils import _add_other_properties, _remove_empty_keys
from ..constants import BOLTZMANN
from .reaction_component import ReactionComponent
from .ancillary import ReactionType

_backend = backend.get_backend()
Arrhenius = _backend._mechanism_configuration._Arrhenius

original_init = Arrhenius.__init__


@property
def type(self):
    """Get the reaction type."""
    return ReactionType.Arrhenius


def __init__(
    self,
    name: Optional[str] = None,
    A: Optional[float] = None,
    B: Optional[float] = None,
    C: Optional[float] = None,
    Ea: Optional[float] = None,
    D: Optional[float] = None,
    E: Optional[float] = None,
    reactants: Optional[List[Union[Species,
                                   Tuple[float, Species]]]] = None,
    products: Optional[List[Union[Species, Tuple[float, Species]]]] = None,
    gas_phase: Optional[Phase] = None,
    other_properties: Optional[Dict[str, Any]] = None,
):
    """
    Initializes the Arrhenius object with the given parameters.

    Args:
        name (str): The name of the Arrhenius rate constant.
        A (float): Pre-exponential factor [(mol m-3)^(n-1)s-1] where n is the number of reactants.
        B (float): Temperature exponent [unitless].
        C (float): Exponential term [K-1].
        Ea (float): Activation energy [J molecule-1].
        D (float): Reference Temperature [K].
        E (float): Pressure scaling term [Pa-1].
        reactants (List[Union[Species, Tuple[float, Species]]]): A list of reactants involved in the reaction.
        products (List[Union[Species, Tuple[float, Species]]]): A list of products formed in the reaction.
        gas_phase (Phase): The gas phase in which the reaction occurs.
        other_properties (Dict[str, Any]): A dictionary of other properties of the Arrhenius rate constant.
    """
    original_init(self)

    # Validate mutually exclusive parameters
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
    Serialize the Arrhenius object to a dictionary using only Python-visible data.

    Returns:
        Dict: A dictionary representation of the Arrhenius object.
    """
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


Arrhenius.__doc__ = """
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

Arrhenius.__init__ = __init__
Arrhenius.serialize = serialize
Arrhenius.type = type
