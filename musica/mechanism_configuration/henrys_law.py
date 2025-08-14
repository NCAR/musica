from typing import Optional, Any, Dict, Union, Tuple
from .. import backend
from .phase import Phase
from .species import Species
from .utils import _add_other_properties

_backend = backend.get_backend()
_HenrysLaw = _backend._mechanism_configuration._HenrysLaw
_ReactionComponent = _backend._mechanism_configuration._ReactionComponent


class HenrysLaw(_HenrysLaw):
    """
    A class representing a Henry's law reaction rate constant.

    Attributes:
        name (str): The name of the Henry's law reaction rate constant.
        other_properties (Dict[str, Any]): A dictionary of other properties of the Henry's law reaction rate constant.
    """

    def __init__(
        self,
        name: Optional[str] = None,
        other_properties: Optional[Dict[str, Any]] = None,
    ):
        """
        Initializes the HenrysLaw object with the given parameters.

        Args:
            name (str): The name of the Henry's law reaction rate constant.
            other_properties (Dict[str, Any]): A dictionary of other properties of the Henry's law reaction rate constant.
        """
        super().__init__()
        self.name = name if name is not None else self.name
        self.other_properties = other_properties if other_properties is not None else self.other_properties

    @staticmethod
    def serialize(instance) -> Dict:
        serialize_dict = {
            "type": "HL_PHASE_TRANSFER",
            "name": instance.name,
        }
        _add_other_properties(serialize_dict, instance.other_properties)
        return serialize_dict
