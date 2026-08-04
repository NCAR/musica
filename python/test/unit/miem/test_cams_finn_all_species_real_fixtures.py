# Copyright (C) 2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0
#
# End-to-end proof that every real CAMS and FINN MVP variable flows through
# MechanismConfiguration -> MUSICA -> MIEM via
# configs/miem/cams_finn_all_species_emissions_config.yaml.
import netCDF4
import numpy as np
import pytest

from musica import backend
from musica.mechanism_configuration import (
    EmissionsConfig,
    Regridding,
    RegriddingType,
    parse,
)
from musica.mechanism_configuration.emissions.emissions import SpeciesMap, SpeciesMapping
from musica.miem import Emissions
from musica.utils import find_config_path

from musica.examples.miem_cams_finn_shared import (
    ALL_SPECIES,
    EMISSIONS_CONFIG_NAME,
    GAS_PHASE_SPECIES,
    AEROSOL_SPECIES,
    N_CELLS,
    SIM_EPOCH,
)

pytestmark = pytest.mark.skipif(not backend.miem_available(), reason="MIEM backend is not available")

DT_SECONDS = 3600.0

# Sources grouped by the species they feed, for the category-independence
# check below. Species fed by exactly one source need no cross-check.
_MULTI_SOURCE_NAMES_BY_SPECIES = {
    "MTERP": [
        "cams anth monoterpenes",
        "cams biog alpha pinene",
        "cams biog beta pinene",
        "cams biog other monoterpenes",
        "finn fire",
    ],
    "NH3": ["cams anth ammonia", "finn fire"],
    "CO": ["cams anth carbon monoxide", "cams biog carbon monoxide", "finn fire"],
    "ISOP": ["cams anth isoprene", "cams biog isoprene", "finn fire"],
    "SO2": ["cams anth sulfur dioxide", "finn fire"],
    "BC": ["cams anth black carbon", "finn fire"],
    "OC": ["cams anth organic carbon", "finn fire"],
}


@pytest.fixture
def config_path():
    return find_config_path("miem", EMISSIONS_CONFIG_NAME)


@pytest.fixture
def mechanism(config_path):
    return parse(config_path)


@pytest.fixture
def emissions(config_path, mechanism, monkeypatch):
    # "file pattern" entries are bare filenames -- chdir into the config dir.
    monkeypatch.chdir(config_path.rsplit("/", 1)[0])
    return Emissions(mechanism=mechanism, n_cells=N_CELLS, n_vert_levels=1)


class TestConfigStructure:
    """The parsed config itself has the shape the whole pipeline depends on."""

    def test_fourteen_sources_with_distinct_categories(self, mechanism):
        sources = mechanism.emissions.sources
        assert len(sources) == 14
        categories = [s.category for s in sources]
        assert len(set(categories)) == 14, (
            "every source must have a distinct category, or MIEM's "
            "highest-hierarchy-wins-within-category rule would silently "
            "override one source's flux with another's instead of summing them"
        )

    def test_fourteen_inventories_and_species_maps(self, mechanism):
        assert len(mechanism.emissions.inventories) == 14
        assert len(mechanism.emissions.species_maps) == 14


class TestEmissionsAllSpecies:
    """The combined 14-source module produces real flux for every species."""

    def test_species_ordering(self, emissions):
        assert emissions.num_species == len(ALL_SPECIES)
        assert set(emissions.species_names) == set(ALL_SPECIES)

    def test_flux_is_real_nonnegative_and_nonzero_somewhere(self, emissions):
        names = list(emissions.species_names)
        flux = emissions.run(SIM_EPOCH, DT_SECONDS)
        assert flux.shape == (len(ALL_SPECIES), N_CELLS)
        assert not np.any(np.isnan(flux))

        for species in ALL_SPECIES:
            species_flux = flux[names.index(species)]
            assert np.all(species_flux >= 0.0), f"{species} flux went negative somewhere"
            assert np.any(species_flux > 0.0), f"{species} flux was zero everywhere"


class TestCategoryIndependence:
    """Sources sharing a species must sum, not override. Build each
    contributing source as its own single-source module, hand-sum their
    flux, and compare to the combined module's total."""

    @pytest.mark.parametrize("species", sorted(_MULTI_SOURCE_NAMES_BY_SPECIES))
    def test_combined_flux_matches_hand_summed_single_source_runs(
        self, species, config_path, mechanism, emissions, monkeypatch
    ):
        combined_names = list(emissions.species_names)
        combined_flux = emissions.run(SIM_EPOCH, DT_SECONDS)[combined_names.index(species)]

        source_names = _MULTI_SOURCE_NAMES_BY_SPECIES[species]
        hand_summed = np.zeros(N_CELLS)
        for source_name in source_names:
            source = next(s for s in mechanism.emissions.sources if s.name == f"{source_name} source")
            inventory = next(i for i in mechanism.emissions.inventories if i.name == source.inventory)
            species_map = next(m for m in mechanism.emissions.species_maps if m.name == source.species_map)

            single_source_config = EmissionsConfig(
                inventories=[inventory],
                species_maps=[species_map],
                regridding=Regridding(type=RegriddingType.None_),
                sources=[source],
            )
            single_mechanism = parse(config_path)
            single_mechanism.emissions = single_source_config
            single_emissions = Emissions(mechanism=single_mechanism, n_cells=N_CELLS, n_vert_levels=1)
            single_flux = single_emissions.run(SIM_EPOCH, DT_SECONDS)
            single_names = list(single_emissions.species_names)
            hand_summed += single_flux[single_names.index(species)]

        np.testing.assert_allclose(
            combined_flux,
            hand_summed,
            err_msg=f"{species}'s combined flux doesn't match its hand-summed sources -- category collision?",
        )


