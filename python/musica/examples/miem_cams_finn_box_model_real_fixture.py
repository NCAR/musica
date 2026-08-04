# Copyright (C) 2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0
#
# Extends miem_nox_box_model_real_fixture.py to every MVP variable across
# CAMS (anthropogenic + biogenic) and FINN (fire): 14 real fixtures, one
# combined cams_finn_all_species_emissions_config.yaml, one Emissions module.
#
# 7 gas-phase species (NH3, CO, ISOP, MTERP, NO, NO2, SO2) get an `Emission`
# reaction and a concentration+flux panel; NOx is split 9:1 NO:NO2, same as
# elsewhere in this repo. BC/OC are aerosol mass with no gas-phase
# representation in ts1.json, so they get a flux-only panel instead.
# FINN's raw fixture is in molecules cm-2 s-1, not kg m-2 s-1 like CAMS; the
# config's species-map scaling factors convert it before miem sums it with
# CAMS.
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
    """Build a musica.miem.Emissions module from the combined CAMS+FINN
    config.

    Returns (emissions, config_dir): the config's "file pattern" entries are
    bare filenames, so the caller must chdir into config_dir for as long as
    emissions.run() is called.
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
        xr.Dataset: Simulation results (concentrations for the 7 gas-phase
        species, plus surface flux for all 9 mapped species, over time).
    """
    num_grid_cells = 1
    grid_cell_index = 1  # skip ground-level index 0, matching ts1_box_model.py

    # 2024-11-01 -- within both CAMS's and FINN's fixture data windows.
    sim_date = datetime(2024, 11, 1, tzinfo=boulder_tz).date()
    morning_local = datetime.combine(sim_date, time(9, 30), tzinfo=boulder_tz)
    sim_time = (morning_local - timedelta(hours=1)).astimezone(ZoneInfo("UTC"))

    mechanism = parse(find_config_path("v1", "ts1", "ts1.json"))
    gas_phase = next(p for p in mechanism.phases if p.name == "gas")

    molecular_weights = {}
    for name in GAS_PHASE_SPECIES:
        species = next(s for s in mechanism.species if s.name == name)
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
    species_names = list(emissions.species_names)  # 9 species: 7 gas-phase + BC + OC

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
        print(f"  {name}: {max(flux_history[name]):.4e} kg m-2 s-1")

    if plot:
        ds.to_netcdf("miem_cams_finn_box_model_real_fixture.nc", engine="scipy")

        fig, axes = plt.subplots(4, 2, figsize=(14, 18))
        time_hours = (ds['time'] - ds['time'].isel(time=0)) / np.timedelta64(1, 'h')

        gas_axes = axes.flat[:7]
        for ax, name in zip(gas_axes, GAS_PHASE_SPECIES):
            ax.plot(time_hours, ds[name], color="tab:blue")
            ax.set_ylabel(f"{name} concentration [mol m-3]", color="tab:blue")
            ax.tick_params(axis="y", labelcolor="tab:blue")

            flux_ax = ax.twinx()
            flux_ax.plot(time_hours, ds[f"{name.lower()}_surface_flux"], color="tab:red", linestyle="--")
            flux_ax.set_ylabel(f"{name} flux [kg m-2 s-1]", color="tab:red")
            flux_ax.tick_params(axis="y", labelcolor="tab:red")

            ax.set_title(name)

        aerosol_ax = axes.flat[7]
        for name in AEROSOL_SPECIES:
            aerosol_ax.plot(time_hours, ds[f"{name.lower()}_surface_flux"], label=name)
        aerosol_ax.set_title("Aerosol species (flux-only, no MICM chemistry)")
        aerosol_ax.set_ylabel("Flux [kg m-2 s-1]")
        aerosol_ax.legend()

        for _ax in list(gas_axes) + [aerosol_ax]:
            _ax.grid(True, alpha=0.5)
            _ax.set_xlim(0, simulation_length / SECONDS_PER_HOUR)
            _ax.set_xlabel('Time [hours]')

        fig.suptitle(f"CAMS + FINN MVP box model (real fixtures, cell {CELL_INDEX})")
        fig.text(
            0.5, 0.005,
            "NOx split 9:1 NO:NO2. FINN flux converted from molecules cm-2 s-1 to kg m-2 s-1 before "
            "summing with CAMS -- see module docstring.",
            ha="center", va="bottom", fontsize=8, wrap=True,
        )
        fig.tight_layout(rect=[0, 0.03, 1, 1])
        fig.savefig('miem_cams_finn_box_model_real_fixture.png', dpi=300)

    return ds


if __name__ == "__main__":
    main()
