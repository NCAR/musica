from .utils import _add_other_properties, _remove_empty_keys
from .species import Species
from .phase import Phase
from typing import Optional, Any, Dict, List, Union, Tuple
from .. import backend
from .reaction_component import ReactionComponent
from .ancillary import ReactionType

_backend = backend.get_backend()
FirstOrderLoss = _backend._mechanism_configuration._FirstOrderLoss

original_init = FirstOrderLoss.__init__


@property
def type(self):
    return ReactionType.FirstOrderLoss


def __init__(
    self,
    name: Optional[str] = None,
    scaling_factor: Optional[float] = None,
    reactants: Optional[List[Union[Species,
                                   Tuple[float, Species]]]] = None,
    gas_phase: Optional[Phase] = None,
    other_properties: Optional[Dict[str, Any]] = None,
):
    """
    Initializes the FirstOrderLoss object with the given parameters.

    Args:
        name (str): The name of the first-order loss reaction rate constant.
        scaling_factor (float): The scaling factor for the first-order loss rate constant.
        reactants (List[Union[Species, Tuple[float, Species]]]): A list of reactants involved in the reaction.
        gas_phase (Phase): The gas phase in which the reaction occurs.
        other_properties (Dict[str, Any]): A dictionary of other properties of the first-order loss reaction rate constant.
    """
    original_init(self)

    self.name = name if name is not None else self.name
    self.scaling_factor = scaling_factor if scaling_factor is not None else self.scaling_factor
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


def serialize(self) -> Dict:
    """
    Serialize the FirstOrderLoss object to a dictionary using only Python-visible data.

    Returns:
        Dict: A dictionary representation of the FirstOrderLoss object.
    """
    serialize_dict = {
        "type": "FIRST_ORDER_LOSS",
        "name": self.name,
        "scaling factor": self.scaling_factor,
        "reactants": [r.serialize() for r in self.reactants],
        "gas phase": self.gas_phase,
    }
    _add_other_properties(serialize_dict, self.other_properties)
    return _remove_empty_keys(serialize_dict)


FirstOrderLoss.__doc__ = """
A class representing a first-order loss reaction rate constant.

Attributes:
    name (str): The name of the first-order loss reaction rate constant.
    scaling_factor (float): The scaling factor for the first-order loss rate constant.
    reactants (List[Union[Species, Tuple[float, Species]]]): A list of reactants involved in the reaction.
    gas_phase (Phase): The gas phase in which the reaction occurs.
    other_properties (Dict[str, Any]): A dictionary of other properties of the first-order loss reaction rate constant.
"""

FirstOrderLoss.__init__ = __init__
FirstOrderLoss.serialize = serialize
FirstOrderLoss.type = type
