from typing import Optional, Any, Dict, List, Union, Tuple
from .. import backend
from .phase import Phase
from .species import Species
from .reactions import ReactionComponentSerializer
from .utils import _add_other_properties, _remove_empty_keys

_backend = backend.get_backend()
_Surface = _backend._mechanism_configuration._Surface
_ReactionComponent = _backend._mechanism_configuration._ReactionComponent


class Surface:
    """
    A class representing a surface in a chemical mechanism.

    (TODO: get details from MusicBox)

    Attributes:
        name (str): The name of the surface.
        reaction_probability (float): The probability of a reaction occurring on the surface.
        gas_phase_species (Union[Species, Tuple[float, Species]]): The gas phase species involved in the reaction.
        gas_phase_products (List[Union[Species, Tuple[float, Species]]]): The gas phase products formed in the reaction.
        gas_phase (Phase): The gas phase in which the reaction occurs.
        aerosol_phase (Phase): The aerosol phase in which the reaction occurs.
        other_properties (Dict[str, Any]): A dictionary of other properties of the surface.
    """

    def __init__(
        self,
        name: Optional[str] = None,
        reaction_probability: Optional[float] = None,
        gas_phase_species: Optional[Union[Species,
                                          Tuple[float, Species]]] = None,
        gas_phase_products: Optional[
            List[Union[Species, Tuple[float, Species]]]
        ] = None,
        gas_phase: Optional[Phase] = None,
        aerosol_phase: Optional[Phase] = None,
        other_properties: Optional[Dict[str, Any]] = None,
    ):
        """
        Initializes the Surface object with the given parameters.

        Args:
            name (str): The name of the surface.
            reaction_probability (float): The probability of a reaction occurring on the surface.
            gas_phase_species (Union[Species, Tuple[float, Species]]): The gas phase species involved in the reaction.
            gas_phase_products (List[Union[Species, Tuple[float, Species]]]): The gas phase products formed in the reaction.
            gas_phase (Phase): The gas phase in which the reaction occurs.
            aerosol_phase (Phase): The aerosol phase in which the reaction occurs.
            other_properties (Dict[str, Any]): A dictionary of other properties of the surface.
        """
        # Create the internal C++ instance
        self._instance = _Surface()
        
        # Store Python objects for public interface
        self._name = name
        self._reaction_probability = reaction_probability
        self._gas_phase_species = gas_phase_species
        self._gas_phase_products = gas_phase_products if gas_phase_products is not None else []
        self._gas_phase = gas_phase
        self._aerosol_phase = aerosol_phase
        self._other_properties = other_properties if other_properties is not None else {}
        
        # Set properties on the C++ instance using the property setters
        if name is not None:
            self.name = name
        if reaction_probability is not None:
            self.reaction_probability = reaction_probability
        if gas_phase_species is not None:
            self.gas_phase_species = gas_phase_species
        if gas_phase_products is not None:
            self.gas_phase_products = gas_phase_products
        if gas_phase is not None:
            self.gas_phase = gas_phase
        if aerosol_phase is not None:
            self.aerosol_phase = aerosol_phase
        if other_properties is not None:
            self.other_properties = other_properties

    @property
    def name(self) -> str:
        """Get the name of the surface."""
        return self._instance.name

    @name.setter
    def name(self, value: str):
        """Set the name of the surface."""
        self._name = value
        self._instance.name = value

    @property
    def reaction_probability(self) -> float:
        """Get the reaction probability."""
        return self._instance.reaction_probability

    @reaction_probability.setter
    def reaction_probability(self, value: float):
        """Set the reaction probability."""
        self._reaction_probability = value
        self._instance.reaction_probability = value

    @property
    def gas_phase_species(self) -> Union[Species, Tuple[float, Species], None]:
        """Get the gas phase species (returns Python objects)."""
        return self._gas_phase_species

    @gas_phase_species.setter
    def gas_phase_species(self, value: Union[Species, Tuple[float, Species]]):
        """Set the gas phase species."""
        self._gas_phase_species = value
        # Convert to C++ ReactionComponent
        if isinstance(value, Species):
            self._instance.gas_phase_species = _ReactionComponent(value.name)
        elif isinstance(value, tuple) and len(value) == 2:
            coefficient, species = value
            self._instance.gas_phase_species = _ReactionComponent(species.name, coefficient)
        else:
            raise ValueError("gas_phase_species must be a Species or Tuple[float, Species]")

    @property
    def gas_phase_products(self) -> List[Union[Species, Tuple[float, Species]]]:
        """Get the gas phase products (returns Python objects)."""
        return self._gas_phase_products

    @gas_phase_products.setter
    def gas_phase_products(self, value: List[Union[Species, Tuple[float, Species]]]):
        """Set the gas phase products."""
        self._gas_phase_products = value
        # Convert to C++ ReactionComponents
        cpp_products = []
        for p in value:
            if isinstance(p, Species):
                cpp_products.append(_ReactionComponent(p.name))
            elif isinstance(p, tuple) and len(p) == 2:
                coefficient, species = p
                cpp_products.append(_ReactionComponent(species.name, coefficient))
            else:
                raise ValueError("Each gas_phase_product must be a Species or Tuple[float, Species]")
        self._instance.gas_phase_products = cpp_products

    @property
    def gas_phase(self) -> Union[Phase, None]:
        """Get the gas phase (returns Python object)."""
        return self._gas_phase

    @gas_phase.setter
    def gas_phase(self, value: Phase):
        """Set the gas phase."""
        self._gas_phase = value
        self._instance.gas_phase = value.name

    @property
    def aerosol_phase(self) -> Union[Phase, None]:
        """Get the aerosol phase (returns Python object)."""
        return self._aerosol_phase

    @aerosol_phase.setter
    def aerosol_phase(self, value: Phase):
        """Set the aerosol phase."""
        self._aerosol_phase = value
        self._instance.aerosol_phase = value.name

    @property
    def other_properties(self) -> Dict[str, Any]:
        """Get other properties."""
        return self._other_properties

    @other_properties.setter
    def other_properties(self, value: Dict[str, Any]):
        """Set other properties."""
        self._other_properties = value
        self._instance.other_properties = value

    @property
    def type(self):
        """Get the reaction type from the C++ instance."""
        return self._instance.type

    def serialize(self) -> Dict:
        """
        Serialize the Surface instance to a dictionary using Python-visible data.
        
        Returns:
            Dict: Serialized representation of the Surface.
        """
        serialize_dict = {
            "type": "SURFACE",
            "name": self._name,
            "reaction probability": self._reaction_probability,
        }
        
        # Handle gas_phase_species serialization
        if self._gas_phase_species is not None:
            if isinstance(self._gas_phase_species, Species):
                serialize_dict["gas-phase species"] = self._gas_phase_species.name
            elif isinstance(self._gas_phase_species, tuple):
                coefficient, species = self._gas_phase_species
                serialize_dict["gas-phase species"] = {
                    "species name": species.name,
                    "coefficient": coefficient,
                }
        
        # Handle gas_phase_products serialization
        if self._gas_phase_products:
            products = []
            for p in self._gas_phase_products:
                if isinstance(p, Species):
                    products.append(p.name)
                elif isinstance(p, tuple):
                    coefficient, species = p
                    products.append({
                        "species name": species.name,
                        "coefficient": coefficient,
                    })
            serialize_dict["gas-phase products"] = products
        
        # Handle phase names
        if self._gas_phase is not None:
            serialize_dict["gas phase"] = self._gas_phase.name
        if self._aerosol_phase is not None:
            serialize_dict["aerosol phase"] = self._aerosol_phase.name
        
        # Add other properties
        _add_other_properties(serialize_dict, self._other_properties)
        return _remove_empty_keys(serialize_dict)

    @staticmethod
    def serialize_static(instance) -> Dict:
        """
        Static serialize method for compatibility with C++ _Surface objects.

        Args:
            instance: The _Surface instance to serialize.

        Returns:
            Dict: A dictionary representation of the Surface object.
        """
        serialize_dict = {
            "type": "SURFACE",
            "name": instance.name,
            "reaction probability": instance.reaction_probability,
        }
        
        # Handle gas_phase_species serialization from C++ object
        if hasattr(instance, 'gas_phase_species') and instance.gas_phase_species is not None:
            gas_species = instance.gas_phase_species
            if hasattr(gas_species, 'coefficient') and gas_species.coefficient != 1.0:
                serialize_dict["gas-phase species"] = {
                    "species name": gas_species.species_name,
                    "coefficient": gas_species.coefficient,
                }
            else:
                serialize_dict["gas-phase species"] = gas_species.species_name
        
        # Handle gas_phase_products serialization from C++ object
        if hasattr(instance, 'gas_phase_products') and instance.gas_phase_products:
            products = []
            for p in instance.gas_phase_products:
                if hasattr(p, 'coefficient') and p.coefficient != 1.0:
                    products.append({
                        "species name": p.species_name,
                        "coefficient": p.coefficient,
                    })
                else:
                    products.append(p.species_name)
            serialize_dict["gas-phase products"] = products
        
        # Handle phase names from C++ object
        if hasattr(instance, 'gas_phase') and instance.gas_phase:
            serialize_dict["gas phase"] = instance.gas_phase
        if hasattr(instance, 'aerosol_phase') and instance.aerosol_phase:
            serialize_dict["aerosol phase"] = instance.aerosol_phase
        
        # Add other properties
        if hasattr(instance, 'other_properties'):
            _add_other_properties(serialize_dict, instance.other_properties)
        
        return _remove_empty_keys(serialize_dict)
