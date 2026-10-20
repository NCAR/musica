"""
Utility functions for the musica package.
"""

import os


def find_config_path(*path_parts: str) -> str:
    """
    Find a config file or directory in either the source tree or the musica-data package.

    This function handles both editable installs (where configs are in the source
    tree) and wheel installs (where configs are in the musica-data package).

    Args:
        *path_parts: Path components relative to the configs directory
                    e.g., "tuvx", "tuv_5_4.json"

    Returns:
        Absolute path to the config file/directory

    Raises:
        FileNotFoundError: If the config file/directory is not found in either location

    Example:
        >>> find_config_path("tuvx", "tuv_5_4.json")
        "/path/to/musica/configs/tuvx/tuv_5_4.json"
    """
    # Try source location first (for editable installs)
    # Navigate up from python/musica/ to the repo root, then to configs/
    package_dir = os.path.dirname(__file__)
    source_root = os.path.abspath(os.path.join(package_dir, "..", ".."))
    source_path = os.path.join(source_root, "configs", *path_parts)

    if os.path.exists(os.path.join(source_root, "CMakeLists.txt")) and os.path.exists(source_path):
        return source_path

    # Try the musica-data package (for wheel installs)
    try:
        from musica_data import configs_path
        installed_path = os.path.join(configs_path(), *path_parts)
    except ImportError:
        installed_path = None

    if installed_path is not None and os.path.exists(installed_path):
        return installed_path

    # Neither location exists
    raise FileNotFoundError(
        f"Config path not found: {os.path.join('configs', *path_parts)}\n"
        f"Tried source location: {source_path}\n"
        f"Tried musica-data location: {installed_path or 'musica-data is not installed'}\n"
        f"For editable installs, ensure the source tree is intact. "
        f"For wheel installs, ensure the musica-data package is installed."
    )
