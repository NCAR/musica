# Copyright (C) 2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0
#
# Constants shared between miem_cams_finn_box_model_real_fixture.py and
# python/test/unit/miem/test_cams_finn_all_species_real_fixtures.py, so the
# "which species are we claiming to cover" list can't drift between the demo
# and its regression test.
EMISSIONS_CONFIG_NAME = "cams_finn_all_species_emissions_config.yaml"

N_CELLS = 4097

# The cell with the most species (8 of 8) carrying nonzero flux at
# SIM_EPOCH, tie-broken by summed flux magnitude across species -- chosen
# directly from the finished fixtures/config, not guessed. See the module
# docstring in miem_cams_finn_box_model_real_fixture.py for how this was
# computed and a note on why it isn't picked to avoid the FINN units issue
# below.
CELL_INDEX = 655

# 2024-11-01 00:00:00 UTC -- inside both CAMS's valid bracket window
# ([2024-01-01, 2024-12-01], mid-range) and the committed FINN fixture's
# covered window (2024-10-14 through 2024-11-30).
SIM_EPOCH = 1730419200.0

# Gas-phase species: already exist in ts1.json, get a MICM `Emission`
# reaction attached in Python (mirroring miem_nox_box_model_real_fixture.py),
# and get a concentration + flux plot panel. Molecular weights are read from
# ts1.json directly at runtime for every species except MTERP, whose ts1.json
# entry is a placeholder "molecular weight [kg mol-1]": 0.0 (true for every
# mechanism config in this repo, not specific to this demo -- MOZART/CAM-chem
# convention for species that don't need a mass-based dry-deposition/
# scavenging calc). MTERP's molecular weight below (0.13623 kg/mol) is the
# standard alpha-pinene-equivalent value used as the lumped-monoterpene
# surrogate molecule in MOZART-T1/CAM-chem.
GAS_PHASE_SPECIES = ("NH3", "CO", "ISOP", "MTERP", "NO2", "SO2")
MTERP_MOLECULAR_WEIGHT_OVERRIDE = 0.13623  # kg mol-1, alpha-pinene surrogate

# Aerosol species: real mass, no gas-phase representation in ts1.json (no
# EMISSION reaction, no MICM coupling) -- flux-only panel, proving miem
# extracts/interpolates/produces real flux for them even with nothing
# downstream to consume it in this pipeline.
AEROSOL_SPECIES = ("BC", "OC")

ALL_SPECIES = GAS_PHASE_SPECIES + AEROSOL_SPECIES

# KNOWN LIMITATION (not fixed here, by explicit choice): the committed FINN
# fixture's flux values are documented (miem's test/data/README.md) as real
# kg m-2 s-1, matching CAMS, but numerically they're ~1e10-1e15 -- about 20
# orders of magnitude too large for that unit (CAMS analogues top out around
# 1e-7). The magnitudes are consistent with molecules cm-2 s-1 instead (a
# common atmospheric-emissions unit), suggesting a units mislabel upstream in
# the FINN source/fixture, not a bug in this config or code. Species fed
# partly by FINN (CO, ISOP, MTERP, NH3, SO2, BC, OC) may show extreme
# flux/concentration behavior or MICM solver non-convergence at CELL_INDEX as
# a direct consequence -- expected until the units question is resolved
# upstream, not a defect in this demo/test.
