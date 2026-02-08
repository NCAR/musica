"""
Coupled TUV-x + MICM 1D Column Model

Runs Chapman stratospheric O3 photochemistry in a 120-cell vertical column
(0-120 km, 1 km resolution) with diurnal photolysis rate updates from
TUV-x v5.4.

The model simulates a 24-hour period at Boulder, CO on the summer solstice.
Each grid cell is an independent box (no transport). TUV-x computes
photolysis rate constants as a function of solar zenith angle, and MICM
integrates the Chapman mechanism chemistry.

Species: O2, O, O1D, O3, N2, M (third body)
Photolysis: O2+hv->2O, O3+hv->O+O2, O3+hv->O1D+O2
Thermal: O+O2+M->O3+M, O+O3->2O2, O1D+N2->O+N2, O1D+O2->O+O2

Usage:
    python column_model.py [--hours 24] [--dt-photo 15] [--output column_model]
"""

import argparse
import numpy as np
import xarray as xr
import matplotlib.pyplot as plt

import musica
from musica.tuvx import v54
from musica.mechanism_configuration import Parser
from musica.micm.solver_result import SolverState
from musica.utils import find_config_path

# Physical constants
AVOGADRO = 6.02214076e23
BOLTZMANN = 1.380649e-23  # J/K
DEG2RAD = np.pi / 180.0

# Location: Boulder, CO
LATITUDE = 40.015    # degrees N
LONGITUDE = -105.27  # degrees W

# TUV-x v5.4 reaction names -> Chapman v1 MICM photolysis parameter names
PHOTOLYSIS_MAP = {
    "O2+hv->O+O":      "PHOTO.jo2_b",
    "O3+hv->O2+O(3P)": "PHOTO.jo3_a",
    "O3+hv->O2+O(1D)": "PHOTO.jo3_b",
}


def compute_sza(hour_utc, day_of_year, lat_deg, lon_deg):
    """Compute solar zenith angle analytically.

    Uses the Spencer (1971) formula for solar declination.

    Args:
        hour_utc: UTC hour (can exceed 24 for multi-day)
        day_of_year: Day of year (1-365)
        lat_deg: Latitude in degrees (positive north)
        lon_deg: Longitude in degrees (positive east, negative west)

    Returns:
        Solar zenith angle in radians.
    """
    lat = lat_deg * DEG2RAD
    gamma = 2 * np.pi * (day_of_year - 1) / 365.0
    dec = (0.006918
           - 0.399912 * np.cos(gamma) + 0.070257 * np.sin(gamma)
           - 0.006758 * np.cos(2 * gamma) + 0.000907 * np.sin(2 * gamma)
           - 0.002697 * np.cos(3 * gamma) + 0.00148 * np.sin(3 * gamma))
    # Solar hour = UTC + longitude/15 (negative for west)
    solar_hour = hour_utc + lon_deg / 15.0
    hour_angle = (solar_hour - 12.0) * 15.0 * DEG2RAD
    cos_sza = (np.sin(lat) * np.sin(dec)
               + np.cos(lat) * np.cos(dec) * np.cos(hour_angle))
    return np.arccos(np.clip(cos_sza, -1.0, 1.0))


def molec_cm3_to_mol_m3(n):
    """Convert molecule/cm^3 to mol/m^3."""
    return n * 1e6 / AVOGADRO


