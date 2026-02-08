"""
Plot results from the coupled TUV-x + MICM column model.

Reads CSV output from the Fortran column model (or optionally compares
with a Python NetCDF output) and produces a multi-panel figure showing
O3 evolution, short-lived species, and photolysis rates.

Usage:
    python plot_column_model.py <csv_file> [--output <png_file>]
    python plot_column_model.py fortran.csv --nc python.nc [--output <png_file>]
"""

import argparse
import csv
import os
import sys

import matplotlib.pyplot as plt
import numpy as np

# Import NCAR style from sibling module
sys.path.insert(0, os.path.dirname(__file__))
import style


def read_csv(path):
    """Read column model CSV output.

    Expected columns: time_hr, height_km, O3, O, O1D, jO3a, jO3b, jO2

    Returns:
        Dict with 'times' (unique sorted), 'heights' (unique sorted),
        and 2D arrays (time x height) for each variable.
    """
    rows = []
    with open(path) as f:
        reader = csv.DictReader(f)
        for row in reader:
            rows.append({k: float(v) for k, v in row.items()})

    times = sorted(set(r["time_hr"] for r in rows))
    heights = sorted(set(r["height_km"] for r in rows))
    nt, nz = len(times), len(heights)

    time_idx = {t: i for i, t in enumerate(times)}
    height_idx = {z: i for i, z in enumerate(heights)}

    fields = ["air_density", "O3", "O", "O1D", "jO3a", "jO3b", "jO2"]
    data = {f: np.zeros((nt, nz)) for f in fields}

    for row in rows:
        ti = time_idx[row["time_hr"]]
        zi = height_idx[row["height_km"]]
        for f in fields:
            data[f][ti, zi] = row[f]

    # Convert concentrations from mol/m³ to ppb
    air = data["air_density"]
    for species in ["O3", "O", "O1D"]:
        mask = air > 0
        data[species][mask] = data[species][mask] / air[mask] * 1.0e9

    return {
        "times": np.array(times),
        "heights": np.array(heights),
        **data,
    }


def read_nc(path):
    """Read column model NetCDF output (from Python version)."""
    import xarray as xr
    ds = xr.open_dataset(path)
    air = ds["air_density"].values if "air_density" in ds else None
    result = {
        "times": ds["time"].values,
        "heights": ds["height"].values,
        "O3": ds["O3"].values.copy(),
        "O": ds["O"].values.copy(),
        "O1D": ds["O1D"].values.copy(),
        "jO3a": ds["jO3a"].values,
        "jO3b": ds["jO3b"].values,
        "jO2": ds["jO2"].values,
    }
    if air is not None:
        mask = air > 0
        for species in ["O3", "O", "O1D"]:
            result[species][mask] = result[species][mask] / air[mask] * 1.0e9
    return result


