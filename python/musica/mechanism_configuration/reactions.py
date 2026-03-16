from typing import Optional, Any, Dict, List, Union
from .. import backend
from .._base import CppWrapper, _unwrap_list
from .species import Species
from .utils import _remove_empty_keys

_backend = backend.get_backend()
_Reactions = _backend._mechanism_configuration._Reactions

# C++ reaction type classes for isinstance checks
_cpp_types = _backend._mechanism_configuration


def _wrap_reaction(cpp_obj):
    """Wrap a raw C++ reaction object in the corresponding Python wrapper class."""
    from .arrhenius import Arrhenius
    from .branched import Branched
    from .emission import Emission
    from .first_order_loss import FirstOrderLoss
    from .photolysis import Photolysis
    from .surface import Surface
    from .taylor_series import TaylorSeries
    from .ternary_chemical_activation import TernaryChemicalActivation
    from .troe import Troe
    from .tunneling import Tunneling
    from .user_defined import UserDefined

    wrapper_map = {
        _cpp_types._Arrhenius: Arrhenius,
        _cpp_types._Branched: Branched,
        _cpp_types._Emission: Emission,
        _cpp_types._FirstOrderLoss: FirstOrderLoss,
        _cpp_types._Photolysis: Photolysis,
        _cpp_types._Surface: Surface,
        _cpp_types._TaylorSeries: TaylorSeries,
        _cpp_types._TernaryChemicalActivation: TernaryChemicalActivation,
        _cpp_types._Troe: Troe,
        _cpp_types._Tunneling: Tunneling,
        _cpp_types._UserDefined: UserDefined,
    }
    for cpp_type, wrapper_cls in wrapper_map.items():
        if isinstance(cpp_obj, cpp_type):
            return wrapper_cls._from_cpp(cpp_obj)
    return cpp_obj


class Reactions(CppWrapper):
    """A collection of reactions in a chemical mechanism.

    Attributes:
        reactions: A list of reactions in the mechanism.
    """

    def __init__(
        self,
        reactions: Optional[List[Any]] = None,
    ):
        """Initialize the Reactions collection.

        Args:
            reactions: A list of reactions in the mechanism.
        """
        unwrapped = _unwrap_list(reactions) if reactions is not None else []
        self._cpp = _Reactions(unwrapped)

    def __iter__(self):
        """Iterate over reactions, yielding wrapped Python objects."""
        for cpp_obj in self._cpp:
            yield _wrap_reaction(cpp_obj)

    def __len__(self):
        return len(self._cpp)

    @property
    def arrhenius(self):
        """List of Arrhenius reactions."""
        from .arrhenius import Arrhenius
        return [Arrhenius._from_cpp(obj) for obj in self._cpp.arrhenius]

    @property
    def branched(self):
        """List of Branched reactions."""
        from .branched import Branched
        return [Branched._from_cpp(obj) for obj in self._cpp.branched]

    @property
    def emission(self):
        """List of Emission reactions."""
        from .emission import Emission
        return [Emission._from_cpp(obj) for obj in self._cpp.emission]

    @property
    def first_order_loss(self):
        """List of FirstOrderLoss reactions."""
        from .first_order_loss import FirstOrderLoss
        return [FirstOrderLoss._from_cpp(obj) for obj in self._cpp.first_order_loss]

    @property
    def photolysis(self):
        """List of Photolysis reactions."""
        from .photolysis import Photolysis
        return [Photolysis._from_cpp(obj) for obj in self._cpp.photolysis]

    @property
    def surface(self):
        """List of Surface reactions."""
        from .surface import Surface
        return [Surface._from_cpp(obj) for obj in self._cpp.surface]

    @property
    def taylor_series(self):
        """List of TaylorSeries reactions."""
        from .taylor_series import TaylorSeries
        return [TaylorSeries._from_cpp(obj) for obj in self._cpp.taylor_series]

    @property
    def ternary_chemical_activation(self):
        """List of TernaryChemicalActivation reactions."""
        from .ternary_chemical_activation import TernaryChemicalActivation
        return [TernaryChemicalActivation._from_cpp(obj) for obj in self._cpp.ternary_chemical_activation]

    @property
    def troe(self):
        """List of Troe reactions."""
        from .troe import Troe
        return [Troe._from_cpp(obj) for obj in self._cpp.troe]

    @property
    def tunneling(self):
        """List of Tunneling reactions."""
        from .tunneling import Tunneling
        return [Tunneling._from_cpp(obj) for obj in self._cpp.tunneling]

    @property
    def user_defined(self):
        """List of UserDefined reactions."""
        from .user_defined import UserDefined
        return [UserDefined._from_cpp(obj) for obj in self._cpp.user_defined]
