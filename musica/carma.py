"""
CARMA aerosol model Python interface.

This module provides a simplified Python interface to the CARMA aerosol model.
It allows users to create a CARMA instance and run simulations with specified parameters.

Note: CARMA is only available on macOS and Linux platforms.
"""

from typing import Dict, Optional, List, Union
from ctypes import c_void_p
import numpy as np
import xarray as xr
import ussa1976
from enum import Enum
from . import backend

_backend = backend.get_backend()

version = _backend._carma._get_carma_version() if backend.carma_available() else None


class ParticleShape(Enum):
    """Enumeration for particle shapes used in CARMA."""
    SPHERE = 1
    HEXAGON = 2
    CYLINDER = 3


class ParticleType(Enum):
    """Enumeration for particle types used in CARMA."""
    INVOLATILE = 1
    VOLATILE = 2
    CORE_MASS = 3
    VOLATILE_CORE = 4
    CORE_MASS_TWO_MOMENTS = 5


class ParticleSwellingAlgorithm(Enum):
    """Enumeration for particle swelling algorithms used in CARMA."""
    NONE = 0
    FITZGERALD = 1
    GERBER = 2
    WEIGHT_PERCENT_H2SO4 = 3
    PETTERS = 4


class ParticleSwellingComposition(Enum):
    """Enumeration for particle swelling compositions used in CARMA."""
    NONE = 0
    AMMONIUM_SULFATE = 1
    SEA_SALT = 2
    URBAN = 3
    RURAL = 4


class ParticleFallVelocityAlgorithm(Enum):
    """Enumeration for particle fall velocity algorithms used in CARMA."""
    NONE = 0
    STANDARD_SPHERICAL_ONLY = 1
    STANDARD_SHAPE_SUPPORT = 2
    HEYMSFIELD_2010 = 3


class MieCalculationAlgorithm(Enum):
    """Enumeration for Mie calculation algorithms used in CARMA."""
    NONE = 0
    TOON_1981 = 1
    BOHREN_1983 = 2
    BOTET_1997 = 3


class OpticsAlgorithm(Enum):
    """Enumeration for optics algorithms used in CARMA."""
    NONE = 0
    FIXED = 1
    MIXED_YU_2015 = 2
    SULFATE_YU_2015 = 3
    MIXED_H2O_YU_2015 = 4
    MIXED_CORE_SHELL = 5
    MIXED_VOLUME = 6
    MIXED_MAXWELL = 7
    SULFATE = 8


class VaporizationAlgorithm(Enum):
    """Enumeration for vaporization algorithms used in CARMA."""
    NONE = 0
    H2O_BUCK_1981 = 1
    H2O_MURPHY_2005 = 2
    H2O_GOFF_1946 = 3
    H2SO4_AYERS_1980 = 4


class GasComposition(Enum):
    """Enumeration for gas compositions used in CARMA."""
    NONE = 0
    H2O = 1
    H2SO4 = 2
    SO2 = 3


class ParticleComposition(Enum):
    """Enumeration for particle compositions used in CARMA."""
    ALUMINUM = 1
    SULFURIC_ACID = 2
    DUST = 3
    ICE = 4
    WATER = 5
    BLACK_CARBON = 6
    ORGANIC_CARBON = 7
    OTHER = 8


class ParticleCollectionAlgorithm(Enum):
    """Enumeration for particle collection algorithms used in CARMA."""
    NONE = 0
    CONSTANT = 1
    FUCHS = 2
    DATA = 3


class ParticleNucleationAlgorithm(Enum):
    """Enumeration for particle nucleation algorithms used in CARMA."""
    NONE = 0
    AEROSOL_FREEZING_TABAZDEH_2000 = 1
    AEROSOL_FREEZING_KOOP_2000 = 2
    AEROSOL_FREEZING_MURRAY_2010 = 3
    DROPLET_ACTIVATION = 256
    AEROSOL_FREEZING = 512
    DROPLET_FREEZING = 1024
    ICE_MELTING = 2048
    HETEROGENEOUS_NUCLEATION = 4096
    HOMOGENEOUS_NUCLEATION = 8192
    HETEROGENEOUS_SULFURIC_ACID_NUCLEATION = 16384


class CarmaCoordinates(Enum):
    """Enumeration for CARMA coordinates."""
    CARTESIAN = 1
    SIGMA = 2
    LONGITUDE_LATITUDE = 3
    LAMBERT_CONFORMAL = 4
    POLAR_STEREOGRAPHIC = 5
    MERCATOR = 6
    HYBRID = 7


class CARMAWavelengthBin:
    """Configuration for a CARMA wavelength bin.

    A CARMA wavelength bin represents a specific wavelength range used in optical calculations.
    """

    def __init__(self, center: float, width: float, do_emission: bool = True):
        """
        Initialize a CARMA wavelength bin.

        Args:
            center: Center wavelength in micrometers.
            width: Width of the wavelength bin in micrometers.
            do_emission: Whether to include this wavelength in emission calculations (default: True).
        """
        self.center = center
        self.width = width
        self.do_emission = do_emission

    def to_dict(self) -> Dict:
        """Convert to dictionary."""
        return {k: v for k, v in self.__dict__.items()}


