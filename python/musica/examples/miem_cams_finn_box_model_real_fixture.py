# Copyright (C) 2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0
#
# Extends miem_nox_box_model_real_fixture.py from a single NOx species pair
# to every MVP variable across CAMS (anthropogenic + biogenic) and FINN
# (fire): 14 real committed fixtures, one combined
# cams_finn_all_species_emissions_config.yaml, one Emissions module.
#
# Differences from miem_nox_box_model_real_fixture.py:
#  - n_cells=4097 (same real x1.163842 mesh subset as the NOx fixture).
#  - CELL_INDEX=655 (see miem_cams_finn_shared.py): the cell with the most
#    species (8 of 8) carrying nonzero flux at SIM_EPOCH, tie-broken by
#    summed flux magnitude -- not chosen to avoid the FINN units issue below.
#  - 6 gas-phase species (NH3, CO, ISOP, MTERP, NO2, SO2) already exist in
#    ts1.json and get an `Emission` reaction attached in Python, exactly like
#    NO/NO2 in the NOx demo -- get a concentration+flux panel each.
#  - 2 aerosol species (BC, OC) have no gas-phase representation in ts1.json
#    (real aerosol mass, no MICM consumer in this pipeline) -- flux-only
#    panel, no Emission reaction, no EMIS.* rate parameter.
#  - KNOWN LIMITATION: the committed FINN fixture's flux values are ~1e10-1e15
#    in magnitude, ~20 orders of magnitude too large to be the kg m-2 s-1
#    the fixture is documented as (CAMS analogues top out ~1e-7); the
#    magnitudes look consistent with molecules cm-2 s-1 instead, suggesting a
#    units mislabel upstream in the FINN source/fixture. Species partly fed by
#    FINN (CO, ISOP, MTERP, NH3, SO2, BC, OC) may show extreme flux/
#    concentration behavior or MICM solver non-convergence as a direct,
#    expected consequence -- wired in as-is per explicit choice, not patched
#    around here.
import json
import os
from datetime import datetime, time, timedelta
from zoneinfo import ZoneInfo

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import ussa1976
import xarray as xr

import musica
from musica.mechanism_configuration import Emission, parse
from musica.micm.solver_result import SolverState
from musica.miem import Emissions
from musica.utils import find_config_path

from musica.examples.miem_cams_finn_shared import (
    AEROSOL_SPECIES,
    EMISSIONS_CONFIG_NAME,
    GAS_PHASE_SPECIES,
    CELL_INDEX,
    MTERP_MOLECULAR_WEIGHT_OVERRIDE,
    N_CELLS,
)

SECONDS_PER_HOUR = 3600

boulder = (40.01879858223568, -105.27492413846649)
boulder_tz = ZoneInfo("America/Denver")

BOX_HEIGHT_M = 100.0


def _get_emissions():
    """Build a musica.miem.Emissions module from the combined 14-source
    CAMS+FINN config -- one inventory/species map/source per real fixture,
    each with its own category so anthropogenic/biogenic/fire flux for a
    shared species (e.g. MTERP, fed by 5 sources) sums instead of one
    overriding another.

    Returns (emissions, config_dir): "file pattern" entries in the config are
    bare filenames (directory: ""), so every emissions.run() call (which
    opens the referenced files lazily, not at construction time) needs the
    CWD to be config_dir for as long as .run() is called -- same convention
    the Fortran examples use with a CMake-fixed working directory. The caller
    is responsible for chdir'ing into config_dir around the whole simulation
    loop, not just around this constructor call.
    """
    config_path = find_config_path("miem", EMISSIONS_CONFIG_NAME)
    mechanism = parse(config_path)
    emissions = Emissions(mechanism=mechanism, n_cells=N_CELLS, n_vert_levels=1)
    return emissions, os.path.dirname(config_path)


def get_tuv_rates(utc_time, grid_cell_index):
    """Calculate photolysis rate constants from TUV-x for a single grid cell."""
    from musica.tuvx import vTS1
    import pvlib

    lat, lon = boulder
    solpos = pvlib.solarposition.get_solarposition(time=utc_time, latitude=lat, longitude=lon)
    sza = solpos['zenith'].item()

    tuvx = vTS1.get_tuvx_calculator()
    tuv_rates = tuvx.run(sza=np.deg2rad(sza), earth_sun_distance=1.0)

    tuv_path = find_config_path("tuvx", "ts1_tsmlt.json")
    with open(tuv_path, 'r') as f:
        data = json.load(f)
    alias_mappings = data.get('__CAM options', {}).get('aliasing', {}).get('pairs', {})

    photolysis_rate_constants = {}
    for mapping in alias_mappings:
        label = mapping['to']
        scale = mapping.get("scale by", 1)
        tuv_label = mapping['from']
        rate = tuv_rates.sel(reaction=tuv_label).photolysis_rate_constants.values * scale
        photolysis_rate_constants[f'PHOTO.{label}'] = [rate[grid_cell_index]]

    return photolysis_rate_constants, tuv_rates


