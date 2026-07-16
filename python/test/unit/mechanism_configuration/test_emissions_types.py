# Copyright (C) 2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0
#
# Unit tests for the emissions configuration types in musica.mechanism_configuration
# (SpeciesMapping, SpeciesMap, Inventory, SourceDescriptor, Regridding, EmissionsConfig)
# — both in-code construction and reading a mechanism config file.

import musica.mechanism_configuration as mc
from musica.mechanism_configuration import parse
from musica.utils import find_config_path


# ═══ In-code construction ═════════════════════════════════════════════════


class TestSpeciesMapping:
    def test_construction(self):
        m = mc.SpeciesMapping(inventory_species="NOx", mechanism_species="NO", scaling_factor=0.9)
        assert m.inventory_species == "NOx"
        assert m.mechanism_species == "NO"
        assert m.scaling_factor == 0.9

    def test_default_scaling_factor(self):
        m = mc.SpeciesMapping(inventory_species="CO", mechanism_species="CO")
        assert m.scaling_factor == 1.0


class TestSpeciesMap:
    def test_construction(self):
        mappings = [
            mc.SpeciesMapping(inventory_species="NOx", mechanism_species="NO", scaling_factor=0.9),
            mc.SpeciesMapping(inventory_species="NOx", mechanism_species="NO2", scaling_factor=0.1),
        ]
        species_map = mc.SpeciesMap(name="anthro map", mappings=mappings)
        assert species_map.name == "anthro map"
        assert len(species_map.mappings) == 2
        assert species_map.mappings[0].mechanism_species == "NO"
        assert species_map.mappings[1].mechanism_species == "NO2"


class TestInventory:
    def test_construction(self):
        inventory = mc.Inventory(
            name="cams anthro",
            directory="cams/v6.2",
            file_pattern="CAMS-GLOB-ANT_v6.2_{YYYY}-{MM}.nc",
            convention="eccad",
        )
        assert inventory.name == "cams anthro"
        assert inventory.directory == "cams/v6.2"
        assert inventory.file_pattern == "CAMS-GLOB-ANT_v6.2_{YYYY}-{MM}.nc"
        assert inventory.convention == "eccad"


class TestSourceDescriptor:
    def test_construction(self):
        source = mc.SourceDescriptor(
            name="cams anthro source",
            mode=mc.SourceMode.Offline,
            type=mc.SourceType.Anthropogenic,
            inventory="cams anthro",
            species_map="anthro map",
            temporal_interpolation=mc.TemporalInterpolation.Linear,
            vertical_injection=mc.VerticalInjection.Surface,
            category=0,
            hierarchy=1,
            scaling_factor=1.0,
            sector="anthropogenic",
        )
        assert source.name == "cams anthro source"
        assert source.mode == mc.SourceMode.Offline
        assert source.type == mc.SourceType.Anthropogenic
        assert source.inventory == "cams anthro"
        assert source.species_map == "anthro map"
        assert source.temporal_interpolation == mc.TemporalInterpolation.Linear
        assert source.vertical_injection == mc.VerticalInjection.Surface
        assert source.category == 0
        assert source.hierarchy == 1
        assert source.scaling_factor == 1.0
        assert source.sector == "anthropogenic"

    def test_other_properties(self):
        source = mc.SourceDescriptor(name="s", other_properties={"__notes": "test"})
        assert source.other_properties == {"__notes": "test"}


class TestRegridding:
    def test_default(self):
        regridding = mc.Regridding()
        assert regridding.type == mc.RegriddingType.None_

    def test_construction(self):
        regridding = mc.Regridding(type=mc.RegriddingType.None_)
        assert regridding.type == mc.RegriddingType.None_