class TestFinnUnitsConversion:
    """Regression test for the FINN units fix in the "finn fire map"
    scaling factors: they're applied as an exact linear multiply to FINN's
    raw values, and the resulting flux is physically plausible."""

    # Same factors as the config's "finn fire map", duplicated (not
    # imported) so an accidental config edit would be caught here too.
    _FINN_SCALING_FACTOR_BY_SPECIES = {
        "SO2": 1.063846e-21,
        "MTERP": 2.262205e-21,
        "NH3": 2.828130e-22,
        "CO": 4.651345e-22,
        "BC": 1.992693e-22,
        "ISOP": 1.131089e-21,
        "OC": 1.992693e-22,
    }

    @pytest.mark.parametrize("species", sorted(_FINN_SCALING_FACTOR_BY_SPECIES))
    def test_scaling_factor_applied_linearly_to_raw_finn_values(self, species, config_path, mechanism, monkeypatch):
        monkeypatch.chdir(config_path.rsplit("/", 1)[0])
        factor = self._FINN_SCALING_FACTOR_BY_SPECIES[species]
        finn_source = next(s for s in mechanism.emissions.sources if s.name == "finn fire source")
        finn_inventory = next(i for i in mechanism.emissions.inventories if i.name == finn_source.inventory)
        finn_map = next(m for m in mechanism.emissions.species_maps if m.name == finn_source.species_map)

        # Same map with every scaling factor forced to 1.0 (FINN's raw value).
        raw_map = SpeciesMap(
            name=finn_map.name,
            mappings=[
                SpeciesMapping(
                    inventory_species=mapping.inventory_species,
                    mechanism_species=mapping.mechanism_species,
                    scaling_factor=1.0,
                )
                for mapping in finn_map.mappings
            ],
        )

        def run_finn_only(species_map):
            single_config = EmissionsConfig(
                inventories=[finn_inventory],
                species_maps=[species_map],
                regridding=Regridding(type=RegriddingType.None_),
                sources=[finn_source],
            )
            single_mechanism = parse(config_path)
            single_mechanism.emissions = single_config
            single_emissions = Emissions(mechanism=single_mechanism, n_cells=N_CELLS, n_vert_levels=1)
            flux = single_emissions.run(SIM_EPOCH, DT_SECONDS)
            names = list(single_emissions.species_names)
            return flux[names.index(species)]

        scaled_flux = run_finn_only(finn_map)
        raw_flux = run_finn_only(raw_map)

        np.testing.assert_allclose(
            scaled_flux,
            raw_flux * factor,
            rtol=1e-6,
            err_msg=f"{species}'s scaled flux doesn't equal raw flux * {factor:.6e}",
        )

    def test_finn_fed_flux_is_physically_plausible(self, emissions):
        names = list(emissions.species_names)
        flux = emissions.run(SIM_EPOCH, DT_SECONDS)
        for species in self._FINN_SCALING_FACTOR_BY_SPECIES:
            max_flux = np.max(flux[names.index(species)])
            assert max_flux < 1e-3, f"{species}'s max flux is {max_flux:.3e} kg m-2 s-1, still too large"


class TestFixtureSanity:
    """Each newly-generated/copied fixture is well-formed independent of the
    config/wiring logic above."""

    @pytest.mark.parametrize(
        "filename,expected_time,mapped_variable",
        [
            ("x1.163842_2024_cams_anth_ammonia_subset.nc", 12, "nh3_anth_sum"),
            ("x1.163842_2024_cams_anth_black-carbon_subset.nc", 12, "bc_anth_sum"),
            ("x1.163842_2024_cams_anth_carbon-monoxide_subset.nc", 12, "co_anth_sum"),
            ("x1.163842_2024_cams_anth_isoprene_subset.nc", 12, "iso_anth_sum"),
            ("x1.163842_2024_cams_anth_monoterpenes_subset.nc", 12, "mnt_anth_sum"),
            ("x1.163842_2024_nox_subset.nc", 12, "nox_anth_sum"),
            ("x1.163842_2024_cams_anth_organic-carbon_subset.nc", 12, "oc_anth_sum"),
            ("x1.163842_2024_cams_anth_sulfur-dioxide_subset.nc", 12, "so2_anth_sum"),
            ("x1.163842_2024_cams_biog_alpha-pinene_subset.nc", 12, "mnta_biog_megan"),
            ("x1.163842_2024_cams_biog_beta-pinene_subset.nc", 12, "mntb_biog_megan"),
            ("x1.163842_2024_cams_biog_carbon-monoxide_subset.nc", 12, "co_biog_megan"),
            ("x1.163842_2024_cams_biog_isoprene_subset.nc", 12, "iso_biog_megan"),
            ("x1.163842_2024_cams_biog_other-monoterpenes_subset.nc", 12, "mnt_biog_megan"),
            ("x1.163842_2024_finn_subset.nc", 48, "co_biob_modis"),
        ],
    )
    def test_fixture_shape_and_real_values(self, filename, expected_time, mapped_variable):
        path = find_config_path("miem", filename)
        ds = netCDF4.Dataset(path)
        try:
            assert len(ds.dimensions["nCells"]) == N_CELLS
            assert ds.dimensions["Time"].size == expected_time
            data = np.ma.filled(ds.variables[mapped_variable][:], np.nan)
            assert np.any(~np.isnan(data) & (data != 0.0)), (
                f"{mapped_variable} in {filename} is all zero/NaN -- subsetting may have dropped real data"
            )
        finally:
            ds.close()
