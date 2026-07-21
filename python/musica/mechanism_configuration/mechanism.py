# Copyright (C) 2025-2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0
#
# This file is part of the musica Python package.
# For more information, see the LICENSE file in the top-level directory of this distribution.
import os
import json
import yaml
from typing import Optional, Any, Dict, List, Union
from .. import backend
from .._base import CppWrapper, CppField, _unwrap, _unwrap_list, _wrap_list

from .species import Species, Phase
from .reactions import Reactions
from .aerosol import Aerosol
from .emissions import EmissionsConfig
from .parse import Version, ReactionType

_backend = backend.get_backend()
_mc = _backend._mechanism_configuration


class Mechanism(CppWrapper):
    """A chemical mechanism.

    Attributes:
        name: The name of the mechanism.
        reactions: A list of reactions in the mechanism.
        species: A list of species in the mechanism.
        phases: A list of phases in the mechanism.
        relative_tolerance: Relative tolerance for the solver (default: 1e-6).
        version: The version of the mechanism.
    """

    name = CppField()
    relative_tolerance = CppField()

    @classmethod
    def _from_cpp(cls, cpp_obj):
        """Wrap a raw C++ Mechanism object."""
        instance = object.__new__(cls)
        instance._cpp = cpp_obj
        return instance

    def __init__(
        self,
        name: Optional[str] = None,
        version: Optional[Version] = None,
        relative_tolerance: Optional[float] = None,
        species: Optional[List[Species]] = None,
        phases: Optional[List[Phase]] = None,
        reactions: Optional[List[Any]] = None,
        aerosol: Optional[Aerosol] = None,
        emissions: Optional[EmissionsConfig] = None,
    ):
        """Initialize the Mechanism.

        Args:
            name: The name of the mechanism.
            version: The version of the mechanism.
            relative_tolerance: Relative tolerance for the solver. Defaults to the
                configuration schema default (1e-6) when not provided.
            species: A list of species in the mechanism.
            phases: A list of phases in the mechanism.
            reactions: A list of reactions in the mechanism.
            aerosol: Aerosol representations, processes, and constraints (optional).
            emissions: Emissions configuration (inventories, species maps, sources) (optional).
        """
        self._cpp = _mc._Mechanism()
        self.name = name if name is not None else ""
        self.version = version if version is not None else Version(1, 0, 0)
        # Leave the C++ default (1e-6) in place unless the caller overrides it.
        if relative_tolerance is not None:
            self.relative_tolerance = relative_tolerance
        self.species = species if species is not None else []
        self.phases = phases if phases is not None else []
        self.reactions = Reactions(reactions=reactions if reactions is not None else [])
        self.aerosol = aerosol
        self.emissions = emissions

    @property
    def version(self):
        """Version of the mechanism."""
        return self._cpp.version

    @version.setter
    def version(self, value):
        self._cpp.version = value

    @property
    def species(self) -> List[Species]:
        """List of species in the mechanism."""
        return _wrap_list(Species, self._cpp.species)

    @species.setter
    def species(self, value: List[Species]):
        self._cpp.species = _unwrap_list(value)

    @property
    def phases(self) -> List[Phase]:
        """List of phases in the mechanism."""
        return _wrap_list(Phase, self._cpp.phases)

    @phases.setter
    def phases(self, value: List[Phase]):
        self._cpp.phases = _unwrap_list(value)

    @property
    def reactions(self) -> Reactions:
        """Reactions in the mechanism.

        Note: This property wraps the C++ reactions object on each access,
        consistent with other properties like species and phases. This ensures
        that changes to the underlying C++ object are always reflected.
        """
        return Reactions._from_cpp(self._cpp.reactions)

    @reactions.setter
    def reactions(self, value: Union[Reactions, List[Any], None]):
        """Set reactions in the mechanism.

        Args:
            value: Either a Reactions wrapper object, a list/tuple of reaction objects,
                   or None (which will be converted to an empty list).
                   If a list or tuple is provided, it will be wrapped in a Reactions object.
                   List items can be reaction objects of any type (Arrhenius, Troe, etc.).

        Raises:
            TypeError: If value is neither a Reactions object nor a list-like object.
        """
        if isinstance(value, Reactions):
            self._cpp.reactions = _unwrap(value)
        elif isinstance(value, (list, tuple)) or value is None:
            # If given a list, tuple, or None, build Reactions object from it
            # (None is handled by Reactions constructor which converts it to empty list)
            self._cpp.reactions = _unwrap(Reactions(reactions=value))
        else:
            raise TypeError(
                f"reactions must be a Reactions object or a list, not {type(value).__name__}"
            )

    @property
    def aerosol(self) -> Optional[Aerosol]:
        """Aerosol configuration (representations, processes, constraints), or None."""
        cpp_aerosol = self._cpp.aerosol
        return Aerosol._from_cpp(cpp_aerosol) if cpp_aerosol is not None else None

    @aerosol.setter
    def aerosol(self, value: Optional[Aerosol]):
        self._cpp.aerosol = _unwrap(value) if value is not None else None

    @property
    def emissions(self) -> Optional[EmissionsConfig]:
        """Emissions configuration (inventories, species maps, sources), or None."""
        cpp_emissions = self._cpp.emissions
        return EmissionsConfig._from_cpp(cpp_emissions) if cpp_emissions is not None else None

    @emissions.setter
    def emissions(self, value: Optional[EmissionsConfig]):
        self._cpp.emissions = _unwrap(value) if value is not None else None

    def serialize(self) -> Dict:
        serialize_dict = {
            "name": self.name,
            "reactions": [r.serialize() for r in self.reactions],
            "species": [s.serialize() for s in self.species],
            "phases": [p.serialize() for p in self.phases],
            "version": self.version.to_string(),
        }
        if self.aerosol is not None:
            # Aerosol contributes two top-level sections ("aerosol representations"
            # and "aerosol processes"), so its keys are merged in rather than nested.
            serialize_dict.update(self.aerosol.serialize())
        if self.emissions is not None:
            serialize_dict["emissions"] = self.emissions.serialize()
        return serialize_dict

    def export(self, file_path: str) -> None:
        directory, file = os.path.split(file_path)
        if directory:
            os.makedirs(directory, exist_ok=True)

        dictionary = self.serialize()

        _, file_ext = os.path.splitext(file)
        file_ext = file_ext.lower()
        if file_ext in ['.yaml', '.yml']:
            with open(file_path, 'w') as file:
                yaml.dump(dictionary, file)
        elif '.json' == file_ext:
            json_str = json.dumps(dictionary, indent=4)
            with open(file_path, 'w') as file:
                file.write(json_str)
        else:
            raise ValueError(
                'Allowable write formats are .json, .yaml, and .yml')
