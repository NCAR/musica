Overview
========

The specific goal of MUSICA is to produce a new model independent infrastructure,
which will enable chemistry and aerosols to be simulated
at a large number of different resolutions in a single, coherent fashion.
At first, MUSICA will be configured within the NSF NCAR Community Earth system Model (CESM)
and through this enable full feedbacks between the atmosphere, ocean and land.
The infrastructure will unify the different chemical transport models
including CAM-Chem, WACCM and WRF-Chem, and NSF NCAR LES with chemistry
and a box model in a single modular framework.
The model infrastructure will be open source,
flexible and computationally efficient in order
to facilitate community co-development and use for scientific and operational purposes. 

At the heart of MUSICA is the standalone Model Independent Chemistry Model (MICM), which is a gas-phase kinetic solver. MICM is made available by the MUSICA wrapper which satisfies the requirements of the Common Community Physics Package (CCPP)
and that can be connected to any CCPP compliant atmosphere model.
MUSICA and MICM will have a flexible design to handle a variety of gas phase and aerosol schemes
and associated chemical modules such as deposition or photolysis.
