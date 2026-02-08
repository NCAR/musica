"""
Plot TUV-x v5.4 O3 photolysis rates from CSV output.

Reads CSV files produced by the Fortran or Python TUV-x v5.4 examples
and plots vertical profiles of the O3 photolysis rate constants.

Usage:
    python plot_tuvx_v54.py <csv_file> [--output <png_file>]
    python plot_tuvx_v54.py fortran.csv python.csv [--output <png_file>]

With one CSV file, plots the four rate profiles (standard and doubled O3
for both O3 reactions). With two CSV files, overlays both sources for
cross-language comparison.
"""

import argparse
import csv
import sys

import matplotlib.pyplot as plt
import numpy as np


def read_csv(path):
    """Read a TUV-x v5.4 results CSV file.

    Returns a dict with arrays: height_km, jo3a_std, jo3a_2xO3, jo3b_std, jo3b_2xO3.
    """
    height = []
    jo3a_std = []
    jo3a_2xo3 = []
    jo3b_std = []
    jo3b_2xo3 = []
    with open(path) as f:
        reader = csv.DictReader(f)
        for row in reader:
            height.append(float(row["height_km"]))
            jo3a_std.append(float(row["jo3a_std"]))
            jo3a_2xo3.append(float(row["jo3a_2xO3"]))
            jo3b_std.append(float(row["jo3b_std"]))
            jo3b_2xo3.append(float(row["jo3b_2xO3"]))
    return {
        "height_km": np.array(height),
        "jo3a_std": np.array(jo3a_std),
        "jo3a_2xO3": np.array(jo3a_2xo3),
        "jo3b_std": np.array(jo3b_std),
        "jo3b_2xO3": np.array(jo3b_2xo3),
    }


def plot_single(data, label_suffix="", **kwargs):
    """Plot four rate profiles from one data source."""
    ls = kwargs.get("linestyle", "-")
    alpha = kwargs.get("alpha", 1.0)
    h = data["height_km"]
    sfx = f" ({label_suffix})" if label_suffix else ""
    plt.semilogx(data["jo3a_std"], h,
                 label=f"O3+hv->O2+O(1D) std{sfx}", linestyle=ls, alpha=alpha, color="C0")
    plt.semilogx(data["jo3a_2xO3"], h,
                 label=f"O3+hv->O2+O(1D) 2xO3{sfx}", linestyle=ls, alpha=alpha, color="C1")
    plt.semilogx(data["jo3b_std"], h,
                 label=f"O3+hv->O2+O(3P) std{sfx}", linestyle=ls, alpha=alpha, color="C2")
    plt.semilogx(data["jo3b_2xO3"], h,
                 label=f"O3+hv->O2+O(3P) 2xO3{sfx}", linestyle=ls, alpha=alpha, color="C3")


def main():
    parser = argparse.ArgumentParser(
        description="Plot TUV-x v5.4 O3 photolysis rates from CSV output.")
    parser.add_argument("csv_files", nargs="+", metavar="CSV",
                        help="One or two CSV files to plot")
    parser.add_argument("--output", "-o", default="tuvx_v54_photolysis_rates.png",
                        help="Output PNG file (default: tuvx_v54_photolysis_rates.png)")
    parser.add_argument("--labels", "-l", nargs="+",
                        help="Labels for each CSV file (default: filenames)")
    args = parser.parse_args()

    if len(args.csv_files) > 2:
        print("Error: at most two CSV files supported", file=sys.stderr)
        sys.exit(1)

    labels = args.labels or [f.rsplit("/", 1)[-1].replace(".csv", "")
                             for f in args.csv_files]
    if len(labels) < len(args.csv_files):
        labels.extend(args.csv_files[len(labels):])

    datasets = []
    for path in args.csv_files:
        datasets.append(read_csv(path))

    plt.figure(figsize=(10, 7))

    if len(datasets) == 1:
        plot_single(datasets[0])
    else:
        plot_single(datasets[0], label_suffix=labels[0], linestyle="-", alpha=1.0)
        plot_single(datasets[1], label_suffix=labels[1], linestyle="--", alpha=0.7)

    plt.xlabel("Photolysis Rate Constant (s$^{-1}$)")
    plt.ylabel("Height (km)")
    plt.title("TUV-x v5.4 O3 Photolysis Rates")
    plt.legend(fontsize=8)
    plt.grid(True, alpha=0.3)
    plt.tight_layout()
    plt.savefig(args.output, dpi=150)
    print(f"Saved {args.output}")


if __name__ == "__main__":
    main()
