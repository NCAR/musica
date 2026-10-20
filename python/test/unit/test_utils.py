import json
import os

import pooch
import pytest

import musica.utils as utils
from musica.utils import find_config_path


def test_configs_dir_override(tmp_path, monkeypatch):
    (tmp_path / "v1").mkdir()
    (tmp_path / "v1" / "config.json").write_text("{}")
    monkeypatch.setenv(utils.CONFIGS_DIR_ENV, str(tmp_path))

    assert find_config_path("v1", "config.json") == str(tmp_path / "v1" / "config.json")
    with pytest.raises(FileNotFoundError):
        find_config_path("v1", "missing.json")


class FakeFetcher:
    """Writes each requested file, in place of a download."""

    def __init__(self, path, base_url, registry, **kwargs):
        self.path = path
        self.base_url = base_url
        self.fetched = []

    def fetch(self, name):
        self.fetched.append(name)
        target = os.path.join(self.path, name)
        os.makedirs(os.path.dirname(target), exist_ok=True)
        with open(target, "w") as f:
            f.write(name)
        return target


@pytest.fixture
def installed_configs(tmp_path, monkeypatch):
    """An installed configs directory without data files, and an empty cache."""
    configs = tmp_path / "installed"
    (configs / "tuvx").mkdir(parents=True)
    (configs / "tuvx" / "config.json").write_text('{"file": "data/cross_sections/O3_1.nc"}')
    (configs / "v1").mkdir()
    (configs / "v1" / "config.json").write_text("{}")
    registry = {
        "tuvx/data/cross_sections/O3_1.nc": "0" * 64,
        "tuvx/data/spectral_weights/NMSC (CIE 2006)_spectral_wght_1.nc": "1" * 64,
    }
    (configs / utils.DATA_REGISTRY).write_text(json.dumps(registry))

    fetchers = []

    def create(**kwargs):
        fetchers.append(FakeFetcher(**kwargs))
        return fetchers[-1]

    monkeypatch.delenv(utils.CONFIGS_DIR_ENV, raising=False)
    monkeypatch.setattr(pooch, "create", create)
    monkeypatch.setattr(pooch, "os_cache", lambda name: tmp_path / "cache")
    return configs, tmp_path / "cache", fetchers


def test_download_data_for_top_level_directory(installed_configs):
    configs, cache, fetchers = installed_configs
    from musica._version import version
    cache_configs = cache / version / "configs"

    path = utils._find_downloaded_config_path(str(configs), ("tuvx", "config.json"))

    assert path == str(cache_configs / "tuvx" / "config.json")
    assert os.path.isfile(cache_configs / "tuvx" / "data" / "cross_sections" / "O3_1.nc")
    assert len(fetchers) == 1
    assert fetchers[0].base_url == utils.DATA_URL.format(version=version)
    assert sorted(fetchers[0].fetched) == sorted(json.loads((configs / utils.DATA_REGISTRY).read_text()))

    # A second call uses the cache and does not download again
    utils._find_downloaded_config_path(str(configs), ("tuvx", "data/cross_sections/O3_1.nc"))
    assert len(fetchers) == 1


def test_no_download_without_data_files(installed_configs):
    configs, cache, fetchers = installed_configs

    path = utils._find_downloaded_config_path(str(configs), ("v1", "config.json"))

    assert os.path.isfile(path)
    assert fetchers == []


def test_missing_config_in_cache(installed_configs):
    configs, cache, fetchers = installed_configs

    with pytest.raises(FileNotFoundError):
        utils._find_downloaded_config_path(str(configs), ("v1", "missing.json"))
