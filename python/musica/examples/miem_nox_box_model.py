# Copyright (C) 2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0
#
# A single well-mixed box model demonstrating musica.miem driving a surface
# NO emission flux into TS1's real, already-validated NOx/tropospheric
# chemistry (rather than inventing new NOx reactions on top of the simpler
# Chapman Ox-only mechanism). Structurally mirrors chapman.py/ts1_box_model.py
# (TUV-x photolysis coupling, MICM Rosenbrock solve loop, xarray/matplotlib
# output), but adds one thing those don't have: an emissions source term,
# computed each time step by a musica.miem.Emissions module and converted
# from surface flux [kg m-2 s-1] into a concentration increment [mol m-3]
# for the box before each solve.
import json
import tempfile
from datetime import datetime, time, timedelta
from pathlib import Path
from zoneinfo import ZoneInfo

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import ussa1976
import xarray as xr

import musica
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
from musica.micm.solver_result import SolverState
from musica.miem import Emissions
from musica.utils import find_config_path

SECONDS_PER_HOUR = 3600
NO_MOLECULAR_WEIGHT = 0.030006  # kg mol-1

# The location this example is set up for (matches chapman.py).
boulder = (40.01879858223568, -105.27492413846649)
boulder_tz = ZoneInfo("America/Denver")

# A representative surface NOx flux [kg m-2 s-1] for the synthetic inventory --
# on the order of a moderately busy urban surface source.
NO_SURFACE_FLUX = 2.0e-9

# Height of the well-mixed box used to convert surface flux into a
# concentration tendency for this single grid cell.
BOX_HEIGHT_M = 100.0


def _write_synthetic_no_inventory(path, flux_value=NO_SURFACE_FLUX):
    """Write a minimal single-snapshot uptempo-convention NetCDF NO inventory.

    Only an "nCells" dimension and a 1D flux variable are required for a
    single-snapshot file (see UptempoReader::DetectDimensions/ReadFlux) --
    no "Time"/"xtime" machinery needed.
    """
    import netCDF4

    ds = netCDF4.Dataset(str(path), "w", format="NETCDF4")
    try:
        ds.createDimension("nCells", 1)
        flux = ds.createVariable("no_surface", "f8", ("nCells",))
        flux[:] = np.array([flux_value])
    finally:
        ds.close()


def _get_emissions(nc_path):
    """Build a musica.miem.Emissions module driving NO from the synthetic inventory."""
    emissions_config = EmissionsConfig(
        inventories=[
            Inventory(name="no inventory", directory="", file_pattern=str(nc_path), convention="uptempo"),
        ],
        species_maps=[
            SpeciesMap(
                name="no map",
                mappings=[SpeciesMapping(inventory_species="no_surface", mechanism_species="NO", scaling_factor=1.0)],
            ),
        ],
        regridding=Regridding(type=RegriddingType.None_),
        sources=[
            SourceDescriptor(
                name="no source",
                mode=SourceMode.Offline,
                type=SourceType.Anthropogenic,
                inventory="no inventory",
                species_map="no map",
                temporal_interpolation=TemporalInterpolation.None_,
                vertical_injection=VerticalInjection.Surface,
                category=0,
                hierarchy=1,
                scaling_factor=1.0,
                sector="anthropogenic",
            ),
        ],
    )
    emissions_mechanism = Mechanism(name="no emissions", emissions=emissions_config)
    return Emissions(mechanism=emissions_mechanism, n_cells=1, n_vert_levels=1)


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
    """Run the single-box TS1 + miem NOx emissions simulation.

    Returns:
        xr.Dataset: Simulation results (concentrations and NO surface flux over time).
    """
    num_grid_cells = 1
    grid_cell_index = 1  # skip ground-level index 0, matching chapman.py/ts1_box_model.py

    today_local = datetime.now(boulder_tz).date()
    noon_local = datetime.combine(today_local, time(7, 30), tzinfo=boulder_tz)
    sim_time = (noon_local - timedelta(hours=1)).astimezone(ZoneInfo("UTC"))

    mechanism = parse(find_config_path("v1", "ts1", "ts1.json"))
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

    with tempfile.TemporaryDirectory() as tmp_dir:
        nc_path = Path(tmp_dir) / "no_surface.nc"
        _write_synthetic_no_inventory(nc_path)
        emissions = _get_emissions(nc_path)

        sim_times = [sim_time]
        concentrations = [state.get_concentrations()]
        no_flux_history = [0.0]
        time_step = 30  # seconds
        simulation_length = 3 * SECONDS_PER_HOUR
        current_time = 0
        last_printed_percent = -5

        while current_time < simulation_length:
            flux = emissions.run(sim_time.timestamp(), time_step)
            no_index = list(emissions.species_names).index("NO")
            no_surface_flux = flux[no_index, 0]  # kg m-2 s-1, single cell

            air_density = state.get_conditions()["air_density"][0]  # mol m-3
            delta_no_mol_m3 = no_surface_flux * time_step / (BOX_HEIGHT_M * NO_MOLECULAR_WEIGHT)

            current_concentrations = state.get_concentrations()
            current_concentrations["NO"] = [current_concentrations["NO"][0] + delta_no_mol_m3]
            state.set_concentrations(current_concentrations)

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
            no_flux_history.append(no_surface_flux)

    data_vars = {}
    species_ordering = state.get_species_ordering()
    for species in species_ordering:
        data_vars[species] = (["time"], [c[species][0] for c in concentrations])
    data_vars["no_surface_flux"] = (["time"], no_flux_history, {"units": "kg m-2 s-1"})

    coords = {"time": np.array([t.timestamp() for t in sim_times], dtype="datetime64[s]")}
    ds = xr.Dataset(data_vars, coords=coords)

    if plot:
        ds.to_netcdf("miem_nox_box_model.nc", engine="scipy")

        fig, axes = plt.subplots(2, 2, figsize=(12, 8))
        time_hours = (ds['time'] - ds['time'].isel(time=0)) / np.timedelta64(1, 'h')

        axes[0, 0].plot(time_hours, ds['NO'])
        axes[0, 0].set_title('NO')
        axes[0, 1].plot(time_hours, ds['NO2'])
        axes[0, 1].set_title('NO2')
        axes[1, 0].plot(time_hours, ds['O3'])
        axes[1, 0].set_title('O3')
        axes[1, 1].plot(time_hours, ds['no_surface_flux'])
        axes[1, 1].set_title('NO surface flux (miem)')
        axes[1, 1].set_ylabel('Flux [kg m-2 s-1]')

        for _ax in axes.flat[:3]:
            _ax.set_ylabel('Concentration [mol m-3]')
        for _ax in axes.flat:
            _ax.grid(True, alpha=0.5)
            _ax.spines[:].set_visible(False)
            _ax.tick_params(width=0)
            _ax.set_xlim(0, simulation_length / SECONDS_PER_HOUR)
            _ax.set_xlabel('Time [hours]')

        fig.tight_layout()
        fig.savefig('miem_nox_box_model.png', dpi=300)

    return ds


if __name__ == "__main__":
    main()