class CARMAGroupConfig:
    """Configuration for a CARMA particle group.

    A CARMA particle group represents a collection of particles with similar properties.
    """

    def __init__(self,
                 name: str = "default_group",
                 shortname: str = "",
                 rmin: float = 1e-9,
                 rmrat: float = 2.0,
                 rmassmin: float = 0.0,
                 ishape: int = ParticleShape.SPHERE,
                 eshape: float = 1.0,
                 swelling_approach: dict = {
                     "algorithm": ParticleSwellingAlgorithm.NONE,
                     "composition": ParticleSwellingComposition.NONE
                 },
                 fall_velocity_routine: int = ParticleFallVelocityAlgorithm.STANDARD_SPHERICAL_ONLY,
                 mie_calculation_algorithm: int = MieCalculationAlgorithm.NONE,
                 optics_algorithm: int = OpticsAlgorithm.FIXED,
                 is_ice: bool = False,
                 is_fractal: bool = False,
                 is_cloud: bool = False,
                 is_sulfate: bool = False,
                 do_wetdep: bool = False,
                 do_drydep: bool = False,
                 do_vtran: bool = True,
                 solfac: float = 0.0,
                 scavcoef: float = 0.0,
                 dpc_threshold: float = 0.0,
                 rmon: float = 0.0,
                 df: Optional[List[float]] = None,
                 falpha: float = 1.0,
                 neutral_volfrc: float = 0.0):
        """
        Initialize a CARMA group configuration.

        Args:
            name: Name of the group (default: "default_group")
            shortname: Short name for the group (default: "")
            rmin: Radius of particles in the first bin [m] (default: 1e-9)
            rmrat: Ratio of masses of particles in consecutive bins (default: 2.0)
            rmassmin: Minimum mass of particles [kg] (default: 0.0)
            ishape: Shape of the particles (default: ParticleShape.SPHERE)
            eshape: Ratio of particle length / diameter (default: 1.0)
            swelling_approach: Dictionary specifying swelling algorithm and composition (default: NONE)
            fall_velocity_routine: Algorithm for fall velocity (default: STANDARD_SPHERICAL_ONLY)
            mie_calculation_algorithm: Algorithm for Mie calculations (default: NONE)
            optics_algorithm: Algorithm for optics (default: FIXED)
            is_ice: Whether the particles are ice (default: False)
            is_fractal: Whether the particles are fractal (default: False)
            is_cloud: Whether the group is a cloud (default: False)
            is_sulfate: Whether the group is sulfate (default: False)
            do_wetdep: Whether to include wet deposition (default: False)
            do_drydep: Whether to include dry deposition (default: False)
            do_vtran: Whether to include vertical transport (default: True)
            solfac: Solubility factor for wet deposition (default: 0.0)
            scavcoef: Scavenging coefficient for wet deposition (default: 0.0)
            dpc_threshold: Threshold for dry particle collection (default: 0.0)
            rmon: Monomer radius of fractal particles [m] (default: 0.0)
            df: List of fractal dimensions for each size bin (default: None)
            falpha: Fractal packing coefficient (default: 1.0)
            neutral_volfrc: Neutral volume fraction for fractal particles (default: 0.0)
        """
        self.name = name
        self.shortname = shortname
        self.rmin = rmin
        self.rmrat = rmrat
        self.rmassmin = rmassmin
        self.ishape = ishape
        self.eshape = eshape
        self.swelling_approach = swelling_approach
        self.fall_velocity_routine = fall_velocity_routine
        self.mie_calculation_algorithm = mie_calculation_algorithm
        self.optics_algorithm = optics_algorithm
        self.is_ice = is_ice
        self.is_fractal = is_fractal
        self.is_cloud = is_cloud
        self.is_sulfate = is_sulfate
        self.do_wetdep = do_wetdep
        self.do_drydep = do_drydep
        self.do_vtran = do_vtran
        self.solfac = solfac
        self.scavcoef = scavcoef
        self.dpc_threshold = dpc_threshold
        self.rmon = rmon
        self.df = df or []
        self.falpha = falpha
        self.neutral_volfrc = neutral_volfrc

    def to_dict(self) -> Dict:
        """Convert to dictionary, serializing enums in swelling_approach as well."""
        result = {}
        for k, v in self.__dict__.items():
            if k == "swelling_approach" and isinstance(v, dict):
                # Serialize enum values inside swelling_approach dict
                result[k] = {sk: (sv.value if isinstance(sv, Enum) else sv)
                             for sk, sv in v.items()}
            else:
                result[k] = v.value if isinstance(v, Enum) else v
        return result


