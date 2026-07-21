# Copyright (C) 2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0
#
# Variant of miem_nox_box_model.py that drives the box model from a real
# committed miem fixture (configs/miem/x1.163842_2024_nox_subset.nc,
# unmodified CAMS-GLOB-ANT anthropogenic NOx, 12 monthly snapshots x 4097
# cells) instead of a synthetic single-cell/single-snapshot inventory.
#
# Differences from miem_nox_box_model.py:
#  - n_cells=4097 (must match the real file), one specific cell selected
#    (1602 -- the single highest-emitting cell in the fixture, a stand-in
#    for "busy urban" since the fixture carries no lat/lon).
#  - species map does the same one-to-many split used in
#    emissions_model_end_to_end_nox.cpp: nox_anth_sum -> NO (0.9) + NO2 (0.1),
#    instead of a 1:1 NO-only map.
#  - temporal interpolation is "linear" across the real monthly brackets
#    (rather than "none" against a single static snapshot), so this
#    actually exercises OfflineEmissionSource's bracket/interpolation path.
#  - sim date is fixed to 2024-07-15 (mid-July 2024), the only calendar
#    year this fixture has data for -- LoadBrackets hard-errors on any
#    timestamp outside [2024-01-01, 2024-12-01].
import json
from datetime import datetime, time, timedelta
from zoneinfo import ZoneInfo

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import ussa1976
import xarray as xr

