from .utils import _add_other_properties
from .reactions import ReactionComponentSerializer
from .species import Species
from .phase import Phase
from typing import Optional, Any, Dict, List, Union, Tuple
from .. import backend

_backend = backend.get_backend()
_Tunneling = _backend._mechanism_configuration._Tunneling
_ReactionComponent = _backend._mechanism_configuration._ReactionComponent
ReactionType = _backend._mechanism_configuration._ReactionType


class Tunneling:
    """
    A class representing a quantum tunneling reaction rate constant.

    k = A * exp( -B / T ) * exp( C / T^3 )

    where:
        k = rate constant
        A = pre-exponential factor [(mol m-3)^(n-1)s-1]
        B = tunneling parameter [K^-1]
        C = tunneling parameter [K^-3]
        T = temperature [K]
        n = number of reactants

    Attributes:
        name (str): The name of the tunneling reaction rate constant.
        A (float): Pre-exponential factor [(mol m-3)^(n-1)s-1].
        B (float): Tunneling parameter [K^-1].
        C (float): Tunneling parameter [K^-3].
        reactants (List[Union[Species, Tuple[float, Species]]]): A list of reactants involved in the reaction.
        products (List[Union[Species, Tuple[float, Species]]]): A list of products formed in the reaction.
        gas_phase (Phase): The gas phase in which the reaction occurs.
        other_properties (Dict[str, Any]): A dictionary of other properties of the tunneling reaction rate constant.
    """

    def __init__(
        self,
        name: Optional[str] = None,
        A: Optional[float] = None,
        B: Optional[float] = None,
        C: Optional[float] = None,
        reactants: Optional[List[Union[Species,
                                       Tuple[float, Species]]]] = None,
        products: Optional[List[Union[Species, Tuple[float, Species]]]] = None,
        gas_phase: Optional[Phase] = None,
        other_properties: Optional[Dict[str, Any]] = None,
    ):
        """
        Initializes the Tunneling object with the given parameters.

        Args:
            name (str): The name of the tunneling reaction rate constant.
            A (float): Pre-exponential factor [(mol m-3)^(n-1)s-1].
            B (float): Tunneling parameter [K^-1].
            C (float): Tunneling parameter [K^-3].
            reactants (List[Union[Species, Tuple[float, Species]]]): A list of reactants involved in the reaction.
            products (List[Union[Species, Tuple[float, Species]]]): A list of products formed in the reaction.
            gas_phase (Phase): The gas phase in which the reaction occurs.
            other_properties (Dict[str, Any]): A dictionary of other properties of the tunneling reaction rate constant.
        """
        # Create the internal C++ instance
        self._instance = _Tunneling()

        # Set all parameters
        if name is not None:
            self.name = name
        if A is not None:
            self.A = A
        if B is not None:
            self.B = B
        if C is not None:
            self.C = C
        if reactants is not None:
            self.reactants = reactants
        if products is not None:
            self.products = products
        if gas_phase is not None:
            self.gas_phase = gas_phase
        if other_properties is not None:
            self.other_properties = other_properties

    # Property delegation to self._instance
    @property
    def name(self) -> str:
        """Get the name of the tunneling reaction rate constant."""
        return self._instance.name

    @name.setter
    def name(self, value: str):
        """Set the name of the tunneling reaction rate constant."""
        self._instance.name = value

    @property
    def A(self) -> float:
        """Get the pre-exponential factor."""
        return self._instance.A

    @A.setter
    def A(self, value: float):
        """Set the pre-exponential factor."""
        self._instance.A = value

    @property
    def B(self) -> float:
        """Get the tunneling parameter B."""
        return self._instance.B

    @B.setter
    def B(self, value: float):
        """Set the tunneling parameter B."""
        self._instance.B = value

    @property
    def C(self) -> float:
        """Get the tunneling parameter C."""
        return self._instance.C

    @C.setter
    def C(self, value: float):
        """Set the tunneling parameter C."""
        self._instance.C = value

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
        return ReactionType.Tunneling

    def serialize(self) -> Dict:
        """
        Serialize the Tunneling object to a dictionary using only Python-visible data.

        Returns:
            Dict: A dictionary representation of the Tunneling object.
        """
        serialize_dict = {
            "type": "TUNNELING",
            "name": self._instance.name,
            "A": self._instance.A,
            "B": self._instance.B,
            "C": self._instance.C,
            "reactants": ReactionComponentSerializer.serialize_list_reaction_components(self._instance.reactants),
            "products": ReactionComponentSerializer.serialize_list_reaction_components(self._instance.products),
            "gas phase": self._instance.gas_phase,
        }
        _add_other_properties(serialize_dict, self.other_properties)
        return serialize_dict

    @staticmethod
    def serialize_static(instance) -> Dict:
        """
        Static serialize method for compatibility with C++ _Tunneling objects.

        Args:
            instance: The _Tunneling instance to serialize.

        Returns:
            Dict: A dictionary representation of the Tunneling object.
        """
        # Create a temporary Tunneling object and use its serialize method
        temp_tunneling = Tunneling()
        temp_tunneling._instance = instance
        return temp_tunneling.serialize()
