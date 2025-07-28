// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file contains the definition of the CARMA class, which represents an aerosol model
// and includes functions for creating and deleting CARMA instances with C binding.
#pragma once

#include <string>
#include <vector>

namespace musica
{
  // Forward declarations for C interface types
  struct CCARMAParameters;

  // Enumeration for particle shapes
  enum class ParticleShape
  {
    SPHERE = 1,
    HEXAGON = 2,
    CYLINDER = 3
  };

  // Enumeration for particle swelling algorithms
  enum class ParticleSwellingAlgorithm
  {
    NONE = 0,
    FITZGERALD = 1,
    GERBER = 2,
    WEIGHT_PERCENT_H2SO4 = 3,
    PETTERS = 4
  };

  // Enumeration for particle swelling composition
  enum class ParticleSwellingComposition
  {
    NONE = 0,
    AMMONIUM_SULFATE = 1,
    SEA_SALT = 2,
    URBAN = 3,
    RURAL = 4
  };

  // Enumeration for fall velocity algorithms
  enum class FallVelocityAlgorithm
  {
    NONE = 0,
    STANDARD_SPHERICAL_ONLY = 1,  // Standard algorithm for spherical particles only
    STANDARD_SHAPE_SUPPORT = 2,   // Standard algorithm with support for different shapes
    HEYMSFIELD_2010 = 3           // Heymsfield and Westbrook 2010
  };

  // Enumeration for Mie calculation methods
  enum class MieCalculationAlgorithm
  {
    NONE = 0,
    TOON_1981 = 1,    // Shell/Core Toon & Ackerman 1981 Mie calculation
    BOHREN_1983 = 2,  // Homogeneous Sphere Bohren and Huffman 1983 Mie calculation
    BOTET_1997 = 3    // Fractal Mean-Field Botet et al. 1997 Mie calculation
  };

  // Enumeration for optics algorithms
  enum class OpticsAlgorithm
  {
    NONE = 0,
    FIXED = 1,              // Fixed composition
    MIXED_YU_2015 = 2,      // Yu (2015) mixed composition
    SULFATE_YU_2015 = 3,    // Yu (2015) pure sulfate composition
    MIXED_H2O_YU_2015 = 4,  // Yu (2015) mixed composition with water in shell
    MIXED_CORE_SHELL = 5,   // Core-Shell mixed composition
    MIXED_VOLUME = 6,       // Volume mixed composition
    MIXED_MAXWELL = 7,      // Maxwell-Garnett mixed composition
    SULFATE = 8             // Sulfate, refractive index varies with WTP/RH
  };

  // Enumeration for vaporization algorithms
  enum class VaporizationAlgorithm
  {
    NONE = 0,
    H2O_BUCK_1981 = 1,    // Buck 1981 for water vaporization
    H2O_MURPHY_2005 = 2,  // Murphy and Koop 2005 for water vaporization
    H2O_GOFF_1946 = 3,    // Goff 1946 for water vaporization (used in CAM)
    H2SO4_AYERS_1980 = 4  // Ayers 1980 & Kumala 1990 for sulfuric acid vaporization
  };

  // Enumeration for particle types
  enum class ParticleType
  {
    INVOLATILE = 1,
    VOLATILE = 2,
    COREMASS = 3,
    VOLCORE = 4,
    CORE2MOM = 5
  };

  // Enumeration for gas compositions
  enum GasComposition
  {
    OTHER = 0,  // Other gas composition
    H2O = 1,    // Water vapor
    H2SO4 = 2,  // Sulfuric acid
    SO2 = 3,    // Sulfur dioxide
  };

  // Enumeration for particle compositions
  enum class ParticleComposition
  {
    ALUMINUM = 1,
    H2SO4 = 2,
    DUST = 3,
    ICE = 4,
    H2O = 5,
    BLACKCARBON = 6,
    ORGANICCARBON = 7,
    OTHER = 8
  };

  // Structure representing a wavelength bin
  struct CARMAWavelengthBin
  {
    double center;            // Center of the wavelength bin [m]
    double width;             // Width of the wavelength bin [m]
    bool do_emission = true;  // Flag to indicate if emission is considered for this bin
  };