def main(plot=True):
    """Run the single-box TS1 + miem CAMS/FINN emissions simulation against
    all 14 real fixtures.

    Returns:
        xr.Dataset: Simulation results (concentrations for the 6 gas-phase
        species, plus surface flux for all 8 mapped species, over time).
    """
    num_grid_cells = 1
    grid_cell_index = 1  # skip ground-level index 0, matching ts1_box_model.py

    # 2024-11-01 -- mid-range for both CAMS's valid bracket window
    # ([2024-01-01, 2024-12-01]) and the committed FINN fixture's covered
    # window (2024-10-14 through 2024-11-30).
    sim_date = datetime(2024, 11, 1, tzinfo=boulder_tz).date()
    morning_local = datetime.combine(sim_date, time(9, 30), tzinfo=boulder_tz)
    sim_time = (morning_local - timedelta(hours=1)).astimezone(ZoneInfo("UTC"))

    mechanism = parse(find_config_path("v1", "ts1", "ts1.json"))
    gas_phase = next(p for p in mechanism.phases if p.name == "gas")

    molecular_weights = {}
    for name in GAS_PHASE_SPECIES:
        species = next(s for s in mechanism.species if s.name == name)
        # ts1.json's own entry for MTERP is a placeholder 0.0 (true for every
        # lumped-monoterpene species in this repo's mechanisms) -- use the
        # standard alpha-pinene-equivalent surrogate weight instead.
        molecular_weights[name] = (
            MTERP_MOLECULAR_WEIGHT_OVERRIDE if name == "MTERP" else species.molecular_weight_kg_mol
        )

    gas_phase_emissions = [
        Emission(name=name, products=[next(s for s in mechanism.species if s.name == name)], gas_phase=gas_phase)
        for name in GAS_PHASE_SPECIES
    ]
    mechanism.reactions = list(mechanism.reactions) + gas_phase_emissions

    solver = musica.MICM(mechanism=mechanism, solver_type=musica.SolverType.rosenbrock_standard_order)
    state = solver.create_state(num_grid_cells)

    photolysis_rate_constants, tuv_rates = get_tuv_rates(sim_time, grid_cell_index)
    vertical_edge = tuv_rates.vertical_edge[grid_cell_index].item()

    conditions = pd.read_csv(
        find_config_path("v1", "ts1", "initial_conditions.csv"),
        sep=',', names=['parameter', 'value1', 'value2'],
        dtype={'parameter': str, 'value1': float, 'value2': float})

    initial_concentrations = conditions[conditions['parameter'].str.contains('CONC')]
    initial_concentrations = initial_concentrations.copy()
    initial_concentrations['parameter'] = initial_concentrations['parameter'].str.replace('CONC.', '', regex=False)

    surface_reactions = conditions[conditions['parameter'].str.contains('SURF')]
    user_defined_conditions = conditions[conditions['parameter'].str.contains('USER')]

    concentration_dict = {row['parameter']: [row['value1']] for _, row in initial_concentrations.iterrows()}

    user_defined_dict = {row['parameter']: [row['value1']] for _, row in user_defined_conditions.iterrows()}
    for _, row in surface_reactions.iterrows():
        user_defined_dict[f"{row['parameter']}.effective radius [m]"] = [row['value1']]
        user_defined_dict[f"{row['parameter']}.particle number concentration [# m-3]"] = [row['value2']]
    user_defined_dict.update(photolysis_rate_constants)

    environmental_conditions = ussa1976.compute(z=np.array([vertical_edge * 1000]), variables=["t", "p"])
    temperature = environmental_conditions['t'].values
    pressure = environmental_conditions['p'].values

    state.set_conditions(temperature, pressure)
    state.set_concentrations(concentration_dict)
    state.set_user_defined_rate_parameters(user_defined_dict)

    emissions, emissions_config_dir = _get_emissions()
    species_names = list(emissions.species_names)  # 8 species: 6 gas-phase + BC + OC

    sim_times = [sim_time]
    concentrations = [state.get_concentrations()]
    flux_history = {name: [0.0] for name in species_names}
    time_step = 30  # seconds
    simulation_length = 3 * SECONDS_PER_HOUR
    current_time = 0
    last_printed_percent = -5

    # emissions.run() opens the config's referenced fixtures by bare filename
    # each call -- CWD must be emissions_config_dir for the whole loop, not
    # just around building `emissions` above.
    original_cwd = os.getcwd()
    os.chdir(emissions_config_dir)
    try:
        while current_time < simulation_length:
            flux = emissions.run(sim_time.timestamp(), time_step)

            for name in GAS_PHASE_SPECIES:
                surface_flux = flux[species_names.index(name), CELL_INDEX]  # kg m-2 s-1
                emis_rate = surface_flux / (BOX_HEIGHT_M * molecular_weights[name])  # mol m-3 s-1
                user_defined_dict[f"EMIS.{name}"] = [emis_rate]
                flux_history[name].append(surface_flux)
            for name in AEROSOL_SPECIES:
                flux_history[name].append(flux[species_names.index(name), CELL_INDEX])
            state.set_user_defined_rate_parameters(user_defined_dict)

            elapsed = 0
            while elapsed < time_step:
                remaining_time = time_step - elapsed
                result = solver.solve(state, remaining_time)
                elapsed += result.stats.final_time
                current_time += result.stats.final_time
                if result.state != SolverState.Converged:
                    print(f"Solver state: {result.state}, time: {current_time}")

            current_percent = (current_time / simulation_length) * 100
            if int(current_percent // 5) * 5 > last_printed_percent:
                last_printed_percent = int(current_percent // 5) * 5
                print(f"Simulation progress: {last_printed_percent}%")

            sim_time += timedelta(seconds=time_step)
            sim_times.append(sim_time)
            concentrations.append(state.get_concentrations())
    finally:
        os.chdir(original_cwd)

    data_vars = {}
    species_ordering = state.get_species_ordering()
    for species in species_ordering:
        data_vars[species] = (["time"], [c[species][0] for c in concentrations])
    for name in species_names:
        data_vars[f"{name.lower()}_surface_flux"] = (["time"], flux_history[name], {"units": "kg m-2 s-1"})

    coords = {"time": np.array([int(t.timestamp()) for t in sim_times], dtype="datetime64[s]")}
    ds = xr.Dataset(data_vars, coords=coords)

    print()
    print("Concentration change over the run (initial -> final), gas-phase species:")
    for name in GAS_PHASE_SPECIES:
        print(f"  {name}: {ds[name].values[0]:.4e} -> {ds[name].values[-1]:.4e} mol m-3")
    print("Flux at the demo cell (max over the run), all mapped species:")
    for name in species_names:
        print(f"  {name}: {max(flux_history[name]):.4e} kg m-2 s-1 (as documented; see module docstring)")

    if plot:
        ds.to_netcdf("miem_cams_finn_box_model_real_fixture.nc", engine="scipy")

        fig, axes = plt.subplots(4, 2, figsize=(14, 16))
        time_hours = (ds['time'] - ds['time'].isel(time=0)) / np.timedelta64(1, 'h')

        gas_axes = axes.flat[:6]
        for ax, name in zip(gas_axes, GAS_PHASE_SPECIES):
            ax.plot(time_hours, ds[name], color="tab:blue")
            ax.set_ylabel(f"{name} concentration [mol m-3]", color="tab:blue")
            ax.tick_params(axis="y", labelcolor="tab:blue")

            flux_ax = ax.twinx()
            flux_ax.plot(time_hours, ds[f"{name.lower()}_surface_flux"], color="tab:red", linestyle="--")
            flux_ax.set_ylabel(f"{name} flux [kg m-2 s-1]", color="tab:red")
            flux_ax.tick_params(axis="y", labelcolor="tab:red")

            ax.set_title(name)

        aerosol_ax = axes.flat[6]
        for name in AEROSOL_SPECIES:
            aerosol_ax.plot(time_hours, ds[f"{name.lower()}_surface_flux"], label=name)
        aerosol_ax.set_title("Aerosol species (flux-only, no MICM chemistry)")
        aerosol_ax.set_ylabel("Flux [kg m-2 s-1]")
        aerosol_ax.legend()

        note_ax = axes.flat[7]
        note_ax.axis("off")
        note_ax.text(
            0.0, 1.0,
            "Known limitation: FINN fixture flux magnitudes are ~1e10-1e15,\n"
            "~20 orders of magnitude too large for the documented kg m-2 s-1\n"
            "unit (consistent instead with molecules cm-2 s-1) -- a suspected\n"
            "upstream units mislabel, not fixed here. See module docstring.",
            transform=note_ax.transAxes, va="top", ha="left", fontsize=9, wrap=True,
        )

        for _ax in list(gas_axes) + [aerosol_ax]:
            _ax.grid(True, alpha=0.5)
            _ax.set_xlim(0, simulation_length / SECONDS_PER_HOUR)
            _ax.set_xlabel('Time [hours]')

        fig.suptitle(f"CAMS + FINN MVP box model (real fixtures, cell {CELL_INDEX})")
        fig.tight_layout()
        fig.savefig('miem_cams_finn_box_model_real_fixture.png', dpi=300)

    return ds


if __name__ == "__main__":
    main()