def get_initial_profiles(tuvx):
    """Extract atmospheric profiles from TUV-x for initial MICM conditions.

    Returns:
        Dict with height_mid, height_edge, temperature, pressure,
        and species concentrations in mol/m^3.
    """
    grids = tuvx.get_grid_map()
    heights = grids["height", "km"]
    profiles = tuvx.get_profile_map()

    air_mid = profiles["air", "molecule cm-3"].midpoint_values
    o3_mid = profiles["O3", "molecule cm-3"].midpoint_values
    o2_mid = profiles["O2", "molecule cm-3"].midpoint_values
    temp_mid = profiles["temperature", "K"].midpoint_values

    # Pressure from ideal gas: P = n * k_B * T (n in molecules/m^3)
    pressure_mid = air_mid * 1e6 * BOLTZMANN * temp_mid

    air_mol = molec_cm3_to_mol_m3(air_mid)
    return {
        "height_mid": np.array(heights.midpoints),
        "height_edge": np.array(heights.edges),
        "temperature": np.array(temp_mid),
        "pressure": np.array(pressure_mid),
        "air_mol_m3": air_mol,
        "O3": molec_cm3_to_mol_m3(o3_mid),
        "O2": molec_cm3_to_mol_m3(o2_mid),
        "N2": air_mol * 0.78084 / (0.78084 + 0.20946),
    }


def update_photolysis_rates(state, tuvx, sza, num_cells):
    """Compute TUV-x photolysis rates and set them on the MICM state.

    For nighttime (SZA >= 90 deg), all rates are set to zero.

    Returns:
        Dict of photolysis rate arrays (120 values each).
    """
    user_params = state.get_user_defined_rate_parameters()

    if sza < np.pi / 2:
        tuv_results = tuvx.run(sza=sza, earth_sun_distance=1.0)
        for tuvx_name, micm_name in PHOTOLYSIS_MAP.items():
            edges = tuv_results["photolysis_rate_constants"].sel(
                reaction=tuvx_name).values
            # Average adjacent edges to get midpoint rates for each cell
            midpoint_rates = 0.5 * (edges[:-1] + edges[1:])
            user_params[micm_name] = list(midpoint_rates[:num_cells])
    else:
        for micm_name in PHOTOLYSIS_MAP.values():
            user_params[micm_name] = [0.0] * num_cells

    state.set_user_defined_rate_parameters(user_params)

    return {name: np.array(user_params[name])
            for name in PHOTOLYSIS_MAP.values()}


