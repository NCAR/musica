# Copyright (C) 2025-2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0
#
# This file is part of the musica Python package.
# For more information, see the LICENSE file in the top-level directory of this distribution.
import os
import json
import yaml
from typing import Optional, Any, Dict, List
from .. import backend
from .._base import CppWrapper, CppField, _unwrap, _unwrap_list, _wrap_list

from .species import Species
from .phase import Phase
from .reactions import Reactions
from .ancillary import Version

_backend = backend.get_backend()
_Mechanism = _backend._mechanism_configuration._Mechanism


class Mechanism(CppWrapper):
    """A chemical mechanism.

    Attributes:
        name: The name of the mechanism.
        reactions: A list of reactions in the mechanism.
        species: A list of species in the mechanism.
        phases: A list of phases in the mechanism.
        version: The version of the mechanism.
    """

    name = CppField()

    @classmethod
    def _from_cpp(cls, cpp_obj):
        """Wrap a raw C++ Mechanism object."""
        instance = object.__new__(cls)
        instance._cpp = cpp_obj
        instance._reactions = Reactions._from_cpp(cpp_obj.reactions)
        return instance

    def __init__(
        self,
        name: Optional[str] = None,
        reactions: Optional[List[Any]] = None,
        species: Optional[List[Species]] = None,
        phases: Optional[List[Phase]] = None,
        version: Optional[Version] = None,
    ):
        """Initialize the Mechanism.

        Args:
            name: The name of the mechanism.
            reactions: A list of reactions in the mechanism.
            species: A list of species in the mechanism.
            phases: A list of phases in the mechanism.
            version: The version of the mechanism.
        """
        self._cpp = _Mechanism()
        self.name = name
        self.species = species if species is not None else []
        self.phases = phases if phases is not None else []
        self.reactions = Reactions(reactions=reactions if reactions is not None else [])
        self.version = version if version is not None else Version()

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
        """Reactions in the mechanism."""
        return self._reactions

    @reactions.setter
    def reactions(self, value: Reactions):
        if isinstance(value, Reactions):
            self._reactions = value
            self._cpp.reactions = _unwrap(value)
        else:
            self._cpp.reactions = value

    @property
    def version(self):
        """Version of the mechanism."""
        return self._cpp.version

    @version.setter
    def version(self, value):
        self._cpp.version = value

    def serialize(self) -> Dict:
        return {
            "name": self.name,
            "reactions": [r.serialize() for r in self.reactions],
            "species": [s.serialize() for s in self.species],
            "phases": [p.serialize() for p in self.phases],
            "version": self.version.to_string(),
        }

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
