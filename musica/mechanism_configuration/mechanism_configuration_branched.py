from typing import Optional, Any, Dict, List, Union, Tuple
from musica import _Branched, _ReactionComponent
from .mechanism_configuration_phase import Phase
from .mechanism_configuration_species import Species
from .mechanism_configuration_reactions import ReactionComponentSerializer
from .mechanism_configuration_utils import add_other_properties, remove_empty_keys


class Branched(_Branched):
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
        reactants: Optional[List[Union[Species, Tuple[float, Species]]]] = None,
        nitrate_products: Optional[List[Union[Species, Tuple[float, Species]]]] = None,
        alkoxy_products: Optional[List[Union[Species, Tuple[float, Species]]]] = None,
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
        super().__init__()
        self.name = name if name is not None else self.name
        self.X = X if X is not None else self.X
        self.Y = Y if Y is not None else self.Y
        self.a0 = a0 if a0 is not None else self.a0
        self.n = n if n is not None else self.n
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
        self.nitrate_products = (
            [
                (
                    _ReactionComponent(p.name)
                    if isinstance(p, Species)
                    else _ReactionComponent(p[1].name, p[0])
                )
                for p in nitrate_products
            ]
            if nitrate_products is not None
            else self.nitrate_products
        )
        self.alkoxy_products = (
            [
                (
                    _ReactionComponent(p.name)
                    if isinstance(p, Species)
                    else _ReactionComponent(p[1].name, p[0])
                )
                for p in alkoxy_products
            ]
            if alkoxy_products is not None
            else self.alkoxy_products
        )
        self.gas_phase = gas_phase.name if gas_phase is not None else self.gas_phase
        self.other_properties = other_properties if other_properties is not None else self.other_properties

    @staticmethod
    def serialize(cls) -> Dict:
        serialize_dict = {
            "type": "BRANCHED_NO_RO2",
            "name": cls.name,
            "X": cls.X,
            "Y": cls.Y,
            "a0": cls.a0,
            "n": cls.n,
            "reactants": ReactionComponentSerializer.serialize_list_reaction_components(cls.reactants),
            "nitrate products": ReactionComponentSerializer.serialize_list_reaction_components(cls.nitrate_products),
            "alkoxy products": ReactionComponentSerializer.serialize_list_reaction_components(cls.alkoxy_products),
            "gas phase": cls.gas_phase,
        }
        add_other_properties(serialize_dict, cls.other_properties)
        return remove_empty_keys(serialize_dict)