class CARMAElementConfig:
    """Configuration for a CARMA particle element.

    A CARMA particle element represents one of the components of a cloud or aerosol particle.
    """

    def __init__(self,
                 igroup: int = 1,
                 isolute: int = 0,
                 name: str = "default_element",
                 shortname: str = "",
                 itype: int = ParticleType.INVOLATILE,
                 icomposition: int = ParticleComposition.OTHER,
                 is_shell: bool = True,
                 rho: float = 1000.0,
                 rhobin: Optional[List[float]] = None,
                 arat: Optional[List[float]] = None,
                 kappa: float = 0.0,
                 refidx: Optional[List[List[float]]] = None):
        """
        Initialize a CARMA element configuration.

        Args:
            igroup: Group ID this element belongs to (default: 1)
            isolute: Index of the solute (default: 0)
            name: Name of the element (default: "default_element")
            shortname: Short name for the element (default: "")
            itype: Type of the particle (default: ParticleType.INVOLATILE)
            icomposition: Composition of the particle (default: ParticleComposition.OTHER)
            is_shell: For core/shell optics, whether this element is part of the shell (True) or core (False) (default: True)
            rho: Density of the element in kg/m3 (default: 1.0)
            rhobin: List of densities for each size bin in kg/m3 (default: None)
            arat: List of area ratios for each size bin (default: None)
            kappa: Hygroscopicity parameter (default: 0.0)
            refidx: List of lists of refractive indices for each wavelength bin (default: None)
        """
        self.igroup = igroup
        self.isolute = isolute
        self.name = name
        self.shortname = shortname
        self.itype = itype
        self.icomposition = icomposition
        self.is_shell = is_shell
        self.rho = rho
        self.rhobin = rhobin or []
        self.arat = arat or []
        self.kappa = kappa
        self.refidx = refidx or []

    def to_dict(self) -> Dict:
        """Convert to dictionary."""
        return {k: (v.value if isinstance(v, Enum) else v) for k, v in self.__dict__.items()}


class CARMASoluteConfig:
    """Configuration for a CARMA solute.

    A CARMA solute represents a chemical species that can dissolve in water and affect particle properties.
    """

    def __init__(self,
                 name: str = "default_solute",
                 shortname: str = "",
                 ions: int = 0,
                 wtmol: float = 0.0,
                 rho: float = 0.0):
        """
        Initialize a CARMA solute configuration.

        Args:
            name: Name of the solute (default: "default_solute")
            shortname: Short name for the solute (default: "")
            ions: Number of ions (default: 0)
            wtmol: Molecular weight in kg/mol (default: 0.0)
            rho: Density in kg/m3 (default: 0.0)
        """
        self.name = name
        self.shortname = shortname
        self.ions = ions
        self.wtmol = wtmol
        self.rho = rho

    def to_dict(self) -> Dict:
        """Convert to dictionary."""
        return {k: v for k, v in self.__dict__.items()}


class CARMAGasConfig:
    """Configuration for a CARMA gas.

    A CARMA gas represents a gaseous species in the atmosphere.
    """

    def __init__(self,
                 name: str = "default_gas",
                 shortname: str = "",
                 wtmol: float = 0.0,
                 ivaprtn: VaporizationAlgorithm = VaporizationAlgorithm.NONE,
                 icomposition: GasComposition = GasComposition.NONE,
                 dgc_threshold: float = 0.0,
                 ds_threshold: float = 0.0,
                 refidx: Optional[List[List[float]]] = None):
        """
        Initialize a CARMA gas configuration.

        Args:
            name: Name of the gas (default: "default_gas")
            shortname: Short name for the gas (default: "")
            wtmol: Molecular weight in kg/mol (default: 0.0)
            ivaprtn: Vaporization algorithm used for this gas (default: VaporizationAlgorithm.NONE)
            icomposition: Composition of the gas (default: GasComposition.NONE)
            dgc_threshold: Threshold for gas density gradient (default: 0.0)
            ds_threshold: Threshold for gas saturation (default: 0.0)
            refidx: Reference indices for gas (default: None)
        """
        self.name = name
        self.shortname = shortname
        self.wtmol = wtmol
        self.ivaprtn = ivaprtn
        self.icomposition = icomposition
        self.dgc_threshold = dgc_threshold
        self.ds_threshold = ds_threshold
        self.refidx = refidx or []

    def to_dict(self) -> Dict:
        """Convert to dictionary."""
        return {k: (v.value if isinstance(v, Enum) else v) for k, v in self.__dict__.items()}


