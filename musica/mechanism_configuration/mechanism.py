# Copyright (C) 2025 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0
#
# This file is part of the musica Python package.
# For more information, see the LICENSE file in the top-level directory of this distribution.
import os
import json
import yaml
from typing import Optional, Any, Dict, List
from .. import backend

from .species import Species
from .phase import Phase
from .reactions import Reactions
from .reactions import Reactions
from .user_defined import UserDefined
from .first_order_loss import FirstOrderLoss
from .emission import Emission
from .photolysis import Photolysis
from .surface import Surface
from .tunneling import Tunneling
from .branched import Branched
from .taylor_series import TaylorSeries
from .troe import Troe
from .ternary_chemical_activation import TernaryChemicalActivation
from .arrhenius import Arrhenius

_backend = backend.get_backend()
_mc = _backend._mechanism_configuration
Version = _mc._Version
Parser = _mc._Parser
Mechanism = _mc._Mechanism

original_init = Mechanism.__init__


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


def __init__(
    self,
    name: Optional[str] = None,
    reactions: Optional[List[Any]] = None,
    species: Optional[List[Species]] = None,
    phases: Optional[List[Phase]] = None,
    version: Optional[Version] = None,
):
    """
    Initializes the Mechanism object with the given parameters.

    Args:
        name (str): The name of the mechanism.
        reactions (List[]): A list of reactions in the mechanism.
        species (List[Species]): A list of species in the mechanism.
        phases (List[Phase]): A list of phases in the mechanism.
        version (Version): The version of the mechanism.
    """
    original_init(self)
    self.name = name
    self.species = species if species is not None else []
    self.phases = phases if phases is not None else []
    self.reactions = Reactions(reactions=reactions if reactions is not None else [])
    self.version = version if version is not None else Version()


def serialize(self) -> Dict:
    species_list = []
    for species in self.species:
        species_list.append(Species.serialize(species))

    phases_list = []
    for phase in self.phases:
        phases_list.append(Phase.serialize(phase))

    reactions_list = []
    for reaction in self.reactions:
        reactions_list.append(reaction.serialize())

    return {
        "name": self.name,
        "reactions": reactions_list,
        "species": species_list,
        "phases": phases_list,
        "version": self.version.to_string(),
    }


Mechanism.__doc__ = """
    A class representing a chemical mechanism.

    Attributes:
        name (str): The name of the mechanism.
        reactions (List[Reaction]): A list of reactions in the mechanism.
        species (List[Species]): A list of species in the mechanism.
        phases (List[Phase]): A list of phases in the mechanism.
        version (Version): The version of the mechanism.
    """

Mechanism.__init__ = __init__
Mechanism.serialize = serialize
Mechanism.export = export