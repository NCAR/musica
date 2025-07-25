from .utils import _add_other_properties, _remove_empty_keys
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
        aerosol_phase (Phase): The aerosol phase in which the reaction occurs.
        aerosol_phase_species (Union[Species, Tuple[float, Species]]): The aerosol phase species involved in the reaction.
        B (List[float]): The B parameters [unitless].
        unknown_properties (Dict[str, Any]): A dictionary of other properties of the simplified phase transfer reaction rate constant.
    """

    def __init__(
        self,
        name: Optional[str] = None,
        gas_phase: Optional[Phase] = None,
        gas_phase_species: Optional[Union[Species,
                                          Tuple[float, Species]]] = None,
        aerosol_phase: Optional[Phase] = None,
        aerosol_phase_species: Optional[Union[Species,
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
            aerosol_phase (Phase): The aerosol phase in which the reaction occurs.
            aerosol_phase_species (Union[Species, Tuple[float, Species]]): The aerosol phase species involved in the reaction.
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
        if aerosol_phase is not None:
            self.aerosol_phase = aerosol_phase
        if aerosol_phase_species is not None:
            self.aerosol_phase_species = aerosol_phase_species
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
    def aerosol_phase(self) -> str:
        """Get the aerosol phase name."""
        return self._instance.aerosol_phase

    @aerosol_phase.setter
    def aerosol_phase(self, value: Union[Phase, str]):
        """Set the aerosol phase."""
        if isinstance(value, Phase):
            self._instance.aerosol_phase = value.name
        elif isinstance(value, str):
            self._instance.aerosol_phase = value
        else:
            raise ValueError(f"Invalid aerosol_phase type: {type(value)}")

    @property
    def aerosol_phase_species(self) -> Union[Species, Tuple[float, Species]]:
        """Get the aerosol phase species as Python object."""
        rc = self._instance.aerosol_phase_species
        if hasattr(rc, 'coefficient') and rc.coefficient != 1.0:
            # Create a tuple with coefficient and species
            species = Species(name=rc.species_name)
            return (rc.coefficient, species)
        else:
            # Just the species
            return Species(name=rc.species_name)

    @aerosol_phase_species.setter
    def aerosol_phase_species(self, value: Union[Species, Tuple[float, Species]]):
        """Set the aerosol phase species, converting from Python to C++ object."""
        if isinstance(value, Species):
            self._instance.aerosol_phase_species = _ReactionComponent(value.name)
        elif isinstance(value, tuple) and len(value) == 2:
            coefficient, species = value
            self._instance.aerosol_phase_species = _ReactionComponent(species.name, coefficient)
        else:
            raise ValueError(f"Invalid aerosol_phase_species format: {value}")

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

    def _create_serialize_dict(self, instance) -> Dict:
        """
        Helper method to create the serialization dictionary.

        Args:
            instance: The instance to serialize (either self._instance or a _SimpolPhaseTransfer object).

        Returns:
            Dict: Base serialization dictionary.
        """
        return {
            "type": "SIMPOL_PHASE_TRANSFER",
            "name": instance.name,
            "gas phase": instance.gas_phase,
            "gas-phase species": instance.gas_phase_species.species_name,
            "aerosol phase": instance.aerosol_phase,
            "aerosol-phase species": instance.aerosol_phase_species.species_name,
            "B": instance.B,
        }

    def serialize(self) -> Dict:
        """
        Serialize the SimpolPhaseTransfer object to a dictionary using only Python-visible data.

        Returns:
            Dict: A dictionary representation of the SimpolPhaseTransfer object.
        """
        serialize_dict = self._create_serialize_dict(self._instance)
        _add_other_properties(serialize_dict, self.other_properties)
        return _remove_empty_keys(serialize_dict)

    @staticmethod
    def serialize_static(instance) -> Dict:
        """
        Static serialize method for compatibility with C++ _SimpolPhaseTransfer objects.

        Args:
            instance: The _SimpolPhaseTransfer instance to serialize.

        Returns:
            Dict: A dictionary representation of the SimpolPhaseTransfer object.
        """
        # Create a temporary SimpolPhaseTransfer object to use the helper method
        temp_simpol = SimpolPhaseTransfer()
        serialize_dict = temp_simpol._create_serialize_dict(instance)
        _add_other_properties(serialize_dict, instance.other_properties)
        return _remove_empty_keys(serialize_dict)
