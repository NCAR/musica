from .utils import _add_other_properties, _remove_empty_keys
from .reactions import ReactionComponentSerializer
from .species import Species
from .phase import Phase
from typing import Optional, Any, Dict, List, Union, Tuple
from .. import backend

_backend = backend.get_backend()
_AqueousEquilibrium = _backend._mechanism_configuration._AqueousEquilibrium
_ReactionComponent = _backend._mechanism_configuration._ReactionComponent
ReactionType = _backend._mechanism_configuration._ReactionType


class AqueousEquilibrium:
    """
    A class representing an aqueous equilibrium reaction rate constant.

    Attributes:
        name (str): The name of the aqueous equilibrium reaction rate constant.
        aerosol_phase (Phase): The aerosol phase in which the reaction occurs.
        aerosol_phase_water (Species): The water species in the aerosol phase.
        reactants (List[Union[Species, Tuple[float, Species]]]): A list of reactants involved in the reaction.
        products (List[Union[Species, Tuple[float, Species]]]): A list of products formed in the reaction.
        A (float): Pre-exponential factor [(mol m-3)^(n-1)s-1].
        C (float): Exponential term [K-1].
        k_reverse (float): Reverse rate constant [(mol m-3)^(n-1)s-1].
        other_properties (Dict[str, Any]): A dictionary of other properties of the aqueous equilibrium reaction rate constant.
    """

    def __init__(
        self,
        name: Optional[str] = None,
        aerosol_phase: Optional[Phase] = None,
        aerosol_phase_water: Optional[Species] = None,
        reactants: Optional[List[Union[Species,
                                       Tuple[float, Species]]]] = None,
        products: Optional[List[Union[Species, Tuple[float, Species]]]] = None,
        A: Optional[float] = None,
        C: Optional[float] = None,
        k_reverse: Optional[float] = None,
        other_properties: Optional[Dict[str, Any]] = None,
    ):
        """
        Initializes the AqueousEquilibrium object with the given parameters.

        Args:
            name (str): The name of the aqueous equilibrium reaction rate constant.
            aerosol_phase (Phase): The aerosol phase in which the reaction occurs.
            aerosol_phase_water (Species): The water species in the aerosol phase.
            reactants (List[Union[Species, Tuple[float, Species]]]): A list of reactants involved in the reaction.
            products (List[Union[Species, Tuple[float, Species]]]): A list of products formed in the reaction.
            A (float): Pre-exponential factor [(mol m-3)^(n-1)s-1].
            C (float): Exponential term [K-1].
            k_reverse (float): Reverse rate constant [(mol m-3)^(n-1)s-1].
            other_properties (Dict[str, Any]): A dictionary of other properties of the aqueous equilibrium reaction rate constant.
        """
        # Create the internal C++ instance
        self._instance = _AqueousEquilibrium()

        # Set all parameters
        if name is not None:
            self.name = name
        if aerosol_phase is not None:
            self.aerosol_phase = aerosol_phase
        if aerosol_phase_water is not None:
            self.aerosol_phase_water = aerosol_phase_water
        if reactants is not None:
            self.reactants = reactants
        if products is not None:
            self.products = products
        if A is not None:
            self.A = A
        if C is not None:
            self.C = C
        if k_reverse is not None:
            self.k_reverse = k_reverse
        if other_properties is not None:
            self.other_properties = other_properties

    # Property delegation to self._instance
    @property
    def name(self) -> str:
        """Get the name of the aqueous equilibrium reaction rate constant."""
        return self._instance.name

    @name.setter
    def name(self, value: str):
        """Set the name of the aqueous equilibrium reaction rate constant."""
        self._instance.name = value

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
    def aerosol_phase_water(self) -> str:
        """Get the aerosol phase water species name."""
        return self._instance.aerosol_phase_water

    @aerosol_phase_water.setter
    def aerosol_phase_water(self, value: Union[Species, str]):
        """Set the aerosol phase water species."""
        if isinstance(value, Species):
            self._instance.aerosol_phase_water = value.name
        elif isinstance(value, str):
            self._instance.aerosol_phase_water = value
        else:
            raise ValueError(f"Invalid aerosol_phase_water type: {type(value)}")

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
    def products(self) -> List[Union[Species, Tuple[float, Species]]]:
        """Get the products as Python objects."""
        # Convert from C++ _ReactionComponent objects to Python Species objects
        result = []
        for rc in self._instance.products:
            if hasattr(rc, 'coefficient') and rc.coefficient != 1.0:
                # Create a tuple with coefficient and species
                species = Species(name=rc.species_name)
                result.append((rc.coefficient, species))
            else:
                # Just the species
                species = Species(name=rc.species_name)
                result.append(species)
        return result

    @products.setter
    def products(self, value: List[Union[Species, Tuple[float, Species]]]):
        """Set the products, converting from Python to C++ objects."""
        cpp_products = []
        for p in value:
            if isinstance(p, Species):
                cpp_products.append(_ReactionComponent(p.name))
            elif isinstance(p, tuple) and len(p) == 2:
                coefficient, species = p
                cpp_products.append(_ReactionComponent(species.name, coefficient))
            else:
                raise ValueError(f"Invalid product format: {p}")
        self._instance.products = cpp_products

    @property
    def A(self) -> float:
        """Get the pre-exponential factor."""
        return self._instance.A

    @A.setter
    def A(self, value: float):
        """Set the pre-exponential factor."""
        self._instance.A = value

    @property
    def C(self) -> float:
        """Get the exponential term."""
        return self._instance.C

    @C.setter
    def C(self, value: float):
        """Set the exponential term."""
        self._instance.C = value

    @property
    def k_reverse(self) -> float:
        """Get the reverse rate constant."""
        return self._instance.k_reverse

    @k_reverse.setter
    def k_reverse(self, value: float):
        """Set the reverse rate constant."""
        self._instance.k_reverse = value

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
        return ReactionType.AqueousEquilibrium

    def _create_serialize_dict(self, instance) -> Dict:
        """
        Helper method to create the serialization dictionary.

        Args:
            instance: The instance to serialize (either self._instance or a _AqueousEquilibrium object).

        Returns:
            Dict: Base serialization dictionary.
        """
        return {
            "type": "AQUEOUS_EQUILIBRIUM",
            "name": instance.name,
            "aerosol phase": instance.aerosol_phase,
            "aerosol-phase water": instance.aerosol_phase_water,
            "reactants": ReactionComponentSerializer.serialize_list_reaction_components(instance.reactants),
            "products": ReactionComponentSerializer.serialize_list_reaction_components(instance.products),
            "A": instance.A,
            "C": instance.C,
            "k_reverse": instance.k_reverse,
        }

    def serialize(self) -> Dict:
        """
        Serialize the AqueousEquilibrium object to a dictionary using only Python-visible data.

        Returns:
            Dict: A dictionary representation of the AqueousEquilibrium object.
        """
        serialize_dict = self._create_serialize_dict(self._instance)
        _add_other_properties(serialize_dict, self.other_properties)
        return _remove_empty_keys(serialize_dict)

    @staticmethod
    def serialize_static(instance) -> Dict:
        """
        Static serialize method for compatibility with C++ _AqueousEquilibrium objects.

        Args:
            instance: The _AqueousEquilibrium instance to serialize.

        Returns:
            Dict: A dictionary representation of the AqueousEquilibrium object.
        """
        # Create a temporary AqueousEquilibrium object to use the helper method
        temp_aqueous_equilibrium = AqueousEquilibrium()
        serialize_dict = temp_aqueous_equilibrium._create_serialize_dict(instance)
        _add_other_properties(serialize_dict, instance.other_properties)
        return _remove_empty_keys(serialize_dict)
