# Copyright (C) 2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0
#
# Python wrappers for the emissions configuration types defined in
# mechanism_configuration/types/emissions.hpp.
from typing import Dict, List, Optional

from ... import backend
from ..._base import CppWrapper, CppField, _unwrap, _unwrap_list, _wrap_list
from ..utils import _add_other_properties, _remove_empty_keys

_backend = backend.get_backend()
_mc = _backend._mechanism_configuration

SourceMode = _mc._SourceMode
SourceType = _mc._SourceType
TemporalInterpolation = _mc._TemporalInterpolation
VerticalInjection = _mc._VerticalInjection
RegriddingType = _mc._RegriddingType

# Enum -> YAML/JSON string, the reverse of the mechanism_configuration parser's
# string -> enum mapping (src/v1/emissions/parsers.cpp).
_SOURCE_MODE_TO_STR = {
    SourceMode.Offline: "offline",
}
_SOURCE_TYPE_TO_STR = {
    SourceType.Anthropogenic: "anthropogenic",
    SourceType.Fire: "fire",
    SourceType.Biogenic: "biogenic",
    SourceType.Dust: "dust",
    SourceType.SeaSalt: "sea salt",
    SourceType.Lightning: "lightning",
}
_TEMPORAL_INTERPOLATION_TO_STR = {
    TemporalInterpolation.Linear: "linear",
    TemporalInterpolation.Nearest: "nearest",
    TemporalInterpolation.None_: "none",
}
_VERTICAL_INJECTION_TO_STR = {
    VerticalInjection.Surface: "surface",
}
_REGRIDDING_TYPE_TO_STR = {
    RegriddingType.None_: "none",
}


class SpeciesMapping(CppWrapper):
    """A single inventory-species to mechanism-species mapping.

    Attributes:
        inventory_species: Name of the species as it appears in the inventory file.
        mechanism_species: Name of the mechanism species it contributes flux to.
        scaling_factor: Fraction of the inventory species' flux applied to this mapping.
    """

    inventory_species = CppField()
    mechanism_species = CppField()
    scaling_factor = CppField()

    def __init__(
        self,
        inventory_species: Optional[str] = None,
        mechanism_species: Optional[str] = None,
        scaling_factor: Optional[float] = None,
    ):
        self._cpp = _mc._SpeciesMapping()
        self.inventory_species = inventory_species if inventory_species is not None else self.inventory_species
        self.mechanism_species = mechanism_species if mechanism_species is not None else self.mechanism_species
        self.scaling_factor = scaling_factor if scaling_factor is not None else self.scaling_factor

    def serialize(self) -> Dict:
        return _remove_empty_keys(
            {
                "inventory species": self.inventory_species,
                "mechanism species": self.mechanism_species,
                "scaling factor": self.scaling_factor,
            }
        )


class SpeciesMap(CppWrapper):
    """A named collection of inventory-to-mechanism species mappings.

    Attributes:
        name: The name of the species map.
        mappings: The list of SpeciesMapping entries.
    """

    name = CppField()

    def __init__(self, name: Optional[str] = None, mappings: Optional[List[SpeciesMapping]] = None):
        self._cpp = _mc._SpeciesMap()
        self.name = name if name is not None else self.name
        self.mappings = mappings if mappings is not None else []

    @property
    def mappings(self) -> List[SpeciesMapping]:
        return _wrap_list(SpeciesMapping, self._cpp.mappings)

    @mappings.setter
    def mappings(self, value: List[SpeciesMapping]):
        self._cpp.mappings = _unwrap_list(value)

    def serialize(self) -> Dict:
        return _remove_empty_keys(
            {
                "name": self.name,
                "mappings": [m.serialize() for m in self.mappings],
            }
        )


class Inventory(CppWrapper):
    """An emissions inventory file source.

    Attributes:
        name: The name of the inventory, referenced by SourceDescriptor.inventory.
        directory: Directory containing the inventory files.
        file_pattern: File name pattern, with optional {YYYY}{MM}{DD}{HH} tokens.
        convention: The on-disk inventory convention (e.g. "eccad", "uptempo").
    """

    name = CppField()
    directory = CppField()
    file_pattern = CppField()
    convention = CppField()

    def __init__(
        self,
        name: Optional[str] = None,
        directory: Optional[str] = None,
        file_pattern: Optional[str] = None,
        convention: Optional[str] = None,
    ):
        self._cpp = _mc._Inventory()
        self.name = name if name is not None else self.name
        self.directory = directory if directory is not None else self.directory
        self.file_pattern = file_pattern if file_pattern is not None else self.file_pattern
        self.convention = convention if convention is not None else self.convention

    def serialize(self) -> Dict:
        return _remove_empty_keys(
            {
                "name": self.name,
                "directory": self.directory,
                "file pattern": self.file_pattern,
                "convention": self.convention,
            }
        )


