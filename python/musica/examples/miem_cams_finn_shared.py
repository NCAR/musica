# Copyright (C) 2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0
#
# Constants shared between miem_cams_finn_box_model_real_fixture.py and its
# regression test, so the species list can't drift between them.
import os

EMISSIONS_CONFIG_NAME = "cams_finn_all_species_emissions_config.yaml"


def resolve_inventory_directories(mechanism, config_dir):
    """Rewrite each emissions inventory's "directory" to an absolute path,
    so Emissions.run() can find the fixtures regardless of the caller's CWD.

    The config's "directory" fields are relative to config_dir (the yaml
    file's own directory), e.g. "" or "data".
    """
    for inventory in mechanism.emissions.inventories:
        inventory.directory = os.path.join(config_dir, inventory.directory)


N_CELLS = 4097

# The cell with the most species carrying nonzero flux at SIM_EPOCH,
# tie-broken by summed flux magnitude.
CELL_INDEX = 655

# 2024-11-01 00:00:00 UTC -- inside both CAMS's and FINN's valid data windows.
SIM_EPOCH = 1730419200.0

# Gas-phase species already in ts1.json; each gets a MICM `Emission`
# reaction and a concentration + flux plot panel. MTERP's ts1.json molecular
# weight is a placeholder 0.0, so it uses the standard alpha-pinene
# surrogate weight instead.
GAS_PHASE_SPECIES = ("NH3", "CO", "ISOP", "MTERP", "NO", "NO2", "SO2")
MTERP_MOLECULAR_WEIGHT_OVERRIDE = 0.13623  # kg mol-1, alpha-pinene surrogate

# Aerosol species: real mass, no gas-phase representation in ts1.json -- get
# real miem flux but no MICM Emission reaction (flux-only panel).
AEROSOL_SPECIES = ("BC", "OC")

ALL_SPECIES = GAS_PHASE_SPECIES + AEROSOL_SPECIES
