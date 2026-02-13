from .. import backend

_backend = backend.get_backend()
Version = _backend._mechanism_configuration._Version
_CppParser = _backend._mechanism_configuration._Parser
ReactionType = _backend._mechanism_configuration._ReactionType


class Parser:
    """Parser for mechanism configuration files."""

    def __init__(self):
        self._cpp = _CppParser()

    def parse(self, path: str):
        """Parse a mechanism configuration file.

        Args:
            path: Path to the configuration file.

        Returns:
            Mechanism: A Mechanism object.
        """
        from .mechanism import Mechanism
        cpp_mechanism = self._cpp.parse(path)
        return Mechanism._from_cpp(cpp_mechanism)

    def parse_and_convert_v0(self, path: str, convert_reaction_units: bool = True):
        """Parse a v0 mechanism configuration file and convert to v1.

        Args:
            path: Path to the v0 configuration file.
            convert_reaction_units: Whether to convert reaction units.

        Returns:
            Mechanism: A Mechanism object.
        """
        from .mechanism import Mechanism
        cpp_mechanism = self._cpp.parse_and_convert_v0(path, convert_reaction_units)
        return Mechanism._from_cpp(cpp_mechanism)
