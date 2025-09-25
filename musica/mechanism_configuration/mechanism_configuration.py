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
from .reactions import Reactions, ReactionType
from .user_defined import UserDefined, _UserDefined
from .first_order_loss import FirstOrderLoss, _FirstOrderLoss
from .emission import Emission, _Emission
from .photolysis import Photolysis, _Photolysis
from .surface import Surface, _Surface
from .tunneling import Tunneling, _Tunneling
from .branched import Branched, _Branched
from .taylor_series import TaylorSeries
from .troe import Troe, _Troe
from .ternary_chemical_activation import TernaryChemicalActivation, _TernaryChemicalActivation
from .arrhenius import Arrhenius, _Arrhenius

from musica.decorators import py_constructor

_backend = backend.get_backend()
_mc = _backend._mechanism_configuration
Version = _mc._Version
Parser = _mc._Parser
Mechanism = _mc._Mechanism


def export(self, file_path: str) -> None:
    MechanismSerializer.serialize(self, file_path)


def init(
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
    self.name = name
    self.species = species if species is not None else []
    self.phases = phases if phases is not None else []
    self.reactions = Reactions(reactions=reactions if reactions is not None else [])
    self.version = version if version is not None else Version()


def to_dict(self) -> Dict:
    species_list = []
    for species in self.species:
        species_list.append(Species.serialize(species))

    phases_list = []
    for phase in self.phases:
        phases_list.append(Phase.serialize(phase))

    reactions_list = []
    for reaction in self.reactions:
        if isinstance(reaction, _Arrhenius):
            # Handle C++ _Arrhenius objects with static serialize call
            reactions_list.append(Arrhenius.serialize_static(reaction))
        elif isinstance(reaction, Arrhenius):
            # Handle Python Arrhenius objects with instance serialize call
            reactions_list.append(reaction.serialize())
        elif isinstance(reaction, _Branched):
            # Handle C++ _Branched objects with static serialize call
            reactions_list.append(Branched.serialize_static(reaction))
        elif isinstance(reaction, Branched):
            # Handle Python Branched objects with instance serialize call
            reactions_list.append(reaction.serialize())
        elif isinstance(reaction, (_Emission, Emission)):
            reactions_list.append(Emission.serialize(reaction))
        elif isinstance(reaction, _FirstOrderLoss):
            # Handle C++ _FirstOrderLoss objects with static serialize call
            reactions_list.append(FirstOrderLoss.serialize_static(reaction))
        elif isinstance(reaction, FirstOrderLoss):
            # Handle Python FirstOrderLoss objects with instance serialize call
            reactions_list.append(reaction.serialize())
        elif isinstance(reaction, (_Photolysis, Photolysis)):
            reactions_list.append(Photolysis.serialize(reaction))
        elif isinstance(reaction, (_Surface, Surface)):
            reactions_list.append(Surface.serialize(reaction))
        elif isinstance(reaction, TaylorSeries):
            reactions_list.append(reaction.serialize())
        elif isinstance(reaction, _Troe):
            # Handle C++ _Troe objects with static serialize call
            reactions_list.append(Troe.serialize_static(reaction))
        elif isinstance(reaction, Troe):
            # Handle Python Troe objects with instance serialize call
            reactions_list.append(reaction.serialize())
        elif isinstance(reaction, _TernaryChemicalActivation):
            # Handle C++ _TernaryChemicalActivation objects with static serialize call
            reactions_list.append(TernaryChemicalActivation.serialize_static(reaction))
        elif isinstance(reaction, TernaryChemicalActivation):
            # Handle Python TernaryChemicalActivation objects with instance serialize call
            reactions_list.append(reaction.serialize())
        elif isinstance(reaction, _Tunneling):
            # Handle C++ _Tunneling objects with static serialize call
            reactions_list.append(Tunneling.serialize_static(reaction))
        elif isinstance(reaction, Tunneling):
            # Handle Python Tunneling objects with instance serialize call
            reactions_list.append(reaction.serialize())
        elif isinstance(reaction, (_UserDefined, UserDefined)):
            reactions_list.append(UserDefined.serialize(reaction))
        else:
            raise TypeError(
                f'Reaction type {type(reaction)} is not supported for export.')

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

Mechanism = py_constructor(init)(Mechanism)
Mechanism.to_dict = to_dict
Mechanism.export = export


class MechanismSerializer():
    """
    A class for exporting a chemical mechanism.
    """

    @staticmethod
    def serialize(mechanism: Mechanism, file_path: str = "./mechanism.json") -> None:
        if not isinstance(mechanism, (Mechanism)):
            raise TypeError(f"Object {mechanism} is not of type Mechanism.")

        directory, file = os.path.split(file_path)
        if directory:
            os.makedirs(directory, exist_ok=True)

        # Now we can use the standard to_dict method
        dictionary = mechanism.to_dict()

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