class CARMACoagulationConfig:
    """Configuration for CARMA coagulation process.

    This class defines how particles coagulate in the CARMA model.
    """

    def __init__(self,
                 igroup1: int = 1,
                 igroup2: int = 1,
                 igroup3: int = 1,
                 algorithm: int = ParticleCollectionAlgorithm.CONSTANT,
                 ck0: float = 0.0,
                 grav_e_coll0: float = 0.0,
                 use_ccd: bool = False):
        """
        Initialize a CARMA coagulation configuration.

        Args:
            igroup1: First group index (default: 1)
            igroup2: Second group index (default: 1)
            igroup3: Third group index (default: 1)
            algorithm: Coagulation algorithm (default: ParticleCollectionAlgorithm.CONSTANT)
            ck0: Collection efficiency constant (default: 0.0)
            grav_e_coll0: Gravitational collection efficiency constant (default: 0.0)
            use_ccd: Whether to use constant collection efficiency data (default: False)
        """
        self.igroup1 = igroup1
        self.igroup2 = igroup2
        self.igroup3 = igroup3
        self.algorithm = algorithm
        self.ck0 = ck0
        self.grav_e_coll0 = grav_e_coll0
        self.use_ccd = use_ccd

    def to_dict(self) -> Dict:
        """Convert to dictionary."""
        return {k: (v.value if isinstance(v, Enum) else v) for k, v in self.__dict__.items()}


class CARMAGrowthConfig:
    """Configuration for CARMA particle growth process.

    This class defines how particles grow in the CARMA model.
    """

    def __init__(self,
                 ielem: int = 0,
                 igas: int = 0):
        """
        Initialize a CARMA growth configuration.

        Args:
            ielem: Element index for the particles (default: 0)
            igas: Index of the gas (default: 0)
        """
        self.ielem = ielem
        self.igas = igas

    def to_dict(self) -> Dict:
        """Convert to dictionary."""
        return {k: (v.value if isinstance(v, Enum) else v) for k, v in self.__dict__.items()}


class CARMANucleationConfig:
    """Configuration for CARMA particle nucleation process.

    This class defines how new particles are formed in the CARMA model.
    """

    def __init__(self,
                 ielemfrom: int = 0,
                 ielemto: int = 0,
                 algorithm: ParticleNucleationAlgorithm = ParticleNucleationAlgorithm.NONE,
                 rlh_nuc: float = 0.0,
                 igas: int = 0,
                 ievp2elem: int = 0):
        """
        Initialize a CARMA nucleation configuration.

        Args:
            ielemfrom: Element index to nucleate from (default: 0)
            ielemto: Element index to nucleate to (default: 0)
            algorithm: Nucleation algorithm (default: ParticleNucleationAlgorithm.NONE)
            rlh_nuc: Latent heat of nucleation [m2 s-2] (default: 0.0)
            igas: Gas index to nucleate from (default: 0)
            ievp2elem: Element index to evaporate to (if applicable) (default: 0)
        """
        self.ielemfrom = ielemfrom
        self.ielemto = ielemto
        self.algorithm = algorithm
        self.rlh_nuc = rlh_nuc
        self.igas = igas
        self.ievp2elem = ievp2elem

    def to_dict(self) -> Dict:
        """Convert to dictionary."""
        return {k: (v.value if isinstance(v, Enum) else v) for k, v in self.__dict__.items()}


