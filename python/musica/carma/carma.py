"""
CARMA aerosol model Python interface.

This module provides a simplified Python interface to the CARMA aerosol model.
It allows users to create a CARMA instance and run simulations with specified parameters.

Note: CARMA is only available on macOS and Linux platforms.
"""

from dataclasses import dataclass, field, fields, MISSING
from typing import Any, Callable, Dict, List, NamedTuple, Optional, Tuple, Union
import numpy as np
import xarray as xr
from enum import Enum
from .. import backend

_backend = backend.get_backend()


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


def _vector(values) -> Any:
    if values is None:
        return _backend.VectorDouble()
    return _backend.VectorDouble(np.asarray(values, dtype=float).ravel().tolist())


class _ShortNameIndex:
    _KINDS = {"groups": "group", "elements": "element", "solutes": "solute", "gases": "gas"}

    def __init__(self, parameters: "CARMAParameters"):
        self._indices = {}
        for kind, label in self._KINDS.items():
            names = [item.short_name for item in getattr(parameters, kind)]
            duplicates = sorted({name for name in names if names.count(name) > 1})
            if duplicates:
                raise ValueError(f"Each {label} must have a unique short_name. Duplicates: {duplicates}")
            self._indices[kind] = {name: index + 1 for index, name in enumerate(names)}

    def __call__(self, kind: str, short_name: Optional[str]) -> int:
        if short_name is None:
            return 0
        label = self._KINDS[kind]
        if not isinstance(short_name, str):
            raise TypeError(f"Reference a {label} by its short_name string, not by {type(short_name).__name__}")
        if short_name not in self._indices[kind]:
            raise ValueError(f"No {label} has short_name '{short_name}'. "
                             f"Available: {list(self._indices[kind])}")
        return self._indices[kind][short_name]


def _to_cpp_value(value, index: Optional[_ShortNameIndex] = None):
    if isinstance(value, Enum):
        return value.value
    if isinstance(value, _CARMAConfig):
        return value._to_cpp(index)
    if isinstance(value, (list, tuple)):
        return [_to_cpp_value(item, index) for item in value]
    return value


def _assign(cpp, name: str, value, index: Optional[_ShortNameIndex] = None):
    if value is None:
        return
    target = getattr(cpp, name)
    if isinstance(value, dict):
        for key, item in value.items():
            _assign(target, key, item, index)
    elif isinstance(target, _backend.VectorDouble):
        setattr(cpp, name, _vector(value))
    else:
        setattr(cpp, name, _to_cpp_value(value, index))


def _make_cpp(type_name: str, values: Dict[str, Any]):
    cpp = getattr(_backend._carma, type_name)()
    for name, value in values.items():
        _assign(cpp, name, value)
    return cpp


def _to_dict_value(value):
    if isinstance(value, Enum):
        return value.value
    if isinstance(value, _CARMAConfig):
        return value.to_dict()
    if isinstance(value, dict):
        return {key: _to_dict_value(item) for key, item in value.items()}
    if isinstance(value, list):
        return [_to_dict_value(item) for item in value]
    return value


def _to_cpp_complex(value):
    if isinstance(value, dict):
        return _backend._carma.CARMAComplex(value["real"], value["imaginary"])
    value = complex(value)
    return _backend._carma.CARMAComplex(value.real, value.imag)


def _refidx_to_cpp(refidx):
    return [[_to_cpp_complex(value) for value in row] for row in refidx]


def _refidx_field():
    return field(default_factory=list, metadata={"to_cpp": _refidx_to_cpp})


def _short_name_field():
    return field(metadata={"cpp": "shortname"})


def _reference_field(kind: str, cpp_name: str, optional: bool = False):
    if optional:
        return field(default=None, metadata={"cpp": cpp_name, "reference": kind})
    return field(metadata={"cpp": cpp_name, "reference": kind})


class _CARMAConfig:
    """Base class for CARMA configuration dataclasses.

    The dataclass fields are the only source of the default values. Each field maps to the
    attribute of the C++ structure with the same class name. The "cpp" metadata gives a
    different C++ attribute name. The "reference" metadata marks a field that holds the
    short_name of a group, element, solute, or gas. The conversion to C++ replaces the
    short_name with the 1-based index that CARMA uses.
    """

    def __post_init__(self):
        for f in fields(self):
            if getattr(self, f.name) is None and f.default_factory is not MISSING:
                setattr(self, f.name, f.default_factory())

    def to_dict(self) -> Dict:
        """Convert to a dictionary. Enum values become integers."""
        return {f.name: _to_dict_value(getattr(self, f.name)) for f in fields(self)}

    def _to_cpp(self, index: Optional[_ShortNameIndex] = None):
        cpp = getattr(_backend._carma, type(self).__name__)()
        for f in fields(self):
            value = getattr(self, f.name)
            if "reference" in f.metadata:
                if index is None:
                    raise ValueError(f"{type(self).__name__} must be part of a CARMAParameters object")
                value = index(f.metadata["reference"], value)
            elif value is not None and "to_cpp" in f.metadata:
                value = f.metadata["to_cpp"](value)
            _assign(cpp, f.metadata.get("cpp", f.name), value, index)
        return cpp


