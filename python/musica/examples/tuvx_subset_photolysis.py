"""
Example: Running TUV-x TS1/TSMLT with a subset of photolysis reactions.

The TS1/TSMLT configuration includes 50+ photolysis reactions. For large-scale
simulations, computing all of them can be expensive if only a small subset is
needed by the coupled chemistry mechanism. This example shows how to configure
TUV-x to compute only the reactions you need by modifying the JSON configuration
before passing it to TUV-x via the config_string API.
"""

import json
import os
import xarray as xr
import musica
from musica.tuvx import vTS1
from musica.tuvx.tuvx import TUVX
from musica.tuvx.grid_map import GridMap
from musica.tuvx.profile_map import ProfileMap
from musica.tuvx.radiator_map import RadiatorMap

available = musica.backend.tuvx_available()


def get_all_ts1_reaction_names() -> list[str]:
    """Return all photolysis reaction names in the TS1/TSMLT configuration."""
    config_path = vTS1.config_file_path()
    with open(config_path) as f:
        config = json.load(f)
    return [r["name"] for r in config["photolysis"]["reactions"]]


def get_tuvx_with_subset(reaction_names: list[str]) -> TUVX:
    """
    Create a TUV-x TS1/TSMLT instance that computes only the specified reactions.

    Removing unused reactions from the configuration reduces computation time
    because TUV-x skips the cross section and quantum yield calculations for
    each omitted reaction.

    Args:
        reaction_names: Reaction names to include. Use get_all_ts1_reaction_names()
                        to see all available names.

    Returns:
        TUVX instance configured for the specified subset of reactions.

    Raises:
        ValueError: If any requested name is not in the TS1/TSMLT configuration.
    """
    config_path = vTS1.config_file_path()

    with open(config_path) as f:
        config = json.load(f)

    all_reactions = config["photolysis"]["reactions"]
    all_names = {r["name"] for r in all_reactions}

    unknown = set(reaction_names) - all_names
    if unknown:
        raise ValueError(
            f"Unknown reaction names: {sorted(unknown)}. "
            f"Available names: {sorted(all_names)}"
        )

    requested = set(reaction_names)
    config["photolysis"]["reactions"] = [
        r for r in all_reactions if r["name"] in requested
    ]

    # Build the same grids, profiles, and radiators as the full TS1/TSMLT setup
    grids = GridMap()
    grids["height", "km"] = vTS1.height_grid()
    grids["wavelength", "nm"] = vTS1.wavelength_grid()

    height = grids["height", "km"]
    wavelength = grids["wavelength", "nm"]

    profiles = ProfileMap()
    profiles["air", "molecule cm-3"] = vTS1.profile("air", height)
    profiles["O3", "molecule cm-3"] = vTS1.profile("O3", height)
    profiles["O2", "molecule cm-3"] = vTS1.profile("O2", height)
    profiles["temperature", "K"] = vTS1.profile("temperature", height)
    profiles["surface albedo", "none"] = vTS1.profile("surface albedo", wavelength)
    profiles["extraterrestrial flux", "photon cm-2 s-1"] = vTS1.profile(
        "extraterrestrial flux", wavelength)

    radiators = RadiatorMap()
    radiators["aerosol"] = vTS1.radiator("aerosol", height, wavelength)

    # Change to the config directory so relative file paths in the config work
    config_dir = os.path.dirname(os.path.abspath(config_path))
    original_cwd = os.getcwd()
    os.chdir(config_dir)
    try:
        tuvx = TUVX(
            grid_map=grids,
            profile_map=profiles,
            radiator_map=radiators,
            config_string=json.dumps(config)
        )
    finally:
        os.chdir(original_cwd)

    return tuvx


def run_example():
    """Show how to run TUV-x with a subset of the TS1/TSMLT photolysis reactions."""

    all_names = get_all_ts1_reaction_names()
    print(f"TS1/TSMLT defines {len(all_names)} photolysis reactions:")
    for name in all_names:
        print(f"  {name}")

    # Select only the reactions needed for a typical tropospheric mechanism.
    # Halogen and stratospheric-only reactions are omitted.
    subset = [
        "jo2_a",    # O2 + hv -> O + O(1D)
        "jo2_b",    # O2 + hv -> O + O
        "jo3_a",    # O3 + hv -> O2 + O(1D)
        "jo3_b",    # O3 + hv -> O2 + O(3P)
        "jno2",     # NO2 + hv -> NO + O(3P)
        "jno3_a",   # NO3 + hv -> NO2 + O(3P)
        "jno3_b",   # NO3 + hv -> NO + O2
        "jhno3",    # HNO3 + hv -> OH + NO2
        "jh2o2",    # H2O2 + hv -> OH + OH
        "jch2o_a",  # CH2O + hv -> H + HCO
        "jch2o_b",  # CH2O + hv -> H2 + CO
    ]

    print(f"\nCreating TUV-x with {len(subset)} of {len(all_names)} reactions...")
    tuvx = get_tuvx_with_subset(subset)

    results: xr.Dataset = tuvx.run(sza=0.0, earth_sun_distance=1.0)

    print("\nSurface-level photolysis rate constants (altitude = 0 km):")
    for name in subset:
        rate = float(results["photolysis_rate_constants"].sel(reaction=name).values[0])
        print(f"  {name:12s}: {rate:.3e} s⁻¹")


if __name__ == "__main__":
    if not available:
        raise RuntimeError("TUV-x backend is not available in this Musica installation.")

    run_example()
