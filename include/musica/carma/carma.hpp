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
    int nwave = 30;

    // Time stepping parameters
    double dtime = 1800.0;
    int nstep = 100;

    // Spatial parameters
    double deltaz = 1000.0;
    double zmin = 16500.0;

    std::vector<CARMAGroupConfig> groups;
    std::vector<CARMAElementConfig> elements;
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
    std::vector<std::vector<std::vector<double>>> particle_concentration; // particle concentration [# cm-3]
    std::vector<std::vector<std::vector<double>>> mass_mixing_ratio;      // mass mixing ratio [kg kg-1]

    // Particle properties (3D: nz x nbin x ngroup)
    std::vector<std::vector<std::vector<double>>> wet_radius;       // wet radius [cm]
    std::vector<std::vector<std::vector<double>>> wet_density;      // wet density [g cm-3]
    std::vector<std::vector<std::vector<double>>> fall_velocity;        // fall velocity [cm s-1] (nz+1 x nbin x ngroup)
    std::vector<std::vector<std::vector<double>>> nucleation_rate;      // nucleation rate [cm-3 s-1] (nz x nbin x ngroup)
    std::vector<std::vector<std::vector<double>>> deposition_velocity;  // deposition velocity [cm s-1] (nz x nbin x ngroup)

    // Group configuration arrays (2D: nbin x ngroup)
    std::vector<std::vector<double>> dry_radius;    // dry radius [cm]
    std::vector<std::vector<double>> mass_per_bin;  // mass per bin [g]
    std::vector<std::vector<double>> radius_ratio;  // radius ratio
    std::vector<std::vector<double>> aspect_ratio;  // area ratio

    // Group mapping and properties (1D arrays)
    std::vector<double> group_particle_number_concentration;  // concentration element per group [ngroup]
    std::vector<double> constituent_type;       // constituent type per group [ngroup]
    std::vector<double> max_prognostic_bin;     // max prognostic bin per group [ngroup]
  };

  class CARMA
  {
   public:
    CARMA();
    ~CARMA();

    /// @brief Get the version of CARMA
    /// @return The version string of the CARMA instance
    static std::string GetVersion();

    /// @brief Run CARMA with the specified parameters and configuration
    /// @param params The CARMA parameters to use for the simulation
    /// @param config The group and element configuration (optional)
    /// @return The CARMA output data
    CARMAOutput Run(const CARMAParameters& params);

    /// @brief Convert CARMAParameters to C-compatible CCARMAParameters
    /// @param params The C++ CARMA parameters to convert
    /// @return The C-compatible CARMA parameters structure
    static struct CCARMAParameters ToCCompatible(const CARMAParameters& params);

    /// @brief Free memory allocated in CCARMAParameters
    /// @param c_params The C-compatible parameters to clean up
    static void FreeCCompatible(struct CCARMAParameters& c_params);

    /// @brief Returns a set of test parameters for the aluminum test case
    /// @return A CARMAParameters object with the aluminum test case configuration
    static CARMAParameters CreateAluminumTestParams();
  };

}  // namespace musica