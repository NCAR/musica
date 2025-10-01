from .utils import _add_other_properties, _remove_empty_keys
from .species import Species
from .phase import Phase
from typing import Optional, Any, Dict, List, Union, Tuple
from .. import backend
from .reaction_component import ReactionComponent
from .ancillary import ReactionType

_backend = backend.get_backend()
Branched = _backend._mechanism_configuration._Branched

original_init = Branched.__init__


@property
def type(self):
    """Get the reaction type."""
    return ReactionType.Branched


def __init__(
    self,
    name: Optional[str] = None,
    X: Optional[float] = None,
    Y: Optional[float] = None,
    a0: Optional[float] = None,
    n: Optional[float] = None,
    reactants: Optional[List[Union[Species,
                                   Tuple[float, Species]]]] = None,
    nitrate_products: Optional[List[Union[Species,
                                          Tuple[float, Species]]]] = None,
    alkoxy_products: Optional[List[Union[Species,
                                         Tuple[float, Species]]]] = None,
    gas_phase: Optional[Phase] = None,
    other_properties: Optional[Dict[str, Any]] = None,
):
    """
    Initializes the Branched object with the given parameters.

    Args:
        name (str): The name of the branched reaction rate constant.
        X (float): Pre-exponential branching factor [(mol m-3)^-(n-1)s-1].
        Y (float): Exponential branching factor [K-1].
        a0 (float): Z parameter [unitless].
        n (float): A parameter [unitless].
        reactants (List[Union[Species, Tuple[float, Species]]]): A list of reactants involved in the reaction.
        nitrate_products (List[Union[Species, Tuple[float, Species]]]): A list of products formed in the nitrate branch.
        alkoxy_products (List[Union[Species, Tuple[float, Species]]]): A list of products formed in the alkoxy branch.
        gas_phase (Phase): The gas phase in which the reaction occurs.
        other_properties (Dict[str, Any]): A dictionary of other properties of the branched reaction rate constant.
    """
    original_init(self)

    self.name = name if name is not None else self.name
    self.X = X if X is not None else self.X
    self.Y = Y if Y is not None else self.Y
    self.a0 = a0 if a0 is not None else self.a0
    self.n = n if n is not None else self.n
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
    self.nitrate_products = (
        [
            (
                ReactionComponent(p.name)
                if isinstance(p, Species)
                else ReactionComponent(p[1].name, p[0])
            )
            for p in nitrate_products
        ]
        if nitrate_products is not None
        else self.nitrate_products
    )
    self.alkoxy_products = (
        [
            (
                ReactionComponent(p.name)
                if isinstance(p, Species)
                else ReactionComponent(p[1].name, p[0])
            )
            for p in alkoxy_products
        ]
        if alkoxy_products is not None
        else self.alkoxy_products
    )


def serialize(self) -> Dict:
    """
    Serialize the Branched object to a dictionary using only Python-visible data.

    Returns:
        Dict: A dictionary representation of the Branched object.
    """
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


Branched.__doc__ = """
    A class representing a branched reaction rate constant.

    Attributes:
        name (str): The name of the branched reaction rate constant.
        X (float): Pre-exponential branching factor [(mol m-3)^-(n-1)s-1].
        Y (float): Exponential branching factor [K-1].
        a0 (float): Z parameter [unitless].
        n (float): A parameter [unitless].
        reactants (List[Union[Species, Tuple[float, Species]]]): A list of reactants involved in the reaction.
        nitrate_products (List[Union[Species, Tuple[float, Species]]]): A list of products formed in the nitrate branch.
        alkoxy_products (List[Union[Species, Tuple[float, Species]]]): A list of products formed in the alkoxy branch.
        gas_phase (Phase): The gas phase in which the reaction occurs.
        other_properties (Dict[str, Any]): A dictionary of other properties of the branched reaction rate constant.
    """

Branched.__init__ = __init__
Branched.serialize = serialize
Branched.type = type
