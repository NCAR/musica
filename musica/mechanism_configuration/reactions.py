from typing import Optional, Any, Dict, List, Union
from musica import _ReactionType, _Reactions, _ReactionsIterator
from .species import Species, _Species
from .utils import _remove_empty_keys


class ReactionType(_ReactionType):
    """
    A enum class representing a reaction type in a chemical mechanism.
    """


class Reactions(_Reactions):
    """
    A class representing a collection of reactions in a chemical mechanism.

    Attributes:
        reactions (List[Any]): A list of reactions in the mechanism.
    """

    def __init__(
        self,
        reactions: Optional[List[Any]] = None,
    ):
        """
        Initializes the Reactions object with the given parameters.

        Args:
            reactions (List[]): A list of reactions in the mechanism.
        """
        super().__init__(reactions)


class ReactionsIterator(_ReactionsIterator):
    """
    An iterator for the Reactions class.
    """


class ReactionComponentSerializer():
    """
    A class for serializing reaction components.
    """

    @staticmethod
    def serialize_reaction_component(rc) -> Union[Dict, str]:
        if isinstance(rc, Species) or isinstance(rc, _Species):
            return rc.name

        return _remove_empty_keys({
            "species name": rc.species_name,
            "coefficient": rc.coefficient,
            "other_properties": rc.other_properties,
        })

    @staticmethod
    def serialize_list_reaction_components(reaction_component_list) -> List[Union[Dict, str]]:
        ret = []
        for rc in reaction_component_list:
            ret.append(ReactionComponentSerializer.serialize_reaction_component(rc))
        return ret
