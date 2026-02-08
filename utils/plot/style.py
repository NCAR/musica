"""Plot styling for DAVINCI-MPAS.

Based on the DAVINCI-MONET style system (NSF NCAR brand guidelines).
Provides colors, fonts, font size presets, chemical species label
formatting, and colormap utilities.

Usage
-----
>>> import style
>>> style.apply_ncar_style()                    # Default sizes
>>> style.apply_ncar_style("publication")       # Smaller, for journals
>>> style.apply_ncar_style("presentation")      # Larger, for slides
>>>
>>> # Chemical species labels for plot text
>>> style.species_label("qO3")                  # r"O$_3$"
>>> style.format_title("NO2 and O3 evolution")  # r"NO$_2$ and O$_3$ evolution"
"""

from __future__ import annotations

import re
from dataclasses import dataclass
from typing import Literal

import matplotlib.pyplot as plt

# =============================================================================
# NSF NCAR Brand Colors
# https://brand.ucar.edu/
# =============================================================================

NCAR_COLORS = {
    # Primary
    "space": "#011837",
    "dark_blue": "#00357A",
    "ncar_blue": "#0A5DDA",
    "aqua": "#00A2B4",
    # Light
    "light_blue": "#CEDFF8",
    "light_gray": "#F1F0EE",
    # Accent
    "orange": "#FF8C00",
    "yellow": "#FFDD31",
    "gray": "#58595B",
    # Data visualization
    "red": "#D62839",
    "green": "#2E8B57",
    "purple": "#7B68EE",
}

NCAR_PRIMARY = NCAR_COLORS["ncar_blue"]
NCAR_SECONDARY = NCAR_COLORS["aqua"]
NCAR_ACCENT = NCAR_COLORS["orange"]

NCAR_PALETTE = [
    NCAR_COLORS["ncar_blue"],
    NCAR_COLORS["aqua"],
    NCAR_COLORS["orange"],
    NCAR_COLORS["purple"],
    NCAR_COLORS["green"],
    NCAR_COLORS["red"],
    NCAR_COLORS["yellow"],
    NCAR_COLORS["dark_blue"],
]

# DAVINCI-MPAS convenience aliases (backward compat with plot_passive.py)
TRACER_COLOR = NCAR_COLORS["ncar_blue"]
DECAY_COLOR = NCAR_COLORS["orange"]
DIFF_CMAP = "RdBu_r"
TRACER_CMAP = "viridis"

# =============================================================================
# Font Configuration
# =============================================================================

NCAR_FONT_FAMILY = "sans-serif"
NCAR_FONT_SANS_SERIF = [
    "Poppins",
    "Helvetica Neue",
    "Helvetica",
    "Arial",
    "DejaVu Sans",
]


@dataclass
class FontSizes:
    """Font size configuration for different contexts.

    All sizes in points (1/72 inch). Absolute — do not scale with figure size.
    """

    figure_title: float = 20.0
    axes_title: float = 16.0
    axes_label: float = 14.0
    tick_label: float = 12.0
    legend: float = 12.0
    legend_small: float = 10.0
    annotation: float = 12.0
    annotation_small: float = 10.0


FONT_SIZES_DEFAULT = FontSizes()

FONT_SIZES_PRESENTATION = FontSizes(
    figure_title=24.0,
    axes_title=18.0,
    axes_label=16.0,
    tick_label=14.0,
    legend=14.0,
    legend_small=12.0,
    annotation=14.0,
    annotation_small=12.0,
)

FONT_SIZES_PUBLICATION = FontSizes(
    figure_title=18.0,
    axes_title=14.0,
    axes_label=12.0,
    tick_label=10.0,
    legend=10.0,
    legend_small=9.0,
    annotation=10.0,
    annotation_small=9.0,
)

# Backward compat alias used by plot_passive.py
SUPTITLE_SIZE = FONT_SIZES_DEFAULT.figure_title
FONT_SIZE = FONT_SIZES_DEFAULT.tick_label
LABEL_SIZE = FONT_SIZES_DEFAULT.axes_label

# =============================================================================
# Chemical Species Labels (LaTeX for matplotlib)
# =============================================================================

