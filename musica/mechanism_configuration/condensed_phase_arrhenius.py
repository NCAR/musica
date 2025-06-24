from typing import Optional, Any, Dict, List, Union, Tuple
from musica import _CondensedPhaseArrhenius, _ReactionComponent
from .phase import Phase
from .species import Species
from .reactions import ReactionComponentSerializer
from .utils import _add_other_properties, _remove_empty_keys
from musica.constants import BOLTZMANN


class CondensedPhaseArrhenius(_CondensedPhaseArrhenius):
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
        aerosol_phase (Phase): The aerosol phase in which the reaction occurs.
        aerosol_phase_water (Species): The water species in the aerosol phase.
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
        reactants: Optional[List[Union[Species, Tuple[float, Species]]]] = None,
        products: Optional[List[Union[Species, Tuple[float, Species]]]] = None,
        aerosol_phase: Optional[Phase] = None,
        aerosol_phase_water: Optional[Species] = None,
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
            aerosol_phase (Phase): The aerosol phase in which the reaction occurs.
            aerosol_phase_water (Species): The water species in the aerosol phase.
            other_properties (Dict[str, Any]): A dictionary of other properties of the condensed phase Arrhenius rate constant.
        """
        super().__init__()
        self.name = name if name is not None else self.name
        self.A = A if A is not None else self.A
        self.B = B if B is not None else self.B
        if C is not None and Ea is not None:
            raise ValueError("Cannot specify both C and Ea.")
        self.C = -Ea / BOLTZMANN if Ea is not None else C if C is not None else self.C
        self.D = D if D is not None else self.D
        self.E = E if E is not None else self.E
        self.reactants = (
            [
                (
                    _ReactionComponent(r.name)
                    if isinstance(r, Species)
                    else _ReactionComponent(r[1].name, r[0])
                )
                for r in reactants
            ]
            if reactants is not None
            else self.reactants
        )
        self.products = (
            [
                (
                    _ReactionComponent(p.name)
                    if isinstance(p, Species)
                    else _ReactionComponent(p[1].name, p[0])
                )
                for p in products
            ]
            if products is not None
            else self.products
        )
        self.aerosol_phase = aerosol_phase.name if aerosol_phase is not None else self.aerosol_phase
        self.aerosol_phase_water = (
            aerosol_phase_water.name if aerosol_phase_water is not None else self.aerosol_phase_water
        )
        self.other_properties = other_properties if other_properties is not None else self.other_properties

    @staticmethod
    def serialize(instance) -> Dict:
        serialize_dict = {
            "type": "CONDENSED_PHASE_ARRHENIUS",
            "name": instance.name,
            "A": instance.A,
            "B": instance.B,
            "C": instance.C,
            "D": instance.D,
            "E": instance.E,
            "reactants": ReactionComponentSerializer.serialize_list_reaction_components(instance.reactants),
            "products":  ReactionComponentSerializer.serialize_list_reaction_components(instance.products),
            "aerosol phase": instance.aerosol_phase,
            "aerosol-phase water": instance.aerosol_phase_water,
        }
        _add_other_properties(serialize_dict, instance.other_properties)
        return _remove_empty_keys(serialize_dict)
