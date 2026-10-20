"""
Configuration and data files for the musica Python package.

The files are a copy of the configs/ directory of the MUSICA repository.
"""

from importlib.resources import files


def configs_path() -> str:
    """Return the path of the installed configs directory."""
    return str(files(__name__) / "configs")
