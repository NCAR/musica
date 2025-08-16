# Copyright (C) 2025 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0
#
# This file is part of the musica Python package.
# For more information, see the LICENSE file in the top-level directory of this distribution.
from .reactions import Reactions, ReactionType
from .user_defined import UserDefined, _UserDefined
from .simpol_phase_transfer import SimpolPhaseTransfer, _SimpolPhaseTransfer
from .henrys_law import HenrysLaw, _HenrysLaw
from .wet_deposition import WetDeposition, _WetDeposition
from .aqueous_equilibrium import AqueousEquilibrium, _AqueousEquilibrium
from .first_order_loss import FirstOrderLoss, _FirstOrderLoss
from .emission import Emission, _Emission
from .condensed_phase_photolysis import CondensedPhasePhotolysis, _CondensedPhasePhotolysis
from .photolysis import Photolysis, _Photolysis
from .surface import Surface, _Surface
from .tunneling import Tunneling, _Tunneling
from .branched import Branched, _Branched
from .troe import Troe, _Troe
from .ternary_chemical_activation import TernaryChemicalActivation, _TernaryChemicalActivation
from .condensed_phase_arrhenius import CondensedPhaseArrhenius, _CondensedPhaseArrhenius
from .arrhenius import Arrhenius, _Arrhenius
from .phase import Phase
from .species import Species
import os
import json
import yaml
from typing import Optional, Any, Dict, List
from .. import backend

_backend = backend.get_backend()
_mc = _backend._mechanism_configuration
_Mechanism = _mc._Mechanism
_Version = _mc._Version
_Parser = _mc._Parser


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
        self.reactions = Reactions(
            reactions=reactions if reactions is not None else [])
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
            elif isinstance(reaction, (_CondensedPhaseArrhenius, CondensedPhaseArrhenius)):
                reactions_list.append(
                    CondensedPhaseArrhenius.serialize_static(reaction))
            elif isinstance(reaction, (_CondensedPhasePhotolysis, CondensedPhasePhotolysis)):
                reactions_list.append(
                    CondensedPhasePhotolysis.serialize(reaction))
            elif isinstance(reaction, (_Emission, Emission)):
                reactions_list.append(Emission.serialize(reaction))
            elif isinstance(reaction, _FirstOrderLoss):
                # Handle C++ _FirstOrderLoss objects with static serialize call
                reactions_list.append(FirstOrderLoss.serialize_static(reaction))
            elif isinstance(reaction, FirstOrderLoss):
                # Handle Python FirstOrderLoss objects with instance serialize call
                reactions_list.append(reaction.serialize())
            elif isinstance(reaction, _SimpolPhaseTransfer):
                # Handle C++ _SimpolPhaseTransfer objects with static serialize call
                reactions_list.append(
                    SimpolPhaseTransfer.serialize_static(reaction))
            elif isinstance(reaction, SimpolPhaseTransfer):
                # Handle Python SimpolPhaseTransfer objects with instance serialize call
                reactions_list.append(reaction.serialize())
            elif isinstance(reaction, _AqueousEquilibrium):
                # Handle C++ _AqueousEquilibrium objects with static serialize call
                reactions_list.append(
                    AqueousEquilibrium.serialize_static(reaction))
            elif isinstance(reaction, AqueousEquilibrium):
                # Handle Python AqueousEquilibrium objects with instance serialize call
                reactions_list.append(reaction.serialize())
            elif isinstance(reaction, (_WetDeposition, WetDeposition)):
                reactions_list.append(WetDeposition.serialize(reaction))
            elif isinstance(reaction, (_HenrysLaw, HenrysLaw)):
                reactions_list.append(HenrysLaw.serialize(reaction))
            elif isinstance(reaction, (_Photolysis, Photolysis)):
                reactions_list.append(Photolysis.serialize(reaction))
            elif isinstance(reaction, (_Surface, Surface)):
                reactions_list.append(Surface.serialize(reaction))
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
    def _convert_cpp_mechanism_to_python(cpp_mechanism: Any) -> Mechanism:
        """
        Convert a C++ _Mechanism object to a Python Mechanism object.
        """
        reactions_list = []
        for reaction in cpp_mechanism.reactions:
            reactions_list.append(reaction)

        python_mechanism = Mechanism(
            name=cpp_mechanism.name,
            reactions=reactions_list,
            species=list(cpp_mechanism.species),
            phases=list(cpp_mechanism.phases),
            version=Version() if cpp_mechanism.version is None else cpp_mechanism.version
        )

        return python_mechanism

    @staticmethod
    def serialize(mechanism: Mechanism, file_path: str = "./mechanism.json") -> None:
        if not isinstance(mechanism, (Mechanism, _Mechanism)):
            raise TypeError(f"Object {mechanism} is not of type Mechanism.")

        directory, file = os.path.split(file_path)
        if directory:
            os.makedirs(directory, exist_ok=True)

        if isinstance(mechanism, _Mechanism) and not isinstance(mechanism, Mechanism):
            mechanism = MechanismSerializer._convert_cpp_mechanism_to_python(
                mechanism)

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
