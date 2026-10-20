"""
Utility functions for the musica package.
"""

import json
import logging
import os
import shutil
import sys
from concurrent.futures import ThreadPoolExecutor

# The PyPI wheels leave the config data files out to keep the wheels small.
# The wheel lists these files in this registry, and find_config_path downloads
# them from the GitHub tag of the installed version on first use.
DATA_REGISTRY = "data_registry.json"
DATA_URL = "https://raw.githubusercontent.com/NCAR/musica/v{version}/configs/"

# Set this environment variable to a configs directory to use it instead of the
# installed configs, for example on a machine without network access.
CONFIGS_DIR_ENV = "MUSICA_CONFIGS_DIR"


def find_config_path(*path_parts: str) -> str:
    """
    Find a config file or directory in either the installed package or source tree.

    This function handles both wheel installs (where configs are in the package)
    and editable installs (where configs are in the source tree).

    The PyPI wheels do not include the config data files. For these wheels, this
    function copies the configs into a user cache directory, and downloads the
    data files into it on first use.

    If the MUSICA_CONFIGS_DIR environment variable is set, this function uses
    that directory instead.

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
    override_dir = os.environ.get(CONFIGS_DIR_ENV)
    if override_dir:
        override_path = os.path.join(os.path.abspath(override_dir), *path_parts)
        if os.path.exists(override_path):
            return override_path
        raise FileNotFoundError(
            f"Config path not found: {override_path}\n"
            f"{CONFIGS_DIR_ENV} is set to {override_dir}"
        )

    # Try installed location first (for wheel installs)
    # This will be at python/musica/configs/ when installed
    package_dir = os.path.dirname(__file__)
    installed_configs = os.path.join(package_dir, "configs")
    installed_path = os.path.join(installed_configs, *path_parts)

    if os.path.exists(os.path.join(installed_configs, DATA_REGISTRY)):
        return _find_downloaded_config_path(installed_configs, path_parts)

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


def _find_downloaded_config_path(installed_configs: str, path_parts: tuple) -> str:
    """
    Find a config path in the user cache, and download the data files that it needs.

    The configs in the cache keep the layout of the configs directory, so that the
    relative data paths in a config resolve. The data files are downloaded for the
    whole top-level directory of the request (for example, all of tuvx/).
    """
    import pooch
    from ._version import version

    cache_configs = os.path.join(str(pooch.os_cache("musica")), version, "configs")
    relative_path = os.path.normpath(os.path.join(*path_parts)) if path_parts else ""
    top = relative_path.split(os.sep)[0]
    if top and top != ".":
        tops = [top]
    else:
        tops = [name for name in os.listdir(installed_configs) if name != DATA_REGISTRY]

    for name in tops:
        _cache_configs(installed_configs, cache_configs, name, version)

    cache_path = os.path.join(cache_configs, *path_parts)
    if os.path.exists(cache_path):
        return cache_path
    raise FileNotFoundError(
        f"Config path not found: {os.path.join('configs', *path_parts)}\n"
        f"Tried cache location: {cache_path}"
    )


def _cache_configs(installed_configs: str, cache_configs: str, top: str, version: str) -> None:
    """Copy one top-level entry of the installed configs into the cache, and download its data files."""
    import pooch

    marker = os.path.join(cache_configs, f".{top}.complete")
    if os.path.exists(marker):
        return

    source = os.path.join(installed_configs, top)
    os.makedirs(cache_configs, exist_ok=True)
    if os.path.isdir(source):
        shutil.copytree(source, os.path.join(cache_configs, top), dirs_exist_ok=True)
    elif os.path.isfile(source):
        shutil.copy2(source, cache_configs)

    with open(os.path.join(installed_configs, DATA_REGISTRY)) as f:
        registry = json.load(f)
    data_files = [name for name in registry if name.split("/")[0] == top]

    if data_files:
        url = DATA_URL.format(version=version)
        print(f"Downloading {len(data_files)} musica data files for '{top}' from {url} into {cache_configs}",
              file=sys.stderr)
        fetcher = pooch.create(path=cache_configs, base_url=url, registry=registry, retry_if_failed=3)
        # pooch makes the parent directory without exist_ok, so the threads can collide
        for name in data_files:
            os.makedirs(os.path.dirname(os.path.join(cache_configs, name)), exist_ok=True)
        logger = pooch.get_logger()
        level = logger.level
        logger.setLevel(logging.WARNING)
        try:
            with ThreadPoolExecutor(max_workers=8) as pool:
                list(pool.map(fetcher.fetch, data_files))
        finally:
            logger.setLevel(level)

    with open(marker, "w"):
        pass
