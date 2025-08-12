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

  // Enumeration for particle collection algorithms
  enum class ParticleCollectionAlgorithm
  {
    NONE = 0,
    CONSTANT = 1,  // Constant collection efficiency
    FUCHS = 2,     // Binwise maxima of Fuchs' and Langmuir's efficiencies
    DATA = 3       // Collection efficiency from input data
  };

  // Enumeration for particle nucleation algorithms
  enum class ParticleNucleationAlgorithm
  {
    NONE = 0,
    AEROSOL_FREEZING_TABAZDEH_2000 = 1,             // Aerosol freezing, Tabazdeh et al. 2000
    AEROSOL_FREEZING_KOOP_2000 = 2,                 // Aerosol freezing, Koop et al. 2000
    AEROSOL_FREEZING_MURRAY_2010 = 3,               // Aerosol freezing, Murray et al. 2010
    DROPLET_ACTIVATION = 256,                       // Droplet activation
    AEROSOL_FREEZING = 512,                         // Aerosol freezing
    DROPLET_FREEZING = 1024,                        // Droplet freezing
    ICE_MELTING = 2048,                             // Ice melting
    HETEROGENEOUS_NUCLEATION = 4096,                // Heterogeneous nucleation
    HOMOGENEOUS_NUCLEATION = 8192,                  // Binary Homogeneous gas-to-particle nucleation
    HETEROGENEOUS_SULFURIC_ACID_NUCLEATION = 16384  // Heterogeneous sulfuric acid nucleation
  };

  enum class SulfateNucleationMethod
  {
    NONE = 0,        // No sulfate nucleation
    ZHAO_TURCO = 1,  // Zhao and Turco sulfate nucleation
    VEHKAMAKI = 2    // Vehkamaki et al. sulfate nucleation
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
    MieCalculationAlgorithm mie_calculation_algorithm = MieCalculationAlgorithm::TOON_1981;  // Mie calculation algorithm
    OpticsAlgorithm optics_algorithm = OpticsAlgorithm::FIXED;                               // Optics algorithm
    bool is_ice = false;
    bool is_fractal = false;
    bool is_cloud = false;
    bool is_sulfate = false;
    bool do_wetdep = false;
    bool do_drydep = false;
    bool do_vtran = true;
    double solfac = 0.3;          // Solubility factor for wet deposition
    double scavcoef = 0.1;        // Scavenging coefficient for wet deposition
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

  // Structure representing CARMA coagulation configuration
  struct CARMACoagulationConfig
  {
    int igroup1 = 0;  // first group index (first group to coagulate)
    int igroup2 = 0;  // second group index (second group to coagulate)
    int igroup3 = 0;  // third group index (coagulated particles)
    ParticleCollectionAlgorithm algorithm = ParticleCollectionAlgorithm::NONE;  // collection algorithm
    double ck0 = 0.0;                                                           // collection efficiency constant (0.0 = off)
    double grav_e_coll0 = 0.0;  // gravitational collection efficiency constant (0.0 = off)
    bool use_ccd = false;       // use constant collection efficiency data
  };

  // Structure representing CARMA growth configuration
  struct CARMAGrowthConfig
  {
    int ielem = 0;  // element index to grow
    int igas = 0;   // gas index to grow from
  };

  // Structure representing CARMA nucleation configuration
  struct CARMANucleationConfig
  {
    int ielemfrom = 0;                                                          // element index to nucleate from
    int ielemto = 0;                                                            // element index to nucleate to
    ParticleNucleationAlgorithm algorithm = ParticleNucleationAlgorithm::NONE;  // nucleation algorithm
    double rlh_nuc = 0.0;                                                       // latent heat of nucleation [m2 s-2]
    int igas = 0;                                                               // gas index to nucleate from
    int ievp2elem = 0;  // element index to evaporate to (if applicable)
  };

  // Structure representing CARMA initialization configuration
  struct CARMAInitializationConfig
  {
    bool do_cnst_rlh = false;       // use constant values for latent heats
    bool do_detrain = false;        // do detrainment
    bool do_fixedinit = false;      // use fixed initialization from reference atmosphere
    bool do_incloud = false;        // do in-cloud processes (growth, coagulation)
    bool do_explised = false;       // do sedimentation with substepping
    bool do_substep = false;        // do substepping
    bool do_thermo = false;         // do thermodynamic processes
    bool do_vdiff = false;          // do Brownian diffusion
    bool do_vtran = true;           // do sedimentation
    bool do_drydep = false;         // do dry deposition
    bool do_pheat = false;          // do particle heating
    bool do_pheatatm = false;       // do particle heating of atmosphere
    bool do_clearsky = false;       // do clear sky growth and coagulation
    bool do_partialinit = false;    // do initialization of coagulation from reference atmosphere (requires do_fixedinit)
    bool do_coremasscheck = false;  // check core mass for particles
    SulfateNucleationMethod sulfnucl_method = SulfateNucleationMethod::NONE;  // method for sulfate nucleation
    double vf_const = 0.0;                                                    // constant fall velocity [m/s] (0: off)
    int minsubsteps = 1;                                                      // minimum number of substeps
    int maxsubsteps = 1;                                                      // maximum number of substeps
    int maxretries = 5;                                                       // maximum number of retries
    double conmax = 1.0e-1;                                                   // minimum relative concentration to consider
    double dt_threshold = 0.0;  // convergence criteria for temperature [fraction] (0: off)
    double cstick = 1.0;        // accommodation coefficient for coagulation
    double gsticki = 0.93;      // accommodation coefficient for growth of ice
    double gstickl = 1.0;       // accommodation coefficient for growth of liquid
    double tstick = 1.0;        // accommodation coefficient temperature
  };

  // Structure representing CARMA parameters
  struct CARMAParameters
  {
    // Model dimensions
    int nbin = 5;
    int nz = 1;  // Number of vertical levels

    // Time stepping parameters
    double dtime = 1800.0;

    // Wavelength grid
    std::vector<CARMAWavelengthBin> wavelength_bins;  // Wavelength bins
    int number_of_refractive_indices = 0;             // Number of refractive indices per wavelength

    // Physical constituents
    std::vector<CARMAGroupConfig> groups;
    std::vector<CARMAElementConfig> elements;
    std::vector<CARMASoluteConfig> solutes;
    std::vector<CARMAGasConfig> gases;

    // Processes
    std::vector<CARMACoagulationConfig> coagulations;
    std::vector<CARMAGrowthConfig> growths;
    std::vector<CARMANucleationConfig> nucleations;

    // Initialization configuration
    CARMAInitializationConfig initialization;
  };

  struct CARMAGroupProperties
  {
    std::vector<double> bin_radius;                // Bin radius for the group [cm]
    std::vector<double> bin_radius_lower_bound;    // Lower bound of the bin radius [cm]
    std::vector<double> bin_radius_upper_bound;    // Upper bound of the bin radius [cm]
    std::vector<double> bin_width;                 // bin width in radius space [cm]
    std::vector<double> bin_mass;                  // Bin mass for the group [g]
    std::vector<double> bin_width_mass;            // Bin width in mass space [g]
    std::vector<double> bin_volume;                // Bin volume for the group [cm3]
    std::vector<double> projected_area_ratio;      // Projected area ratio for the group, area / area enclosing sphere
    std::vector<double> radius_ratio;              // maximum dimension / radius of enclosing sphere
    std::vector<double> porosity_ratio;            // scaled porosity radius / equiv. sphere
    std::vector<double> extinction_coefficient;    // Extinction coefficient for the group
    std::vector<double> single_scattering_albedo;  // Single scattering albedo for the group
    std::vector<double> asymmetry_factor;          // Asymmetry factor for the group
    int particle_number_element_for_group;         // index of the element that is used to calculate the particle number
                                                   // concentration for this group
    int number_of_core_mass_elements_for_group;    // number of elements that are used to calculate the core mass for this
                                                   // group
    std::vector<int> element_index_of_core_mass_elements;  // indices of the elements that are used to calculate the core
                                                           // mass for this group
    int last_prognostic_bin;
    std::vector<double> number_of_monomers_per_bin;
  };

  struct CARMAElementProperties
  {
    int group_index;                   // Index of the group this element belongs to
    int solute_index;                  // Index of the solute this element belongs to
    ParticleComposition composition;   // Composition of the element
    ParticleType type;                 // Type of the element
    bool is_shell;                     // Is this part of shell or core
    double kappa;                      // Hygroscopicity parameter [unitless]
    std::vector<double> rho;           // Mass density of the particle element [kg/m3]
    std::vector<CARMAComplex> refidx;  // Refractive indices (n_indices, n_wavelengths)
    int number_of_refractive_indices;  // Number of refractive indices per wavelength
    int number_of_wavelengths;         // Number of wavelengths for refractive indices
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

    CCARMAParameters* GetCParameters() const
    {
      return c_carma_parameters_;
    }

    CARMAParameters GetParameters() const
    {
      return carma_parameters_;
    }

    void* GetCarmaInstance() const
    {
      return f_carma_type_;
    }

    /// @brief Get properties calculated in CARMA for a specific group
    /// @param group_index The index of the group to retrieve properties for
    /// @return A CARMAGroupProperties object containing the properties for the specified group
    CARMAGroupProperties GetGroupProperties(int group_index) const;

    /// @brief Get properties calculated in CARMA for a specific element
    /// @param element_index The index of the element to retrieve properties for
    /// @return A CARMAElementProperties object containing the properties for the specified element
    CARMAElementProperties GetElementProperties(int element_index) const;

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
    CARMAParameters carma_parameters_;      // C++ parameters
    CCARMAParameters* c_carma_parameters_;  // C-compatible parameters
    void* f_carma_type_ = nullptr;          // Pointer to the Fortran CARMA type
  };

}  // namespace musica