from .utils import _add_other_properties
from .reactions import ReactionComponentSerializer
from .species import Species
from .phase import Phase
from typing import Optional, Any, Dict, List, Union, Tuple
from .. import backend

_backend = backend.get_backend()
_FirstOrderLoss = _backend._mechanism_configuration._FirstOrderLoss
_ReactionComponent = _backend._mechanism_configuration._ReactionComponent
ReactionType = _backend._mechanism_configuration._ReactionType


class FirstOrderLoss:
    """
    A class representing a first-order loss reaction rate constant.

    Attributes:
        name (str): The name of the first-order loss reaction rate constant.
        scaling_factor (float): The scaling factor for the first-order loss rate constant.
        reactants (List[Union[Species, Tuple[float, Species]]]): A list of reactants involved in the reaction.
        gas_phase (Phase): The gas phase in which the reaction occurs.
        other_properties (Dict[str, Any]): A dictionary of other properties of the first-order loss reaction rate constant.
    """

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
        # Create the internal C++ instance
        self._instance = _FirstOrderLoss()

        # Set properties if provided
        if name is not None:
            self.name = name
        if scaling_factor is not None:
            self.scaling_factor = scaling_factor
        if reactants is not None:
            self.reactants = reactants
        if gas_phase is not None:
            self.gas_phase = gas_phase
        if other_properties is not None:
            self.other_properties = other_properties

    @property
    def name(self) -> str:
        """Get the name."""
        return self._instance.name

    @name.setter
    def name(self, value: str):
        """Set the name."""
        self._instance.name = value

    @property
    def scaling_factor(self) -> float:
        """Get the scaling factor."""
        return self._instance.scaling_factor

    @scaling_factor.setter
    def scaling_factor(self, value: float):
        """Set the scaling factor."""
        self._instance.scaling_factor = value

    @property
    def reactants(self) -> List[Union[Species, Tuple[float, Species]]]:
        """Get the reactants as Python objects."""
        # Convert from C++ _ReactionComponent objects to Python Species objects
        result = []
        for rc in self._instance.reactants:
            if hasattr(rc, 'coefficient') and rc.coefficient != 1.0:
                # Create a tuple with coefficient and species
                species = Species(name=rc.species_name)
                result.append((rc.coefficient, species))
            else:
                # Just the species
                species = Species(name=rc.species_name)
                result.append(species)
        return result

    @reactants.setter
    def reactants(self, value: List[Union[Species, Tuple[float, Species]]]):
        """Set the reactants, converting from Python to C++ objects."""
        cpp_reactants = []
        for r in value:
            if isinstance(r, Species):
                cpp_reactants.append(_ReactionComponent(r.name))
            elif isinstance(r, tuple) and len(r) == 2:
                coefficient, species = r
                cpp_reactants.append(_ReactionComponent(species.name, coefficient))
            else:
                raise ValueError(f"Invalid reactant format: {r}")
        self._instance.reactants = cpp_reactants

    @property
    def gas_phase(self) -> str:
        """Get the gas phase name."""
        return self._instance.gas_phase

    @gas_phase.setter
    def gas_phase(self, value: Union[Phase, str]):
        """Set the gas phase."""
        if isinstance(value, Phase):
            self._instance.gas_phase = value.name
        else:
            self._instance.gas_phase = value

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
        return ReactionType.FirstOrderLoss

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
            "reactants": ReactionComponentSerializer.serialize_list_reaction_components(self._instance.reactants),
            "gas phase": self.gas_phase,
        }
        _add_other_properties(serialize_dict, self.other_properties)
        return _remove_empty_keys(serialize_dict)

    @staticmethod
    def serialize_static(instance) -> Dict:
        """
        Static serialize method for compatibility with C++ _FirstOrderLoss objects.

        Args:
            instance: The _FirstOrderLoss instance to serialize.

        Returns:
            Dict: A dictionary representation of the FirstOrderLoss object.
        """
        serialize_dict = {
            "type": "FIRST_ORDER_LOSS",
            "name": instance.name,
            "scaling factor": instance.scaling_factor,
            "reactants": ReactionComponentSerializer.serialize_list_reaction_components(instance.reactants),
            "gas phase": instance.gas_phase,
        }
        _add_other_properties(serialize_dict, instance.other_properties)
        return serialize_dict
