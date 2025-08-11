from typing import Optional, Any, Dict, List, Union, Tuple
from .. import backend
from .phase import Phase
from .species import Species
from .reactions import ReactionComponentSerializer
from .utils import _add_other_properties

_backend = backend.get_backend()
_Troe = _backend._mechanism_configuration._Troe
_ReactionComponent = _backend._mechanism_configuration._ReactionComponent
ReactionType = _backend._mechanism_configuration._ReactionType


class Troe:
    """
    A class representing a Troe rate constant.

    Attributes:
        name (str): The name of the Troe rate constant.
        k0_A (float): Pre-exponential factor for the low-pressure limit [(mol m-3)^(n-1)s-1].
        k0_B (float): Temperature exponent for the low-pressure limit [unitless].
        k0_C (float): Exponential term for the low-pressure limit [K-1].
        kinf_A (float): Pre-exponential factor for the high-pressure limit [(mol m-3)^(n-1)s-1].
        kinf_B (float): Temperature exponent for the high-pressure limit [unitless].
        kinf_C (float): Exponential term for the high-pressure limit [K-1].
        Fc (float): Troe parameter [unitless].
        N (float): Troe parameter [unitless].
        reactants (List[Union[Species, Tuple[float, Species]]]): A list of reactants involved in the reaction.
        products (List[Union[Species, Tuple[float, Species]]]): A list of products formed in the reaction.
        gas_phase (Phase): The gas phase in which the reaction occurs.
        other_properties (Dict[str, Any]): A dictionary of other properties of the Troe rate constant.
    """

    def __init__(
        self,
        name: Optional[str] = None,
        k0_A: Optional[float] = None,
        k0_B: Optional[float] = None,
        k0_C: Optional[float] = None,
        kinf_A: Optional[float] = None,
        kinf_B: Optional[float] = None,
        kinf_C: Optional[float] = None,
        Fc: Optional[float] = None,
        N: Optional[float] = None,
        reactants: Optional[List[Union[Species,
                                       Tuple[float, Species]]]] = None,
        products: Optional[List[Union[Species, Tuple[float, Species]]]] = None,
        gas_phase: Optional[Phase] = None,
        other_properties: Optional[Dict[str, Any]] = None,
    ):
        """
        Initializes the Troe object with the given parameters.

        k0 = k0_A * exp( k0_C / T ) * ( T / 300.0 )^k0_B
        kinf = kinf_A * exp( kinf_C / T ) * ( T / 300.0 )^kinf_B
        k = k0[M] / ( 1 + k0[M] / kinf ) * Fc^(1 + 1/N*(log10(k0[M]/kinf))^2)^-1

        where:
            k = rate constant
            k0 = low-pressure limit rate constant
            kinf = high-pressure limit rate constant
            k0_A = pre-exponential factor for the low-pressure limit [(mol m-3)^(n-1)s-1]
            k0_B = temperature exponent for the low-pressure limit [unitless]
            k0_C = exponential term for the low-pressure limit [K-1]
            kinf_A = pre-exponential factor for the high-pressure limit [(mol m-3)^(n-1)s-1]
            kinf_B = temperature exponent for the high-pressure limit [unitless]
            kinf_C = exponential term for the high-pressure limit [K-1]
            Fc = Troe parameter [unitless]
            N = Troe parameter [unitless]
            T = temperature [K]
            M = concentration of the third body [mol m-3]

        Args:
            name (str): The name of the Troe rate constant.
            k0_A (float): Pre-exponential factor for the low-pressure limit [(mol m-3)^(n-1)s-1].
            k0_B (float): Temperature exponent for the low-pressure limit [unitless].
            k0_C (float): Exponential term for the low-pressure limit [K-1].
            kinf_A (float): Pre-exponential factor for the high-pressure limit [(mol m-3)^(n-1)s-1].
            kinf_B (float): Temperature exponent for the high-pressure limit [unitless].
            kinf_C (float): Exponential term for the high-pressure limit [K-1].
            Fc (float): Troe parameter [unitless].
            N (float): Troe parameter [unitless].
            reactants (List[Union[Species, Tuple[float, Species]]]): A list of reactants involved in the reaction.
            products (List[Union[Species, Tuple[float, Species]]]): A list of products formed in the reaction.
            gas_phase (Phase): The gas phase in which the reaction occurs.
            other_properties (Dict[str, Any]): A dictionary of other properties of the Troe rate constant.
        """
        # Create the internal C++ instance
        self._instance = _Troe()

        # Set all parameters using properties
        if name is not None:
            self.name = name
        if k0_A is not None:
            self.k0_A = k0_A
        if k0_B is not None:
            self.k0_B = k0_B
        if k0_C is not None:
            self.k0_C = k0_C
        if kinf_A is not None:
            self.kinf_A = kinf_A
        if kinf_B is not None:
            self.kinf_B = kinf_B
        if kinf_C is not None:
            self.kinf_C = kinf_C
        if Fc is not None:
            self.Fc = Fc
        if N is not None:
            self.N = N
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
        """Get the name of the Troe rate constant."""
        return self._instance.name

    @name.setter
    def name(self, value: str):
        """Set the name of the Troe rate constant."""
        self._instance.name = value

    @property
    def k0_A(self) -> float:
        """Get the pre-exponential factor for the low-pressure limit."""
        return self._instance.k0_A

    @k0_A.setter
    def k0_A(self, value: float):
        """Set the pre-exponential factor for the low-pressure limit."""
        self._instance.k0_A = value

    @property
    def k0_B(self) -> float:
        """Get the temperature exponent for the low-pressure limit."""
        return self._instance.k0_B

    @k0_B.setter
    def k0_B(self, value: float):
        """Set the temperature exponent for the low-pressure limit."""
        self._instance.k0_B = value

    @property
    def k0_C(self) -> float:
        """Get the exponential term for the low-pressure limit."""
        return self._instance.k0_C

    @k0_C.setter
    def k0_C(self, value: float):
        """Set the exponential term for the low-pressure limit."""
        self._instance.k0_C = value

    @property
    def kinf_A(self) -> float:
        """Get the pre-exponential factor for the high-pressure limit."""
        return self._instance.kinf_A

    @kinf_A.setter
    def kinf_A(self, value: float):
        """Set the pre-exponential factor for the high-pressure limit."""
        self._instance.kinf_A = value

    @property
    def kinf_B(self) -> float:
        """Get the temperature exponent for the high-pressure limit."""
        return self._instance.kinf_B

    @kinf_B.setter
    def kinf_B(self, value: float):
        """Set the temperature exponent for the high-pressure limit."""
        self._instance.kinf_B = value

    @property
    def kinf_C(self) -> float:
        """Get the exponential term for the high-pressure limit."""
        return self._instance.kinf_C

    @kinf_C.setter
    def kinf_C(self, value: float):
        """Set the exponential term for the high-pressure limit."""
        self._instance.kinf_C = value

    @property
    def Fc(self) -> float:
        """Get the Troe parameter Fc."""
        return self._instance.Fc

    @Fc.setter
    def Fc(self, value: float):
        """Set the Troe parameter Fc."""
        self._instance.Fc = value

    @property
    def N(self) -> float:
        """Get the Troe parameter N."""
        return self._instance.N

    @N.setter
    def N(self, value: float):
        """Set the Troe parameter N."""
        self._instance.N = value

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
        return ReactionType.Troe

    def _create_serialize_dict(self, instance) -> Dict:
        """
        Helper method to create the serialization dictionary.

        Args:
            instance: The instance to serialize (either self._instance or a _Troe object).

        Returns:
            Dict: Base serialization dictionary.
        """
        return {
            "type": "TROE",
            "name": instance.name,
            "k0_A": instance.k0_A,
            "k0_B": instance.k0_B,
            "k0_C": instance.k0_C,
            "kinf_A": instance.kinf_A,
            "kinf_B": instance.kinf_B,
            "kinf_C": instance.kinf_C,
            "Fc": instance.Fc,
            "N": instance.N,
            "reactants": ReactionComponentSerializer.serialize_list_reaction_components(instance.reactants),
            "products": ReactionComponentSerializer.serialize_list_reaction_components(instance.products),
            "gas phase": instance.gas_phase,
        }

    def serialize(self) -> Dict:
        """
        Serialize the Troe object to a dictionary using only Python-visible data.

        Returns:
            Dict: A dictionary representation of the Troe object.
        """
        serialize_dict = self._create_serialize_dict(self._instance)
        _add_other_properties(serialize_dict, self.other_properties)
        return serialize_dict

    @staticmethod
    def serialize_static(instance) -> Dict:
        """
        Static serialize method for compatibility with C++ _Troe objects.

        Args:
            instance: The _Troe instance to serialize.

        Returns:
            Dict: A dictionary representation of the Troe object.
        """
        # Create a temporary Troe object to use the helper method
        temp_troe = Troe()
        serialize_dict = temp_troe._create_serialize_dict(instance)
        _add_other_properties(serialize_dict, instance.other_properties)
        return serialize_dict