import musica
from musica.mechanism_configuration import (
    Emission,
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
from musica.micm.solver_result import SolverState
from musica.miem import Emissions
from musica.utils import find_config_path

SECONDS_PER_HOUR = 3600
NO_MOLECULAR_WEIGHT = 0.030006  # kg mol-1
NO2_MOLECULAR_WEIGHT = 0.0460055  # kg mol-1

boulder = (40.01879858223568, -105.27492413846649)
boulder_tz = ZoneInfo("America/Denver")

N_CELLS = 4097
CELL_INDEX = 1602  # highest-emitting cell in the fixture across all 12 months

BOX_HEIGHT_M = 100.0

MOLECULAR_WEIGHTS = {"NO": NO_MOLECULAR_WEIGHT, "NO2": NO2_MOLECULAR_WEIGHT}


def _get_emissions():
    """Build a musica.miem.Emissions module against the real NOx fixture,
    splitting nox_anth_sum into NO (0.9) and NO2 (0.1), matching
    emissions_model_end_to_end_nox.cpp's real-data test."""
    fixture_path = find_config_path("miem", "x1.163842_2024_nox_subset.nc")

    emissions_config = EmissionsConfig(
        inventories=[
            Inventory(
                name="nox subset",
                directory="",
                file_pattern=fixture_path,
                convention="uptempo",
            ),
        ],
        species_maps=[
            SpeciesMap(
                name="nox map",
                mappings=[
                    SpeciesMapping(
                        inventory_species="nox_anth_sum", mechanism_species="NO", scaling_factor=0.9
                    ),
                    SpeciesMapping(
                        inventory_species="nox_anth_sum", mechanism_species="NO2", scaling_factor=0.1
                    ),
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
                temporal_interpolation=TemporalInterpolation.Linear,
                vertical_injection=VerticalInjection.Surface,
                category=0,
                hierarchy=1,
                scaling_factor=1.0,
                sector="anthropogenic",
            ),
        ],
    )
    emissions_mechanism = Mechanism(name="nox emissions", emissions=emissions_config)
    return Emissions(mechanism=emissions_mechanism, n_cells=N_CELLS, n_vert_levels=1)


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
        photolysis_rate_constants[f'USER.{label}'] = [rate[grid_cell_index]]

    return photolysis_rate_constants, tuv_rates


def main(plot=True):
    """Run the single-box TS1 + miem NOx emissions simulation against the real fixture.

    Returns:
        xr.Dataset: Simulation results (concentrations and NO/NO2 surface flux over time).
    """
    num_grid_cells = 1
    grid_cell_index = 1  # skip ground-level index 0, matching chapman.py/ts1_box_model.py

    # Fixed to mid-2024 -- the only calendar year the real fixture covers.
    sim_date = datetime(2024, 7, 15, tzinfo=boulder_tz).date()
    noon_local = datetime.combine(sim_date, time(7, 30), tzinfo=boulder_tz)
    sim_time = (noon_local - timedelta(hours=1)).astimezone(ZoneInfo("UTC"))

    mechanism = parse(find_config_path("v1", "ts1", "ts1.json"))
    gas_phase = next(p for p in mechanism.phases if p.name == "gas")
    nox_emissions = [
        Emission(name=name, products=[next(s for s in mechanism.species if s.name == name)], gas_phase=gas_phase)
        for name in MOLECULAR_WEIGHTS
    ]
    mechanism.reactions = list(mechanism.reactions) + nox_emissions

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

    emissions = _get_emissions()
    species_names = list(emissions.species_names)  # ["NO", "NO2"]
    species_indices = {name: species_names.index(name) for name in species_names}

    sim_times = [sim_time]
    concentrations = [state.get_concentrations()]
    flux_history = {name: [0.0] for name in species_names}
    time_step = 30  # seconds
    simulation_length = 3 * SECONDS_PER_HOUR
    current_time = 0
    last_printed_percent = -5

    while current_time < simulation_length:
        flux = emissions.run(sim_time.timestamp(), time_step)

        for name in species_names:
            surface_flux = flux[species_indices[name], CELL_INDEX]  # kg m-2 s-1
            emis_rate = surface_flux / (BOX_HEIGHT_M * MOLECULAR_WEIGHTS[name])  # mol m-3 s-1
            user_defined_dict[f"EMIS.{name}"] = [emis_rate]
            flux_history[name].append(surface_flux)
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

    data_vars = {}
    species_ordering = state.get_species_ordering()
    for species in species_ordering:
        data_vars[species] = (["time"], [c[species][0] for c in concentrations])
    for name in species_names:
        data_vars[f"{name.lower()}_surface_flux"] = (["time"], flux_history[name], {"units": "kg m-2 s-1"})

    coords = {"time": np.array([int(t.timestamp()) for t in sim_times], dtype="datetime64[s]")}
    ds = xr.Dataset(data_vars, coords=coords)

    if plot:
        ds.to_netcdf("miem_nox_box_model_real_fixture.nc", engine="scipy")

        fig, axes = plt.subplots(2, 2, figsize=(12, 8))
        time_hours = (ds['time'] - ds['time'].isel(time=0)) / np.timedelta64(1, 'h')

        axes[0, 0].plot(time_hours, ds['NO'])
        axes[0, 0].set_title('NO')
        axes[0, 1].plot(time_hours, ds['NO2'])
        axes[0, 1].set_title('NO2')
        axes[1, 0].plot(time_hours, ds['O3'])
        axes[1, 0].set_title('O3')
        axes[1, 1].plot(time_hours, ds['no_surface_flux'], label='NO')
        axes[1, 1].plot(time_hours, ds['no2_surface_flux'], label='NO2')
        axes[1, 1].set_title('NOx surface flux (miem, real fixture, cell %d)' % CELL_INDEX)
        axes[1, 1].set_ylabel('Flux [kg m-2 s-1]')
        axes[1, 1].legend()

        for _ax in axes.flat[:3]:
            _ax.set_ylabel('Concentration [mol m-3]')
        for _ax in axes.flat:
            _ax.grid(True, alpha=0.5)
            _ax.spines[:].set_visible(False)
            _ax.tick_params(width=0)
            _ax.set_xlim(0, simulation_length / SECONDS_PER_HOUR)
            _ax.set_xlabel('Time [hours]')

        fig.tight_layout()
        fig.savefig('miem_nox_box_model_real_fixture.png', dpi=300)

    return ds


if __name__ == "__main__":
    main()