@dataclass
class CARMAWavelengthBin(_CARMAConfig):
    """Configuration for a CARMA wavelength bin.

    A CARMA wavelength bin represents a specific wavelength range used in optical calculations.

    Attributes:
        center: Center wavelength [m].
        width: Width of the wavelength bin [m].
        do_emission: Whether to include this wavelength in emission calculations (default: True).
    """
    center: float
    width: float
    do_emission: bool = True


@dataclass
class CARMAGroupConfig(_CARMAConfig):
    """Configuration for a CARMA particle group.

    A CARMA particle group represents a collection of particles with similar properties.

    Attributes:
        short_name: Unique short name. Other configurations use it to reference the group.
        name: Name of the group (default: "default_group")
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
        dpc_threshold: Convergence criteria for particle concentration [fraction] (default: 0.0)
        rmon: Monomer radius of fractal particles [m] (default: 0.0)
        df: List of fractal dimensions for each size bin (default: [])
        falpha: Fractal packing coefficient (default: 1.0)
        neutral_volfrc: Neutral volume fraction for fractal particles (default: 0.0)
    """
    short_name: str = _short_name_field()
    name: str = "default_group"
    rmin: float = 1e-9
    rmrat: float = 2.0
    rmassmin: float = 0.0
    ishape: ParticleShape = ParticleShape.SPHERE
    eshape: float = 1.0
    swelling_approach: Dict[str, Any] = field(default_factory=lambda: {
        "algorithm": ParticleSwellingAlgorithm.NONE,
        "composition": ParticleSwellingComposition.NONE
    })
    fall_velocity_routine: ParticleFallVelocityAlgorithm = ParticleFallVelocityAlgorithm.STANDARD_SPHERICAL_ONLY
    mie_calculation_algorithm: MieCalculationAlgorithm = MieCalculationAlgorithm.TOON_1981
    optics_algorithm: OpticsAlgorithm = OpticsAlgorithm.FIXED
    is_ice: bool = False
    is_fractal: bool = False
    is_cloud: bool = False
    is_sulfate: bool = False
    do_wetdep: bool = False
    do_drydep: bool = False
    do_vtran: bool = True
    solfac: float = 0.3
    scavcoef: float = 0.1
    dpc_threshold: float = 0.0
    rmon: float = 0.0
    df: List[float] = field(default_factory=list)
    falpha: float = 1.0
    neutral_volfrc: float = 0.0


@dataclass
class CARMAElementConfig(_CARMAConfig):
    """Configuration for a CARMA particle element.

    A CARMA particle element represents one of the components of a cloud or aerosol particle.

    Attributes:
        short_name: Unique short name. Other configurations use it to reference the element.
        group: Short name of the group this element belongs to
        solute: Short name of the solute of this element (default: None)
        name: Name of the element (default: "default_element")
        itype: Type of the particle (default: ParticleType.INVOLATILE)
        icomposition: Composition of the particle (default: ParticleComposition.OTHER)
        is_shell: For core/shell optics, whether this element is part of the shell (True) or core (False) (default: True)
        rho: Density of the element [kg m-3] (default: 1000.0)
        rhobin: List of densities for each size bin [kg m-3] (default: [])
        arat: List of area ratios for each size bin (default: [])
        kappa: Hygroscopicity parameter (default: 0.0)
        refidx: Refractive indices (n_refidx, n_wavelength) as complex numbers or
            dictionaries with "real" and "imaginary" keys (default: [])
    """
    short_name: str = _short_name_field()
    group: str = _reference_field("groups", "igroup")
    solute: Optional[str] = _reference_field("solutes", "isolute", optional=True)
    name: str = "default_element"
    itype: ParticleType = ParticleType.INVOLATILE
    icomposition: ParticleComposition = ParticleComposition.OTHER
    is_shell: bool = True
    rho: float = 1000.0
    rhobin: List[float] = field(default_factory=list)
    arat: List[float] = field(default_factory=list)
    kappa: float = 0.0
    refidx: List[List[Any]] = _refidx_field()


@dataclass
class CARMASoluteConfig(_CARMAConfig):
    """Configuration for a CARMA solute.

    A CARMA solute represents a chemical species that can dissolve in water and affect particle properties.

    Attributes:
        short_name: Unique short name. Other configurations use it to reference the solute.
        name: Name of the solute (default: "default_solute")
        ions: Number of ions (default: 0)
        wtmol: Molecular weight [kg mol-1] (default: 0.0)
        rho: Density [kg m-3] (default: 0.0)
    """
    short_name: str = _short_name_field()
    name: str = "default_solute"
    ions: int = 0
    wtmol: float = 0.0
    rho: float = 0.0


