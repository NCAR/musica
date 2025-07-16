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
    void Run(const CARMAParameters& params);
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