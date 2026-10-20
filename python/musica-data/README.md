# musica-data

Configuration and data files for the [musica](https://pypi.org/project/musica/) Python package.

The musica package depends on this package, so `pip install musica` installs it. The files are a copy of the `configs/` directory of the [MUSICA repository](https://github.com/NCAR/musica). They are in a separate package because they do not depend on the platform or the Python version. Thus each release uploads them once, and not in each platform wheel.

musica-data has its own dated version (YYYY.M.D), and a new version is released only when the data changes. Each musica version requires a minimum musica-data version.
