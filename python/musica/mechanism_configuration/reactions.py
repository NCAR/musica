from typing import Optional, Any, Dict, List, Union
from .. import backend
from .species import Species
from .utils import _remove_empty_keys

_backend = backend.get_backend()
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
    original_init(self, reactions)


Reactions.__init__ = __init__