# MPAS tracer name -> display label with LaTeX subscripts
# Used for axis labels, legends, titles, and colorbars
SPECIES_LABELS: dict[str, str] = {
    # DAVINCI tracers (MPAS Registry names)
    "qPassive": "qPassive",
    "qA": "A",
    "qB": "B",
    "qAB": "AB",
    "qO": "O",
    "qO3": r"O$_3$",
    "qNO": "NO",
    "qNO2": r"NO$_2$",
    "qCO": "CO",
    "qCH4": r"CH$_4$",
    "qOH": "OH",
    "qHO2": r"HO$_2$",
    "qHCHO": "HCHO",
    "qSO2": r"SO$_2$",
    "qSulfate": "Sulfate",
    # Standard chemical names (no q prefix)
    "O3": r"O$_3$",
    "O": "O",
    "O2": r"O$_2$",
    "NO": "NO",
    "NO2": r"NO$_2$",
    "NOx": r"NO$_x$",
    "N2O": r"N$_2$O",
    "N2O5": r"N$_2$O$_5$",
    "CO": "CO",
    "CO2": r"CO$_2$",
    "CH4": r"CH$_4$",
    "OH": "OH",
    "HO2": r"HO$_2$",
    "HOx": r"HO$_x$",
    "H2O": r"H$_2$O",
    "H2O2": r"H$_2$O$_2$",
    "HCHO": "HCHO",
    "SO2": r"SO$_2$",
    "SO3": r"SO$_3$",
    "H2SO4": r"H$_2$SO$_4$",
    "NH3": r"NH$_3$",
    "HNO3": r"HNO$_3$",
    "PM25": r"PM$_{2.5}$",
    "PM10": r"PM$_{10}$",
}

# Patterns for formatting chemical formulas in title strings.
# Order matters: longer patterns first to avoid partial matches
# (e.g., N2O5 before N2O, HNO3 before NO).
TITLE_FORMULA_REPLACEMENTS: list[tuple[str, str]] = [
    ("H2SO4", r"H$_2$SO$_4$"),
    ("H2O2", r"H$_2$O$_2$"),
    ("PM2.5", r"PM$_{2.5}$"),
    ("PM25", r"PM$_{2.5}$"),
    ("PM10", r"PM$_{10}$"),
    ("N2O5", r"N$_2$O$_5$"),
    ("HNO3", r"HNO$_3$"),
    ("N2O", r"N$_2$O"),
    ("NO2", r"NO$_2$"),
    ("SO2", r"SO$_2$"),
    ("SO3", r"SO$_3$"),
    ("CO2", r"CO$_2$"),
    ("NH3", r"NH$_3$"),
    ("CH4", r"CH$_4$"),
    ("HO2", r"HO$_2$"),
    ("NOx", r"NO$_x$"),
    ("NOX", r"NO$_x$"),
    ("HOx", r"HO$_x$"),
    ("O3", r"O$_3$"),
    ("O2", r"O$_2$"),
    ("H2O", r"H$_2$O"),
]


def species_label(name: str) -> str:
    """Get a display label for a chemical species or MPAS tracer.

    Parameters
    ----------
    name
        MPAS tracer name (e.g., "qO3") or chemical name (e.g., "O3").

    Returns
    -------
    str
        Display label with LaTeX subscripts for matplotlib rendering.

    Examples
    --------
    >>> species_label("qO3")
    'O$_3$'
    >>> species_label("NO2")
    'NO$_2$'
    >>> species_label("qPassive")
    'qPassive'
    """
    if name in SPECIES_LABELS:
        return SPECIES_LABELS[name]
    # Strip q prefix and try again
    if name.startswith("q") and name[1:] in SPECIES_LABELS:
        return SPECIES_LABELS[name[1:]]
    return name


def format_title(title: str) -> str:
    """Format a plot title, replacing chemical formulas with LaTeX.

    Applies case-insensitive regex replacements for known chemical
    formulas. Longer patterns are matched first to avoid partial
    substitution (e.g., "N2O5" matches before "N2O" or "O5").

    Parameters
    ----------
    title
        Raw title string (e.g., "NO2 and O3 evolution").

    Returns
    -------
    str
        Title with LaTeX-formatted chemical formulas.

    Examples
    --------
    >>> format_title("NO2 and O3 at surface")
    'NO$_2$ and O$_3$ at surface'
    """
    result = title
    for pattern, replacement in TITLE_FORMULA_REPLACEMENTS:
        result = re.sub(re.escape(pattern), replacement, result,
                        flags=re.IGNORECASE)
    return result


def species_color(name: str, index: int = 0) -> str:
    """Get a color for a species from the NCAR palette.

    Parameters
    ----------
    name
        Species or tracer name (used for special cases).
    index
        Index into NCAR_PALETTE for default color cycling.

    Returns
    -------
    str
        Hex color code.
    """
    # Special cases
    special = {
        "qPassive": NCAR_COLORS["ncar_blue"],
        "qO3": NCAR_COLORS["ncar_blue"],
        "qNO": NCAR_COLORS["green"],
        "qNO2": NCAR_COLORS["orange"],
        "qCO": NCAR_COLORS["gray"],
        "qA": NCAR_COLORS["ncar_blue"],
        "qB": NCAR_COLORS["aqua"],
        "qAB": NCAR_COLORS["orange"],
    }
    if name in special:
        return special[name]
    return NCAR_PALETTE[index % len(NCAR_PALETTE)]


# =============================================================================
# Style Application
# =============================================================================


