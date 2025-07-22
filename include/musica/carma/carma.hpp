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
    int idx_wave = 0; // TODO: is there a better name?

    // Time stepping parameters
    double dtime = 1800.0;
    int nstep = 100;

    // Spatial parameters
    double deltaz = 1000.0;
    double zmin = 16500.0;

    // Optical parameters
    double* extinction_coefficient = nullptr;  // Extinction coefficient qext [NWAVE * NBIN * NGROUP]

    // Constructor
    CARMAParameters() = default;

    // Destructor to clean up dynamically allocated memory
    ~CARMAParameters()
    {
      if (extinction_coefficient != nullptr)
      {
        delete[] extinction_coefficient;
        extinction_coefficient = nullptr;
      }
    }

    // Copy constructor
    CARMAParameters(const CARMAParameters& other)
        : max_bins(other.max_bins),
          max_groups(other.max_groups),
          nz(other.nz),
          ny(other.ny),
          nx(other.nx),
          nelem(other.nelem),
          ngroup(other.ngroup),
          nbin(other.nbin),
          nsolute(other.nsolute),
          ngas(other.ngas),
          nwave(other.nwave),
          idx_wave(other.idx_wave),
          dtime(other.dtime),
          nstep(other.nstep),
          deltaz(other.deltaz),
          zmin(other.zmin),
          extinction_coefficient(nullptr)
    {
      if (other.extinction_coefficient != nullptr)
      {
        size_t size = nwave * nbin * ngroup;
        extinction_coefficient = new double[size];
        std::copy(other.extinction_coefficient, other.extinction_coefficient + size, extinction_coefficient);
      }
    }

    // Copy assignment operator
    CARMAParameters& operator=(const CARMAParameters& other)
    {
      if (this != &other)
      {
        // Clean up existing memory
        if (extinction_coefficient != nullptr)
        {
          delete[] extinction_coefficient;
          extinction_coefficient = nullptr;
        }

        // Copy basic members
        max_bins = other.max_bins;
        max_groups = other.max_groups;
        nz = other.nz;
        ny = other.ny;
        nx = other.nx;
        nelem = other.nelem;
        ngroup = other.ngroup;
        nbin = other.nbin;
        nsolute = other.nsolute;
        ngas = other.ngas;
        nwave = other.nwave;
        idx_wave = other.idx_wave;
        dtime = other.dtime;
        nstep = other.nstep;
        deltaz = other.deltaz;
        zmin = other.zmin;

        // Copy extinction coefficient if it exists
        if (other.extinction_coefficient != nullptr)
        {
          size_t size = nwave * nbin * ngroup;
          extinction_coefficient = new double[size];
          std::copy(other.extinction_coefficient, other.extinction_coefficient + size, extinction_coefficient);
        }
      }
      return *this;
    }
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
      params.idx_wave = 5;

      // Time parameters
      params.dtime = 1800.0;
      params.nstep = 432000 / static_cast<int>(params.dtime);

      // Spatial parameters
      params.deltaz = 1000.0;
      params.zmin = 16500.0;

      return params;
    }
  }  // namespace CARMATestConfigs
}  // namespace musica