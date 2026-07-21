# Copyright (C) 2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0
#
# Tests for musica.miem.Emissions: a file-based configure-and-run path (a
# mechanism YAML is parsed from disk) and an in-code emissions-object
# construction path (the Mechanism/EmissionsConfig are built directly in
# Python). Both need a real NetCDF inventory file at Run() time -- since no
# such fixture is committed anywhere in this repo (miem's own C++ tests rely
# on files only available via CMake FetchContent), a minimal single-snapshot
# "uptempo" convention file is synthesized here with netCDF4: per
# UptempoReader::DetectDimensions/ReadFlux, only an "nCells" dimension and a
# 1D flux variable named after the inventory species are required -- the
# "Time"/"xtime" machinery is entirely optional for a single-snapshot file.

import numpy as np
import pytest

from musica import backend
from musica.miem import Emissions
from musica.mechanism_configuration import (
    EmissionsConfig,
    Inventory,
    Mechanism,
    Regridding,
    RegriddingType,
    SourceDescriptor,
    SourceMode,
    SourceType,
    SpeciesMap,
    SpeciesMapping,
    TemporalInterpolation,
    VerticalInjection,
    parse,
)

pytestmark = pytest.mark.skipif(not backend.miem_available(), reason="MIEM backend is not available")

N_CELLS = 4
EPOCH_2024_07_01 = 1719792000.0  # 2024-07-01 00:00:00 UTC
INVENTORY_SPECIES = "nox_anth_sum"
FLUX_VALUE = 1.0e-9


def _write_uptempo_netcdf(path, inventory_species=INVENTORY_SPECIES, flux_value=FLUX_VALUE, n_cells=N_CELLS):
    """Write a minimal single-snapshot uptempo-convention NetCDF inventory file."""
    netCDF4 = pytest.importorskip("netCDF4")
    ds = netCDF4.Dataset(str(path), "w", format="NETCDF4")
    try:
        ds.createDimension("nCells", n_cells)
        flux = ds.createVariable(inventory_species, "f8", ("nCells",))
        flux[:] = np.full(n_cells, flux_value)
    finally:
        ds.close()


def _assert_nox_split(emissions):
    assert emissions.num_species == 2
    assert set(emissions.species_names) == {"NO", "NO2"}

    flux = emissions.run(EPOCH_2024_07_01, 3600.0)
    assert flux.shape == (2, N_CELLS)

    names = list(emissions.species_names)
    no_flux = flux[names.index("NO")]
    no2_flux = flux[names.index("NO2")]
    assert np.all(no_flux >= 0.0)
    assert np.all(no2_flux >= 0.0)
    assert np.any(no_flux > 0.0)
    # nox_anth_sum is split 0.9 (NO) / 0.1 (NO2) from the same underlying flux.
    np.testing.assert_allclose(no_flux, 9.0 * no2_flux)


class TestEmissionsFileBasedConfigureAndRun:
    """Configure via a parsed mechanism YAML file, then run."""

    def test_run_produces_expected_flux_split(self, tmp_path):
        nc_path = tmp_path / "nox_subset.nc"
        _write_uptempo_netcdf(nc_path)

        yaml_path = tmp_path / "mechanism.yaml"
        yaml_path.write_text(
            "version: 1.0.0\n"
            "species: []\n"
            "phases: []\n"
            "reactions: []\n"
            "emissions:\n"
            "  inventories:\n"
            "    - name: nox subset\n"
            "      directory: \"\"\n"
            f"      file pattern: \"{nc_path.as_posix()}\"\n"
            "      convention: uptempo\n"
            "  species maps:\n"
            "    - name: nox map\n"
            "      mappings:\n"
            "        - inventory species: nox_anth_sum\n"
            "          mechanism species: NO\n"
            "          scaling factor: 0.9\n"
            "        - inventory species: nox_anth_sum\n"
            "          mechanism species: NO2\n"
            "          scaling factor: 0.1\n"
            "  regridding:\n"
            "    type: none\n"
            "  sources:\n"
            "    - name: nox source\n"
            "      mode: offline\n"
            "      type: anthropogenic\n"
            "      inventory: nox subset\n"
            "      species map: nox map\n"
            "      temporal interpolation: none\n"
            "      vertical injection: surface\n"
            "      category: 0\n"
            "      hierarchy: 1\n"
            "      scaling factor: 1.0\n"
            "      sector: anthropogenic\n"
        )

        mechanism = parse(str(yaml_path))
        emissions = Emissions(mechanism=mechanism, n_cells=N_CELLS, n_vert_levels=1)
        _assert_nox_split(emissions)


class TestEmissionsInCodeConstruction:
    """Build the Mechanism/EmissionsConfig directly in Python; no file parse for the config."""

    def test_run_produces_expected_flux_split(self, tmp_path):
        nc_path = tmp_path / "nox_subset.nc"
        _write_uptempo_netcdf(nc_path)

        emissions_config = EmissionsConfig(
            inventories=[
                Inventory(name="nox subset", directory="", file_pattern=str(nc_path), convention="uptempo"),
            ],
            species_maps=[
                SpeciesMap(
                    name="nox map",
                    mappings=[
                        SpeciesMapping(inventory_species="nox_anth_sum", mechanism_species="NO", scaling_factor=0.9),
                        SpeciesMapping(inventory_species="nox_anth_sum", mechanism_species="NO2", scaling_factor=0.1),
                    ],
                ),
            ],
            regridding=Regridding(type=RegriddingType.None_),
            sources=[
                SourceDescriptor(
                    name="nox source",
                    mode=SourceMode.Offline,
                    type=SourceType.Anthropogenic,
                    inventory="nox subset",
                    species_map="nox map",
                    temporal_interpolation=TemporalInterpolation.None_,
                    vertical_injection=VerticalInjection.Surface,
                    category=0,
                    hierarchy=1,
                    scaling_factor=1.0,
                    sector="anthropogenic",
                ),
            ],
        )
        mechanism = Mechanism(name="in-code nox", emissions=emissions_config)

        emissions = Emissions(mechanism=mechanism, n_cells=N_CELLS, n_vert_levels=1)
        _assert_nox_split(emissions)
