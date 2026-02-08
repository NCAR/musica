from .carma import (
    # Main classes
    CARMA, CARMAParameters, CARMAState, CARMASurfaceProperties,

    # Configuration classes
    CARMAGroupConfig, CARMAElementConfig, CARMASoluteConfig, CARMAGasConfig,
    CARMAWavelengthBin, CARMACoagulationConfig, CARMAGrowthConfig,
    CARMANucleationConfig, CARMAInitializationConfig,

    # Enums
    ParticleShape, ParticleType, ParticleSwellingAlgorithm,
    ParticleSwellingComposition, ParticleFallVelocityAlgorithm,
    MieCalculationAlgorithm, OpticsAlgorithm, VaporizationAlgorithm,
    GasComposition, ParticleComposition, ParticleCollectionAlgorithm,
    ParticleNucleationAlgorithm, SulfateNucleationMethod, CarmaCoordinates,
)
from .. import backend
_backend = backend.get_backend()

__version__ = _backend._carma._get_carma_version() if backend.carma_available() else None