class TestEmissionsConfig:
    def test_construction(self):
        inventory = mc.Inventory(
            name="cams anthro",
            directory="cams/v6.2",
            file_pattern="CAMS-GLOB-ANT_v6.2_{YYYY}-{MM}.nc",
            convention="eccad",
        )
        species_map = mc.SpeciesMap(
            name="anthro map",
            mappings=[mc.SpeciesMapping(inventory_species="CO", mechanism_species="CO")],
        )
        source = mc.SourceDescriptor(
            name="cams anthro source",
            mode=mc.SourceMode.Offline,
            type=mc.SourceType.Anthropogenic,
            inventory="cams anthro",
            species_map="anthro map",
        )
        emissions = mc.EmissionsConfig(
            inventories=[inventory],
            species_maps=[species_map],
            sources=[source],
        )
        assert len(emissions.inventories) == 1
        assert emissions.inventories[0].name == "cams anthro"
        assert len(emissions.species_maps) == 1
        assert emissions.species_maps[0].name == "anthro map"
        assert len(emissions.sources) == 1
        assert emissions.sources[0].name == "cams anthro source"
        assert emissions.regridding.type == mc.RegriddingType.None_

    def test_mechanism_round_trip(self):
        emissions = mc.EmissionsConfig(
            inventories=[mc.Inventory(name="inv", directory="d", file_pattern="p", convention="eccad")],
        )
        mechanism = mc.Mechanism(name="test", emissions=emissions)
        assert mechanism.emissions is not None
        assert len(mechanism.emissions.inventories) == 1
        assert mechanism.emissions.inventories[0].name == "inv"

    def test_mechanism_defaults_to_no_emissions(self):
        mechanism = mc.Mechanism(name="test")
        assert mechanism.emissions is None


# ═══ Reading a mechanism config file ══════════════════════════════════════


def test_parsed_emissions_configuration():
    for extension in (".yaml", ".json"):
        path = find_config_path("v1", "emissions", f"config{extension}")
        mechanism = parse(path)
        emissions = mechanism.emissions
        assert emissions is not None

        assert len(emissions.inventories) == 2
        inventories_by_name = {i.name: i for i in emissions.inventories}
        assert inventories_by_name["cams anthro"].convention == "eccad"
        assert inventories_by_name["cams anthro"].directory == "cams/v6.2"
        assert inventories_by_name["gfed fire"].file_pattern == "GFED4_{YYYY}{MM}.nc"

        assert len(emissions.species_maps) == 2
        species_maps_by_name = {m.name: m for m in emissions.species_maps}
        anthro_map = species_maps_by_name["anthro map"]
        assert len(anthro_map.mappings) == 3
        nox_mappings = [m for m in anthro_map.mappings if m.inventory_species == "NOx"]
        assert len(nox_mappings) == 2
        no_mapping = next(m for m in nox_mappings if m.mechanism_species == "NO")
        no2_mapping = next(m for m in nox_mappings if m.mechanism_species == "NO2")
        assert no_mapping.scaling_factor == 0.9
        assert no2_mapping.scaling_factor == 0.1

        assert emissions.regridding.type == mc.RegriddingType.None_

        assert len(emissions.sources) == 2
        sources_by_name = {s.name: s for s in emissions.sources}
        anthro_source = sources_by_name["cams anthro source"]
        assert anthro_source.mode == mc.SourceMode.Offline
        assert anthro_source.type == mc.SourceType.Anthropogenic
        assert anthro_source.inventory == "cams anthro"
        assert anthro_source.species_map == "anthro map"
        assert anthro_source.temporal_interpolation == mc.TemporalInterpolation.Linear
        assert anthro_source.vertical_injection == mc.VerticalInjection.Surface
        assert anthro_source.category == 0
        assert anthro_source.hierarchy == 1
        assert anthro_source.sector == "anthropogenic"

        fire_source = sources_by_name["gfed fire source"]
        assert fire_source.type == mc.SourceType.Fire
        assert fire_source.temporal_interpolation == mc.TemporalInterpolation.Nearest
        assert fire_source.category == 1
        assert fire_source.sector == "fire"
