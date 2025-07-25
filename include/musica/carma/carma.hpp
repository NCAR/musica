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

  // Enumeration for particle types
  enum class ParticleType
  {
    INVOLATILE = 1,
    VOLATILE = 2,
    COREMASS = 3,
    VOLCORE = 4,
    CORE2MOM = 5
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
    double center;           // Center of the wavelength bin [m]
    double width;            // Width of the wavelength bin [m]
    bool do_emission = true; // Flag to indicate if emission is considered for this bin
  };

  // Structure representing a CARMA group configuration
  struct CARMAGroupConfig
  {
    int id = 1;
    std::string name = "default_group";
    std::string shortname = "";
    double rmin = 1e-7;  // minimum radius [cm]
    double rmrat = 2.0;  // volume ratio between bins
    ParticleShape ishape = ParticleShape::SPHERE;
    double eshape = 1.0;  // aspect ratio
    bool is_ice = false;
    bool is_fractal = false;
    bool do_mie = true;
    bool do_wetdep = false;
    bool do_drydep = false;
    bool do_vtran = true;
    double solfac = 0.0;
    double scavcoef = 0.0;
    double rmon = 0.0;       // monomer radius [cm]
    std::vector<double> df;  // fractal dimension per bin
    double falpha = 1.0;     // fractal packing coefficient
  };

  // Structure representing a CARMA element configuration
  struct CARMAElementConfig
  {
    int id = 1;
    int igroup = 1;  // group this element belongs to
    std::string name = "default_element";
    std::string shortname = "";
    double rho = 1.0;  // bulk density [g/cm3]
    ParticleType itype = ParticleType::INVOLATILE;
    ParticleComposition icomposition = ParticleComposition::ALUMINUM;
    int isolute = 0;             // solute index
    std::vector<double> rhobin;  // density per bin [g/cm3]
    std::vector<double> arat;    // projected area ratio per bin
    double kappa = 0.0;          // hygroscopicity parameter
    bool isShell = true;         // is this part of shell or core
  };

  struct CARMAParameters
  {
    int max_bins = 100;
    int max_groups = 10;

    // Model dimensions
    int nz = 1;
    int ny = 1;
    int nx = 1;
    int nelem = 1;
    int ngroup = 1;
    int nbin = 5;
    int nsolute = 0;
    int ngas = 0;
    int idx_wave = 0;  // TODO: is there a better name?

    // Time stepping parameters
    double dtime = 1800.0;
    int nstep = 100;

    // Spatial parameters
    double deltaz = 1000.0;
    double zmin = 16500.0;

    // Wavelength grid
    std::vector<CARMAWavelengthBin> wavelength_bins;   // Wavelength bins
    int number_of_refractive_indices = 0;              // Number of refractive indices per wavelength

    // Optical parameters
    std::vector<double> extinction_coefficient;  // Extinction coefficient qext [NWAVE * NBIN * NGROUP]

    std::vector<CARMAGroupConfig> groups;
    std::vector<CARMAElementConfig> elements;
  };

  struct CARMAOutput
  {
    // Dimensions for validation
    int nz = 0;
    int ny = 0;
    int nx = 0;
    int nelem = 0;
    int ngroup = 0;
    int nbin = 0;
    int ngas = 0;
    int nstep = 0;

    // Grid and coordinate arrays
    std::vector<double> lat;              // Latitude [degrees]
    std::vector<double> lon;              // Longitude [degrees]
    std::vector<double> vertical_center;  // Height at cell centers [m]
    std::vector<double> vertical_levels;  // Height at cell interfaces [m]

    // Atmospheric state variables (nz elements)
    std::vector<double> pressure;           // Pressure [Pa]
    std::vector<double> temperature;        // Temperature [K]
    std::vector<double> air_density;        // Air density [kg/m3]
    std::vector<double> radiative_heating;  // Radiative heating [K/s]
    std::vector<double> delta_temperature;  // Temperature change [K]

    // Gas variables (nz x ngas)
    std::vector<std::vector<double>> gas_mmr;                    // Gas mass mixing ratio [kg/kg]
    std::vector<std::vector<double>> gas_saturation_liquid;      // Saturation over liquid
    std::vector<std::vector<double>> gas_saturation_ice;         // Saturation over ice
    std::vector<std::vector<double>> gas_vapor_pressure_ice;     // Evaporation rate over ice
    std::vector<std::vector<double>> gas_vapor_pressure_liquid;  // Evaporation rate over liquid
    std::vector<std::vector<double>> gas_weight_percent;         // Gas weight

    // Group-integrated variables (nz x ngroup)
    std::vector<std::vector<double>> number_density;        // Number density [#/cm3]
    std::vector<std::vector<double>> surface_area;          // Surface area density [cm2/cm3]
    std::vector<std::vector<double>> mass_density;          // Mass density [g/cm3]
    std::vector<std::vector<double>> effective_radius;      // Effective radius [cm]
    std::vector<std::vector<double>> effective_radius_wet;  // Wet effective radius [cm]
    std::vector<std::vector<double>> mean_radius;           // Mean radius [cm]
    std::vector<std::vector<double>> nucleation_rate;       // Nucleation rate [#/cm3/s]
    std::vector<std::vector<double>> mass_mixing_ratio;     // Mass mixing ratio [kg/kg]
    std::vector<std::vector<double>> projected_area;        // Projected area [cm2/cm3]
    std::vector<std::vector<double>> aspect_ratio;          // Aspect ratio
    std::vector<std::vector<double>> vertical_mass_flux;    // Vertical mass flux [g/cm2/s]
    std::vector<std::vector<double>> extinction;            // Extinction coefficient [1/km]
    std::vector<std::vector<double>> optical_depth;         // Optical depth

    // Bin-resolved variables (nz x ngroup x nbin)
    std::vector<std::vector<std::vector<double>>> bin_wet_radius;           // Wet radius [um]
    std::vector<std::vector<std::vector<double>>> bin_number_density;       // Number density [#/cm3]
    std::vector<std::vector<std::vector<double>>> bin_density;              // Particle density [g/cm3]
    std::vector<std::vector<std::vector<double>>> bin_mass_mixing_ratio;    // Mass mixing ratio [kg/kg]
    std::vector<std::vector<std::vector<double>>> bin_deposition_velocity;  // Deposition velocity [cm/s]

    // Group properties (constant for each group)
    std::vector<std::vector<double>> group_radius;             // Bin center radius [cm] (nbin x ngroup)
    std::vector<std::vector<double>> group_mass;               // Bin mass [g] (nbin x ngroup)
    std::vector<std::vector<double>> group_volume;             // Bin volume [cm3] (nbin x ngroup)
    std::vector<std::vector<double>> group_radius_ratio;       // Radius ratio (nbin x ngroup)
    std::vector<std::vector<double>> group_aspect_ratio;       // Aspect ratio (nbin x ngroup)
    std::vector<std::vector<double>> group_fractal_dimension;  // Fractal dimension (nbin x ngroup)

    // Element and group names for identification
    std::vector<std::string> element_names;
    std::vector<std::string> group_names;
    std::vector<std::string> gas_names;
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
    static struct CCARMAParameters * ToCCompatible(const CARMAParameters& params);

    /// @brief Free memory allocated in CCARMAParameters
    /// @param c_params The C-compatible parameters to clean up
    static void FreeCCompatible(struct CCARMAParameters * c_params);

    /// @brief Returns a set of test parameters for the aluminum test case
    /// @return A CARMAParameters object with the aluminum test case configuration
    static CARMAParameters CreateAluminumTestParams();

   private:
    CCARMAParameters * carma_parameters_;  // C-compatible parameters
    void * f_carma_type_ = nullptr;  // Pointer to the Fortran CARMA type
  };

}  // namespace musica