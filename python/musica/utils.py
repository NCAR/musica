"""
Utility functions for the musica package.
"""

import os


def find_config_path(*path_parts: str) -> str:
    """
    Find a config file or directory in either the installed package or source tree.

    This function handles both wheel installs (where configs are in the package)
    and editable installs (where configs are in the source tree).

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
    # Try installed location first (for wheel installs)
    # This will be at python/musica/configs/ when installed
    package_dir = os.path.dirname(__file__)
    installed_path = os.path.join(package_dir, "configs", *path_parts)

    if os.path.exists(installed_path):
        return installed_path

    # Try source location (for editable installs)
    # Navigate up from python/musica/ to the repo root, then to configs/
    source_path = os.path.join(package_dir, "..", "..", "configs", *path_parts)
    source_path = os.path.abspath(source_path)

    if os.path.exists(source_path):
        return source_path

    # Neither location exists
    raise FileNotFoundError(
        f"Config path not found: {os.path.join('configs', *path_parts)}\n"
        f"Tried installed location: {installed_path}\n"
        f"Tried source location: {source_path}\n"
        f"For editable installs, ensure the source tree is intact. "
        f"For wheel installs, ensure configs were installed correctly."
    )
