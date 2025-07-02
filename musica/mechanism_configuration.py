"""
Mechanism configuration module for MUSICA.

This module provides classes and functions for creating and configuring
chemical mechanisms for use with MICM solvers.
"""

from ._backend_loader import get_backend

# Get the backend and expose mechanism configuration classes
_backend = get_backend()
_mc = _backend._mechanism_configuration

# Re-export all mechanism configuration classes without underscores
ReactionType = _mc._ReactionType
Species = _mc._Species
Phase = _mc._Phase
ReactionComponent = _mc._ReactionComponent
Arrhenius = _mc._Arrhenius
CondensedPhaseArrhenius = _mc._CondensedPhaseArrhenius
Troe = _mc._Troe
Branched = _mc._Branched
Tunneling = _mc._Tunneling
Surface = _mc._Surface
Photolysis = _mc._Photolysis
CondensedPhasePhotolysis = _mc._CondensedPhasePhotolysis
Emission = _mc._Emission
FirstOrderLoss = _mc._FirstOrderLoss
AqueousEquilibrium = _mc._AqueousEquilibrium
WetDeposition = _mc._WetDeposition
HenrysLaw = _mc._HenrysLaw
SimpolPhaseTransfer = _mc._SimpolPhaseTransfer
UserDefined = _mc._UserDefined
Reactions = _mc._Reactions
ReactionsIterator = _mc._ReactionsIterator
Mechanism = _mc._Mechanism
Version = _mc._Version
Parser = _mc._Parser

__all__ = [
    "ReactionType", "Species", "Phase", "ReactionComponent", "Arrhenius",
    "CondensedPhaseArrhenius", "Troe", "Branched", "Tunneling", "Surface",
    "Photolysis", "CondensedPhasePhotolysis", "Emission", "FirstOrderLoss",
    "AqueousEquilibrium", "WetDeposition", "HenrysLaw", "SimpolPhaseTransfer",
    "UserDefined", "Reactions", "ReactionsIterator", "Mechanism", "Version", "Parser"
]