class CARMAParameters:
    """
    Parameters for CARMA aerosol model simulation.

    This class encapsulates all the parameters needed to configure and run
    a CARMA simulation, including model dimensions, time stepping, and
    spatial parameters.
    """

    def __init__(self,
                 nz: int = 1,
                 nbin: int = 5,
                 dtime: float = 1800.0,
                 nstep: int = 100,
                 deltaz: float = 1000.0,
                 zmin: float = 16500.0,
                 wavelength_bins: Optional[List[CARMAWavelengthBin]] = None,
                 groups: Optional[List[CARMAGroupConfig]] = None,
                 elements: Optional[List[CARMAElementConfig]] = None,
                 solutes: Optional[List[CARMASoluteConfig]] = None,
                 gases: Optional[List[CARMAGasConfig]] = None,
                 coagulations: Optional[List[CARMACoagulationConfig]] = None,
                 growths: Optional[List[CARMAGrowthConfig]] = None,
                 nucleations: Optional[List[CARMANucleationConfig]] = None):
        """
        Initialize CARMA parameters.

        Args:
            nz: Number of vertical levels (default: 1)
            nbin: Number of size bins (default: 5)
            dtime: Time step in seconds (default: 1800.0)
            nstep: Number of time steps (default: 100)
            deltaz: Vertical grid spacing in meters (default: 1000.0)
            zmin: Minimum altitude in meters (default: 16500.0)
            wavelength_bins: List of CARMAWavelengthBin objects defining the wavelength grid (default: None)
            groups: List of group configurations (default: None)
            elements: List of element configurations (default: None)
            solutes: List of solute configurations (default: None)
            gases: List of gas configurations (default: None)
            coagulations: List of coagulation configurations (default: None)
            growths: List of growth configurations (default: None)
            nucleations: List of nucleation configurations (default: None)
        """
        self.nz = nz
        self.nbin = nbin
        self.dtime = dtime
        self.nstep = nstep
        self.deltaz = deltaz
        self.zmin = zmin

        # Initialize lists
        self.wavelength_bins = wavelength_bins or []
        self.groups = groups or []
        self.elements = elements or []
        self.solutes = solutes or []
        self.gases = gases or []
        self.coagulations = coagulations or []
        self.growths = growths or []
        self.nucleations = nucleations or []

    def add_wavelength_bin(self, wavelength_bin: CARMAWavelengthBin):
        """Add a wavelength bin configuration."""
        self.wavelength_bins.append(wavelength_bin)

    def add_group(self, group: CARMAGroupConfig):
        """Add a group configuration."""
        self.groups.append(group)

    def add_element(self, element: CARMAElementConfig):
        """Add an element configuration."""
        self.elements.append(element)

    def add_solute(self, solute: CARMASoluteConfig):
        """Add a solute configuration."""
        self.solutes.append(solute)

    def add_gas(self, gas: CARMAGasConfig):
        """Add a gas configuration."""
        self.gases.append(gas)

    def add_coagulation(self, coagulation: CARMACoagulationConfig):
        """Add a coagulation configuration."""
        self.coagulations.append(coagulation)

    def add_growth(self, growth: CARMAGrowthConfig):
        """Add a growth configuration."""
        self.growths.append(growth)

    def add_nucleation(self, nucleation: CARMANucleationConfig):
        """Add a nucleation configuration."""
        self.nucleations.append(nucleation)

    def __repr__(self):
        """String representation of CARMAParameters."""
        return (f"CARMAParameters(nz={self.nz}, "
                f"nbin={self.nbin}, dtime={self.dtime}, "
                f"nstep={self.nstep}, deltaz={self.deltaz}, zmin={self.zmin})")

    def __str__(self):
        """String representation of CARMAParameters."""
        return (f"CARMAParameters(nz={self.nz}, "
                f"nbin={self.nbin}, dtime={self.dtime}, "
                f"nstep={self.nstep}, deltaz={self.deltaz}, zmin={self.zmin})")

    def to_dict(self) -> Dict:
        """Convert parameters to dictionary for C++ interface."""
        # Get all basic attributes
        params_dict = {}
        for k, v in self.__dict__.items():
            if not k.startswith('__') and not callable(v):
                if k == 'groups':
                    params_dict[k] = [group.to_dict() for group in v]
                elif k == 'elements':
                    params_dict[k] = [element.to_dict() for element in v]
                elif k == 'solutes':
                    params_dict[k] = [solute.to_dict() for solute in v]
                elif k == 'gases':
                    params_dict[k] = [gas.to_dict() for gas in v]
                elif k == 'coagulations':
                    params_dict[k] = [coagulation.to_dict()
                                      for coagulation in v]
                elif k == 'growths':
                    params_dict[k] = [growth.to_dict() for growth in v]
                elif k == 'nucleations':
                    params_dict[k] = [nucleation.to_dict() for nucleation in v]
                elif k == 'wavelength_bins':
                    params_dict[k] = [bin.to_dict() for bin in v]
                else:
                    if isinstance(v, Enum):
                        params_dict[k] = v.value
                    else:
                        params_dict[k] = v
        return params_dict

    @classmethod
    def from_dict(cls, params_dict: Dict) -> 'CARMAParameters':
        """Create parameters from dictionary."""
        # Handle lists separately
        wavelength_bins = []
        if 'wavelength_bins' in params_dict:
            wavelength_bins = [CARMAWavelengthBin(**bin_dict)
                               for bin_dict in params_dict['wavelength_bins']]
            del params_dict['wavelength_bins']
        groups = []
        if 'groups' in params_dict:
            groups = [CARMAGroupConfig(**group_dict)
                      for group_dict in params_dict['groups']]
            del params_dict['groups']

        elements = []
        if 'elements' in params_dict:
            elements = [CARMAElementConfig(**element_dict)
                        for element_dict in params_dict['elements']]
            del params_dict['elements']

        solutes = []
        if 'solutes' in params_dict:
            solutes = [CARMASoluteConfig(**solute_dict)
                       for solute_dict in params_dict['solutes']]
            del params_dict['solutes']

        gases = []
        if 'gases' in params_dict:
            gases = [CARMAGasConfig(**gas_dict)
                     for gas_dict in params_dict['gases']]
            del params_dict['gases']

        coagulations = []
        if 'coagulations' in params_dict:
            coagulations = [CARMACoagulationConfig(**coag_dict)
                            for coag_dict in params_dict['coagulations']]
            del params_dict['coagulations']

        growths = []
        if 'growths' in params_dict:
            growths = [CARMAGrowthConfig(**growth_dict)
                       for growth_dict in params_dict['growths']]
            del params_dict['growths']

        nucleations = []
        if 'nucleations' in params_dict:
            nucleations = [CARMANucleationConfig(**nucleation_dict)
                           for nucleation_dict in params_dict['nucleations']]
            del params_dict['nucleations']

        return cls(
            wavelength_bins=wavelength_bins,
            groups=groups,
            elements=elements,
            solutes=solutes,
            gases=gases,
            coagulations=coagulations,
            growths=growths,
            nucleations=nucleations,
            **params_dict)

    @classmethod
    def create_aluminum_test_config(cls) -> 'CARMAParameters':
        """Create parameters for aluminum test configuration."""
        # Set up a wavelength grid
        wavelength_bins = [
            CARMAWavelengthBin(
                center=0.55e-6, width=0.01e-6, do_emission=True),
            CARMAWavelengthBin(
                center=0.65e-6, width=0.01e-6, do_emission=True),
            CARMAWavelengthBin(
                center=0.75e-6, width=0.01e-6, do_emission=True),
            CARMAWavelengthBin(
                center=0.85e-6, width=0.01e-6, do_emission=True),
            CARMAWavelengthBin(center=0.95e-6, width=0.01e-6, do_emission=True)
        ]

        # Create aluminum group
        group = CARMAGroupConfig(
            name="aluminum",
            shortname="PRALUM",
            rmin=21.5e-8,
            rmrat=2.0,
            ishape=ParticleShape.SPHERE,
            eshape=1.0,
            mie_calculation_algorithm=MieCalculationAlgorithm.TOON_1981,
            is_ice=False,
            is_fractal=True,
            do_wetdep=False,
            do_drydep=True,
            do_vtran=True,
            solfac=0.0,
            scavcoef=0.0,
            rmon=21.5e-8,
            df=[1.6] * 5,  # 5 bins with fractal dimension 1.6
            falpha=1.0
        )

        # Create aluminum element
        element = CARMAElementConfig(
            igroup=1,
            isolute=0,
            name="Aluminum",
            shortname="ALUM",
            itype=ParticleType.INVOLATILE,
            icomposition=ParticleComposition.ALUMINUM,
            is_shell=True,
            rho=2700.0,  # kg/m3
            arat=[1.0] * 5,  # 5 bins with area ratio 1.0
            kappa=0.0,
        )

        # Create coagulation
        coagulation = CARMACoagulationConfig(
            igroup1=1,
            igroup2=1,
            igroup3=1,
            algorithm=ParticleCollectionAlgorithm.FUCHS)

        params = cls(
            nz=1,
            nbin=5,
            deltaz=1000.0,
            zmin=16500.0,
            wavelength_bins=wavelength_bins,
            groups=[group],
            elements=[element],
            coagulations=[coagulation]
        )

        FIVE_DAYS_IN_SECONDS = 432000
        params.dtime = 1800.0
        params.nstep = FIVE_DAYS_IN_SECONDS / params.dtime

        return params


