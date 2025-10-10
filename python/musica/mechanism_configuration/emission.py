from .utils import _add_other_properties, _remove_empty_keys
from .species import Species
from .phase import Phase
from typing import Optional, Any, Dict, List, Union, Tuple
from .. import backend
from .reaction_component import ReactionComponent
from .ancillary import ReactionType

_backend = backend.get_backend()
Emission = _backend._mechanism_configuration._Emission

original_init = Emission.__init__


@property
def type(self):
    return ReactionType.Emission


def __init__(
    self,
    name: Optional[str] = None,
    scaling_factor: Optional[float] = None,
    products: Optional[List[Union[Species, Tuple[float, Species]]]] = None,
    gas_phase: Optional[Phase] = None,
    other_properties: Optional[Dict[str, Any]] = None,
):
    """
    Initializes the Emission object with the given parameters.

    Args:
        name (str): The name of the emission reaction rate constant.
        scaling_factor (float): The scaling factor for the emission rate constant.
        products (List[Union[Species, Tuple[float, Species]]]): A list of products formed in the reaction.
        gas_phase (Phase): The gas phase in which the reaction occurs.
        other_properties (Dict[str, Any]): A dictionary of other properties of the emission reaction rate constant.
    """
    original_init(self)
    self.name = name if name is not None else self.name
    self.scaling_factor = scaling_factor if scaling_factor is not None else self.scaling_factor
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
    self.gas_phase = gas_phase.name if gas_phase is not None else self.gas_phase
    self.other_properties = other_properties if other_properties is not None else self.other_properties


def serialize(self) -> Dict:
    serialize_dict = {
        "type": "EMISSION",
        "name": self.name,
        "scaling factor": self.scaling_factor,
        "products": [r.serialize() for r in self.products],
        "gas phase": self.gas_phase,
    }
    _add_other_properties(serialize_dict, self.other_properties)
    return serialize_dict


Emission.__doc__ = """
A class representing an emission reaction rate constant.

Attributes:
    name (str): The name of the emission reaction rate constant.
    scaling_factor (float): The scaling factor for the emission rate constant.
    products (List[Union[Species, Tuple[float, Species]]]): A list of products formed in the reaction.
    gas_phase (Phase): The gas phase in which the reaction occurs.
    other_properties (Dict[str, Any]): A dictionary of other properties of the emission reaction rate constant.
"""

Emission.__init__ = __init__
Emission.serialize = serialize
Emission.type = type
