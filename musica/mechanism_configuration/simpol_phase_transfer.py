from typing import Optional, Any, Dict, List, Union, Tuple
from musica import _SimpolPhaseTransfer, _ReactionComponent
from .phase import Phase
from .species import Species
from .utils import _add_other_properties, _remove_empty_keys


class SimpolPhaseTransfer(_SimpolPhaseTransfer):
    """
    A class representing a simplified phase transfer reaction rate constant.

    Attributes:
        name (str): The name of the simplified phase transfer reaction rate constant.
        gas_phase (Phase): The gas phase in which the reaction occurs.
        gas_phase_species (Union[Species, Tuple[float, Species]]): The gas phase species involved in the reaction.
        aerosol_phase (Phase): The aerosol phase in which the reaction occurs.
        aerosol_phase_species (Union[Species, Tuple[float, Species]]): The aerosol phase species involved in the reaction.
        B (List[float]): The B parameters [unitless].
        unknown_properties (Dict[str, Any]): A dictionary of other properties of the simplified phase transfer reaction rate constant.
    """

    def __init__(
        self,
        name: Optional[str] = None,
        gas_phase: Optional[Phase] = None,
        gas_phase_species: Optional[Union[Species, Tuple[float, Species]]] = None,
        aerosol_phase: Optional[Phase] = None,
        aerosol_phase_species: Optional[Union[Species, Tuple[float, Species]]] = None,
        B: Optional[List[float]] = None,
        other_properties: Optional[Dict[str, Any]] = None,
    ):
        """
        Initializes the SimpolPhaseTransfer object with the given parameters.

        Args:
            name (str): The name of the simplified phase transfer reaction rate constant.
            gas_phase (Phase): The gas phase in which the reaction occurs.
            gas_phase_species (Union[Species, Tuple[float, Species]]): The gas phase species involved in the reaction.
            aerosol_phase (Phase): The aerosol phase in which the reaction occurs.
            aerosol_phase_species (Union[Species, Tuple[float, Species]]): The aerosol phase species involved in the reaction.
            B (List[float]): The B parameters [unitless].
            other_properties (Dict[str, Any]): A dictionary of other properties of the simplified phase transfer reaction rate constant.
        """
        super().__init__()
        self.name = name if name is not None else self.name
        self.gas_phase = gas_phase.name if gas_phase is not None else self.gas_phase
        self.gas_phase_species = (
            (
                _ReactionComponent(gas_phase_species.name)
                if isinstance(gas_phase_species, Species)
                else _ReactionComponent(gas_phase_species[1].name, gas_phase_species[0])
            )
            if gas_phase_species is not None
            else self.gas_phase_species
        )
        self.aerosol_phase = aerosol_phase.name if aerosol_phase is not None else self.aerosol_phase
        self.aerosol_phase_species = (
            (
                _ReactionComponent(aerosol_phase_species.name)
                if isinstance(aerosol_phase_species, Species)
                else _ReactionComponent(
                    aerosol_phase_species[1].name, aerosol_phase_species[0]
                )
            )
            if aerosol_phase_species is not None
            else self.aerosol_phase_species
        )
        if B is not None:
            if len(B) != 4:
                raise ValueError("B must be a list of 4 elements.")
            self.B = B
        else:
            self.B = [0, 0, 0, 0]
        self.other_properties = other_properties if other_properties is not None else self.other_properties

    @staticmethod
    def serialize(instance) -> Dict:
        serialize_dict = {
            "type": "SIMPOL_PHASE_TRANSFER",
            "name": instance.name,
            "gas phase": instance.gas_phase,
            "gas-phase species": instance.gas_phase_species.species_name,
            "aerosol phase": instance.aerosol_phase,
            "aerosol-phase species": instance.aerosol_phase_species.species_name,
            "B": instance.B,
        }
        _add_other_properties(serialize_dict, instance.other_properties)
        return _remove_empty_keys(serialize_dict)