def _carma_dict_to_xarray(output_dict: Dict, parameters: 'CARMAParameters') -> xr.Dataset:
    """
    Convert CARMA output dictionary to xarray Dataset for netCDF export.

    This function creates an xarray Dataset with proper dimensions and coordinates
    following the structure used in the CARMA Fortran aluminum test.

    Args:
        output_dict: Dictionary containing CARMA output data from C++ backend
        parameters: The CARMAParameters object containing model configuration

    Returns:
        xr.Dataset: Dataset containing all CARMA output variables with proper
                   dimensions and metadata
    """
    # Extract dimensions from the parameters
    nz = parameters.nz
    nbin = parameters.nbin
    nelem = len(parameters.elements)
    ngroup = len(parameters.groups)
    ngas = len(parameters.gases)
    nstep = parameters.nstep

    # Create coordinates
    coords = {}

    # Spatial coordinates
    lat = output_dict.get('lat', [])
    lon = output_dict.get('lon', [])
    vertical_center = output_dict.get('vertical_center', [])
    vertical_levels = output_dict.get('vertical_levels', [])

    coords['lat'] = ('y', lat)
    coords['lon'] = ('x', lon)
    coords['z'] = ('z', vertical_center)
    coords['z_levels'] = ('z_levels', vertical_levels)

    # Bin coordinates (1-indexed like Fortran)
    coords['bin'] = ('bin', list(range(1, nbin + 1)))
    coords['group'] = ('group', list(range(1, ngroup + 1)))
    coords['elem'] = ('elem', list(range(1, nelem + 1)))
    coords['nwave'] = ('nwave', list(
        range(1, len(parameters.wavelength_bins) + 1)))

    # Create z_interface coordinate for variables defined at interfaces (nz+1 levels)
    coords['z_interface'] = ('z_interface', list(range(1, nz + 2)))

    data_vars = {}

    # Atmospheric state variables
    pressure = output_dict.get('pressure', [])
    data_vars['pressure'] = (
        'z', pressure, {'units': 'Pa', 'long_name': 'Pressure'})

    temperature = output_dict.get('temperature', [])
    data_vars['temperature'] = (
        'z', temperature, {'units': 'K', 'long_name': 'Temperature'})

    air_density = output_dict.get('air_density', [])
    data_vars['air_density'] = (
        'z', air_density, {'units': 'kg m-3', 'long_name': 'Air density'})

    # Particle state variables (3D: nz x nbin x nelem)
    particle_concentration = output_dict.get('particle_concentration', [])
    data_vars['particle_concentration'] = (
        ('z', 'bin', 'elem'), np.array(particle_concentration), {
            'units': '# cm-3', 'long_name': 'Particle concentration'})

    mass_mixing_ratio = output_dict.get('mass_mixing_ratio', [])
    data_vars['mass_mixing_ratio'] = (
        ('z', 'bin', 'elem'), np.array(mass_mixing_ratio), {
            'units': 'kg kg-1', 'long_name': 'Mass mixing ratio'})

    wet_radius = output_dict.get('wet_radius', [])
    data_vars['wet_radius'] = (
        ('z', 'bin', 'group'), np.array(wet_radius), {
            'units': 'cm', 'long_name': 'Wet radius of particles'})

    wet_density = output_dict.get('wet_density', [])
    data_vars['wet_density'] = (
        ('z', 'bin', 'group'), np.array(wet_density), {
            'units': 'g cm-3', 'long_name': 'Wet density of particles'})

    fall_velocity = output_dict.get('fall_velocity', [])
    data_vars['fall_velocity'] = (('z_interface', 'bin', 'group'), np.array(fall_velocity), {
                                  'units': 'cm s-1', 'long_name': 'Fall velocity of particles'})

    nucleation_rate = output_dict.get('nucleation_rate', [])
    data_vars['nucleation_rate'] = (
        ('z', 'bin', 'group'), np.array(nucleation_rate), {
            'units': 'cm-3 s-1', 'long_name': 'Nucleation rate of particles'})

    deposition_velocity = output_dict.get('deposition_velocity', [])
    data_vars['deposition_velocity'] = (
        ('z', 'bin', 'group'), np.array(deposition_velocity), {
            'units': 'cm s-1', 'long_name': 'Deposition velocity of particles'})

    dry_radius = output_dict.get('dry_radius', [])
    data_vars['dry_radius'] = (
        ('bin', 'group'), np.array(dry_radius), {
            'units': 'cm', 'long_name': 'Dry radius of particles'})

    mass_per_bin = output_dict.get('mass_per_bin', [])
    data_vars['mass_per_bin'] = (
        ('bin', 'group'), np.array(mass_per_bin), {
            'units': 'g', 'long_name': 'Mass per bin of particles'})

    radius_ratio = output_dict.get('radius_ratio', [])
    data_vars['radius_ratio'] = (
        ('bin', 'group'), np.array(radius_ratio), {
            'units': '1', 'long_name': 'Radius ratio of particles'})

    aspect_ratio = output_dict.get('aspect_ratio', [])
    data_vars['aspect_ratio'] = (
        ('bin', 'group'), np.array(aspect_ratio), {
            'units': '1', 'long_name': 'Aspect ratio of particles'})

    group_particle_number_concentration = output_dict.get(
        'group_particle_number_concentration', [])
    data_vars['group_particle_number_concentration'] = (
        ('group'), np.array(group_particle_number_concentration), {
            'units': '# cm-3', 'long_name': 'Group particle number concentration'})

    constituent_type = output_dict.get('constituent_type', [])
    data_vars['constituent_type'] = (
        ('group'), np.array(constituent_type), {
            'units': '1', 'long_name': 'Constituent type of particle groups'})

    max_prognostic_bin = output_dict.get('max_prognostic_bin', [])
    data_vars['max_prognostic_bin'] = (
        ('group'), np.array(max_prognostic_bin), {
            'units': '1', 'long_name': 'Maximum prognostic bin for each group'})

    # Create the dataset
    ds = xr.Dataset(
        data_vars=data_vars,
        coords=coords,
        attrs={
            'title': 'CARMA aerosol model output',
            'description': 'Output from CARMA aerosol simulation',
            'nz': nz,
            'ny': 1,  # TODO: replace this with the y dimensions corresponding to states or something
            'nx': 1,
            'nbin': nbin,
            'nelem': nelem,
            'ngroup': ngroup,
            'ngas': ngas,
            'nstep': nstep
        }
    )

    return ds