class SourceDescriptor(CppWrapper):
    """Description of a single emission source.

    Attributes:
        name: Diagnostic name for the source.
        mode: SourceMode (offline vs online).
        type: SourceType (inventory sector).
        inventory: Name of the referenced Inventory.
        species_map: Name of the referenced SpeciesMap.
        temporal_interpolation: TemporalInterpolation scheme between time slices.
        vertical_injection: VerticalInjection (where emissions are deposited).
        category: HEMCO-style category (sources in different categories are summed).
        hierarchy: HEMCO-style hierarchy (highest wins within a category).
        scaling_factor: Overall scaling factor applied to this source.
        sector: Diagnostic sector name.
        other_properties: A dictionary of other properties of the source.
    """

    name = CppField()
    mode = CppField()
    type = CppField()
    inventory = CppField()
    species_map = CppField()
    temporal_interpolation = CppField()
    vertical_injection = CppField()
    category = CppField()
    hierarchy = CppField()
    scaling_factor = CppField()
    sector = CppField()
    other_properties = CppField()

    def __init__(
        self,
        name: Optional[str] = None,
        mode: Optional[SourceMode] = None,
        type: Optional[SourceType] = None,
        inventory: Optional[str] = None,
        species_map: Optional[str] = None,
        temporal_interpolation: Optional[TemporalInterpolation] = None,
        vertical_injection: Optional[VerticalInjection] = None,
        category: Optional[int] = None,
        hierarchy: Optional[int] = None,
        scaling_factor: Optional[float] = None,
        sector: Optional[str] = None,
        other_properties: Optional[dict] = None,
    ):
        self._cpp = _mc._SourceDescriptor()
        self.name = name if name is not None else self.name
        self.mode = mode if mode is not None else self.mode
        self.type = type if type is not None else self.type
        self.inventory = inventory if inventory is not None else self.inventory
        self.species_map = species_map if species_map is not None else self.species_map
        self.temporal_interpolation = (
            temporal_interpolation if temporal_interpolation is not None else self.temporal_interpolation
        )
        self.vertical_injection = vertical_injection if vertical_injection is not None else self.vertical_injection
        self.category = category if category is not None else self.category
        self.hierarchy = hierarchy if hierarchy is not None else self.hierarchy
        self.scaling_factor = scaling_factor if scaling_factor is not None else self.scaling_factor
        self.sector = sector if sector is not None else self.sector
        self.other_properties = other_properties if other_properties is not None else self.other_properties

    def serialize(self) -> Dict:
        serialize_dict = {
            "name": self.name,
            "mode": _SOURCE_MODE_TO_STR[self.mode],
            "type": _SOURCE_TYPE_TO_STR[self.type],
            "inventory": self.inventory,
            "species map": self.species_map,
            "temporal interpolation": _TEMPORAL_INTERPOLATION_TO_STR[self.temporal_interpolation],
            "vertical injection": _VERTICAL_INJECTION_TO_STR[self.vertical_injection],
            "category": self.category,
            "hierarchy": self.hierarchy,
            "scaling factor": self.scaling_factor,
            "sector": self.sector,
        }
        _add_other_properties(serialize_dict, self.other_properties)
        return _remove_empty_keys(serialize_dict)


class Regridding(CppWrapper):
    """Horizontal regridding specification for a Mechanism's emissions.

    Attributes:
        type: RegriddingType (only RegriddingType.None_ is currently supported).
    """

    type = CppField()

    def __init__(self, type: Optional[RegriddingType] = None):
        self._cpp = _mc._Regridding()
        self.type = type if type is not None else self.type

    def serialize(self) -> Dict:
        return {"type": _REGRIDDING_TYPE_TO_STR[self.type]}


class EmissionsConfig(CppWrapper):
    """Emissions configuration: inventories, species maps, regridding, and sources.

    Attributes:
        inventories: A list of Inventory objects.
        species_maps: A list of SpeciesMap objects.
        regridding: The Regridding specification.
        sources: A list of SourceDescriptor objects.
    """

    def __init__(
        self,
        inventories: Optional[List[Inventory]] = None,
        species_maps: Optional[List[SpeciesMap]] = None,
        regridding: Optional[Regridding] = None,
        sources: Optional[List[SourceDescriptor]] = None,
    ):
        self._cpp = _mc._EmissionsConfig()
        self.inventories = inventories if inventories is not None else []
        self.species_maps = species_maps if species_maps is not None else []
        self.regridding = regridding if regridding is not None else Regridding()
        self.sources = sources if sources is not None else []

    @property
    def inventories(self) -> List[Inventory]:
        return _wrap_list(Inventory, self._cpp.inventories)

    @inventories.setter
    def inventories(self, value: List[Inventory]):
        self._cpp.inventories = _unwrap_list(value)

    @property
    def species_maps(self) -> List[SpeciesMap]:
        return _wrap_list(SpeciesMap, self._cpp.species_maps)

    @species_maps.setter
    def species_maps(self, value: List[SpeciesMap]):
        self._cpp.species_maps = _unwrap_list(value)

    @property
    def regridding(self) -> Regridding:
        return Regridding._from_cpp(self._cpp.regridding)

    @regridding.setter
    def regridding(self, value: Regridding):
        self._cpp.regridding = _unwrap(value)

    @property
    def sources(self) -> List[SourceDescriptor]:
        return _wrap_list(SourceDescriptor, self._cpp.sources)

    @sources.setter
    def sources(self, value: List[SourceDescriptor]):
        self._cpp.sources = _unwrap_list(value)

    def serialize(self) -> Dict:
        return {
            "inventories": [i.serialize() for i in self.inventories],
            "species maps": [m.serialize() for m in self.species_maps],
            "regridding": self.regridding.serialize(),
            "sources": [s.serialize() for s in self.sources],
        }
