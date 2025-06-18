from typing import Optional, Any, Dict
from musica import _WetDeposition
from musica.mechanism_configuration_phase import Phase
from musica.mechanism_configuration_utils import add_other_properties, remove_empty_keys


class WetDeposition(_WetDeposition):
    """
    A class representing a wet deposition reaction rate constant.

    Attributes:
        name (str): The name of the wet deposition reaction rate constant.
        scaling_factor (float): The scaling factor for the wet deposition rate constant.
        aerosol_phase (Phase): The aerosol phase which undergoes wet deposition.
        unknown_properties (Dict[str, Any]): A dictionary of other properties of the wet deposition reaction rate constant.
    """

    def __init__(
        self,
        name: Optional[str] = None,
        scaling_factor: Optional[float] = None,
        aerosol_phase: Optional[Phase] = None,
        other_properties: Optional[Dict[str, Any]] = None,
    ):
        """
        Initializes the WetDeposition object with the given parameters.

        Args:
            name (str): The name of the wet deposition reaction rate constant.
            scaling_factor (float): The scaling factor for the wet deposition rate constant.
            aerosol_phase (Phase): The aerosol phase which undergoes wet deposition.
            other_properties (Dict[str, Any]): A dictionary of other properties of the wet deposition reaction rate constant.
        """
        super().__init__()
        self.name = name if name is not None else self.name
        self.scaling_factor = scaling_factor if scaling_factor is not None else self.scaling_factor
        self.aerosol_phase = aerosol_phase.name if aerosol_phase is not None else self.aerosol_phase
        self.other_properties = other_properties if other_properties is not None else self.other_properties

    @staticmethod
    def serialize(cls) -> Dict:
        serialize_dict = {
            "type": "WET_DEPOSITION",
            "name": cls.name,
            "scaling factor": cls.scaling_factor,
            "aerosol phase": cls.aerosol_phase,
        }
        add_other_properties(serialize_dict, cls.other_properties)
        return remove_empty_keys(serialize_dict)