  // Structure defining an approach to particle swelling
  struct CARMASwellingApproach
  {
    ParticleSwellingAlgorithm algorithm = ParticleSwellingAlgorithm::NONE;        // Swelling algorithm
    ParticleSwellingComposition composition = ParticleSwellingComposition::NONE;  // Composition for swelling
  };

  // Structure representing a complex number
  struct CARMAComplex
  {
    double real;       // Real part
    double imaginary;  // Imaginary part
  };

  // Structure representing a CARMA group configuration
  struct CARMAGroupConfig
  {
    std::string name = "default_group";
    std::string shortname = "";
    double rmin = 1e-9;     // minimum radius [m]
    double rmrat = 2.0;     // volume ratio between bins
    double rmassmin = 0.0;  // minimum mass [kg] (When rmassmin > 0, rmin is ignored)
    ParticleShape ishape = ParticleShape::SPHERE;
    double eshape = 1.0;                      // aspect ratio (length/width)
    CARMASwellingApproach swelling_approach;  // Swelling from RH approach
    FallVelocityAlgorithm fall_velocity_routine = FallVelocityAlgorithm::STANDARD_SPHERICAL_ONLY;
    MieCalculationAlgorithm mie_calculation_algorithm = MieCalculationAlgorithm::NONE;
    OpticsAlgorithm optics_algorithm = OpticsAlgorithm::FIXED;  // Optics algorithm
    bool is_ice = false;
    bool is_fractal = false;
    bool is_cloud = false;
    bool is_sulfate = false;
    bool do_wetdep = false;
    bool do_drydep = false;
    bool do_vtran = true;
    double solfac = 0.0;          // Solubility factor for wet deposition
    double scavcoef = 0.0;        // Scavenging coefficient for wet deposition
    double dpc_threshold = 0.0;   // convergence criteria for particle concentration [fraction]
    double rmon = 0.0;            // monomer radius [m]
    std::vector<double> df;       // fractal dimension per bin
    double falpha = 1.0;          // fractal packing coefficient
    double neutral_volfrc = 0.0;  // neutral volume fraction for fractal particles
  };

  // Structure representing a CARMA element configuration
  struct CARMAElementConfig
  {
    int igroup = 1;   // group this element belongs to
    int isolute = 0;  // solute index
    std::string name = "default_element";
    std::string shortname = "";
    ParticleType itype = ParticleType::INVOLATILE;
    ParticleComposition icomposition = ParticleComposition::ALUMINUM;
    bool isShell = true;                            // is this part of shell or core
    double rho = 1000.0;                            // bulk density [kg/m3]
    std::vector<double> rhobin;                     // density per bin [kg/m3]
    std::vector<double> arat;                       // projected area ratio per bin
    double kappa = 0.0;                             // hygroscopicity parameter
    std::vector<std::vector<CARMAComplex>> refidx;  // wavelength-resolved refractive indices (n_ref_idx, n_wave)
  };

  // Structure representing a CARMA solute configuration
  struct CARMASoluteConfig
  {
    std::string name = "default_solute";
    std::string shortname = "";
    int ions = 0;        // number of ions the solute dissociates into
    double wtmol = 0.0;  // molar mass of the solute [kg/mol]
    double rho = 0.0;    // mass density of the solute [kg/m3]
  };

  // Structure representing a CARMA gas species configuration
  struct CARMAGasConfig
  {
    std::string name = "default_gas";
    std::string shortname = "";
    double wtmol = 0.0;                                           // molar mass of the gas [kg/mol]
    VaporizationAlgorithm ivaprtn = VaporizationAlgorithm::NONE;  // vaporization routine
    GasComposition icomposition = GasComposition::OTHER;          // composition of the gas
    double dgc_threshold = 0.0;  // convergence criteria for gas concentration [0 : off; > 0 : fraction]
    double ds_threshold =
        0.0;  // convergence criteria for gas saturation [0 : off; > 0 : fraction; < 0 : amount past 0 crossing]
    std::vector<std::vector<CARMAComplex>> refidx;  // wavelength-resolved refractive indices (n_ref_idx, n_wave)
  };

  struct CARMAParameters
  {
    // Model dimensions
    int nz = 1;
    int ny = 1;
    int nx = 1;
    int nbin = 5;