def apply_ncar_style(
    context: Literal["default", "presentation", "publication"] = "default",
    use_seaborn: bool = True,
    seaborn_style: str = "whitegrid",
) -> None:
    """Apply NCAR brand styling to matplotlib globally."""
    if context == "presentation":
        sizes = FONT_SIZES_PRESENTATION
    elif context == "publication":
        sizes = FONT_SIZES_PUBLICATION
    else:
        sizes = FONT_SIZES_DEFAULT

    if use_seaborn:
        try:
            import seaborn as sns
            sns.set_theme(style=seaborn_style, palette="deep")
        except ImportError:
            pass

    plt.rcParams["font.family"] = NCAR_FONT_FAMILY
    plt.rcParams["font.sans-serif"] = NCAR_FONT_SANS_SERIF
    plt.rcParams["mathtext.fontset"] = "dejavusans"

    plt.rcParams["axes.labelsize"] = sizes.axes_label
    plt.rcParams["axes.titlesize"] = sizes.axes_title
    plt.rcParams["xtick.labelsize"] = sizes.tick_label
    plt.rcParams["ytick.labelsize"] = sizes.tick_label
    plt.rcParams["legend.fontsize"] = sizes.legend
    plt.rcParams["figure.titlesize"] = sizes.figure_title

    plt.rcParams["axes.prop_cycle"] = plt.cycler(color=NCAR_PALETTE)

    plt.rcParams["lines.linewidth"] = 1.5
    plt.rcParams["lines.markersize"] = 6

    plt.rcParams["axes.grid"] = True
    plt.rcParams["grid.alpha"] = 0.3
    plt.rcParams["grid.linestyle"] = "-"

    plt.rcParams["figure.facecolor"] = "white"
    plt.rcParams["axes.facecolor"] = "white"
    plt.rcParams["savefig.facecolor"] = "white"
    plt.rcParams["savefig.dpi"] = 300
    plt.rcParams["savefig.bbox"] = "tight"


def setup(
    context: Literal["default", "presentation", "publication"] = "default",
) -> None:
    """Apply NCAR style. Backward-compatible alias for apply_ncar_style()."""
    _register_ncar_cmaps()
    apply_ncar_style(context=context, use_seaborn=True)


def reset_style() -> None:
    """Reset matplotlib to default styling."""
    plt.rcdefaults()


def get_palette(n_colors: int | None = None) -> list[str]:
    """Get colors from the NCAR palette."""
    if n_colors is None:
        return list(NCAR_PALETTE)
    return [NCAR_PALETTE[i % len(NCAR_PALETTE)] for i in range(n_colors)]


def get_bias_cmap() -> str:
    """Diverging colormap for bias plots."""
    return "RdBu_r"


def get_sequential_cmap() -> str:
    """Sequential colormap for concentration fields."""
    return "ncar_sunset"


def _register_ncar_cmaps() -> None:
    """Register custom NCAR colormaps with matplotlib."""
    from matplotlib.colors import LinearSegmentedColormap

    # NCAR Blue sequential: white -> light_blue -> aqua -> ncar_blue -> dark_blue -> space
    # Zero = white, increasing concentration = deeper blue
    ncar_blue_colors = [
        (1.0, 1.0, 1.0),         # white
        _hex_to_rgb(NCAR_COLORS["light_blue"]),
        _hex_to_rgb(NCAR_COLORS["aqua"]),
        _hex_to_rgb(NCAR_COLORS["ncar_blue"]),
        _hex_to_rgb(NCAR_COLORS["dark_blue"]),
        _hex_to_rgb(NCAR_COLORS["space"]),
    ]
    cmap = LinearSegmentedColormap.from_list("ncar_blue", ncar_blue_colors, N=256)
    plt.colormaps.register(cmap)
    plt.colormaps.register(cmap.reversed())

    # NCAR Sunset sequential: white -> yellow -> orange -> red -> dark_blue
    # Zero = white, wide perceptual range for concentration fields
    ncar_sunset_colors = [
        (1.0, 1.0, 1.0),         # white
        _hex_to_rgb(NCAR_COLORS["yellow"]),
        _hex_to_rgb(NCAR_COLORS["orange"]),
        _hex_to_rgb(NCAR_COLORS["red"]),
        _hex_to_rgb(NCAR_COLORS["dark_blue"]),
    ]
    cmap = LinearSegmentedColormap.from_list("ncar_sunset", ncar_sunset_colors, N=256)
    plt.colormaps.register(cmap)
    plt.colormaps.register(cmap.reversed())


def _hex_to_rgb(hex_color: str) -> tuple[float, float, float]:
    """Convert hex color string to (r, g, b) tuple in [0, 1]."""
    h = hex_color.lstrip("#")
    return tuple(int(h[i:i+2], 16) / 255.0 for i in (0, 2, 4))