def run_simulation(tuvx, solver, state, num_cells, profiles,
                   sim_hours=24, dt_photo_min=15, day_of_year=172):
    """Run the coupled column model simulation.

    Args:
        tuvx: Configured TUV-x calculator.
        solver: MICM solver.
        state: MICM state (already initialized).
        num_cells: Number of vertical grid cells.
        profiles: Dict of initial atmospheric profiles.
        sim_hours: Simulation length in hours.
        dt_photo_min: Photolysis update interval in minutes.
        day_of_year: Day of year for SZA calculation.

    Returns:
        xarray.Dataset with time-varying concentrations and rates.
    """
    dt_photo = dt_photo_min * 60  # seconds
    sim_length = sim_hours * 3600  # seconds
    n_steps = int(sim_length / dt_photo)

    # Start at midnight local (MDT = UTC-6 in summer)
    # Boulder midnight MDT = 06:00 UTC
    start_utc = 6.0

    # Storage
    local_times = [0.0]
    species_history = {"O3": [], "O": [], "O1D": [], "O2": [], "N2": []}
    rate_history = {name: [] for name in PHOTOLYSIS_MAP.values()}

    # Record initial state
    conc = state.get_concentrations()
    for sp in species_history:
        species_history[sp].append(np.array(conc[sp]))
    for name in PHOTOLYSIS_MAP.values():
        rate_history[name].append(np.zeros(num_cells))

    print(f"Running {sim_hours}-hour simulation "
          f"({n_steps} steps of {dt_photo_min} min)...")

    for step in range(n_steps):
        current_sec = step * dt_photo
        hour_utc = start_utc + current_sec / 3600.0
        sza = compute_sza(hour_utc, day_of_year, LATITUDE, LONGITUDE)

        # Update photolysis rates from TUV-x
        rates = update_photolysis_rates(state, tuvx, sza, num_cells)

        # Integrate chemistry
        elapsed = 0.0
        while elapsed < dt_photo:
            result = solver.solve(state, dt_photo - elapsed)
            elapsed += result.stats.final_time
            if result.state != SolverState.Converged:
                local_hr = (hour_utc - start_utc + dt_photo / 3600)
                print(f"  Warning: solver {result.state} at local hour "
                      f"{local_hr:.2f}, height cell range")

        # Record state
        local_hr = (current_sec + dt_photo) / 3600.0
        local_times.append(local_hr)
        conc = state.get_concentrations()
        for sp in species_history:
            species_history[sp].append(np.array(conc[sp]))
        for name in PHOTOLYSIS_MAP.values():
            rate_history[name].append(rates[name])

        # Progress
        pct = (step + 1) / n_steps * 100
        if n_steps >= 10 and (step + 1) % (n_steps // 10) == 0:
            print(f"  {pct:5.1f}%  local time {local_hr:5.1f} hr  "
                  f"SZA = {np.degrees(sza):5.1f} deg")

    # Build xarray Dataset
    height_mid = profiles["height_mid"]
    times = np.array(local_times)

    data_vars = {}
    for sp, hist in species_history.items():
        data_vars[sp] = (["time", "height"], np.array(hist),
                         {"units": "mol m-3"})

    data_vars["jO3a"] = (["time", "height"],
                         np.array(rate_history["PHOTO.jo3_a"]),
                         {"units": "s-1",
                          "long_name": "j(O3->O+O2)"})
    data_vars["jO3b"] = (["time", "height"],
                         np.array(rate_history["PHOTO.jo3_b"]),
                         {"units": "s-1",
                          "long_name": "j(O3->O1D+O2)"})
    data_vars["jO2"] = (["time", "height"],
                        np.array(rate_history["PHOTO.jo2_b"]),
                        {"units": "s-1",
                         "long_name": "j(O2->2O)"})

    data_vars["temperature"] = (["height"], profiles["temperature"],
                                {"units": "K"})
    data_vars["pressure"] = (["height"], profiles["pressure"],
                             {"units": "Pa"})

    ds = xr.Dataset(
        data_vars,
        coords={"time": times, "height": height_mid},
        attrs={
            "description": "Coupled TUV-x + MICM Chapman column model",
            "latitude": LATITUDE,
            "longitude": LONGITUDE,
            "day_of_year": day_of_year,
            "mechanism": "Chapman (O2, O, O1D, O3, N2, M)",
            "time_units": "hours since midnight local (MDT)",
            "dt_photo_min": dt_photo_min,
        },
    )
    return ds


def plot_results(ds, output_base):
    """Create multi-panel figure of column model results."""
    fig, axes = plt.subplots(2, 2, figsize=(14, 10))
    times = ds["time"].values
    heights = ds["height"].values

    # --- Panel 1: O3 vertical profiles at selected local times ---
    ax = axes[0, 0]
    for t_hr, style in [(0, "k--"), (6, "C0-"), (12, "C1-"),
                        (18, "C2-"), (23.75, "k:")]:
        idx = np.argmin(np.abs(times - t_hr))
        label = f"{int(t_hr):02d}:00 local"
        o3_vals = ds["O3"].isel(time=idx).values
        mask = o3_vals > 0
        ax.semilogx(o3_vals[mask], heights[mask], style, label=label)
    ax.set_xlabel("O3 (mol m$^{-3}$)")
    ax.set_ylabel("Height (km)")
    ax.set_title("O3 Vertical Profiles")
    ax.legend(fontsize=8)
    ax.grid(True, alpha=0.3)

    # --- Panel 2: O3 time series at selected altitudes ---
    ax = axes[0, 1]
    for z_km in [15, 25, 35, 50]:
        idx = np.argmin(np.abs(heights - z_km))
        ax.plot(times, ds["O3"][:, idx].values, label=f"{z_km} km")
    ax.set_xlabel("Local Time (hours)")
    ax.set_ylabel("O3 (mol m$^{-3}$)")
    ax.set_title("O3 Diurnal Evolution")
    ax.legend()
    ax.grid(True, alpha=0.3)
    ax.set_xlim(0, times[-1])

    # --- Panel 3: Short-lived species at 30 km ---
    ax = axes[1, 0]
    z_idx = np.argmin(np.abs(heights - 30))
    ax.plot(times, ds["O"][:, z_idx].values, label="O")
    ax.plot(times, ds["O1D"][:, z_idx].values, label="O1D")
    ax.set_xlabel("Local Time (hours)")
    ax.set_ylabel("Concentration (mol m$^{-3}$)")
    ax.set_title(f"Short-lived Species at {heights[z_idx]:.0f} km")
    ax.legend()
    ax.grid(True, alpha=0.3)
    ax.set_xlim(0, times[-1])

    # --- Panel 4: Photolysis rates at 30 km ---
    ax = axes[1, 1]
    ax.plot(times, ds["jO3a"][:, z_idx].values,
            label="j(O3$\\rightarrow$O+O$_2$)")
    ax.plot(times, ds["jO3b"][:, z_idx].values,
            label="j(O3$\\rightarrow$O($^1$D)+O$_2$)")
    ax.set_xlabel("Local Time (hours)")
    ax.set_ylabel("Photolysis Rate (s$^{-1}$)")
    ax.set_title(f"Photolysis Rates at {heights[z_idx]:.0f} km")
    ax.legend()
    ax.grid(True, alpha=0.3)
    ax.set_xlim(0, times[-1])

    fig.suptitle("Coupled TUV-x + MICM Chapman Column Model\n"
                 "Boulder CO, Summer Solstice", fontsize=14)
    fig.tight_layout()
    png_path = f"{output_base}.png"
    fig.savefig(png_path, dpi=150)
    print(f"Plot saved to {png_path}")


def main():
    parser = argparse.ArgumentParser(
        description="Coupled TUV-x + MICM 1D column model")
    parser.add_argument("--hours", type=float, default=24,
                        help="Simulation length in hours (default: 24)")
    parser.add_argument("--dt-photo", type=float, default=15,
                        help="Photolysis update interval in minutes "
                             "(default: 15)")
    parser.add_argument("--day", type=int, default=172,
                        help="Day of year (default: 172 = June 21)")
    parser.add_argument("--output", "-o", default="column_model",
                        help="Output file base name (default: column_model)")
    args = parser.parse_args()

    num_cells = 120

    # --- Set up TUV-x v5.4 ---
    print("Setting up TUV-x v5.4...")
    tuvx = v54.get_tuvx_calculator()
    profiles = get_initial_profiles(tuvx)

    # --- Set up MICM Chapman solver ---
    print("Setting up MICM Chapman solver...")
    mech_parser = Parser()
    mechanism = mech_parser.parse(
        find_config_path("v1", "chapman", "config.json"))
    solver = musica.MICM(
        mechanism=mechanism,
        solver_type=musica.SolverType.rosenbrock_standard_order)
    state = solver.create_state(num_cells)

    # Set environmental conditions
    state.set_conditions(
        temperatures=list(profiles["temperature"]),
        pressures=list(profiles["pressure"]))

    # Set initial species concentrations
    state.set_concentrations({
        "N2": list(profiles["N2"]),
        "O2": list(profiles["O2"]),
        "O":  [0.0] * num_cells,
        "O1D": [0.0] * num_cells,
        "O3": list(profiles["O3"]),
    })

    # Initialize photolysis rates to zero (midnight start)
    user_params = state.get_user_defined_rate_parameters()
    for name in PHOTOLYSIS_MAP.values():
        user_params[name] = [0.0] * num_cells
    state.set_user_defined_rate_parameters(user_params)

    # --- Run simulation ---
    ds = run_simulation(tuvx, solver, state, num_cells, profiles,
                        sim_hours=args.hours, dt_photo_min=args.dt_photo,
                        day_of_year=args.day)

    # --- Save and plot ---
    nc_path = f"{args.output}.nc"
    try:
        ds.to_netcdf(nc_path)
        print(f"Results saved to {nc_path}")
    except Exception as e:
        print(f"Could not save NetCDF ({e}), skipping.")

    plot_results(ds, args.output)


if __name__ == "__main__":
    main()