@dataclass
class CARMAGasConfig(_CARMAConfig):
    """Configuration for a CARMA gas.

    A CARMA gas represents a gaseous species in the atmosphere.

    Attributes:
        short_name: Unique short name. Other configurations use it to reference the gas.
        name: Name of the gas (default: "default_gas")
        wtmol: Molecular weight [kg mol-1] (default: 0.0)
        ivaprtn: Vaporization algorithm used for this gas (default: VaporizationAlgorithm.NONE)
        icomposition: Composition of the gas (default: GasComposition.NONE)
        dgc_threshold: Convergence criteria for gas concentration (default: 0.0)
        ds_threshold: Convergence criteria for gas saturation (default: 0.0)
        refidx: Refractive indices (n_refidx, n_wavelength) as complex numbers or
            dictionaries with "real" and "imaginary" keys (default: [])
    """
    short_name: str = _short_name_field()
    name: str = "default_gas"
    wtmol: float = 0.0
    ivaprtn: VaporizationAlgorithm = VaporizationAlgorithm.NONE
    icomposition: GasComposition = GasComposition.NONE
    dgc_threshold: float = 0.0
    ds_threshold: float = 0.0
    refidx: List[List[Any]] = _refidx_field()


@dataclass
class CARMACoagulationConfig(_CARMAConfig):
    """Configuration for CARMA coagulation process.

    This class defines how particles coagulate in the CARMA model.

    Attributes:
        group1: Short name of the first group that coagulates
        group2: Short name of the second group that coagulates
        group3: Short name of the group that receives the coagulated particles
        algorithm: Coagulation algorithm (default: ParticleCollectionAlgorithm.CONSTANT)
        ck0: Collection efficiency constant (default: -1.0). If -1.0, it will not be specified
            when setting up the coagulation process in carma
        grav_e_coll0: Gravitational collection efficiency constant (default: 0.0)
        use_ccd: Whether to use constant collection efficiency data (default: False)
    """
    group1: str = _reference_field("groups", "igroup1")
    group2: str = _reference_field("groups", "igroup2")
    group3: str = _reference_field("groups", "igroup3")
    algorithm: ParticleCollectionAlgorithm = ParticleCollectionAlgorithm.CONSTANT
    ck0: float = -1.0
    grav_e_coll0: float = 0.0
    use_ccd: bool = False


@dataclass
class CARMAGrowthConfig(_CARMAConfig):
    """Configuration for CARMA particle growth process.

    This class defines how particles grow in the CARMA model.

    Attributes:
        element: Short name of the element that grows
        gas: Short name of the gas that condenses onto the element
    """
    element: str = _reference_field("elements", "ielem")
    gas: str = _reference_field("gases", "igas")


@dataclass
class CARMANucleationConfig(_CARMAConfig):
    """Configuration for CARMA particle nucleation process.

    This class defines how new particles are formed in the CARMA model.

    Attributes:
        element_from: Short name of the element to nucleate from
        element_to: Short name of the element to nucleate to
        algorithm: Nucleation algorithm (default: ParticleNucleationAlgorithm.NONE)
        rlh_nuc: Latent heat of nucleation [m2 s-2] (default: 0.0)
        gas: Short name of the gas to nucleate from (default: None)
        evaporation_element: Short name of the element to evaporate to, if applicable (default: None)
    """
    element_from: str = _reference_field("elements", "ielemfrom")
    element_to: str = _reference_field("elements", "ielemto")
    algorithm: ParticleNucleationAlgorithm = ParticleNucleationAlgorithm.NONE
    rlh_nuc: float = 0.0
    gas: Optional[str] = _reference_field("gases", "igas", optional=True)
    evaporation_element: Optional[str] = _reference_field("elements", "ievp2elem", optional=True)


