# Copyright (C) 2025 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0
#
# This file is part of the musica Python package.
# For more information, see the LICENSE file in the top-level directory of this distribution.
import os
import json
import yaml
from typing import Optional, Any, Dict, List
from musica import _Mechanism, _Version, _Parser
from .species import Species
from .phase import Phase
from .arrhenius import Arrhenius, _Arrhenius
from .condensed_phase_arrhenius import CondensedPhaseArrhenius, _CondensedPhaseArrhenius
from .troe import Troe, _Troe
from .branched import Branched, _Branched
from .tunneling import Tunneling, _Tunneling
from .surface import Surface, _Surface
from .photolysis import Photolysis, _Photolysis
from .condensed_phase_photolysis import CondensedPhasePhotolysis, _CondensedPhasePhotolysis
from .emission import Emission, _Emission
from .first_order_loss import FirstOrderLoss, _FirstOrderLoss
from .aqueous_equilibrium import AqueousEquilibrium, _AqueousEquilibrium
from .wet_deposition import WetDeposition, _WetDeposition
from .henrys_law import HenrysLaw, _HenrysLaw
from .simpol_phase_transfer import SimpolPhaseTransfer, _SimpolPhaseTransfer
from .user_defined import UserDefined, _UserDefined
from .reactions import Reactions, ReactionType


class Version(_Version):
    """
    A class representing the version of the mechanism.
    """


class Mechanism(_Mechanism):
    """
    A class representing a chemical mechanism.

    Attributes:
        name (str): The name of the mechanism.
        reactions (List[Reaction]): A list of reactions in the mechanism.
        species (List[Species]): A list of species in the mechanism.
        phases (List[Phase]): A list of phases in the mechanism.
        version (Version): The version of the mechanism.
    """

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
        super().__init__()
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
            if isinstance(reaction, (_Arrhenius, Arrhenius)):
                reactions_list.append(Arrhenius.serialize(reaction))
            elif isinstance(reaction, (_Branched, Branched)):
                reactions_list.append(Branched.serialize(reaction))
            elif isinstance(reaction, (_CondensedPhaseArrhenius, CondensedPhaseArrhenius)):
                reactions_list.append(CondensedPhaseArrhenius.serialize(reaction))
            elif isinstance(reaction, (_CondensedPhasePhotolysis, CondensedPhasePhotolysis)):
                reactions_list.append(CondensedPhasePhotolysis.serialize(reaction))
            elif isinstance(reaction, (_Emission, Emission)):
                reactions_list.append(Emission.serialize(reaction))
            elif isinstance(reaction, (_FirstOrderLoss, FirstOrderLoss)):
                reactions_list.append(FirstOrderLoss.serialize(reaction))
            elif isinstance(reaction, (_SimpolPhaseTransfer, SimpolPhaseTransfer)):
                reactions_list.append(SimpolPhaseTransfer.serialize(reaction))
            elif isinstance(reaction, (_AqueousEquilibrium, AqueousEquilibrium)):
                reactions_list.append(AqueousEquilibrium.serialize(reaction))
            elif isinstance(reaction, (_WetDeposition, WetDeposition)):
                reactions_list.append(WetDeposition.serialize(reaction))
            elif isinstance(reaction, (_HenrysLaw, HenrysLaw)):
                reactions_list.append(HenrysLaw.serialize(reaction))
            elif isinstance(reaction, (_Photolysis, Photolysis)):
                reactions_list.append(Photolysis.serialize(reaction))
            elif isinstance(reaction, (_Surface, Surface)):
                reactions_list.append(Surface.serialize(reaction))
            elif isinstance(reaction, (_Troe, Troe)):
                reactions_list.append(Troe.serialize(reaction))
            elif isinstance(reaction, (_Tunneling, Tunneling)):
                reactions_list.append(Tunneling.serialize(reaction))
            elif isinstance(reaction, (_UserDefined, UserDefined)):
                reactions_list.append(UserDefined.serialize(reaction))
            else:
                raise TypeError(f'Reaction type {type(reaction)} is not supported for export.')

        return {
            "name": self.name,
            "reactions": reactions_list,
            "species": species_list,
            "phases": phases_list,
            "version": self.version.to_string(),
        }

    def export(self, file_path: str) -> None:
        MechanismSerializer.serialize(self, file_path)


class Parser(_Parser):
    """
    A class for parsing a chemical mechanism.
    """


class MechanismSerializer():
    """
    A class for exporting a chemical mechanism.
    """

    @staticmethod
    def serialize(mechanism: Mechanism, file_path: str = "./mechanism.json") -> None:        
        if not isinstance(mechanism, Mechanism):
            raise TypeError(f"Object {mechanism} is not of type Mechanism.")

        directory, file = os.path.split(file_path)
        if directory:
            os.makedirs(directory, exist_ok=True)
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
            raise Exception('Allowable write formats are .json, .yaml, and .yml')
