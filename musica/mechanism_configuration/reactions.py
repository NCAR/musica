from typing import Optional, Any, Dict, List, Union
from .. import backend
from .species import Species
from .utils import _remove_empty_keys

_backend = backend.get_backend()
ReactionType = _backend._mechanism_configuration._ReactionType
Reactions = _backend._mechanism_configuration._Reactions

original_init = Reactions.__init__

Reactions.__doc__ = """
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
    # Convert Python Arrhenius objects to C++ _Arrhenius objects for the C++ constructor
    if reactions is not None:
        cpp_reactions = []
        for reaction in reactions:
            if hasattr(reaction, '_instance'):
                # This is a Python wrapper around a C++ object, use the internal instance
                cpp_reactions.append(reaction._instance)
            else:
                # This is already a C++ object or other supported type
                cpp_reactions.append(reaction)
        original_init(self, cpp_reactions)
    else:
        original_init(self, reactions)
    
Reactions.__init__ = __init__

class ReactionComponentSerializer():
    """
    A class for serializing reaction components.
    """

    @staticmethod
    def serialize_reaction_component(rc) -> Union[Dict, str]:
        if isinstance(rc, Species):
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
            ret.append(
                ReactionComponentSerializer.serialize_reaction_component(rc))
        return ret
