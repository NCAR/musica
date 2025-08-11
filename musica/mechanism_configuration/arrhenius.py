from typing import Optional, Any, Dict, List, Union, Tuple
from .. import backend
from .phase import Phase
from .species import Species
from .reactions import ReactionComponentSerializer
from .utils import _add_other_properties
from ..constants import BOLTZMANN

_backend = backend.get_backend()
_Arrhenius = _backend._mechanism_configuration._Arrhenius
_ReactionComponent = _backend._mechanism_configuration._ReactionComponent
ReactionType = _backend._mechanism_configuration._ReactionType


class Arrhenius:
    """
    A class representing an Arrhenius rate constant.

    k = A * exp( C / T ) * ( T / D )^B * exp( 1 - E * P )

    where:
        k = rate constant
        A = pre-exponential factor [(mol m-3)^(n-1)s-1]
        B = temperature exponent [unitless]
        C = exponential term [K-1]
        D = reference temperature [K]
        E = pressure scaling term [Pa-1]
        T = temperature [K]
        P = pressure [Pa]
        n = number of reactants

    Attributes:
        name (str): The name of the Arrhenius rate constant.
        A (float): Pre-exponential factor [(mol m-3)^(n-1)s-1] where n is the number of reactants.
        B (float): Temperature exponent [unitless].
        C (float): Exponential term [K-1].
        D (float): Reference Temperature [K].
        E (float): Pressure scaling term [Pa-1].
        reactants (List[Union[Species, Tuple[float, Species]]]): A list of reactants involved in the reaction.
        products (List[Union[Species, Tuple[float, Species]]]): A list of products formed in the reaction.
        gas_phase (Phase): The gas phase in which the reaction occurs.
        other_properties (Dict[str, Any]): A dictionary of other properties of the Arrhenius rate constant.
    """

    def __init__(
        self,
        name: Optional[str] = None,
        A: Optional[float] = None,
        B: Optional[float] = None,
        C: Optional[float] = None,
        Ea: Optional[float] = None,
        D: Optional[float] = None,
        E: Optional[float] = None,
        reactants: Optional[List[Union[Species,
                                       Tuple[float, Species]]]] = None,
        products: Optional[List[Union[Species, Tuple[float, Species]]]] = None,
        gas_phase: Optional[Phase] = None,
        other_properties: Optional[Dict[str, Any]] = None,
    ):
        """
        Initializes the Arrhenius object with the given parameters.

        Args:
            name (str): The name of the Arrhenius rate constant.
            A (float): Pre-exponential factor [(mol m-3)^(n-1)s-1] where n is the number of reactants.
            B (float): Temperature exponent [unitless].
            C (float): Exponential term [K-1].
            Ea (float): Activation energy [J molecule-1].
            D (float): Reference Temperature [K].
            E (float): Pressure scaling term [Pa-1].
            reactants (List[Union[Species, Tuple[float, Species]]]): A list of reactants involved in the reaction.
            products (List[Union[Species, Tuple[float, Species]]]): A list of products formed in the reaction.
            gas_phase (Phase): The gas phase in which the reaction occurs.
            other_properties (Dict[str, Any]): A dictionary of other properties of the Arrhenius rate constant.
        """
        # Create the internal C++ instance
        self._instance = _Arrhenius()

        # Validate mutually exclusive parameters
        if C is not None and Ea is not None:
            raise ValueError("Cannot specify both C and Ea.")

        # Set all parameters
        if name is not None:
            self.name = name
        if A is not None:
            self.A = A
        if B is not None:
            self.B = B
        if Ea is not None:
            self.C = -Ea / BOLTZMANN
        elif C is not None:
            self.C = C
        if D is not None:
            self.D = D
        if E is not None:
            self.E = E
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
        """Get the name of the Arrhenius rate constant."""
        return self._instance.name

    @name.setter
    def name(self, value: str):
        """Set the name of the Arrhenius rate constant."""
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
        """Get the temperature exponent."""
        return self._instance.B

    @B.setter
    def B(self, value: float):
        """Set the temperature exponent."""
        self._instance.B = value

    @property
    def C(self) -> float:
        """Get the exponential term."""
        return self._instance.C

    @C.setter
    def C(self, value: float):
        """Set the exponential term."""
        self._instance.C = value

    @property
    def D(self) -> float:
        """Get the reference temperature."""
        return self._instance.D

    @D.setter
    def D(self, value: float):
        """Set the reference temperature."""
        self._instance.D = value

    @property
    def E(self) -> float:
        """Get the pressure scaling term."""
        return self._instance.E

    @E.setter
    def E(self, value: float):
        """Set the pressure scaling term."""
        self._instance.E = value

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
        return ReactionType.Arrhenius

    def _create_serialize_dict(self, instance) -> Dict:
        """
        Helper method to create the serialization dictionary.

        Args:
            instance: The instance to serialize (either self._instance or a _Arrhenius object).

        Returns:
            Dict: Base serialization dictionary.
        """
        return {
            "type": "ARRHENIUS",
            "name": instance.name,
            "A": instance.A,
            "B": instance.B,
            "C": instance.C,
            "D": instance.D,
            "E": instance.E,
            "reactants": ReactionComponentSerializer.serialize_list_reaction_components(instance.reactants),
            "products": ReactionComponentSerializer.serialize_list_reaction_components(instance.products),
            "gas phase": instance.gas_phase,
        }

    def serialize(self) -> Dict:
        """
        Serialize the Arrhenius object to a dictionary using only Python-visible data.

        Returns:
            Dict: A dictionary representation of the Arrhenius object.
        """
        serialize_dict = self._create_serialize_dict(self._instance)
        _add_other_properties(serialize_dict, self.other_properties)
        return serialize_dict

    @staticmethod
    def serialize_static(instance) -> Dict:
        """
        Static serialize method for compatibility with C++ _Arrhenius objects.

        Args:
            instance: The _Arrhenius instance to serialize.

        Returns:
            Dict: A dictionary representation of the Arrhenius object.
        """
        # Create a temporary Arrhenius object to use the helper method
        temp_arrhenius = Arrhenius()
        serialize_dict = temp_arrhenius._create_serialize_dict(instance)
        _add_other_properties(serialize_dict, instance.other_properties)
        return serialize_dict
