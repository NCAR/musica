from .. import backend

_backend = backend.get_backend()
Version = _backend._mechanism_configuration._Version
_CppParse = _backend._mechanism_configuration._parse
ReactionType = _backend._mechanism_configuration._ReactionType


def parse(path: str):
    """Parse a mechanism configuration file.

    Args:
        path: Path to the configuration file.

    Returns:
        Mechanism: A Mechanism object.
    """
    from .mechanism import Mechanism
    return Mechanism._from_cpp(_CppParse(path))
