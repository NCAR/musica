# MUSICA
[![GitHub Releases](https://img.shields.io/github/release/NCAR/musica.svg)](https://github.com/NCAR/musica/releases)
[![License](https://img.shields.io/github/license/NCAR/musica.svg)](https://github.com/NCAR/musica/blob/main/LICENSE)
[![docker](https://github.com/NCAR/musica/actions/workflows/docker.yml/badge.svg)](https://github.com/NCAR/musica/actions/workflows/docker.yml)
[![macOS](https://github.com/NCAR/musica/actions/workflows/mac.yml/badge.svg)](https://github.com/NCAR/musica/actions/workflows/mac.yml)
[![ubuntu](https://github.com/NCAR/musica/actions/workflows/ubuntu.yml/badge.svg)](https://github.com/NCAR/musica/actions/workflows/ubuntu.yml)
[![windows](https://github.com/NCAR/musica/actions/workflows/windows.yml/badge.svg)](https://github.com/NCAR/musica/actions/workflows/windows.yml)
[![DOI](https://zenodo.org/badge/550370528.svg)](https://zenodo.org/doi/10.5281/zenodo.7458559)

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
