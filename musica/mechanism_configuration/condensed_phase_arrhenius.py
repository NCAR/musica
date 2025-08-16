from ..constants import BOLTZMANN
from .utils import _add_other_properties
from .reactions import ReactionComponentSerializer
from .species import Species
from .phase import Phase
from typing import Optional, Any, Dict, List, Union, Tuple
from .. import backend

_backend = backend.get_backend()
_CondensedPhaseArrhenius = _backend._mechanism_configuration._CondensedPhaseArrhenius
_ReactionComponent = _backend._mechanism_configuration._ReactionComponent


class CondensedPhaseArrhenius:
    """
    A class representing a condensed phase Arrhenius rate constant.

    Attributes:
        name (str): The name of the condensed phase Arrhenius rate constant.
        A (float): Pre-exponential factor [(mol m-3)^(n-1)s-1].
        B (float): Temperature exponent [unitless].
        C (float): Exponential term [K-1].
        Ea (float): Activation energy [J molecule-1].
        D (float): Reference Temperature [K].
        E (float): Pressure scaling term [Pa-1].
        reactants (List[Union[Species, Tuple[float, Species]]]): A list of reactants involved in the reaction.
        products (List[Union[Species, Tuple[float, Species]]]): A list of products formed in the reaction.
        condensed_phase (Phase): The condensed phase in which the reaction occurs.
        other_properties (Dict[str, Any]): A dictionary of other properties of the condensed phase Arrhenius rate constant.
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
        condensed_phase: Optional[Phase] = None,
        other_properties: Optional[Dict[str, Any]] = None,
    ):
        """
        Initializes the CondensedPhaseArrhenius object with the given parameters.

        Args:
            name (str): The name of the condensed phase Arrhenius rate constant.
            A (float): Pre-exponential factor [(mol m-3)^(n-1)s-1].
            B (float): Temperature exponent [unitless].
            C (float): Exponential term [K-1].
            Ea (float): Activation energy [J molecule-1].
            D (float): Reference Temperature [K].
            E (float): Pressure scaling term [Pa-1].
            reactants (List[Union[Species, Tuple[float, Species]]]]: A list of reactants involved in the reaction.
            products (List[Union[Species, Tuple[float, Species]]]]: A list of products formed in the reaction.
            condensed_phase (Phase): The condensed phase in which the reaction occurs.
            other_properties (Dict[str, Any]): A dictionary of other properties of the condensed phase Arrhenius rate constant.
        """
        # Create the internal C++ instance
        self._instance = _CondensedPhaseArrhenius()

        # Store Python objects for reactants, products, phases
        self._reactants = reactants if reactants is not None else []
        self._products = products if products is not None else []
        self._condensed_phase = condensed_phase
        self._other_properties = other_properties if other_properties is not None else {}

        # Set basic properties on the C++ instance
        if name is not None:
            self._instance.name = name
        if A is not None:
            self._instance.A = A
        if B is not None:
            self._instance.B = B
        if C is not None and Ea is not None:
            raise ValueError("Cannot specify both C and Ea.")
        if Ea is not None:
            self._instance.C = -Ea / BOLTZMANN
        elif C is not None:
            self._instance.C = C
        if D is not None:
            self._instance.D = D
        if E is not None:
            self._instance.E = E

        # Set reactants on the C++ instance
        if reactants is not None:
            self._instance.reactants = [
                (
                    _ReactionComponent(r.name)
                    if isinstance(r, Species)
                    else _ReactionComponent(r[1].name, r[0])
                )
                for r in reactants
            ]

        # Set products on the C++ instance
        if products is not None:
            self._instance.products = [
                (
                    _ReactionComponent(p.name)
                    if isinstance(p, Species)
                    else _ReactionComponent(p[1].name, p[0])
                )
                for p in products
            ]

        # Set phase information on the C++ instance
        if condensed_phase is not None:
            self._instance.condensed_phase = condensed_phase.name
        if other_properties is not None:
            self._instance.other_properties = other_properties

    def __getattr__(self, name):
        """Delegate unknown attribute access to the internal instance."""
        return getattr(self._instance, name)

    @property
    def type(self):
        """The reaction type."""
        return self._instance.type

    @property
    def name(self) -> str:
        """The name of the condensed phase Arrhenius rate constant."""
        return self._instance.name

    @name.setter
    def name(self, value: str):
        self._instance.name = value

    @property
    def A(self) -> float:
        """Pre-exponential factor [(mol m-3)^(n-1)s-1]."""
        return self._instance.A

    @A.setter
    def A(self, value: float):
        self._instance.A = value

    @property
    def B(self) -> float:
        """Temperature exponent [unitless]."""
        return self._instance.B

    @B.setter
    def B(self, value: float):
        self._instance.B = value

    @property
    def C(self) -> float:
        """Exponential term [K-1]."""
        return self._instance.C

    @C.setter
    def C(self, value: float):
        self._instance.C = value

    @property
    def D(self) -> float:
        """Reference Temperature [K]."""
        return self._instance.D

    @D.setter
    def D(self, value: float):
        self._instance.D = value

    @property
    def E(self) -> float:
        """Pressure scaling term [Pa-1]."""
        return self._instance.E

    @E.setter
    def E(self, value: float):
        self._instance.E = value

    @property
    def reactants(self) -> List[Union[Species, Tuple[float, Species]]]:
        """A list of reactants involved in the reaction."""
        return self._reactants

    @reactants.setter
    def reactants(self, value: List[Union[Species, Tuple[float, Species]]]):
        self._reactants = value
        # Update the C++ instance
        self._instance.reactants = [
            (
                _ReactionComponent(r.name)
                if isinstance(r, Species)
                else _ReactionComponent(r[1].name, r[0])
            )
            for r in value
        ]

    @property
    def products(self) -> List[Union[Species, Tuple[float, Species]]]:
        """A list of products formed in the reaction."""
        return self._products

    @products.setter
    def products(self, value: List[Union[Species, Tuple[float, Species]]]):
        self._products = value
        # Update the C++ instance
        self._instance.products = [
            (
                _ReactionComponent(p.name)
                if isinstance(p, Species)
                else _ReactionComponent(p[1].name, p[0])
            )
            for p in value
        ]

    @property
    def condensed_phase(self) -> Phase:
        """The condensed phase in which the reaction occurs."""
        return self._condensed_phase

    @condensed_phase.setter
    def condensed_phase(self, value: Phase):
        self._condensed_phase = value
        # Update the C++ instance
        self._instance.condensed_phase = value.name if value is not None else ""

    @property
    def other_properties(self) -> Dict[str, Any]:
        """A dictionary of other properties of the condensed phase Arrhenius rate constant."""
        return self._other_properties

    @other_properties.setter
    def other_properties(self, value: Dict[str, Any]):
        self._other_properties = value
        # Update the C++ instance
        self._instance.other_properties = value

    def serialize(self) -> Dict:
        """
        Serialize the CondensedPhaseArrhenius instance to a dictionary.

        Returns:
            Dict: A dictionary representation of the condensed phase Arrhenius rate constant.
        """
        # Convert Python reactants/products to serializable format
        def serialize_python_components(components):
            result = []
            for component in components:
                if isinstance(component, Species):
                    result.append(component.name)
                elif isinstance(component, tuple) and len(component) == 2:
                    # Handle (coefficient, Species) tuples
                    coefficient, species = component
                    result.append({
                        "species name": species.name,
                        "coefficient": coefficient
                    })
                else:
                    # Fallback: treat as Species
                    result.append(component.name if hasattr(component, 'name') else str(component))
            return result

        serialize_dict = {
            "type": "CONDENSED_PHASE_ARRHENIUS",
            "name": self.name,
            "A": self.A,
            "B": self.B,
            "C": self.C,
            "D": self.D,
            "E": self.E,
            "reactants": serialize_python_components(self._reactants),
            "products": serialize_python_components(self._products),
            "condensed phase": self._condensed_phase.name if self._condensed_phase is not None else "",
        }
        _add_other_properties(serialize_dict, self._other_properties)
        return serialize_dict

    @staticmethod
    def serialize_static(instance) -> Dict:
        """
        Static serialize method for backward compatibility.

        Args:
            instance: The CondensedPhaseArrhenius instance to serialize (can be Python wrapper or C++ type).

        Returns:
            Dict: A dictionary representation of the condensed phase Arrhenius rate constant.
        """
        # Check if it's the new composition-based Python wrapper
        if hasattr(instance, '_instance'):
            # New Python wrapper - use instance method
            return instance.serialize()
        else:
            # Old C++ wrapper type - use direct attribute access
            serialize_dict = {
                "type": "CONDENSED_PHASE_ARRHENIUS",
                "name": instance.name,
                "A": instance.A,
                "B": instance.B,
                "C": instance.C,
                "D": instance.D,
                "E": instance.E,
                "reactants": ReactionComponentSerializer.serialize_list_reaction_components(instance.reactants),
                "products": ReactionComponentSerializer.serialize_list_reaction_components(instance.products),
                "condensed phase": instance.condensed_phase,
            }
            _add_other_properties(serialize_dict, instance.other_properties)
            return serialize_dict
