# MUSICA
[![GitHub Releases](https://img.shields.io/github/release/NCAR/musica.svg)](https://github.com/NCAR/musica/releases)
[![License](https://img.shields.io/github/license/NCAR/musica.svg)](https://github.com/NCAR/musica/blob/main/LICENSE)
[![docker](https://github.com/NCAR/musica/actions/workflows/docker.yml/badge.svg)](https://github.com/NCAR/musica/actions/workflows/docker.yml)
[![macOS](https://github.com/NCAR/musica/actions/workflows/mac.yml/badge.svg)](https://github.com/NCAR/musica/actions/workflows/mac.yml)
[![ubuntu](https://github.com/NCAR/musica/actions/workflows/ubuntu.yml/badge.svg)](https://github.com/NCAR/musica/actions/workflows/ubuntu.yml)
[![windows](https://github.com/NCAR/musica/actions/workflows/windows.yml/badge.svg)](https://github.com/NCAR/musica/actions/workflows/windows.yml)
[![Python tests](https://github.com/NCAR/musica/actions/workflows/python-tests.yml/badge.svg)](https://github.com/NCAR/musica/actions/workflows/python-tests.yml)
[![DOI](https://zenodo.org/badge/550370528.svg)](https://zenodo.org/doi/10.5281/zenodo.7458559)
[![PyPI version](https://badge.fury.io/py/musica.svg)](https://pypi.org/p/musica)
[![FAIR checklist badge](https://fairsoftwarechecklist.net/badge.svg)](https://fairsoftwarechecklist.net/v0.2?f=31&a=32113&i=22322&r=123)
[![codecov](https://codecov.io/gh/NCAR/musica/branch/main/graph/badge.svg)](https://codecov.io/gh/NCAR/musica)

Multi-Scale Infrastructure for Chemistry and Aerosols

MUSICA is a collection of modeling software, tools, and grids, that
allow for robust modeling of chemistry in Earth's atmosphere.

At present the project encompasses these components
- [TUV-x](https://github.com/NCAR/tuv-x)
    - A photolysis rate calculator 

- [MICM](https://github.com/NCAR/micm)
    - Model Independent Chemical Module

## Available grids

Pre-made grids for use in MUSICA are available [here](https://wiki.ucar.edu/display/MUSICA/Available+Grids).

# Contributors guide
Checkout our [software development plan](docs/Software%20Development%20Plan.pdf)
to see how you can contribute new science to MUSICA software.

## Developer Options

### Specifying dependency versions via paramaterization at configure time

Introduced in [Pull Request #124](https://github.com/NCAR/musica/pull/124), it is possible for developers to specify which versions of various dependencies should be used. These options are currently limited to those dependencies managed via `FetchContent`.  This change allows for more easily testing `musica` against changes committed in different repositories and branches.  The environmental variables introduced are outlined in the following table. 

#### CMake Dependency Variables

| Musica Dependency                                      | Repository                | Branch, Tag or Hash|
| ------------------------------------------------------ | --------------------------|--------------------|
| [Google Test](https://github.com/google/googletest.git)| GOOGLETEST_GIT_REPOSITORY | GOOGLETEST_GIT_TAG |
| [MICM](https://github.com/NCAR/mcim.git)               | MICM_GIT_REPOSITORY       | MICM_GIT_TAG       | 
| [TUV-X](https://github.com/NCAR/tuv-x.git)             | TUVX_GIT_REPOSITORY       | TUVX_GIT_TAG       |
| [PyBind11](https://github.com/pybind/pybind11)         | PYBIND11_GIT_REPOSITORY   | PYBIND11_GIT_TAG   |

#### Example Usage

> The following examples assume the working directory is a `build/` directory inside the `musica` source directory.

Specifying a different version of `tuv-x`, to ensure a change won't break anything.

    $ cmake .. \
        -DTUVX_GIT_REPOSITORY="https://github.com/WardF/tuv-x.git" \
        -DTUVX_GIT_TAG=test-fix

Specifying a specific version of `tuv-x` by has, but using the official repository.

    $ cmake .. \
        -DTUVX_GIT_TAG=a6b2c4d8745


### Python build
Musica has python bindings. If you want to install the python package, you may `pip install musica`.

To build the package locally,

```
pip install -e .
```

If you have an NVIDIA GPU and cuda installed, you can enable a build of musica with GPU support by setting the environment 
variable `BUILD_GPU`.

```
BUILD_GPU=1 pip install -e .
```

## Citing MUSICA

MUSICA can be cited in at least two ways. The first is to cite [the paper](https://doi.org/10.1175/BAMS-D-19-0331.1) that defines the vision
of the MUSICA software. The bibtex entry below can be used to generate a citaiton for this.

```
@Article { acom.software.musica-vision,
    author = "Gabriele G. Pfister and Sebastian D. Eastham and Avelino F. Arellano and Bernard Aumont and Kelley C. Barsanti and Mary C. Barth and Andrew Conley and Nicholas A. Davis and Louisa K. Emmons and Jerome D. Fast and Arlene M. Fiore and Benjamin Gaubert and Steve Goldhaber and Claire Granier and Georg A. Grell and Marc Guevara and Daven K. Henze and Alma Hodzic and Xiaohong Liu and Daniel R. Marsh and John J. Orlando and John M. C. Plane and Lorenzo M. Polvani and Karen H. Rosenlof and Allison L. Steiner and Daniel J. Jacob and Guy P. Brasseur",
    title = "The Multi-Scale Infrastructure for Chemistry and Aerosols (MUSICA)",
    journal = "Bulletin of the American Meteorological Society",
    year = "2020",
    publisher = "American Meteorological Society",
    address = "Boston MA, USA",
    volume = "101",
    number = "10",
    doi = "10.1175/BAMS-D-19-0331.1",
    pages= "E1743 - E1760",
    url = "https://journals.ametsoc.org/view/journals/bams/101/10/bamsD190331.xml"
}
```

At present MUSICA is on version zero. MUSICAv0 can be cited using the bibtex entry below.
MUSICAv0 description and evaluation:

```
@Article{acom.software.musica,
    author = {Schwantes, Rebecca H. and Lacey, Forrest G. and Tilmes, Simone and Emmons, Louisa K. and Lauritzen, Peter H. and Walters, Stacy and Callaghan, Patrick and Zarzycki, Colin M. and Barth, Mary C. and Jo, Duseong S. and Bacmeister, Julio T. and Neale, Richard B. and Vitt, Francis and Kluzek, Erik and Roozitalab, Behrooz and Hall, Samuel R. and Ullmann, Kirk and Warneke, Carsten and Peischl, Jeff and Pollack, Ilana B. and Flocke, Frank and Wolfe, Glenn M. and Hanisco, Thomas F. and Keutsch, Frank N. and Kaiser, Jennifer and Bui, Thao Paul V. and Jimenez, Jose L. and Campuzano-Jost, Pedro and Apel, Eric C. and Hornbrook, Rebecca S. and Hills, Alan J. and Yuan, Bin and Wisthaler, Armin},
    title = {Evaluating the Impact of Chemical Complexity and Horizontal Resolution on Tropospheric Ozone Over the Conterminous US With a Global Variable Resolution Chemistry Model},
    journal = {Journal of Advances in Modeling Earth Systems},
    volume = {14},
    number = {6},
    pages = {e2021MS002889},
    doi = {https://doi.org/10.1029/2021MS002889},
    url = {https://agupubs.onlinelibrary.wiley.com/doi/abs/10.1029/2021MS002889},
    eprint = {https://agupubs.onlinelibrary.wiley.com/doi/pdf/10.1029/2021MS002889},
    year = {2022}
}
```
