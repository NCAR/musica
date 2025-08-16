from .utils import _add_other_properties
from .reactions import ReactionComponentSerializer
from .species import Species
from .phase import Phase
from typing import Optional, Any, Dict, List, Union, Tuple
from .. import backend

_backend = backend.get_backend()
_Branched = _backend._mechanism_configuration._Branched
_ReactionComponent = _backend._mechanism_configuration._ReactionComponent
ReactionType = _backend._mechanism_configuration._ReactionType


class Branched:
    """
    A class representing a branched reaction rate constant.

    (TODO: get details from MusicBox)

    Attributes:
        name (str): The name of the branched reaction rate constant.
        X (float): Pre-exponential branching factor [(mol m-3)^-(n-1)s-1].
        Y (float): Exponential branching factor [K-1].
        a0 (float): Z parameter [unitless].
        n (float): A parameter [unitless].
        reactants (List[Union[Species, Tuple[float, Species]]]): A list of reactants involved in the reaction.
        nitrate_products (List[Union[Species, Tuple[float, Species]]]): A list of products formed in the nitrate branch.
        alkoxy_products (List[Union[Species, Tuple[float, Species]]]): A list of products formed in the alkoxy branch.
        gas_phase (Phase): The gas phase in which the reaction occurs.
        other_properties (Dict[str, Any]): A dictionary of other properties of the branched reaction rate constant.
    """

    def __init__(
        self,
        name: Optional[str] = None,
        X: Optional[float] = None,
        Y: Optional[float] = None,
        a0: Optional[float] = None,
        n: Optional[float] = None,
        reactants: Optional[List[Union[Species,
                                       Tuple[float, Species]]]] = None,
        nitrate_products: Optional[List[Union[Species,
                                              Tuple[float, Species]]]] = None,
        alkoxy_products: Optional[List[Union[Species,
                                             Tuple[float, Species]]]] = None,
        gas_phase: Optional[Phase] = None,
        other_properties: Optional[Dict[str, Any]] = None,
    ):
        """
        Initializes the Branched object with the given parameters.

        Args:
            name (str): The name of the branched reaction rate constant.
            X (float): Pre-exponential branching factor [(mol m-3)^-(n-1)s-1].
            Y (float): Exponential branching factor [K-1].
            a0 (float): Z parameter [unitless].
            n (float): A parameter [unitless].
            reactants (List[Union[Species, Tuple[float, Species]]]): A list of reactants involved in the reaction.
            nitrate_products (List[Union[Species, Tuple[float, Species]]]): A list of products formed in the nitrate branch.
            alkoxy_products (List[Union[Species, Tuple[float, Species]]]): A list of products formed in the alkoxy branch.
            gas_phase (Phase): The gas phase in which the reaction occurs.
            other_properties (Dict[str, Any]): A dictionary of other properties of the branched reaction rate constant.
        """
        # Create the internal C++ instance
        self._instance = _Branched()

        # Set all parameters
        if name is not None:
            self.name = name
        if X is not None:
            self.X = X
        if Y is not None:
            self.Y = Y
        if a0 is not None:
            self.a0 = a0
        if n is not None:
            self.n = n
        if reactants is not None:
            self.reactants = reactants
        if nitrate_products is not None:
            self.nitrate_products = nitrate_products
        if alkoxy_products is not None:
            self.alkoxy_products = alkoxy_products
        if gas_phase is not None:
            self.gas_phase = gas_phase
        if other_properties is not None:
            self.other_properties = other_properties

    # Property delegation to self._instance
    @property
    def name(self) -> str:
        """Get the name of the branched reaction rate constant."""
        return self._instance.name

    @name.setter
    def name(self, value: str):
        """Set the name of the branched reaction rate constant."""
        self._instance.name = value

    @property
    def X(self) -> float:
        """Get the pre-exponential branching factor."""
        return self._instance.X

    @X.setter
    def X(self, value: float):
        """Set the pre-exponential branching factor."""
        self._instance.X = value

    @property
    def Y(self) -> float:
        """Get the exponential branching factor."""
        return self._instance.Y

    @Y.setter
    def Y(self, value: float):
        """Set the exponential branching factor."""
        self._instance.Y = value

    @property
    def a0(self) -> float:
        """Get the Z parameter."""
        return self._instance.a0

    @a0.setter
    def a0(self, value: float):
        """Set the Z parameter."""
        self._instance.a0 = value

    @property
    def n(self) -> float:
        """Get the A parameter."""
        return self._instance.n

    @n.setter
    def n(self, value: float):
        """Set the A parameter."""
        self._instance.n = value

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
    def nitrate_products(self) -> List[Union[Species, Tuple[float, Species]]]:
        """Get the nitrate products as Python objects."""
        # Convert from C++ _ReactionComponent objects to Python Species objects
        result = []
        for rc in self._instance.nitrate_products:
            if hasattr(rc, 'coefficient') and rc.coefficient != 1.0:
                # Create a tuple with coefficient and species
                species = Species(name=rc.species_name)
                result.append((rc.coefficient, species))
            else:
                # Just the species
                species = Species(name=rc.species_name)
                result.append(species)
        return result

    @nitrate_products.setter
    def nitrate_products(self, value: List[Union[Species, Tuple[float, Species]]]):
        """Set the nitrate products, converting from Python to C++ objects."""
        cpp_products = []
        for p in value:
            if isinstance(p, Species):
                cpp_products.append(_ReactionComponent(p.name))
            elif isinstance(p, tuple) and len(p) == 2:
                coefficient, species = p
                cpp_products.append(_ReactionComponent(species.name, coefficient))
            else:
                raise ValueError(f"Invalid nitrate product format: {p}")
        self._instance.nitrate_products = cpp_products

    @property
    def alkoxy_products(self) -> List[Union[Species, Tuple[float, Species]]]:
        """Get the alkoxy products as Python objects."""
        # Convert from C++ _ReactionComponent objects to Python Species objects
        result = []
        for rc in self._instance.alkoxy_products:
            if hasattr(rc, 'coefficient') and rc.coefficient != 1.0:
                # Create a tuple with coefficient and species
                species = Species(name=rc.species_name)
                result.append((rc.coefficient, species))
            else:
                # Just the species
                species = Species(name=rc.species_name)
                result.append(species)
        return result

    @alkoxy_products.setter
    def alkoxy_products(self, value: List[Union[Species, Tuple[float, Species]]]):
        """Set the alkoxy products, converting from Python to C++ objects."""
        cpp_products = []
        for p in value:
            if isinstance(p, Species):
                cpp_products.append(_ReactionComponent(p.name))
            elif isinstance(p, tuple) and len(p) == 2:
                coefficient, species = p
                cpp_products.append(_ReactionComponent(species.name, coefficient))
            else:
                raise ValueError(f"Invalid alkoxy product format: {p}")
        self._instance.alkoxy_products = cpp_products

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
        return ReactionType.Branched

    def serialize(self) -> Dict:
        """
        Serialize the Branched object to a dictionary using only Python-visible data.

        Returns:
            Dict: A dictionary representation of the Branched object.
        """
        serialize_dict = {
            "type": "BRANCHED_NO_RO2",
            "name": self.name,
            "X": self.X,
            "Y": self.Y,
            "a0": self.a0,
            "n": self.n,
            "reactants": ReactionComponentSerializer.serialize_list_reaction_components(
                self._instance.reactants),
            "nitrate products": ReactionComponentSerializer.serialize_list_reaction_components(
                self._instance.nitrate_products),
            "alkoxy products": ReactionComponentSerializer.serialize_list_reaction_components(
                self._instance.alkoxy_products),
            "gas phase": self.gas_phase,
        }
        _add_other_properties(serialize_dict, self.other_properties)
        return serialize_dict

    @staticmethod
    def serialize_static(instance) -> Dict:
        """
        Static serialize method for compatibility with C++ _Branched objects.

        Args:
            instance: The _Branched instance to serialize.

        Returns:
            Dict: A dictionary representation of the Branched object.
        """
        # Create a temporary Branched object and use its instance serialize method
        temp_branched = Branched()
        temp_branched._instance = instance
        return temp_branched.serialize()
