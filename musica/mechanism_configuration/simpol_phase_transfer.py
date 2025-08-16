from .utils import _add_other_properties
from .species import Species
from .phase import Phase
from typing import Optional, Any, Dict, List, Union, Tuple
from .. import backend

_backend = backend.get_backend()
_SimpolPhaseTransfer = _backend._mechanism_configuration._SimpolPhaseTransfer
_ReactionComponent = _backend._mechanism_configuration._ReactionComponent
ReactionType = _backend._mechanism_configuration._ReactionType


class SimpolPhaseTransfer:
    """
    A class representing a simplified phase transfer reaction rate constant.

    Attributes:
        name (str): The name of the simplified phase transfer reaction rate constant.
        gas_phase (Phase): The gas phase in which the reaction occurs.
        gas_phase_species (Union[Species, Tuple[float, Species]]): The gas phase species involved in the reaction.
        condensed_phase (Phase): The condensed phase in which the reaction occurs.
        condensed_phase_species (Union[Species, Tuple[float, Species]]): The condensed phase species involved in the reaction.
        B (List[float]): The B parameters [unitless].
        other_properties (Dict[str, Any]): A dictionary of other properties of the simplified phase transfer reaction rate constant.
    """

    def __init__(
        self,
        name: Optional[str] = None,
        gas_phase: Optional[Phase] = None,
        gas_phase_species: Optional[Union[Species,
                                          Tuple[float, Species]]] = None,
        condensed_phase: Optional[Phase] = None,
        condensed_phase_species: Optional[Union[Species,
                                                Tuple[float, Species]]] = None,
        B: Optional[List[float]] = None,
        other_properties: Optional[Dict[str, Any]] = None,
    ):
        """
        Initializes the SimpolPhaseTransfer object with the given parameters.

        Args:
            name (str): The name of the simplified phase transfer reaction rate constant.
            gas_phase (Phase): The gas phase in which the reaction occurs.
            gas_phase_species (Union[Species, Tuple[float, Species]]): The gas phase species involved in the reaction.
            condensed_phase (Phase): The condensed phase in which the reaction occurs.
            condensed_phase_species (Union[Species, Tuple[float, Species]]): The condensed phase species involved in the reaction.
            B (List[float]): The B parameters [unitless].
            other_properties (Dict[str, Any]): A dictionary of other properties of the simplified phase transfer reaction rate constant.
        """
        # Create the internal C++ instance
        self._instance = _SimpolPhaseTransfer()

        # Set all parameters
        if name is not None:
            self.name = name
        if gas_phase is not None:
            self.gas_phase = gas_phase
        if gas_phase_species is not None:
            self.gas_phase_species = gas_phase_species
        if condensed_phase is not None:
            self.condensed_phase = condensed_phase
        if condensed_phase_species is not None:
            self.condensed_phase_species = condensed_phase_species
        if B is not None:
            self.B = B
        if other_properties is not None:
            self.other_properties = other_properties

    # Property delegation to self._instance
    @property
    def name(self) -> str:
        """Get the name of the simplified phase transfer reaction rate constant."""
        return self._instance.name

    @name.setter
    def name(self, value: str):
        """Set the name of the simplified phase transfer reaction rate constant."""
        self._instance.name = value

    @property
    def gas_phase(self) -> str:
        """Get the gas phase name."""
        return self._instance.gas_phase

    @gas_phase.setter
    def gas_phase(self, value: Union[Phase, str]):
        """Set the gas phase."""
        if isinstance(value, Phase):
            self._instance.gas_phase = value.name
        elif isinstance(value, str):
            self._instance.gas_phase = value
        else:
            raise ValueError(f"Invalid gas_phase type: {type(value)}")

    @property
    def gas_phase_species(self) -> Union[Species, Tuple[float, Species]]:
        """Get the gas phase species as Python object."""
        rc = self._instance.gas_phase_species
        if hasattr(rc, 'coefficient') and rc.coefficient != 1.0:
            # Create a tuple with coefficient and species
            species = Species(name=rc.species_name)
            return (rc.coefficient, species)
        else:
            # Just the species
            return Species(name=rc.species_name)

    @gas_phase_species.setter
    def gas_phase_species(self, value: Union[Species, Tuple[float, Species]]):
        """Set the gas phase species, converting from Python to C++ object."""
        if isinstance(value, Species):
            self._instance.gas_phase_species = _ReactionComponent(value.name)
        elif isinstance(value, tuple) and len(value) == 2:
            coefficient, species = value
            self._instance.gas_phase_species = _ReactionComponent(species.name, coefficient)
        else:
            raise ValueError(f"Invalid gas_phase_species format: {value}")

    @property
    def condensed_phase(self) -> str:
        """Get the condensed phase name."""
        return self._instance.condensed_phase

    @condensed_phase.setter
    def condensed_phase(self, value: Union[Phase, str]):
        """Set the condensed phase."""
        if isinstance(value, Phase):
            self._instance.condensed_phase = value.name
        elif isinstance(value, str):
            self._instance.condensed_phase = value
        else:
            raise ValueError(f"Invalid condensed_phase type: {type(value)}")

    @property
    def condensed_phase_species(self) -> Union[Species, Tuple[float, Species]]:
        """Get the condensed phase species as Python object."""
        rc = self._instance.condensed_phase_species
        if hasattr(rc, 'coefficient') and rc.coefficient != 1.0:
            # Create a tuple with coefficient and species
            species = Species(name=rc.species_name)
            return (rc.coefficient, species)
        else:
            # Just the species
            return Species(name=rc.species_name)

    @condensed_phase_species.setter
    def condensed_phase_species(self, value: Union[Species, Tuple[float, Species]]):
        """Set the condensed phase species, converting from Python to C++ object."""
        if isinstance(value, Species):
            self._instance.condensed_phase_species = _ReactionComponent(value.name)
        elif isinstance(value, tuple) and len(value) == 2:
            coefficient, species = value
            self._instance.condensed_phase_species = _ReactionComponent(species.name, coefficient)
        else:
            raise ValueError(f"Invalid condensed_phase_species format: {value}")

    @property
    def B(self) -> List[float]:
        """Get the B parameters."""
        return self._instance.B

    @B.setter
    def B(self, value: List[float]):
        """Set the B parameters with validation."""
        if not isinstance(value, list) or len(value) != 4:
            raise ValueError("B must be a list of 4 elements.")
        self._instance.B = value

    @property
    def other_properties(self) -> Dict[str, Any]:
        """Get the other properties."""
        return self._instance.other_properties

    @other_properties.setter
    def other_properties(self, value: Dict[str, Any]):
        """Set the other properties."""
        self._instance.other_properties = value

    @property
    def type(self):
        """Get the reaction type."""
        return ReactionType.SimpolPhaseTransfer

    def serialize(self) -> Dict:
        """
        Serialize the SimpolPhaseTransfer object to a dictionary using only Python-visible data.

        Returns:
            Dict: A dictionary representation of the SimpolPhaseTransfer object.
        """
        serialize_dict = {
            "type": "SIMPOL_PHASE_TRANSFER",
            "name": self._instance.name,
            "gas phase": self._instance.gas_phase,
            "gas-phase species": self._instance.gas_phase_species.species_name,
            "condensed phase": self._instance.condensed_phase,
            "condensed-phase species": self._instance.condensed_phase_species.species_name,
            "B": self._instance.B,
        }
        _add_other_properties(serialize_dict, self.other_properties)
        return serialize_dict

    @staticmethod
    def serialize_static(instance) -> Dict:
        """
        Static serialize method for compatibility with C++ _SimpolPhaseTransfer objects.

        Args:
            instance: The _SimpolPhaseTransfer instance to serialize.

        Returns:
            Dict: A dictionary representation of the SimpolPhaseTransfer object.
        """
        # Create a local copy to call serialize
        temp_obj = SimpolPhaseTransfer()
        temp_obj._instance = instance
        return temp_obj.serialize()
