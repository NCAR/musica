"""
CARMA aerosol model Python interface.

This module provides a simplified Python interface to the CARMA aerosol model.
It allows users to create a CARMA instance and run simulations with specified parameters.

Note: CARMA is only available on macOS and Linux platforms.
"""

from typing import Dict, Optional, List, Union, Any, Tuple
from ctypes import c_void_p
import numpy as np
import xarray as xr
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


class SulfateNucleationMethod(Enum):
    """Enumeration for sulfate nucleation methods used in CARMA."""
    NONE = 0
    ZHAO_TURCO = 1
    VEHKAMAKI = 2


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
                 mie_calculation_algorithm: int = MieCalculationAlgorithm.TOON_1981,
                 optics_algorithm: int = OpticsAlgorithm.FIXED,
                 is_ice: bool = False,
                 is_fractal: bool = False,
                 is_cloud: bool = False,
                 is_sulfate: bool = False,
                 do_wetdep: bool = False,
                 do_drydep: bool = False,
                 do_vtran: bool = True,
                 solfac: float = 0.3,
                 scavcoef: float = 0.1,
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
            mie_calculation_algorithm: Algorithm for Mie calculations (default: TOON_1981)
            optics_algorithm: Algorithm for optics (default: FIXED)
            is_ice: Whether the particles are ice (default: False)
            is_fractal: Whether the particles are fractal (default: False)
            is_cloud: Whether the group is a cloud (default: False)
            is_sulfate: Whether the group is sulfate (default: False)
            do_wetdep: Whether to include wet deposition (default: False)
            do_drydep: Whether to include dry deposition (default: False)
            do_vtran: Whether to include vertical transport (default: True)
            solfac: Solubility factor for wet deposition (default: 0.3)
            scavcoef: Scavenging coefficient for wet deposition (default: 0.1)
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
                 ck0: float = -1.0,
                 grav_e_coll0: float = 0.0,
                 use_ccd: bool = False):
        """
        Initialize a CARMA coagulation configuration.

        Args:
            igroup1: First group index (default: 1)
            igroup2: Second group index (default: 1)
            igroup3: Third group index (default: 1)
            algorithm: Coagulation algorithm (default: ParticleCollectionAlgorithm.CONSTANT)
            ck0: Collection efficiency constant (default: -1.0). If -1.0, it will not be specified when setting up the coagulation process in carma
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


class CARMAInitializationConfig:
    """Configuration for CARMA initialization.

    This class defines how the CARMA model is initialized before running simulations.
    """

    def __init__(self,
                 do_cnst_rlh: bool = False,
                 do_detrain: bool = False,
                 do_fixedinit: bool = False,
                 do_incloud: bool = False,
                 do_explised: bool = False,
                 do_substep: bool = False,
                 do_thermo: bool = False,
                 do_vdiff: bool = False,
                 do_vtran: bool = False,
                 do_drydep: bool = False,
                 do_pheat: bool = False,
                 do_pheatatm: bool = False,
                 do_clearsky: bool = False,
                 do_partialinit: bool = False,
                 do_coremasscheck: bool = False,
                 sulfnucl_method: SulfateNucleationMethod = SulfateNucleationMethod.NONE,
                 vf_const: float = 0.0,
                 minsubsteps: int = 1,
                 maxsubsteps: int = 1,
                 maxretries: int = 5,
                 conmax: float = 1.0e-1,
                 dt_threshold: float = 0.0,
                 cstick: float = 1.0,
                 gsticki: float = 0.93,
                 gstickl: float = 1.0,
                 tstick: float = 1.0):
        """
        Initialize a CARMA initialization configuration.

        Args:
            do_cnst_rlh: Use constant values for latent heats (default: False)
            do_detrain: Do detrainment (default: False)
            do_fixedinit: Use fixed initialization from reference atmosphere (default: False)
            do_incloud: Do in-cloud processes (growth, coagulation) (default: False)
            do_explised: Do sedimentation with substepping (default: False)
            do_substep: Do substepping (default: False)
            do_thermo: Do thermodynamic processes (default: False)
            do_vdiff: Do Brownian diffusion (default: False)
            do_vtran: Do sedimentation (default: True)
            do_drydep: Do dry deposition (default: False)
            do_pheat: Do particle heating (default: False)
            do_pheatatm: Do particle heating of atmosphere (default: False)
            do_clearsky: Do clear sky growth and coagulation (default: False)
            do_partialinit: Do initialization of coagulation from reference atmosphere (requires do_fixedinit) (default: False)
            do_coremasscheck: Check core mass for particles (default: False)
            sulfnucl_method: Method for sulfate nucleation (default: SulfateNucleationMethod.NONE)
            vf_const: Constant fall velocity [m/s] (0: off) (default: 0.0)
            minsubsteps: Minimum number of substeps (default: 1)
            maxsubsteps: Maximum number of substeps (default: 1)
            maxretries: Maximum number of retries (default: 5)
            conmax: Minimum relative concentration to consider (default: 1.0e-1)
            dt_threshold: Convergence criteria for temperature [fraction] (0: off) (default: 0.0)
            cstick: Accommodation coefficient for coagulation (default: 1.0)
            gsticki: Accommodation coefficient for growth of ice (default: 0.93)
            gstickl: Accommodation coefficient for growth of liquid (default: 1.0)
            tstick: Accommodation coefficient temperature (default: 1.0)
        """
        self.do_cnst_rlh = do_cnst_rlh
        self.do_detrain = do_detrain
        self.do_fixedinit = do_fixedinit
        self.do_incloud = do_incloud
        self.do_explised = do_explised
        self.do_substep = do_substep
        self.do_thermo = do_thermo
        self.do_vdiff = do_vdiff
        self.do_vtran = do_vtran
        self.do_drydep = do_drydep
        self.do_pheat = do_pheat
        self.do_pheatatm = do_pheatatm
        self.do_clearsky = do_clearsky
        self.do_partialinit = do_partialinit
        self.do_coremasscheck = do_coremasscheck
        self.sulfnucl_method = sulfnucl_method
        self.vf_const = vf_const
        self.minsubsteps = minsubsteps
        self.maxsubsteps = maxsubsteps
        self.maxretries = maxretries
        self.conmax = conmax
        self.dt_threshold = dt_threshold
        self.cstick = cstick
        self.gsticki = gsticki
        self.gstickl = gstickl
        self.tstick = tstick

    def to_dict(self) -> Dict:
        """Convert to dictionary, converting Enums to ints."""
        return {k: (v.value if isinstance(v, Enum) else v) for k, v in self.__dict__.items()}


class CARMAParameters:
    """
    Parameters for CARMA aerosol model simulation.

    This class encapsulates all the parameters needed to configure and run
    a CARMA simulation, including model dimensions, time stepping, and
    spatial parameters.
    """

    def __init__(self,
                 nbin: int = 5,
                 nz: int = 1,
                 dtime: float = 1800.0,
                 wavelength_bins: Optional[List[CARMAWavelengthBin]] = None,
                 groups: Optional[List[CARMAGroupConfig]] = None,
                 elements: Optional[List[CARMAElementConfig]] = None,
                 solutes: Optional[List[CARMASoluteConfig]] = None,
                 gases: Optional[List[CARMAGasConfig]] = None,
                 coagulations: Optional[List[CARMACoagulationConfig]] = None,
                 growths: Optional[List[CARMAGrowthConfig]] = None,
                 nucleations: Optional[List[CARMANucleationConfig]] = None,
                 initialization: Optional[CARMAInitializationConfig] = None):
        """
        Initialize CARMA parameters.

        Args:
            nbin: Number of size bins (default: 5)
            nz: Number of vertical levels (default: 1)
            dtime: Time step in seconds (default: 1800.0)
            wavelength_bins: List of CARMAWavelengthBin objects defining the wavelength grid (default: None)
            groups: List of group configurations (default: None)
            elements: List of element configurations (default: None)
            solutes: List of solute configurations (default: None)
            gases: List of gas configurations (default: None)
            coagulations: List of coagulation configurations (default: None)
            growths: List of growth configurations (default: None)
            nucleations: List of nucleation configurations (default: None)
            initialization: Initialization configuration (default: None)
        """
        self.nbin = nbin
        self.dtime = dtime
        self.nz = nz

        # Initialize lists
        self.wavelength_bins = wavelength_bins or []
        self.groups = groups or []
        self.elements = elements or []
        self.solutes = solutes or []
        self.gases = gases or []
        self.coagulations = coagulations or []
        self.growths = growths or []
        self.nucleations = nucleations or []
        self.initialization = initialization or CARMAInitializationConfig()

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

    def set_initialization(self, initialization: CARMAInitializationConfig):
        """Set the initialization configuration."""
        self.initialization = initialization

    def __repr__(self):
        """String representation of CARMAParameters."""
        return (f"CARMAParameters(nbin={self.nbin}, dtime={self.dtime}, nz={self.nz}, "
                f"wavelength_bins={len(self.wavelength_bins)}, "
                f"groups={len(self.groups)}, elements={len(self.elements)}, "
                f"solutes={len(self.solutes)}, gases={len(self.gases)}, "
                f"coagulations={len(self.coagulations)}, growths={len(self.growths)}, "
                f"nucleations={len(self.nucleations)})")

    def __str__(self):
        """String representation of CARMAParameters."""
        return (f"CARMAParameters(nbin={self.nbin}, dtime={self.dtime}, nz={self.nz}, "
                f"wavelength_bins={len(self.wavelength_bins)}, "
                f"groups={len(self.groups)}, elements={len(self.elements)}, "
                f"solutes={len(self.solutes)}, gases={len(self.gases)}, "
                f"coagulations={len(self.coagulations)}, growths={len(self.growths)}, "
                f"nucleations={len(self.nucleations)})")

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
                elif k == 'initialization':
                    params_dict[k] = v.to_dict() if v else None
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

        initialization = None
        if 'initialization' in params_dict and params_dict['initialization']:
            initialization = CARMAInitializationConfig(
                **params_dict['initialization'])
            del params_dict['initialization']

        return cls(
            wavelength_bins=wavelength_bins,
            groups=groups,
            elements=elements,
            solutes=solutes,
            gases=gases,
            coagulations=coagulations,
            growths=growths,
            nucleations=nucleations,
            initialization=initialization,
            **params_dict)


class CARMASurfaceProperties:
    """
    Represents the surface properties used in CARMA simulations.

    This class encapsulates the surface properties such as friction velocity,
    aerodynamic resistance, and area fraction, which are used in CARMA simulations
    to model the interaction between the atmosphere and the surface.
    """

    def __init__(self,
                 surface_friction_velocity: float = 0.0,
                 aerodynamic_resistance: float = 0.0,
                 area_fraction: float = 0.0):
        """
        Initialize CARMASurfaceProperties instance.

        Args:
            surface_friction_velocity: Friction velocity at the surface [m/s] (default: 0.0)
            aerodynamic_resistance: Aerodynamic resistance at the surface [s/m] (default: 0.0)
            area_fraction: Area fraction of the surface [fraction] (default: 0.0)
        """
        self.surface_friction_velocity = surface_friction_velocity
        self.aerodynamic_resistance = aerodynamic_resistance
        self.area_fraction = area_fraction

    def __repr__(self):
        """Represent the surface properties as a string."""
        return (f"CARMASurfaceProperties("
                f"surface_friction_velocity={self.surface_friction_velocity}, "
                f"aerodynamic_resistance={self.aerodynamic_resistance}, "
                f"area_fraction={self.area_fraction})")

    def __str__(self):
        """String representation of surface properties."""
        return (f"Surface Friction Velocity: {self.surface_friction_velocity} m/s, "
                f"Aerodynamic Resistance: {self.aerodynamic_resistance} s/m, "
                f"Area Fraction: {self.area_fraction}")

    def to_dict(self) -> Dict:
        """Convert surface properties to dictionary."""
        return {
            'surface_friction_velocity': self.surface_friction_velocity,
            'aerodynamic_resistance': self.aerodynamic_resistance,
            'area_fraction': self.area_fraction
        }


class CARMAState:
    """
    Represents the environmental variables used in CARMA simulations."""

    def __init__(self,
                 carma_pointer: c_void_p,
                 vertical_center: List[float],
                 vertical_levels: List[float],
                 pressure: List[float],
                 pressure_levels: List[float],
                 temperature: List[float],
                 original_temperature: Optional[List[float]] = None,
                 relative_humidity: Optional[List[float]] = None,
                 specific_humidity: Optional[List[float]] = None,
                 radiative_intensity: Optional[List[List[float]]] = None,
                 time: float = 0.0,
                 time_step: float = 1.0,
                 latitude: float = 0.0,
                 longitude: float = 0.0,
                 coordinates: CarmaCoordinates = CarmaCoordinates.CARTESIAN,
                 gases: Optional[List[CARMAGasConfig]] = None,
                 ):
        """
        Initialize a CARMAState instance.

        Args:
            carma_pointer: Pointer to the CARMA C++ instance
            vertical_center: List of vertical center heights in meters
            vertical_levels: List of vertical levels in meters
            pressure: List of pressures at vertical centers in Pascals
            pressure_levels: List of pressures at vertical levels in Pascals
            temperature: List of temperatures at vertical centers in Kelvin
            original_temperature: List of original temperatures at vertical centers in Kelvin (default: None) If None, will use temperature
            relative_humidity: List of relative humidity at vertical centers in percent (default: None)
            specific_humidity: List of specific humidity at vertical centers in kg/kg (default: None)
            radiative_intensity: List of radiative intensity at vertical centers in W/m² (wavelength, vertical_center) (default: None)
            time: Simulation time in seconds (default: 0.0)
            time_step: Time step in seconds (default: 1.0)
            latitude: Latitude in degrees (default: 0.0)
            longitude: Longitude in degrees (default: 0.0)
            coordinates: Coordinate system for the simulation (default: Cartesian)
            gases: List of gas configurations
        """
        self.gases = gases or []
        self.longitude = longitude
        self.latitude = latitude
        self.coordinates = coordinates
        self.n_levels = len(vertical_center)
        if original_temperature is None:
            original_temperature = temperature
        self.vertical_center = vertical_center
        self.vertical_levels = vertical_levels

        self.dimensions = _backend._carma._get_dimensions(carma_pointer)
        self._carma_state_instance = _backend._carma._create_carma_state(
            carma_pointer=carma_pointer,
            time=time,
            time_step=time_step,
            latitude=latitude,
            longitude=longitude,
            coordinates=coordinates.value,
            temperature=temperature,
            original_temperature=original_temperature,
            pressure=pressure,
            pressure_levels=pressure_levels,
            vertical_center=vertical_center,
            vertical_levels=vertical_levels,
            relative_humidity=relative_humidity,
            specific_humidity=specific_humidity,
            radiative_intensity=radiative_intensity
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

    def set_bin(self,
                bin_index: int,
                element_index: int,
                value: Union[float, List[float]],
                surface_mass: Optional[float] = 0.0):
        """
        Set the value for a specific bin and element.

        Args:
            bin_index: Index of the size bin (1-indexed)
            element_index: Index of the element (1-indexed)
            value: Value to set, can be a single float or a list of floats
            surface_mass: Optional surface mass for the bin [kg m-2] (default: 0.0)
        """
        if np.isscalar(value):
            value = np.repeat(value, self.n_levels).tolist()
        elif not isinstance(value, list):
            # Ensure value is a list of floats with length n_levels
            if isinstance(value, np.ndarray):
                value = value.tolist()
        if not isinstance(value, list):
            # Convert to list if not already
            value = list(value)
        if len(value) != self.n_levels:
            raise ValueError(
                f"Value must be a scalar or a list of length {self.n_levels}, got length {len(value)}")
        if not all(isinstance(v, float) for v in value):
            raise ValueError("All elements in value must be floats")
        _backend._carma._set_bin(
            self._carma_state_instance, bin_index, element_index, value, surface_mass)

    def set_detrain(self, bin_index: int, element_index: int, value: float):
        """
        Set the mass of the detrained condensate for the bin

        Args:
            bin_index: Index of the size bin (1-indexed)
            element_index: Index of the element (1-indexed)
            value: Value to set
        """
        if np.isscalar(value):
            value = np.repeat(value, self.n_levels).tolist()
        elif not isinstance(value, list):
            value = list(value)
        _backend._carma._set_detrain(
            self._carma_state_instance, bin_index, element_index, value)

    def set_gas(self,
                gas_index: int,
                value: Union[float,
                             List[float]],
                old_mmr: Optional[List[float]] = None,
                gas_saturation_wrt_ice: Optional[List[float]] = None,
                gas_saturation_wrt_liquid: Optional[List[float]] = None):
        """
        Set the value for a specific gas.

        Args:
            gas_index: Index of the gas (1-indexed)
            value: Value to set, can be a single float or a list of floats
            old_mmr: Optional list of old mass mixing ratios for the gas (default: None)
            gas_saturation_wrt_ice: Optional list of gas saturation with respect to ice (default: None)
            gas_saturation_wrt_liquid: Optional list of gas saturation with respect to liquid (default: None)
        """
        if np.isscalar(value):
            value = np.repeat(value, self.n_levels).tolist()
        elif not isinstance(value, list):
            value = list(value)
        if old_mmr is None:
            old_mmr = []
        if gas_saturation_wrt_ice is None:
            gas_saturation_wrt_ice = []
        if gas_saturation_wrt_liquid is None:
            gas_saturation_wrt_liquid = []
        if not isinstance(old_mmr, list):
            old_mmr = list(old_mmr)
        if not isinstance(gas_saturation_wrt_ice, list):
            gas_saturation_wrt_ice = list(gas_saturation_wrt_ice)
        if not isinstance(gas_saturation_wrt_liquid, list):
            gas_saturation_wrt_liquid = list(gas_saturation_wrt_liquid)
        _backend._carma._set_gas(
            self._carma_state_instance,
            gas_index,
            value,
            old_mmr,
            gas_saturation_wrt_ice,
            gas_saturation_wrt_liquid)

    def get_step_statistics(self) -> Dict[str, Any]:
        """
        Get the step statistics for the current CARMAState.

        Returns:
            Dict[str, Any]: Dictionary containing step statistics such as
                            number of substeps, convergence status, etc.
        """
        return _backend._carma._get_step_statistics(self._carma_state_instance)

    def get_bins(self) -> xr.Dataset:
        """
        Get the CARMA aerosol state data for all bins and elements.

        Returns:
            Dataset: Aerosol bin properties for all bins and elements
        """

        # Collect bin data for each property into arrays
        # Shape: [number_of_bins, number_of_elements, n_levels] for vertical properties
        # and [number_of_bins, number_of_elements] for surface/level properties

        number_of_bins = self.dimensions["number_of_bins"]
        number_of_elements = self.dimensions["number_of_elements"]
        n_levels = len(self.vertical_center)
        n_edges = len(self.vertical_levels)

        # Initialize property arrays
        properties = [
            "mass_mixing_ratio",
            "number_mixing_ratio",
            "number_density",
            "nucleation_rate",
            "wet_particle_radius",
            "wet_particle_density",
            "dry_particle_density",
            "particle_mass_on_surface",
            "sedimentation_flux",
            "fall_velocity",
            "deposition_velocity",
            "delta_particle_temperature",
            "kappa",
            "total_mass_mixing_ratio"
        ]

        # Determine which properties are per vertical_center, per vertical_level, or per bin/element only
        per_vertical_center = [
            "mass_mixing_ratio",
            "number_mixing_ratio",
            "number_density",
            "nucleation_rate",
            "wet_particle_radius",
            "wet_particle_density",
            "dry_particle_density",
            "delta_particle_temperature",
            "kappa",
            "total_mass_mixing_ratio"
        ]
        per_vertical_level = [
            "fall_velocity"
        ]
        per_bin_element = [
            "particle_mass_on_surface",
            "sedimentation_flux",
            "deposition_velocity"
        ]

        # Prepare arrays
        data = {prop: [] for prop in properties}
        for i_bin in range(number_of_bins):
            for i_elem in range(number_of_elements):
                bin_value = _backend._carma._get_bin(
                    self._carma_state_instance,
                    i_bin + 1,
                    i_elem + 1
                )
                for prop in properties:
                    data[prop].append(bin_value.get(prop))

        # Reshape arrays
        def reshape(arr, shape):
            return np.array(arr).reshape(shape)

        # Helper for units and long names
        def _get_attributes(prop):
            attributes = {
                "mass_mixing_ratio": {
                    "units": "kg kg-1",
                    "long_name": "Aerosol particle mass mixing ratio"
                },
                "number_mixing_ratio": {
                    "units": "kg-1",
                    "long_name": "Aerosol particle number mixing ratio"
                },
                "number_density": {
                    "units": "m-3",
                    "long_name": "Aerosol particle number density"
                },
                "nucleation_rate": {
                    "units": "m-3 s-1",
                    "long_name": "Aerosol particle nucleation rate"
                },
                "wet_particle_radius": {
                    "units": "m",
                    "long_name": "Wet aerosol particle radius"
                },
                "wet_particle_density": {
                    "units": "kg m-3",
                    "long_name": "Wet aerosol particle density"
                },
                "dry_particle_density": {
                    "units": "kg m-3",
                    "long_name": "Dry aerosol particle density"
                },
                "particle_mass_on_surface": {
                    "units": "kg m-2",
                    "long_name": "Aerosol particle mass on surface"
                },
                "sedimentation_flux": {
                    "units": "kg m-2 s-1",
                    "long_name": "Aerosol particle sedimentation flux"
                },
                "fall_velocity": {
                    "units": "m s-1",
                    "long_name": "Aerosol particle fall velocity"
                },
                "deposition_velocity": {
                    "units": "m s-1",
                    "long_name": "Aerosol particle deposition velocity"
                },
                "delta_particle_temperature": {
                    "units": "K",
                    "long_name": "Aerosol particle temperature change"
                },
                "kappa": {
                    "units": "-",
                    "long_name": "Aerosol particle hygroscopicity parameter"
                },
                "total_mass_mixing_ratio": {
                    "units": "kg m-3",
                    "long_name": "Total aerosol particle mass mixing ratio"
                }
            }
            return attributes.get(prop, {"units": "", "long_name": ""})

        dataset_vars = {}

        # Per-vertical_center properties: shape (number_of_bins, number_of_elements, n_levels)
        for prop in per_vertical_center:
            arr = reshape(data[prop], (number_of_bins, number_of_elements, n_levels))
            dataset_vars[prop] = (("bin", "element", "vertical_center"), arr, _get_attributes(prop))

        # Per-vertical_level properties: shape (number_of_bins, number_of_elements, n_edges)
        for prop in per_vertical_level:
            arr = reshape(data[prop], (number_of_bins, number_of_elements, n_edges))
            dataset_vars[prop] = (("bin", "element", "vertical_level"), arr, _get_attributes(prop))

        # Per-bin/element only properties: shape (number_of_bins, number_of_elements)
        for prop in per_bin_element:
            arr = reshape(data[prop], (number_of_bins, number_of_elements))
            dataset_vars[prop] = (("bin", "element"), arr, _get_attributes(prop))

        return xr.Dataset(
            data_vars=dataset_vars,
            coords={
                "bin": np.arange(1, number_of_bins + 1),
                "element": np.arange(1, number_of_elements + 1),
                "vertical_center": self.vertical_center,
                "vertical_level": self.vertical_levels
            }
        )

    def get_detrained_masses(self) -> xr.Dataset:
        """
        Get the mass of the detrained condensate for the bin for each particle in the grid

        Returns:
            List[float]: Mass of the detrained condensate for the specified bin and element
        """

        number_of_bins = self.dimensions["number_of_bins"]
        number_of_elements = self.dimensions["number_of_elements"]
        n_levels = len(self.vertical_center)

        # Initialize property arrays
        properties = [
            "mass_mixing_ratio",
            "number_mixing_ratio",
            "number_density",
            "wet_particle_radius",
            "wet_particle_density"
        ]

        # Prepare arrays
        data = {prop: [] for prop in properties}
        for i_bin in range(number_of_bins):
            for i_elem in range(number_of_elements):
                bin_value = _backend._carma._get_detrain(
                    self._carma_state_instance,
                    i_bin + 1,
                    i_elem + 1
                )
                for prop in properties:
                    data[prop].append(bin_value.get(prop))

        # Reshape arrays
        def reshape(arr, shape):
            return np.array(arr).reshape(shape)

        # Helper for units
        def _get_units(prop):
            units = {
                "mass_mixing_ratio": "kg kg-1",
                "number_mixing_ratio": "kg-1",
                "number_density": "m-3",
                "wet_particle_radius": "m",
                "wet_particle_density": "kg m-3"
            }
            return units.get(prop, "")

        dataset_vars = {}

        # Per-vertical_center properties: shape (number_of_bins, number_of_elements, n_levels)
        for prop in properties:
            arr = reshape(data[prop], (number_of_bins, number_of_elements, n_levels))
            dataset_vars[prop] = (("bin", "element", "vertical_center"), arr, {"units": _get_units(prop)})

        return xr.Dataset(
            data_vars=dataset_vars,
            coords={
                "bin": np.arange(1, number_of_bins + 1),
                "element": np.arange(1, number_of_elements + 1),
                "vertical_center": self.vertical_center
            }
        )

    def get_gases(self) -> Tuple[xr.Dataset, Dict[str, int]]:
        """
        Get the values for all gases.

        Returns:
            Tuple[xr.Dataset, Dict[str, int]] A dataset containing values for all gases and a mapping of gas names to their indices.
        """
        number_of_gases = self.dimensions["number_of_gases"]
        n_levels = len(self.vertical_center)

        # Initialize property arrays
        properties = [
            "mass_mixing_ratio",
            "gas_saturation_wrt_ice",
            "gas_saturation_wrt_liquid",
            "gas_vapor_pressure_wrt_ice",
            "gas_vapor_pressure_wrt_liquid",
            "weight_pct_aerosol_composition"
        ]

        # Prepare arrays
        data = {prop: [] for prop in properties}
        for i_gas in range(number_of_gases):
            gas_value = _backend._carma._get_gas(
                self._carma_state_instance,
                i_gas + 1
            )
            for prop in properties:
                data[prop].append(gas_value.get(prop))

        # Reshape arrays
        def reshape(arr, shape):
            return np.array(arr).reshape(shape)

        # Helper for units
        def _get_units(prop):
            units = {
                "mass_mixing_ratio": "kg kg-1",
                "gas_saturation_wrt_ice": "none",
                "gas_saturation_wrt_liquid": "none",
                "gas_vapor_pressure_wrt_ice": "none",
                "gas_vapor_pressure_wrt_liquid": "none",
                "weight_pct_aerosol_composition": "none"
            }
            return units.get(prop, "")

        dataset_vars = {}

        # Per-vertical_center properties: shape (number_of_gases, n_levels)
        for prop in properties:
            arr = reshape(data[prop], (number_of_gases, n_levels))
            if prop == 'mass_mixing_ratio':
                dataset_vars[f'gas_{prop}'] = (("gas", "vertical_center"), arr, {"units": _get_units(prop)})
            else:
                dataset_vars[prop] = (("gas", "vertical_center"), arr, {"units": _get_units(prop)})

        return xr.Dataset(
            data_vars=dataset_vars,
            coords={
                "gas": [gas.shortname for gas in self.gases],
                "vertical_center": self.vertical_center
            }
        ), {gas.shortname: idx for idx, gas in enumerate(self.gases)}

    def get_environmental_values(self) -> xr.Dataset:
        """
        Get all environmental conditions for the current CARMAState.

        Returns:
            xr.Dataset: Dataset containing all environmental conditions
        """
        n_levels = len(self.vertical_center)

        data = _backend._carma._get_environmental_values(self._carma_state_instance)

        # Reshape arrays
        def reshape(arr, shape):
            return np.array(arr).reshape(shape)

        # Helper for units and long names
        def _get_attributes(prop):
            attributes = {
                "temperature": {
                    "units": "K",
                    "long_name": "Temperature"
                },
                "pressure": {
                    "units": "Pa",
                    "long_name": "Pressure"
                },
                "air_density": {
                    "units": "kg m-3",
                    "long_name": "Air density"
                },
                "latent_heat": {
                    "units": "K s-1",
                    "long_name": "Latent heat release rate"
                }
            }
            return attributes.get(prop, {"units": "", "long_name": ""})

        dataset_vars = {}

        for prop in data:
            if prop is not None:
                arr = reshape(data[prop], (n_levels,))
                dataset_vars[prop] = (("vertical_center"), arr, _get_attributes(prop))

        return xr.Dataset(
            data_vars=dataset_vars,
            coords={
                "vertical_center": self.vertical_center
            }
        )

    def set_temperature(self, temperature: Union[float, List[float]]):
        """
        Set the temperature for the vertical levels.

        Args:
            temperature: Temperature value to set, can be a single float or a list of floats
        """
        if np.isscalar(temperature):
            temperature = np.repeat(temperature, self.n_levels).tolist()
        elif not isinstance(temperature, list):
            temperature = list(temperature)
        _backend._carma._set_temperature(
            self._carma_state_instance, temperature)

    def set_air_density(self, air_density: Union[float, List[float]]):
        """
        Set the air density for the vertical levels.

        Args:
            air_density: Air density value to set, can be a single float or a list of floats
        """
        if np.isscalar(air_density):
            air_density = np.repeat(air_density, self.n_levels).tolist()
        elif not isinstance(air_density, list):
            air_density = list(air_density)
        _backend._carma._set_air_density(
            self._carma_state_instance, air_density)

    def step(
        self,
        cloud_fraction: Optional[List[float]] = None,
        critical_relative_humidity: Optional[List[float]] = None,
        land: Optional[CARMASurfaceProperties] = None,
        ocean: Optional[CARMASurfaceProperties] = None,
        ice: Optional[CARMASurfaceProperties] = None
    ):
        """
        Perform a single step in the CARMA simulation.

        Args:
            cloud_fraction: Optional list of cloud fractions for each vertical level (default: None)
            critical_relative_humidity: Optional list of critical relative humidities for each vertical level (default: None)
            land: Optional CARMASurfaceProperties instance representing land surface properties (default: None)
            ocean: Optional CARMASurfaceProperties instance representing ocean surface properties (default: None)
            ice: Optional CARMASurfaceProperties instance representing ice surface properties (default: None)
        """
        _backend._carma._step(
            self._carma_state_instance,
            cloud_fraction=cloud_fraction,
            critical_relative_humidity=critical_relative_humidity,
            land=land,
            ocean=ocean,
            ice=ice
        )


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
            gases=self.__parameters.gases,
            **kwargs
        )

    def get_group_properties(self) -> Tuple[xr.Dataset, Dict[str, int]]:
        """
        Get the group properties for all groups.

        Returns:
            Tuple[xr.Dataset, Dict[str, int]]: The group properties for all groups and a dictionary with their indices
        """
        number_of_groups = len(self.__parameters.groups)
        number_of_elements = len(self.__parameters.elements)
        number_of_bins = self.__parameters.nbin
        number_of_wavelength_bins = len(self.__parameters.wavelength_bins)

        if number_of_groups == 0:
            return xr.Dataset(), {}

        # Initialize the property arrays
        properties = [
            "bin_radius",
            "bin_radius_lower_bound",
            "bin_radius_upper_bound",
            "bin_width",
            "bin_mass",
            "bin_width_mass",
            "bin_volume",
            "projected_area_ratio",
            "radius_ratio",
            "porosity_ratio",
            "extinction_coefficient",
            "single_scattering_albedo",
            "asymmetry_factor",
            "element_index_of_core_mass_elements",
            "number_of_monomers_per_bin",
            "particle_number_element_for_group",
            "number_of_core_mass_elements_for_group",
            "last_prognostic_bin"
        ]

        # Group arrays by shape
        per_bin = [
            "bin_radius",
            "bin_radius_lower_bound",
            "bin_radius_upper_bound",
            "bin_width",
            "bin_mass",
            "bin_width_mass",
            "bin_volume",
            "projected_area_ratio",
            "radius_ratio",
            "porosity_ratio",
            "number_of_monomers_per_bin"
        ]
        per_bin_and_wavelength = [
            "extinction_coefficient",
            "single_scattering_albedo",
            "asymmetry_factor"
        ]
        per_element = [
            "element_index_of_core_mass_elements",
        ]
        per_group = [
            "particle_number_element_for_group",
            "number_of_core_mass_elements_for_group",
            "last_prognostic_bin"
        ]

        # Prepare arrays
        data = {prop: [] for prop in properties}
        for i_group in range(number_of_groups):
            group_value = _backend._carma._get_group_properties(self._carma_instance, i_group + 1)
            for prop in properties:
                data[prop].append(group_value.get(prop))

        # Reshape arrays
        def reshape(arr, shape):
            return np.array(arr).reshape(shape)

        # Helper for units
        def _get_units(prop):
            units = {
                "bin_radius": "m",
                "bin_radius_lower_bound": "m",
                "bin_radius_upper_bound": "m",
                "bin_width": "m",
                "bin_mass": "kg",
                "bin_width_mass": "kg",
                "bin_volume": "m^3",
                "projected_area_ratio": "-",
                "radius_ratio": "-",
                "porosity_ratio": "-",
                "extinction_coefficient": "-",
                "single_scattering_albedo": "-",
                "asymmetry_factor": "-",
                "element_index_of_core_mass_elements": "-",
                "number_of_monomers_per_bin": "-",
                "particle_number_element_for_group": "-",
                "number_of_core_mass_elements_for_group": "-",
                "last_prognostic_bin": "-"
            }
            return units.get(prop, "")

        dataset_vars = {}

        # Per-bin properties: shape (number_of_groups, number_of_bins)
        for prop in per_bin:
            arr = reshape(data[prop], (number_of_groups, number_of_bins))
            dataset_vars[prop] = (("group", "bin"), arr, {"units": _get_units(prop)})

        # Per-bin-and-wavelength properties: shape (number_of_groups, number_of_bins, number_of_wavelength_bins)
        for prop in per_bin_and_wavelength:
            arr = reshape(data[prop], (number_of_groups, number_of_bins, number_of_wavelength_bins))
            dataset_vars[prop] = (("group", "bin", "wavelength"), arr, {"units": _get_units(prop)})

        # Per-element properties: shape (number_of_groups, number_of_elements)
        for prop in per_element:
            arr = reshape(data[prop], (number_of_groups, number_of_elements))
            dataset_vars[prop] = (("group", "element"), arr, {"units": _get_units(prop)})

        # Per-group properties: shape (number_of_groups,)
        for prop in per_group:
            arr = reshape(data[prop], (number_of_groups,))
            dataset_vars[prop] = (("group",), arr, {"units": _get_units(prop)})

        return (xr.Dataset(
            data_vars=dataset_vars,
            coords={
                "group": np.arange(1, number_of_groups + 1),
                "bin": np.arange(1, number_of_bins + 1),
                "wavelength": np.arange(1, number_of_wavelength_bins + 1),
                "element": np.arange(1, number_of_elements + 1)
            }
        ), self.__parameters.groups)

    def get_element_properties(self) -> Tuple[xr.Dataset, Dict[str, int]]:
        """
        Get the element properties for all elements

        Returns:
            Tuple[xr.Dataset, Dict[str, Any]]: The properties for each element and a dictionary of element indices
        """
        number_of_elements = len(self.__parameters.elements)
        number_of_bins = self.__parameters.nbin
        number_of_wavelength_bins = len(self.__parameters.wavelength_bins)
        number_of_refractive_indices = None

        if number_of_elements == 0:
            return xr.Dataset(), {}

        # Initialize the property arrays
        properties = [
            "mass_density",
            "refractive_indices",
            "hygroscopicity_parameter"
        ]

        # Group arrays by shape
        per_bin = ["mass_density"]
        per_wavelength_and_refidx = ["refractive_indices"]
        per_element = ["hygroscopicity_parameter"]

        # Prepare arrays
        data = {prop: [] for prop in properties}
        for i_elem in range(number_of_elements):
            element_value = _backend._carma._get_element_properties(self._carma_instance, i_elem + 1)
            if number_of_refractive_indices is None:
                number_of_refractive_indices = element_value["number_of_refractive_indices"]
            else:
                if number_of_refractive_indices != element_value["number_of_refractive_indices"]:
                    raise ValueError("Inconsistent number of refractive indices found.")
            for prop in properties:
                data[prop].append(element_value.get(prop))

        # Reshape arrays
        def reshape(arr, shape):
            return np.array(arr).reshape(shape)

        # Helper for units
        def _get_units(prop):
            units = {
                "mass_density": "kg m-3",
                "refractive_indices": "-",
                "hygroscopicity_parameter": "-"
            }
            return units.get(prop, "")

        dataset_vars = {}

        # Per-bin properties: shape (number_of_elements, number_of_bins)
        for prop in per_bin:
            arr = reshape(data[prop], (number_of_elements, number_of_bins))
            dataset_vars[prop] = (("element", "bin"), arr, {"units": _get_units(prop)})

        # Per-wavelength and refractive index properties: shape
        # (number_of_elements, number_of_refractive_indices,
        # number_of_wavelength_bins)
        for prop in per_wavelength_and_refidx:
            arr = reshape(data[prop], (number_of_elements, number_of_refractive_indices, number_of_wavelength_bins))
            dataset_vars[prop] = (("element", "refractive_index", "wavelength"), arr, {"units": _get_units(prop)})

        # Per-element properties: shape (number_of_elements,)
        for prop in per_element:
            arr = reshape(data[prop], (number_of_elements,))
            dataset_vars[prop] = (("element",), arr, {"units": _get_units(prop)})

        return (xr.Dataset(
            data_vars=dataset_vars,
            coords={
                "bin": np.arange(1, number_of_bins + 1),
                "wavelength": np.arange(1, number_of_wavelength_bins + 1),
                "refractive_index": np.arange(1, number_of_refractive_indices + 1),
                "element": np.arange(1, number_of_elements + 1)
            }
        ), self.__parameters.elements)

    def get_gas_properties(self) -> CARMAGasConfig:
        """
        Get the gas properties for all gases.

        Returns:
            CARMAGasConfig: The gas configurations
        """
        return self.__parameters.gases

    def get_solute_properties(self) -> CARMASoluteConfig:
        """
        Get the solute properties for all solutes.

        Returns:
            CARMASoluteConfig: The solute configurations
        """
        return self.__parameters.solutes