    // Time stepping parameters
    double dtime = 1800.0;
    int nstep = 100;

    // Spatial parameters
    double deltaz = 1000.0;
    double zmin = 16500.0;

    // Wavelength grid
    std::vector<CARMAWavelengthBin> wavelength_bins;  // Wavelength bins
    int number_of_refractive_indices = 0;             // Number of refractive indices per wavelength

    std::vector<CARMAGroupConfig> groups;
    std::vector<CARMAElementConfig> elements;
    std::vector<CARMASoluteConfig> solutes;
    std::vector<CARMAGasConfig> gases;
  };

  struct CARMAOutput
  {
    // Grid and coordinate arrays
    std::vector<double> lat;              // Latitude [degrees]
    std::vector<double> lon;              // Longitude [degrees]
    std::vector<double> vertical_center;  // Height at cell centers [m]
    std::vector<double> vertical_levels;  // Height at cell interfaces [m]

    // Atmospheric state variables (nz elements)
    std::vector<double> pressure;     // Pressure [Pa]
    std::vector<double> temperature;  // Temperature [K]
    std::vector<double> air_density;  // Air density [kg/m3]

    // Fundamental CARMA data for Python calculations
    // Particle state arrays (3D: nz x nbin x nelem)
    std::vector<std::vector<std::vector<double>>> particle_concentration;  // particle concentration [# cm-3]
    std::vector<std::vector<std::vector<double>>> mass_mixing_ratio;       // mass mixing ratio [kg kg-1]

    // Particle properties (3D: nz x nbin x ngroup)
    std::vector<std::vector<std::vector<double>>> wet_radius;           // wet radius [cm]
    std::vector<std::vector<std::vector<double>>> wet_density;          // wet density [g cm-3]
    std::vector<std::vector<std::vector<double>>> fall_velocity;        // fall velocity [cm s-1] (nz+1 x nbin x ngroup)
    std::vector<std::vector<std::vector<double>>> nucleation_rate;      // nucleation rate [cm-3 s-1] (nz x nbin x ngroup)
    std::vector<std::vector<std::vector<double>>> deposition_velocity;  // deposition velocity [cm s-1] (nz x nbin x ngroup)

    // Group configuration arrays (2D: nbin x ngroup)
    std::vector<std::vector<double>> dry_radius;    // dry radius [cm]
    std::vector<std::vector<double>> mass_per_bin;  // mass per bin [g]
    std::vector<std::vector<double>> radius_ratio;  // radius ratio
    std::vector<std::vector<double>> aspect_ratio;  // area ratio

    // Group mapping and properties (1D arrays)
    std::vector<int> group_particle_number_concentration;  // concentration element per group [ngroup]
    std::vector<int> constituent_type;                     // constituent type per group [ngroup]
    std::vector<int> max_prognostic_bin;                   // max prognostic bin per group [ngroup]
  };

  class CARMA
  {
   public:
    /// @brief Constructor for CARMA
    /// @param params The CARMA parameters to initialize the model
    /// @throws std::runtime_error if the CARMA instance cannot be created
    explicit CARMA(const CARMAParameters& params);

    ~CARMA();

    /// @brief Get the version of CARMA
    /// @return The version string of the CARMA instance
    static std::string GetVersion();

    /// @brief Run CARMA with the specified parameters and configuration
    /// @return The CARMA output data
    CARMAOutput Run();

    /// @brief Convert CARMAParameters to C-compatible CCARMAParameters
    /// @param params The C++ CARMA parameters to convert
    /// @return The C-compatible CARMA parameters structure
    static struct CCARMAParameters* ToCCompatible(const CARMAParameters& params);

    /// @brief Free memory allocated in CCARMAParameters
    /// @param c_params The C-compatible parameters to clean up
    static void FreeCCompatible(struct CCARMAParameters* c_params);

    /// @brief Returns a set of test parameters for the aluminum test case
    /// @return A CARMAParameters object with the aluminum test case configuration
    static CARMAParameters CreateAluminumTestParams();

   private:
    CCARMAParameters* carma_parameters_;  // C-compatible parameters
    void* f_carma_type_ = nullptr;        // Pointer to the Fortran CARMA type
  };

}  // namespace musica