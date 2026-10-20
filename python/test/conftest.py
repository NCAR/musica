import os

# The PyPI wheels download the config data files from the GitHub tag of the
# installed version. Use the configs of this source tree instead, so that the
# tests do not need network access and see the data of the code under test.
_source_configs = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "configs"))
if os.path.isdir(_source_configs):
    os.environ.setdefault("MUSICA_CONFIGS_DIR", _source_configs)