@dataclass
class CARMAInitializationConfig(_CARMAConfig):
    """Configuration for CARMA initialization.

    This class defines how the CARMA model is initialized before running simulations.

    Attributes:
        do_cnst_rlh: Use constant values for latent heats (default: False)
        do_detrain: Do detrainment (default: False)
        do_fixedinit: Use fixed initialization from reference atmosphere (default: False)
        do_incloud: Do in-cloud processes (growth, coagulation) (default: False)
        do_explised: Do sedimentation with substepping (default: False)
        do_substep: Do substepping (default: False)
        do_thermo: Do thermodynamic processes (default: False)
        do_vdiff: Do Brownian diffusion (default: False)
        do_vtran: Do sedimentation (default: False)
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
    do_cnst_rlh: bool = False
    do_detrain: bool = False
    do_fixedinit: bool = False
    do_incloud: bool = False
    do_explised: bool = False
    do_substep: bool = False
    do_thermo: bool = False
    do_vdiff: bool = False
    do_vtran: bool = False
    do_drydep: bool = False
    do_pheat: bool = False
    do_pheatatm: bool = False
    do_clearsky: bool = False
    do_partialinit: bool = False
    do_coremasscheck: bool = False
    sulfnucl_method: SulfateNucleationMethod = SulfateNucleationMethod.NONE
    vf_const: float = 0.0
    minsubsteps: int = 1
    maxsubsteps: int = 1
    maxretries: int = 5
    conmax: float = 1.0e-1
    dt_threshold: float = 0.0
    cstick: float = 1.0
    gsticki: float = 0.93
    gstickl: float = 1.0
    tstick: float = 1.0


@dataclass(repr=False)
class CARMAParameters(_CARMAConfig):
    """
    Parameters for CARMA aerosol model simulation.

    This class encapsulates all the parameters needed to configure a CARMA
    simulation. The vertical grid and the time step are not CARMA parameters.
    CARMA.create_state() sets them: the length of vertical_center sets the
    number of vertical levels, and time_step sets the time step.

    Groups, elements, solutes, and gases each have a unique short_name. The
    process configurations and the CARMAState setters use these short names.

    Process order:
        CARMA applies the processes in a fixed order in each time step. The order
        of the add_coagulation(), add_growth(), and add_nucleation() calls has no
        effect. In each step, CARMA does these operations:

        1. Vertical transport, when initialization.do_vtran is True.
        2. Coagulation, when there is a coagulation configuration.
        3. Nucleation, growth, and evaporation, when there is a growth or
           nucleation configuration.

    Attributes:
        nbin: Number of size bins (default: 5)
        wavelength_bins: List of CARMAWavelengthBin objects defining the wavelength grid (default: [])
        groups: List of group configurations (default: [])
        elements: List of element configurations (default: [])
        solutes: List of solute configurations (default: [])
        gases: List of gas configurations (default: [])
        coagulations: List of coagulation configurations (default: [])
        growths: List of growth configurations (default: [])
        nucleations: List of nucleation configurations (default: [])
        initialization: Initialization configuration (default: CARMAInitializationConfig())
    """
    nbin: int = 5
    wavelength_bins: List[CARMAWavelengthBin] = field(default_factory=list)
    groups: List[CARMAGroupConfig] = field(default_factory=list)
    elements: List[CARMAElementConfig] = field(default_factory=list)
    solutes: List[CARMASoluteConfig] = field(default_factory=list)
    gases: List[CARMAGasConfig] = field(default_factory=list)
    coagulations: List[CARMACoagulationConfig] = field(default_factory=list)
    growths: List[CARMAGrowthConfig] = field(default_factory=list)
    nucleations: List[CARMANucleationConfig] = field(default_factory=list)
    initialization: CARMAInitializationConfig = field(default_factory=CARMAInitializationConfig)

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
        """Add a coagulation configuration. See the class documentation for the process order."""
        self.coagulations.append(coagulation)

    def add_growth(self, growth: CARMAGrowthConfig):
        """Add a growth configuration. See the class documentation for the process order."""
        self.growths.append(growth)

    def add_nucleation(self, nucleation: CARMANucleationConfig):
        """Add a nucleation configuration. See the class documentation for the process order."""
        self.nucleations.append(nucleation)

    def set_initialization(self, initialization: CARMAInitializationConfig):
        """Set the initialization configuration."""
        self.initialization = initialization

    def __repr__(self):
        """String representation of CARMAParameters."""
        return (f"CARMAParameters(nbin={self.nbin}, "
                f"wavelength_bins={len(self.wavelength_bins)}, "
                f"groups={len(self.groups)}, elements={len(self.elements)}, "
                f"solutes={len(self.solutes)}, gases={len(self.gases)}, "
                f"coagulations={len(self.coagulations)}, growths={len(self.growths)}, "
                f"nucleations={len(self.nucleations)})")

    def _to_cpp(self, index: Optional[_ShortNameIndex] = None):
        return super()._to_cpp(index or _ShortNameIndex(self))

    @classmethod
    def from_dict(cls, params_dict: Dict) -> 'CARMAParameters':
        """Create parameters from dictionary."""
        list_types = {
            "wavelength_bins": CARMAWavelengthBin,
            "groups": CARMAGroupConfig,
            "elements": CARMAElementConfig,
            "solutes": CARMASoluteConfig,
            "gases": CARMAGasConfig,
            "coagulations": CARMACoagulationConfig,
            "growths": CARMAGrowthConfig,
            "nucleations": CARMANucleationConfig,
        }
        kwargs = dict(params_dict)
        for key, item_type in list_types.items():
            if key in kwargs:
                kwargs[key] = [item_type(**item) for item in kwargs[key]]
        if kwargs.get("initialization"):
            kwargs["initialization"] = CARMAInitializationConfig(**kwargs["initialization"])
        return cls(**kwargs)


@dataclass(repr=False)
class CARMASurfaceProperties(_CARMAConfig):
    """
    Represents the surface properties used in CARMA simulations.

    This class encapsulates the surface properties such as friction velocity,
    aerodynamic resistance, and area fraction, which are used in CARMA simulations
    to model the interaction between the atmosphere and the surface.

    Attributes:
        surface_friction_velocity: Friction velocity at the surface [m/s] (default: 0.0)
        aerodynamic_resistance: Aerodynamic resistance at the surface [s/m] (default: 0.0)
        area_fraction: Area fraction of the surface [fraction] (default: 0.0)
    """
    surface_friction_velocity: float = 0.0
    aerodynamic_resistance: float = 0.0
    area_fraction: float = 0.0

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


class _Variable(NamedTuple):
    dims: Tuple[str, ...]
    units: str
    long_name: Optional[str] = None
    source: Optional[str] = None
    convert: Optional[Callable[[Any], Any]] = None

    @property
    def attrs(self) -> Dict[str, str]:
        if self.long_name is None:
            return {"units": self.units}
        return {"units": self.units, "long_name": self.long_name}


def _none_if_unset(values):
    return None if all(value == -1 for value in values) else values


def _to_complex_list(values):
    return [complex(value.real, value.imaginary) for value in values]


def _is_enabled(value) -> bool:
    return (value.value if isinstance(value, Enum) else value) != 0


def _mask(dataset: xr.Dataset, names: Tuple[str, ...], dim: str, keep: List[bool]):
    keep = xr.DataArray(np.asarray(keep, dtype=bool), dims=dim)
    for name in names:
        dataset[name] = dataset[name].where(keep)


def _build_dataset(records: List[Any],
                   leading_dims: Tuple[str, ...],
                   variables: Dict[str, _Variable],
                   coords: Dict[str, Any]) -> xr.Dataset:
    data_vars = {}
    for name, variable in variables.items():
        values = [getattr(record, variable.source or name) for record in records]
        if variable.convert is not None:
            values = [variable.convert(value) for value in values]
        dims = leading_dims + variable.dims
        shape = tuple(len(coords[dim]) for dim in dims)
        data_vars[name] = (dims, np.array(values).reshape(shape), variable.attrs)
    return xr.Dataset(data_vars=data_vars, coords=coords)


_BIN_VARIABLES = {
    "mass_mixing_ratio": _Variable(("vertical_center",), "kg kg-1", "Aerosol particle mass mixing ratio"),
    "number_mixing_ratio": _Variable(("vertical_center",), "kg-1", "Aerosol particle number mixing ratio"),
    "number_density": _Variable(("vertical_center",), "m-3", "Aerosol particle number density"),
    "nucleation_rate": _Variable(("vertical_center",), "m-3 s-1", "Aerosol particle nucleation rate"),
    "wet_particle_radius": _Variable(("vertical_center",), "m", "Wet aerosol particle radius"),
    "wet_particle_density": _Variable(("vertical_center",), "kg m-3", "Wet aerosol particle density"),
    "dry_particle_density": _Variable(("vertical_center",), "kg m-3", "Dry aerosol particle density"),
    "delta_particle_temperature": _Variable(("vertical_center",), "K", "Aerosol particle temperature change"),
    "kappa": _Variable(("vertical_center",), "-", "Aerosol particle hygroscopicity parameter"),
    "total_mass_mixing_ratio": _Variable(("vertical_center",), "kg m-3", "Total aerosol particle mass mixing ratio"),
    "fall_velocity": _Variable(("vertical_level",), "m s-1", "Aerosol particle fall velocity"),
    "particle_mass_on_surface": _Variable((), "kg m-2", "Aerosol particle mass on surface"),
    "sedimentation_flux": _Variable((), "kg m-2 s-1", "Aerosol particle sedimentation flux"),
    "deposition_velocity": _Variable((), "m s-1", "Aerosol particle deposition velocity"),
}

_DETRAIN_VARIABLES = {
    "mass_mixing_ratio": _Variable(("vertical_center",), "kg kg-1"),
    "number_mixing_ratio": _Variable(("vertical_center",), "kg-1"),
    "number_density": _Variable(("vertical_center",), "m-3"),
    "wet_particle_radius": _Variable(("vertical_center",), "m"),
    "wet_particle_density": _Variable(("vertical_center",), "kg m-3"),
}

_GAS_VARIABLES = {
    "gas_mass_mixing_ratio": _Variable(("vertical_center",), "kg kg-1", source="mass_mixing_ratio"),
    "gas_saturation_wrt_ice": _Variable(("vertical_center",), "none"),
    "gas_saturation_wrt_liquid": _Variable(("vertical_center",), "none"),
    "gas_vapor_pressure_wrt_ice": _Variable(("vertical_center",), "none"),
    "gas_vapor_pressure_wrt_liquid": _Variable(("vertical_center",), "none"),
    "weight_pct_aerosol_composition": _Variable(("vertical_center",), "none"),
}

_ENVIRONMENTAL_VARIABLES = {
    "temperature": _Variable(("vertical_center",), "K", "Temperature"),
    "pressure": _Variable(("vertical_center",), "Pa", "Pressure"),
    "air_density": _Variable(("vertical_center",), "kg m-3", "Air density"),
    "latent_heat": _Variable(("vertical_center",), "K s-1", "Latent heat release rate"),
}

_GROUP_VARIABLES = {
    "bin_radius": _Variable(("bin",), "m"),
    "bin_radius_lower_bound": _Variable(("bin",), "m"),
    "bin_radius_upper_bound": _Variable(("bin",), "m"),
    "bin_width": _Variable(("bin",), "m"),
    "bin_mass": _Variable(("bin",), "kg"),
    "bin_width_mass": _Variable(("bin",), "kg"),
    "bin_volume": _Variable(("bin",), "m^3"),
    "projected_area_ratio": _Variable(("bin",), "-"),
    "radius_ratio": _Variable(("bin",), "-"),
    "porosity_ratio": _Variable(("bin",), "-"),
    "number_of_monomers_per_bin": _Variable(("bin",), "-"),
    "extinction_coefficient": _Variable(("bin", "wavelength"), "-"),
    "single_scattering_albedo": _Variable(("bin", "wavelength"), "-"),
    "asymmetry_factor": _Variable(("bin", "wavelength"), "-"),
    "element_index_of_core_mass_elements": _Variable(("element",), "-"),
    "particle_number_element_for_group": _Variable((), "-"),
    "number_of_core_mass_elements_for_group": _Variable((), "-"),
    "last_prognostic_bin": _Variable((), "-"),
}

_ELEMENT_VARIABLES = {
    "mass_density": _Variable(("bin",), "kg m-3", source="rho"),
    "refractive_indices": _Variable(("refractive_index", "wavelength"), "-",
                                    source="refidx", convert=_to_complex_list),
    "hygroscopicity_parameter": _Variable((), "-", source="kappa"),
}


class CARMAState:
    """
    Represents the environmental variables used in CARMA simulations."""

    def __init__(self,
                 carma_instance: Any,
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
                 parameters: Optional[CARMAParameters] = None,
                 ):
        """
        Initialize a CARMAState instance.

        Args:
            carma_instance: The C++ CARMA instance
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
            parameters: The CARMA parameters used to create the CARMA instance
        """
        parameters = parameters or CARMAParameters()
        self._parameters = parameters
        self._index = _ShortNameIndex(parameters)
        self.gases = parameters.gases
        self.longitude = longitude
        self.latitude = latitude
        self.coordinates = coordinates
        self.n_levels = len(vertical_center)
        if original_temperature is None:
            original_temperature = temperature
        self.vertical_center = vertical_center
        self.vertical_levels = vertical_levels
        self.dimensions = {
            "number_of_bins": parameters.nbin,
            "number_of_vertical_levels": self.n_levels,
            "number_of_wavelength_bins": len(parameters.wavelength_bins),
            "number_of_refractive_indices": 0,
            "number_of_groups": len(parameters.groups),
            "number_of_elements": len(parameters.elements),
            "number_of_solutes": len(parameters.solutes),
            "number_of_gases": len(parameters.gases),
        }

        state_values = {
            "time": time,
            "time_step": time_step,
            "latitude": latitude,
            "longitude": longitude,
            "coordinates": coordinates,
            "temperature": temperature,
            "original_temperature": original_temperature,
            "pressure": pressure,
            "pressure_levels": pressure_levels,
            "vertical_center": vertical_center,
            "vertical_levels": vertical_levels,
            "relative_humidity": relative_humidity,
            "specific_humidity": specific_humidity,
        }
        if radiative_intensity is not None:
            radiative_intensity = np.asarray(radiative_intensity, dtype=float)
            if radiative_intensity.ndim != 2:
                raise ValueError("Expected 2D array for radiative_intensity")
            state_values["radiative_intensity"] = radiative_intensity
            state_values["radiative_intensity_dim_1_size"] = radiative_intensity.shape[0]
            state_values["radiative_intensity_dim_2_size"] = radiative_intensity.shape[1]

        self._cpp = _backend._carma.CARMAState(carma_instance, _make_cpp("CARMAStateParameters", state_values))

    def __repr__(self):
        """String representation of CARMAState."""
        return (f"CARMAState")

    def __str__(self):
        """String representation of CARMAState."""
        return (f"CARMAState")

    def to_dict(self) -> Dict:
        """Convert CARMAState to dictionary."""
        return {k: v for k, v in self.__dict__.items() if not k.startswith('_') and not callable(v)}

    def _column(self, value: Union[float, List[float]]) -> List[float]:
        if np.isscalar(value):
            return np.repeat(value, self.n_levels).tolist()
        return list(value)

    def set_bin(self,
                bin_index: int,
                element: str,
                value: Union[float, List[float]],
                surface_mass: Optional[float] = 0.0):
        """
        Set the value for a specific bin and element.

        Args:
            bin_index: Index of the size bin (1-indexed)
            element: Short name of the element
            value: Value to set, can be a single float or a list of floats
            surface_mass: Optional surface mass for the bin [kg m-2] (default: 0.0)
        """
        value = self._column(value)
        if len(value) != self.n_levels:
            raise ValueError(
                f"Value must be a scalar or a list of length {self.n_levels}, got length {len(value)}")
        if not all(isinstance(v, float) for v in value):
            raise ValueError("All elements in value must be floats")
        self._cpp.set_bin(bin_index, self._index("elements", element), _vector(value), surface_mass)

    def set_detrain(self, bin_index: int, element: str, value: float):
        """
        Set the mass of the detrained condensate for the bin

        Args:
            bin_index: Index of the size bin (1-indexed)
            element: Short name of the element
            value: Value to set
        """
        self._cpp.set_detrain(bin_index, self._index("elements", element), _vector(self._column(value)))

    def set_gas(self,
                gas: str,
                value: Union[float,
                             List[float]],
                old_mmr: Optional[List[float]] = None,
                gas_saturation_wrt_ice: Optional[List[float]] = None,
                gas_saturation_wrt_liquid: Optional[List[float]] = None):
        """
        Set the value for a specific gas.

        Args:
            gas: Short name of the gas
            value: Value to set, can be a single float or a list of floats
            old_mmr: Optional list of old mass mixing ratios for the gas (default: None)
            gas_saturation_wrt_ice: Optional list of gas saturation with respect to ice (default: None)
            gas_saturation_wrt_liquid: Optional list of gas saturation with respect to liquid (default: None)
        """
        self._cpp.set_gas(
            self._index("gases", gas),
            _vector(self._column(value)),
            _vector(old_mmr),
            _vector(gas_saturation_wrt_ice),
            _vector(gas_saturation_wrt_liquid))

    def get_step_statistics(self) -> Dict[str, Any]:
        """
        Get the step statistics for the current CARMAState.

        Returns:
            Dict[str, Any]: Dictionary containing step statistics such as
                            number of substeps, convergence status, etc.
        """
        stats = self._cpp.get_step_statistics()
        return {
            "max_number_of_substeps": stats.max_number_of_substeps,
            "max_number_of_retries": stats.max_number_of_retries,
            "total_number_of_steps": stats.total_number_of_steps,
            "total_number_of_substeps": stats.total_number_of_substeps,
            "total_number_of_retries": stats.total_number_of_retries,
            "z_substeps": _none_if_unset(stats.z_substeps),
            "xc": stats.xc,
            "yc": stats.yc,
        }

    def _bin_element_records(self, getter: Callable[[int, int], Any]) -> List[Any]:
        return [getter(i_bin + 1, i_elem + 1)
                for i_bin in range(self.dimensions["number_of_bins"])
                for i_elem in range(self.dimensions["number_of_elements"])]

    def _bin_element_coords(self) -> Dict[str, Any]:
        return {
            "bin": np.arange(1, self.dimensions["number_of_bins"] + 1),
            "element": [element.short_name for element in self._parameters.elements],
            "vertical_center": self.vertical_center,
        }

    def get_bins(self) -> xr.Dataset:
        """
        Get the CARMA aerosol state data for all bins and elements.

        A field is NaN when the physics for the field is off, or when CARMA does not
        calculate the field for the element:

        - fall_velocity and sedimentation_flux, when do_vtran is False for the run or for the group.
        - deposition_velocity and particle_mass_on_surface, when do_drydep is False for the run or for the group.
        - nucleation_rate, for an element that no nucleation process targets.
        - delta_particle_temperature, when do_pheat is False.
        - The number, radius, density, and velocity fields, for an element that is not
          the particle number element of its group.

        Returns:
            Dataset: Aerosol bin properties for all bins and elements
        """
        dataset = _build_dataset(
            self._bin_element_records(self._cpp.get_bin_values),
            ("bin", "element"),
            _BIN_VARIABLES,
            {**self._bin_element_coords(), "vertical_level": self.vertical_levels})
        initialization = self._parameters.initialization
        groups = {group.short_name: group for group in self._parameters.groups}
        elements = self._parameters.elements
        element_groups = [groups[element.group] for element in elements]
        nucleated = {nucleation.element_to for nucleation in self._parameters.nucleations}
        _mask(dataset, ("fall_velocity", "sedimentation_flux"), "element",
              [initialization.do_vtran and group.do_vtran for group in element_groups])
        _mask(dataset, ("deposition_velocity", "particle_mass_on_surface"), "element",
              [initialization.do_drydep and group.do_drydep for group in element_groups])
        _mask(dataset, ("nucleation_rate",), "element", [element.short_name in nucleated for element in elements])
        _mask(dataset, ("delta_particle_temperature",), "element", [initialization.do_pheat] * len(elements))
        return dataset

    def get_detrained_masses(self) -> xr.Dataset:
        """
        Get the mass of the detrained condensate for the bin for each particle in the grid

        Returns:
            xr.Dataset: Detrained condensate values for all bins and elements
        """
        return _build_dataset(
            self._bin_element_records(self._cpp.get_detrain),
            ("bin", "element"),
            _DETRAIN_VARIABLES,
            self._bin_element_coords())

    def get_gases(self) -> Tuple[xr.Dataset, Dict[str, int]]:
        """
        Get the values for all gases.

        The saturation and vapor pressure fields are NaN for a gas that has no
        vaporization routine (ivaprtn is VaporizationAlgorithm.NONE).

        Returns:
            Tuple[xr.Dataset, Dict[str, int]] A dataset containing values for all gases and a mapping of gas names to their indices.
        """
        records = [self._cpp.get_gas(i_gas + 1) for i_gas in range(self.dimensions["number_of_gases"])]
        coords = {
            "gas": [gas.short_name for gas in self.gases],
            "vertical_center": self.vertical_center
        }
        dataset = _build_dataset(records, ("gas",), _GAS_VARIABLES, coords)
        _mask(dataset,
              ("gas_saturation_wrt_ice", "gas_saturation_wrt_liquid",
               "gas_vapor_pressure_wrt_ice", "gas_vapor_pressure_wrt_liquid"),
              "gas",
              [_is_enabled(gas.ivaprtn) for gas in self.gases])
        return dataset, {gas.short_name: idx for idx, gas in enumerate(self.gases)}

    def get_environmental_values(self) -> xr.Dataset:
        """
        Get all environmental conditions for the current CARMAState.

        The latent_heat field is NaN when CARMA does not calculate latent heat
        (initialization.do_thermo is False).

        Returns:
            xr.Dataset: Dataset containing all environmental conditions
        """
        dataset = _build_dataset(
            [self._cpp.get_environmental_values()],
            (),
            _ENVIRONMENTAL_VARIABLES,
            {"vertical_center": self.vertical_center})
        if not self._parameters.initialization.do_thermo:
            dataset["latent_heat"] = dataset["latent_heat"].where(False)
        return dataset

    def set_temperature(self, temperature: Union[float, List[float]]):
        """
        Set the temperature for the vertical levels.

        Args:
            temperature: Temperature value to set, can be a single float or a list of floats
        """
        self._cpp.set_temperature(_vector(self._column(temperature)))

    def set_air_density(self, air_density: Union[float, List[float]]):
        """
        Set the air density for the vertical levels.

        Args:
            air_density: Air density value to set, can be a single float or a list of floats
        """
        self._cpp.set_air_density(_vector(self._column(air_density)))

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
        self._cpp.step(_make_cpp("CARMAStateStepConfig", {
            "cloud_fraction": cloud_fraction,
            "critical_relative_humidity": critical_relative_humidity,
            "land": land,
            "ocean": ocean,
            "ice": ice,
        }))


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

        self._cpp = _backend._carma.CARMA(parameters._to_cpp())
        self.__parameters = parameters

    def __repr__(self):
        """String representation of CARMA instance."""
        return f"CARMA() - Version: {_backend._carma._get_carma_version()}"

    def __str__(self):
        """String representation of CARMA instance."""
        return self.__repr__()

    def create_state(self, **kwargs) -> CARMAState:
        """
        Create a CARMAState instance based on the current parameters.

        Args:
            **kwargs: Additional keyword arguments to pass to CARMAState

        Returns:
            CARMAState: Instance containing environmental variables for the simulation
        """

        return CARMAState(
            self._cpp,
            parameters=self.__parameters,
            **kwargs
        )

    def get_group_properties(self) -> Tuple[xr.Dataset, List[CARMAGroupConfig]]:
        """
        Get the group properties for all groups.

        Returns:
            Tuple[xr.Dataset, List[CARMAGroupConfig]]: The group properties for all groups and the group configurations
        """
        groups = self.__parameters.groups
        if len(groups) == 0:
            return xr.Dataset(), {}

        records = [self._cpp.get_group_properties(i_group + 1) for i_group in range(len(groups))]
        coords = {
            "group": [group.short_name for group in groups],
            "bin": np.arange(1, self.__parameters.nbin + 1),
            "wavelength": np.arange(1, len(self.__parameters.wavelength_bins) + 1),
            "element": [element.short_name for element in self.__parameters.elements]
        }
        return _build_dataset(records, ("group",), _GROUP_VARIABLES, coords), groups

    def get_element_properties(self) -> Tuple[xr.Dataset, List[CARMAElementConfig]]:
        """
        Get the element properties for all elements

        Returns:
            Tuple[xr.Dataset, List[CARMAElementConfig]]: The properties for each element and the element configurations
        """
        elements = self.__parameters.elements
        if len(elements) == 0:
            return xr.Dataset(), {}

        records = [self._cpp.get_element_properties(i_elem + 1) for i_elem in range(len(elements))]
        number_of_refractive_indices = {record.number_of_refractive_indices for record in records}
        if len(number_of_refractive_indices) != 1:
            raise ValueError("Inconsistent number of refractive indices found.")
        coords = {
            "bin": np.arange(1, self.__parameters.nbin + 1),
            "wavelength": np.arange(1, len(self.__parameters.wavelength_bins) + 1),
            "refractive_index": np.arange(1, number_of_refractive_indices.pop() + 1),
            "element": [element.short_name for element in elements]
        }
        return _build_dataset(records, ("element",), _ELEMENT_VARIABLES, coords), elements

    def get_gas_properties(self) -> List[CARMAGasConfig]:
        """
        Get the gas properties for all gases.

        Returns:
            List[CARMAGasConfig]: The gas configurations
        """
        return self.__parameters.gases

    def get_solute_properties(self) -> List[CARMASoluteConfig]:
        """
        Get the solute properties for all solutes.

        Returns:
            List[CARMASoluteConfig]: The solute configurations
        """
        return self.__parameters.solutes
