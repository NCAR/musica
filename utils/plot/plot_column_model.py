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
import sys

import matplotlib.pyplot as plt
import numpy as np


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

    fields = ["O3", "O", "O1D", "jO3a", "jO3b", "jO2"]
    data = {f: np.zeros((nt, nz)) for f in fields}

    for row in rows:
        ti = time_idx[row["time_hr"]]
        zi = height_idx[row["height_km"]]
        for f in fields:
            data[f][ti, zi] = row[f]

    return {
        "times": np.array(times),
        "heights": np.array(heights),
        **data,
    }


def read_nc(path):
    """Read column model NetCDF output (from Python version)."""
    import xarray as xr
    ds = xr.open_dataset(path)
    return {
        "times": ds["time"].values,
        "heights": ds["height"].values,
        "O3": ds["O3"].values,
        "O": ds["O"].values,
        "O1D": ds["O1D"].values,
        "jO3a": ds["jO3a"].values,
        "jO3b": ds["jO3b"].values,
        "jO2": ds["jO2"].values,
    }


def plot(data, output_path, label="Fortran", data2=None, label2="Python"):
    """Create multi-panel column model figure."""
    fig, axes = plt.subplots(2, 2, figsize=(14, 10))
    times = data["times"]
    heights = data["heights"]
    compare = data2 is not None

    # --- Panel 1: O3 vertical profiles at selected local times ---
    ax = axes[0, 0]
    for t_hr, style in [(0, "k--"), (6, "C0-"), (12, "C1-"),
                        (18, "C2-"), (23.75, "k:")]:
        idx = np.argmin(np.abs(times - t_hr))
        lbl = f"{int(t_hr):02d}:00"
        o3 = data["O3"][idx]
        mask = o3 > 0
        ax.semilogx(o3[mask], heights[mask], style,
                     label=f"{lbl} ({label})" if compare else lbl)
        if compare:
            idx2 = np.argmin(np.abs(data2["times"] - t_hr))
            o3_2 = data2["O3"][idx2]
            mask2 = o3_2 > 0
            ax.semilogx(o3_2[mask2], data2["heights"][mask2], style,
                         alpha=0.4, linewidth=3)
    ax.set_xlabel("O3 (mol m$^{-3}$)")
    ax.set_ylabel("Height (km)")
    ax.set_title("O3 Vertical Profiles")
    ax.legend(fontsize=7)
    ax.grid(True, alpha=0.3)

    # --- Panel 2: O3 time series at selected altitudes ---
    ax = axes[0, 1]
    for z_km, color in [(15, "C0"), (25, "C1"), (35, "C2"), (50, "C3")]:
        idx = np.argmin(np.abs(heights - z_km))
        ax.plot(times, data["O3"][:, idx], color=color,
                label=f"{z_km} km" + (f" ({label})" if compare else ""))
        if compare:
            idx2 = np.argmin(np.abs(data2["heights"] - z_km))
            ax.plot(data2["times"], data2["O3"][:, idx2],
                    color=color, linestyle="--", alpha=0.6)
    ax.set_xlabel("Local Time (hours)")
    ax.set_ylabel("O3 (mol m$^{-3}$)")
    ax.set_title("O3 Diurnal Evolution")
    ax.legend(fontsize=8)
    ax.grid(True, alpha=0.3)
    ax.set_xlim(0, times[-1])

    # --- Panel 3: Short-lived species at 30 km ---
    ax = axes[1, 0]
    z_idx = np.argmin(np.abs(heights - 30))
    ax.plot(times, data["O"][:, z_idx], "C0",
            label="O" + (f" ({label})" if compare else ""))
    ax.plot(times, data["O1D"][:, z_idx], "C1",
            label="O1D" + (f" ({label})" if compare else ""))
    if compare:
        z_idx2 = np.argmin(np.abs(data2["heights"] - 30))
        ax.plot(data2["times"], data2["O"][:, z_idx2],
                "C0--", alpha=0.6)
        ax.plot(data2["times"], data2["O1D"][:, z_idx2],
                "C1--", alpha=0.6)
    ax.set_xlabel("Local Time (hours)")
    ax.set_ylabel("Concentration (mol m$^{-3}$)")
    ax.set_title(f"Short-lived Species at {heights[z_idx]:.0f} km")
    ax.legend(fontsize=8)
    ax.grid(True, alpha=0.3)
    ax.set_xlim(0, times[-1])

    # --- Panel 4: Photolysis rates at 30 km ---
    ax = axes[1, 1]
    ax.plot(times, data["jO3a"][:, z_idx], "C0",
            label="j(O3$\\rightarrow$O+O$_2$)"
                  + (f" ({label})" if compare else ""))
    ax.plot(times, data["jO3b"][:, z_idx], "C1",
            label="j(O3$\\rightarrow$O($^1$D)+O$_2$)"
                  + (f" ({label})" if compare else ""))
    if compare:
        ax.plot(data2["times"], data2["jO3a"][:, z_idx2],
                "C0--", alpha=0.6)
        ax.plot(data2["times"], data2["jO3b"][:, z_idx2],
                "C1--", alpha=0.6)
    ax.set_xlabel("Local Time (hours)")
    ax.set_ylabel("Photolysis Rate (s$^{-1}$)")
    ax.set_title(f"Photolysis Rates at {heights[z_idx]:.0f} km")
    ax.legend(fontsize=8)
    ax.grid(True, alpha=0.3)
    ax.set_xlim(0, times[-1])

    title = "Coupled TUV-x + MICM Chapman Column Model\nBoulder CO, Summer Solstice"
    if compare:
        title += f"\nSolid: {label}, Dashed: {label2}"
    fig.suptitle(title, fontsize=14)
    fig.tight_layout()
    fig.savefig(output_path, dpi=150)
    print(f"Saved {output_path}")


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
    args = parser.parse_args()

    data = read_csv(args.csv_file)
    data2 = None
    if args.nc:
        data2 = read_nc(args.nc)

    plot(data, args.output, label=args.labels[0],
         data2=data2, label2=args.labels[1])


if __name__ == "__main__":
    main()
