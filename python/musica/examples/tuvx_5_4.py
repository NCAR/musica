"""
Python example for TUV-x with the version 5.4 photolysis configuration.

This script sets up a vertical column up to 120 km with 1 km resolution. The
radiation grid, atmospheric composition, and photolysis reaction configuration
mirror those used in the stand-alone TUV-x configuration version 5.4.
"""

import xarray as xr
import musica
from musica.tuvx import v54

available = musica.backend.tuvx_available()


def run_tuvx_5_4_example():
    """Run TUV-x with version 5.4 configuration in a simple vertical column."""

    tuvx = v54.get_tuvx_calculator()

    # Run TUV-x with standard v5.4 conditions for SZA = 0 degrees and average Earth-Sun distance
    standard_results: xr.Dataset = tuvx.run(sza=0.0, earth_sun_distance=1.0)

    # Double the O3 concentrations in the column
    grids = tuvx.get_grid_map()
    heights = grids["height", "km"]
    profiles = tuvx.get_profile_map()
    o3 = profiles["O3", "molecule cm-3"]
    o3.midpoint_values[:] *= 2.0
    o3.edge_values[:] *= 2.0
    o3.calculate_layer_densities(heights)

    # Rerun TUV-x with modified O3 profile
    modified_results: xr.Dataset = tuvx.run(sza=0.0, earth_sun_distance=1.0)

    # Get reaction names to find the correct indices
    reaction_names = list(tuvx.photolysis_rate_names.keys())
    print("Photolysis reactions in TUV-x v5.4 configuration:")
    for name in reaction_names:
        print(f" - {name}")

    # Find the indices for the O3 photolysis reactions
    jo3a_name = "O3+hv->O2+O(1D)"
    jo3b_name = "O3+hv->O2+O(3P)"

    # Plot the vertical profiles of "O3+hv->O2+O(1D)" and "O3+hv->O2+O(3P)" photolysis rates
    jo3a_std = standard_results["photolysis_rate_constants"].sel(reaction=jo3a_name)
    jo3b_std = standard_results["photolysis_rate_constants"].sel(reaction=jo3b_name)
    jo3a_mod = modified_results["photolysis_rate_constants"].sel(reaction=jo3a_name)
    jo3b_mod = modified_results["photolysis_rate_constants"].sel(reaction=jo3b_name)

    import matplotlib.pyplot as plt
    plt.figure(figsize=(8, 6))
    plt.semilogx(jo3a_std.values, standard_results["vertical_edge"].values,
                 label="O3+hv->O2+O(1D) (standard)")
    plt.semilogx(jo3a_mod.values, modified_results["vertical_edge"].values,
                 label="O3+hv->O2+O(1D) (doubled O3 top layer)")
    plt.semilogx(jo3b_std.values, standard_results["vertical_edge"].values,
                 label="O3+hv->O2+O(3P) (standard)")
    plt.semilogx(jo3b_mod.values, modified_results["vertical_edge"].values,
                 label="O3+hv->O2+O(3P) (doubled O3 top layer)")
    plt.xlabel("Photolysis Rate (s$^{-1}$)")
    plt.ylabel("Height (km)")
    plt.title("TUV-x v5.4 Photolysis Rates")
    plt.legend()
    plt.grid(True)
    plt.savefig("tuvx_v54_photolysis_rates.png")

    # Write CSV for cross-language comparison with Fortran example
    height_edges = standard_results["vertical_edge"].values
    csv_path = "tuvx_v54_python_results.csv"
    with open(csv_path, "w") as f:
        f.write("level,height_km,jo3a_std,jo3a_2xO3,jo3b_std,jo3b_2xO3\n")
        for i in range(len(height_edges)):
            f.write(f"{i+1},{height_edges[i]:.1f},"
                    f"{jo3a_std.values[i]:.16E},"
                    f"{jo3a_mod.values[i]:.16E},"
                    f"{jo3b_std.values[i]:.16E},"
                    f"{jo3b_mod.values[i]:.16E}\n")
    print(f"\nResults written to {csv_path}")


if __name__ == "__main__":
    if not available:
        raise RuntimeError("TUV-x backend is not available in this Musica installation.")

    run_tuvx_5_4_example()
