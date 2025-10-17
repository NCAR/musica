from .utils import _add_other_properties, _remove_empty_keys
from .species import Species
from .phase import Phase
from typing import Optional, Any, Dict, List, Union, Tuple
from .. import backend
from .reaction_component import ReactionComponent
from .ancillary import ReactionType

_backend = backend.get_backend()
Tunneling = _backend._mechanism_configuration._Tunneling

original_init = Tunneling.__init__


@property
def type(self):
    return ReactionType.Tunneling


def __init__(
    self,
    name: Optional[str] = None,
    A: Optional[float] = None,
    B: Optional[float] = None,
    C: Optional[float] = None,
    reactants: Optional[List[Union[Species,
                                   Tuple[float, Species]]]] = None,
    products: Optional[List[Union[Species, Tuple[float, Species]]]] = None,
    gas_phase: Optional[Phase] = None,
    other_properties: Optional[Dict[str, Any]] = None,
):
    """
    Initializes the Tunneling object with the given parameters.

    Args:
        name (str): The name of the tunneling reaction rate constant.
        A (float): Pre-exponential factor [(mol m-3)^(n-1)s-1].
        B (float): Tunneling parameter [K^-1].
        C (float): Tunneling parameter [K^-3].
        reactants (List[Union[Species, Tuple[float, Species]]]): A list of reactants involved in the reaction.
        products (List[Union[Species, Tuple[float, Species]]]): A list of products formed in the reaction.
        gas_phase (Phase): The gas phase in which the reaction occurs.
        other_properties (Dict[str, Any]): A dictionary of other properties of the tunneling reaction rate constant.
    """
    original_init(self)

    self.name = name if name is not None else self.name
    self.A = A if A is not None else self.A
    self.B = B if B is not None else self.B
    self.C = C if C is not None else self.C
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
    Serialize the Tunneling object to a dictionary using only Python-visible data.

    Returns:
        Dict: A dictionary representation of the Tunneling object.
    """
    serialize_dict = {
        "type": "TUNNELING",
        "name": self.name,
        "A": self.A,
        "B": self.B,
        "C": self.C,
        "reactants": [r.serialize() for r in self.reactants],
        "products": [r.serialize() for r in self.products],
        "gas phase": self.gas_phase,
    }
    _add_other_properties(serialize_dict, self.other_properties)
    return _remove_empty_keys(serialize_dict)


Tunneling.__doc__ = """
A class representing a quantum tunneling reaction rate constant.

k = A * exp( -B / T ) * exp( C / T^3 )

where:
    k = rate constant
    A = pre-exponential factor [(mol m-3)^(n-1)s-1]
    B = tunneling parameter [K^-1]
    C = tunneling parameter [K^-3]
    T = temperature [K]
    n = number of reactants

Attributes:
    name (str): The name of the tunneling reaction rate constant.
    A (float): Pre-exponential factor [(mol m-3)^(n-1)s-1].
    B (float): Tunneling parameter [K^-1].
    C (float): Tunneling parameter [K^-3].
    reactants (List[Union[Species, Tuple[float, Species]]]): A list of reactants involved in the reaction.
    products (List[Union[Species, Tuple[float, Species]]]): A list of products formed in the reaction.
    gas_phase (Phase): The gas phase in which the reaction occurs.
    other_properties (Dict[str, Any]): A dictionary of other properties of the tunneling reaction rate constant.
"""

Tunneling.__init__ = __init__
Tunneling.serialize = serialize
Tunneling.type = type
