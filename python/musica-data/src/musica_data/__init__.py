"""
Configuration and data files for the musica Python package.
"""

from importlib.resources import files


def configs_path() -> str:
    """Return the path of the installed configs directory."""
    return str(files(__name__) / "configs")