class CARMAState:
    """
    Represents the environmental variables used in CARMA simulations."""

    def __init__(self,
                 carma_pointer: c_void_p,
                 time: float = 0.0,
                 latitude: float = 0.0,
                 longitude: float = 0.0,
                 coordinates: CarmaCoordinates = CarmaCoordinates.CARTESIAN,
                 zmin: float = 0.0,
                 n_levels: int = 1,
                 delta_z: float = 1000.0,
                 temperature: Optional[List[float]] = None,
                 pressure: Optional[List[float]] = None,
                 pressure_levels: Optional[List[float]] = None,
                 ):
        """
        Initialize a CARMAState instance.

        Args:
            carma_pointer: Pointer to the CARMA C++ instance
            time: Simulation time in seconds (default: 0.0)
            latitude: Latitude in degrees (default: 0.0)
            longitude: Longitude in degrees (default: 0.0)
            coordinates: Coordinate system for the simulation (default: Cartesian)
            zmin: Minimum altitude in meters (default: 0.0)
            n_levels: Number of vertical levels (default: 1)
            delta_z: Vertical grid spacing in meters (default: 1000.0)
            temperature: Optional list of temperatures at vertical centers (default: None). If None, will be derived from the US Standard Atmosphere model.
            pressure: Optional list of pressures at vertical centers (default: None). If None, will be derived from the US Standard Atmosphere model.
            pressure_levels: Optional list of pressures at vertical levels (default: None). If None, will be derived from the US Standard Atmosphere model.
        """
        self.n_levels = n_levels
        vertical_center = zmin + (np.arange(n_levels) + 0.5) * delta_z
        vertical_levels = zmin + np.arange(n_levels + 1) * delta_z

        centered_variables = ussa1976.compute(
            z=vertical_center, variables=["t", "p", "rho"])
        edge_variables = ussa1976.compute(z=vertical_levels, variables=["p"])

        # Get standard atmosphere properties at these heights
        if temperature is None:
            temperature = centered_variables.t.values
        if pressure is None:
            pressure = centered_variables.p.values
        if pressure_levels is None:
            pressure_levels = edge_variables.p.values

        self._carma_state_instance = _backend._carma._create_carma_state(
            carma_pointer=carma_pointer,
            time=time,
            latitude=latitude,
            longitude=longitude,
            coordinates=coordinates.value,
            temperature=temperature,
            pressure=pressure,
            pressure_levels=pressure_levels,
            vertical_center=vertical_center.tolist(),
            vertical_levels=vertical_levels.tolist(),
        )
    
    def __del__(self):
        """Clean up the CARMAState instance."""
        if hasattr(self, '_carma_state_instance') and self._carma_state_instance is not None:
            _backend._carma._delete_carma_state(self._carma_state_instance)

    def __repr__(self):
        """String representation of CARMAState."""
        return (f"CARMAState")

    def __str__(self):
        """String representation of CARMAState."""
        return (f"CARMAState")

    def to_dict(self) -> Dict:
        """Convert CARMAState to dictionary."""
        return {k: v for k, v in self.__dict__.items() if not k.startswith('__') and not callable(v)}
    

    def set_bin(self, bin_index: int, element_index: int, value: Union[float, List[float]]):
        """
        Set the value for a specific bin and element.

        Args:
            bin_index: Index of the size bin (1-indexed)
            element_index: Index of the element (1-indexed)
            value: Value to set, can be a single float or a list of floats
        """
        if not isinstance(value, float):
            value = np.repeat(value, self.n_levels)
        print("Calling set_bin with value:", value)
        _backend._carma._set_bin(self._carma_state_instance, bin_index, element_index, value)