def plot(data, output_path, label="Fortran", data2=None, label2="Python"):
    """Create multi-panel column model figure."""
    palette = style.get_palette(8)
    compare = data2 is not None

    fig, axes = plt.subplots(2, 2, figsize=(14, 10))
    times = data["times"]
    heights = data["heights"]

    # --- Panel 1: O3 vertical profiles at selected local times ---
    ax = axes[0, 0]
    time_styles = [
        (0,     palette[0], "00:00"),   # ncar_blue
        (6,     palette[1], "06:00"),   # aqua
        (12,    palette[4], "12:00"),   # green
        (18,    palette[2], "18:00"),   # orange
        (24,    palette[5], "24:00"),   # red
    ]
    for t_hr, color, t_label in time_styles:
        idx = np.argmin(np.abs(times - t_hr))
        o3 = data["O3"][idx] / 1.0e3  # ppb -> ppm
        mask = (o3 > 0) & (heights <= 80)
        lbl = f"{t_label} ({label})" if compare else t_label
        ax.semilogx(o3[mask], heights[mask], color=color, label=lbl)
        if compare:
            idx2 = np.argmin(np.abs(data2["times"] - t_hr))
            o3_2 = data2["O3"][idx2] / 1.0e3
            mask2 = (o3_2 > 0) & (data2["heights"] <= 80)
            ax.semilogx(o3_2[mask2], data2["heights"][mask2],
                         color=color, alpha=0.35, linewidth=3)
    ax.set_xlabel(style.species_label("O3") + " (ppm)")
    ax.set_ylabel("Height (km)")
    ax.set_title(style.species_label("O3") + " Vertical Profiles")
    ax.legend(fontsize=style.FONT_SIZES_DEFAULT.legend_small)

    # --- Panel 2: O3 time series at selected altitudes ---
    ax = axes[0, 1]
    alt_colors = [palette[0], palette[1], palette[4], palette[2]]
    for i, z_km in enumerate([15, 25, 35, 50]):
        idx = np.argmin(np.abs(heights - z_km))
        lbl = f"{z_km} km" + (f" ({label})" if compare else "")
        ax.plot(times, data["O3"][:, idx] / 1.0e3, color=alt_colors[i], label=lbl)
        if compare:
            idx2 = np.argmin(np.abs(data2["heights"] - z_km))
            ax.plot(data2["times"], data2["O3"][:, idx2] / 1.0e3,
                    color=alt_colors[i], ls="--", alpha=0.6)
    ax.set_xlabel("Local Time (hours)")
    ax.set_ylabel(style.species_label("O3") + " (ppm)")
    ax.set_title(style.species_label("O3") + " Diurnal Evolution")
    ax.legend()
    ax.set_xlim(0, times[-1])

    # --- Panel 3: Short-lived species at 30 km ---
    ax = axes[1, 0]
    z_idx = np.argmin(np.abs(heights - 30))
    lbl_o   = style.species_label("O")   + (f" ({label})" if compare else "")
    lbl_o1d = "O($^1$D)" + (f" ({label})" if compare else "")
    ax.plot(times, data["O"][:, z_idx], color=palette[0], label=lbl_o)
    ax.plot(times, data["O1D"][:, z_idx], color=palette[2], label=lbl_o1d)
    if compare:
        z_idx2 = np.argmin(np.abs(data2["heights"] - 30))
        ax.plot(data2["times"], data2["O"][:, z_idx2],
                color=palette[0], ls="--", alpha=0.6)
        ax.plot(data2["times"], data2["O1D"][:, z_idx2],
                color=palette[2], ls="--", alpha=0.6)
    ax.set_xlabel("Local Time (hours)")
    ax.set_ylabel("Concentration (ppb)")
    ax.set_title(f"Short-lived Species at {heights[z_idx]:.0f} km")
    ax.legend()
    ax.set_xlim(0, times[-1])

    # --- Panel 4: Photolysis rates at 30 km ---
    ax = axes[1, 1]
    jlabel_a = r"j(" + style.species_label("O3") + r"$\rightarrow$" \
               + style.species_label("O") + "+" + style.species_label("O2") + ")"
    jlabel_b = r"j(" + style.species_label("O3") + r"$\rightarrow$" \
               + "O($^1$D)+" + style.species_label("O2") + ")"
    ax.plot(times, data["jO3a"][:, z_idx], color=palette[0],
            label=jlabel_a + (f" ({label})" if compare else ""))
    ax.plot(times, data["jO3b"][:, z_idx], color=palette[2],
            label=jlabel_b + (f" ({label})" if compare else ""))
    if compare:
        ax.plot(data2["times"], data2["jO3a"][:, z_idx2],
                color=palette[0], ls="--", alpha=0.6)
        ax.plot(data2["times"], data2["jO3b"][:, z_idx2],
                color=palette[2], ls="--", alpha=0.6)
    ax.set_xlabel("Local Time (hours)")
    ax.set_ylabel("Photolysis Rate (s$^{-1}$)")
    ax.set_title(f"Photolysis Rates at {heights[z_idx]:.0f} km")
    ax.legend()
    ax.set_xlim(0, times[-1])

    title = style.format_title(
        "Coupled TUV-x + MICM Chapman Column Model\n"
        "Boulder CO, Summer Solstice")
    if compare:
        title += f"\nSolid: {label}, Dashed: {label2}"
    fig.suptitle(title)
    fig.tight_layout()
    stem = os.path.splitext(output_path)[0]
    fig.savefig(stem + ".png", dpi=300)
    fig.savefig(stem + ".pdf", dpi=300)
    print(f"Saved {stem}.png and {stem}.pdf")


def main():
    parser = argparse.ArgumentParser(
        description="Plot coupled column model results.")
    parser.add_argument("csv_file", help="Fortran CSV output file")
    parser.add_argument("--nc", help="Python NetCDF output for comparison")
    parser.add_argument("--output", "-o",
                        default="column_model_results.png",
                        help="Output PNG file")
    parser.add_argument("--labels", "-l", nargs=2,
                        default=["Fortran", "Python"],
                        help="Labels for comparison (default: Fortran Python)")
    parser.add_argument("--style", "-s",
                        choices=["default", "presentation", "publication"],
                        default="default",
                        help="Style preset (default: default)")
    args = parser.parse_args()

    style.setup(context=args.style)

    data = read_csv(args.csv_file)
    data2 = None
    if args.nc:
        data2 = read_nc(args.nc)

    plot(data, args.output, label=args.labels[0],
         data2=data2, label2=args.labels[1])


if __name__ == "__main__":
    main()
