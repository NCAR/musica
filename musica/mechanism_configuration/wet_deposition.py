from typing import Optional, Any, Dict
from .. import backend
from .phase import Phase
from .utils import _add_other_properties

_backend = backend.get_backend()
_WetDeposition = _backend._mechanism_configuration._WetDeposition


class WetDeposition(_WetDeposition):
    """
    A class representing a wet deposition reaction rate constant.

    Attributes:
        name (str): The name of the wet deposition reaction rate constant.
        scaling_factor (float): The scaling factor for the wet deposition rate constant.
        condensed_phase (Phase): The condensed phase which undergoes wet deposition.
        unknown_properties (Dict[str, Any]): A dictionary of other properties of the wet deposition reaction rate constant.
    """

    def __init__(
        self,
        name: Optional[str] = None,
        scaling_factor: Optional[float] = None,
        condensed_phase: Optional[Phase] = None,
        other_properties: Optional[Dict[str, Any]] = None,
    ):
        """
        Initializes the WetDeposition object with the given parameters.

        Args:
            name (str): The name of the wet deposition reaction rate constant.
            scaling_factor (float): The scaling factor for the wet deposition rate constant.
            condensed_phase (Phase): The condensed phase which undergoes wet deposition.
            other_properties (Dict[str, Any]): A dictionary of other properties of the wet deposition reaction rate constant.
        """
        super().__init__()
        self.name = name if name is not None else self.name
        self.scaling_factor = scaling_factor if scaling_factor is not None else self.scaling_factor
        self.condensed_phase = condensed_phase.name if condensed_phase is not None else self.condensed_phase
        self.other_properties = other_properties if other_properties is not None else self.other_properties

    @staticmethod
    def serialize(instance) -> Dict:
        serialize_dict = {
            "type": "WET_DEPOSITION",
            "name": instance.name,
            "scaling factor": instance.scaling_factor,
            "condensed phase": instance.condensed_phase,
        }
        _add_other_properties(serialize_dict, instance.other_properties)
        return serialize_dict