class CARMA:
    """
    A Python interface to the CARMA aerosol model.

    This class provides a simplified interface for running CARMA simulations
    with configurable parameters.
    """

    def __init__(self, parameters: CARMAParameters):
        """
        Initialize a CARMA instance.

        Raises:
            ValueError: If CARMA backend is not available
        """
        if not backend.carma_available():
            raise ValueError(
                "CARMA backend is not available on this platform.")

        self._carma_instance = _backend._carma._create_carma(
            parameters.to_dict())
        self.__parameters = parameters

    def __del__(self):
        """Clean up the CARMA instance."""
        if hasattr(self, '_carma_instance') and self._carma_instance is not None:
            _backend._carma._delete_carma(self._carma_instance)

    def __repr__(self):
        """String representation of CARMA instance."""
        return f"CARMA() - Version: {version if version else 'Not available'}"

    def __str__(self):
        """String representation of CARMA instance."""
        return f"CARMA() - Version: {version if version else 'Not available'}"

    def run(self) -> xr.Dataset:
        """
        Run the CARMA aerosol model simulation.

        Args:
            parameters: CARMAParameters instance containing simulation configuration

        Returns:
            xr.Dataset: Dataset containing all CARMA output variables with proper
                       dimensions and metadata

        Raises:
            ValueError: If the simulation fails
        """
        output_dict = _backend._carma._run_carma(self._carma_instance)

        # Convert dictionary directly to xarray Dataset
        return _carma_dict_to_xarray(output_dict, self.__parameters)

    def create_state(self, **kwargs) -> CARMAState:
        """
        Create a CARMAState instance based on the current parameters.

        Args:
            **kwargs: Additional keyword arguments to pass to CARMAState

        Returns:
            CARMAState: Instance containing environmental variables for the simulation
        """

        return CARMAState(
            self._carma_instance,
            zmin=self.__parameters.zmin,
            n_levels=self.__parameters.nz,
            delta_z=self.__parameters.deltaz,
            **kwargs
        )
