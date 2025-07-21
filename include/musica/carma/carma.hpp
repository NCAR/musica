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
  struct CARMAParameters
  {
    int max_bins = 100;   // Maximum number of size bins
    int max_groups = 10;  // Maximum number of groups for fractal dimension

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
    std::vector<double> lat;  // Latitude [degrees]
    std::vector<double> lon;  // Longitude [degrees]
    std::vector<double> zc;   // Height at cell centers [m]
    std::vector<double> zl;   // Height at cell interfaces [m]

    // Atmospheric state variables (nz elements)
    std::vector<double> pressure;           // Pressure [Pa]
    std::vector<double> temperature;        // Temperature [K]
    std::vector<double> air_density;        // Air density [kg/m3]
    std::vector<double> radiative_heating;  // Radiative heating [K/s]
    std::vector<double> delta_temperature;  // Temperature change [K]

    // Gas variables (nz x ngas)
    std::vector<std::vector<double>> gas_mmr;                // Gas mass mixing ratio [kg/kg]
    std::vector<std::vector<double>> gas_saturation_liquid;  // Saturation over liquid
    std::vector<std::vector<double>> gas_saturation_ice;     // Saturation over ice
    std::vector<std::vector<double>> gas_ei;                 // Evaporation rate over ice
    std::vector<std::vector<double>> gas_el;                 // Evaporation rate over liquid
    std::vector<std::vector<double>> gas_wt;                 // Gas weight

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

    // Time information
    double current_time = 0.0;  // Current simulation time [s]
    int current_step = 0;       // Current time step
  };

  class CARMA
  {
   public:
    CARMA();
    ~CARMA();

    /// @brief Get the version of CARMA
    /// @return The version string of the CARMA instance
    static std::string GetVersion();

    /// @brief Run CARMA with the specified parameters
    /// @param params The CARMA parameters to use for the simulation
    /// @param output The structure to fill with CARMA output data
    CARMAOutput Run(const CARMAParameters& params);
  };

  /// @brief Factory functions for creating test-specific CARMA parameters
  namespace CARMATestConfigs
  {

    /// @brief Create parameters for aluminum test (carma_aluminumtest_2nc.F90)
    static CARMAParameters CreateAluminumTestParams()
    {
      CARMAParameters params;

      // Model dimensions
      params.nz = 1;
      params.ny = 1;
      params.nx = 1;
      params.nelem = 1;
      params.ngroup = 1;
      params.nbin = 5;
      params.nsolute = 0;
      params.ngas = 0;
      params.nwave = 30;

      // Time parameters
      params.dtime = 1800.0;
      params.nstep = 432000 / static_cast<int>(params.dtime);

      // Spatial parameters
      params.deltaz = 1000.0;
      params.zmin = 16500.0;

      return params;
    }

    /// @brief Create parameters for fractal optics test (carma_fractalopticstest_2nc.F90)
    static CARMAParameters CreateFractalOpticsTestParams()
    {
      CARMAParameters params;

      // Model dimensions
      params.nz = 1;
      params.ny = 1;
      params.nx = 1;
      params.nelem = 1;
      params.ngroup = 1;
      params.nbin = 5;
      params.nsolute = 0;
      params.ngas = 0;
      params.nwave = 30;

      return params;
    }

    /// @brief Create parameters for fractal optics test IQ variant (carma_fractalopticstest_2nc_iq.F90)
    static CARMAParameters CreateFractalOpticsIQTestParams()
    {
      CARMAParameters params;

      // Model dimensions
      params.nz = 1;
      params.ny = 1;
      params.nx = 1;
      params.nelem = 1;
      params.ngroup = 1;
      params.nbin = 10;  // Different from standard test
      params.nsolute = 0;
      params.ngas = 0;
      params.nwave = 30;

      return params;
    }

    /// @brief Create parameters for sulfate test (carma_sulfatetest_2nc.F90)
    static CARMAParameters CreateSulfateTestParams()
    {
      CARMAParameters params;

      // Model dimensions
      params.nz = 1;
      params.ny = 1;
      params.nx = 1;
      params.nelem = 1;
      params.ngroup = 1;
      params.nbin = 22;  // More bins for sulfate
      params.nsolute = 0;
      params.ngas = 2;   // Water vapor and H2SO4
      params.nwave = 0;  // No optics for this test

      // Time parameters
      params.dtime = 1800.0;
      params.nstep = 180000 / static_cast<int>(params.dtime);

      // Spatial parameters
      params.deltaz = 10000.0;  // Larger vertical spacing
      params.zmin = 145000.0;   // Higher altitude

      return params;
    }

  }  // namespace CARMATestConfigs
}  // namespace musica