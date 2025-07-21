// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
#include <musica/carma/carma.hpp>
#include <musica/carma/carma_c_interface.hpp>

#include <cstring>
#include <iostream>
#include <vector>

namespace musica
{
#ifdef __cplusplus
  extern "C"
  {
#endif

    // The external C API for CARMA
    // callable by wrappers in other languages

    char* GetCarmaVersion()
    {
      char* version_ptr = nullptr;
      int version_length = 0;
      char* return_value = nullptr;

      InternalGetCarmaVersion(&version_ptr, &version_length);

      return_value = new char[version_length + 1];
      std::memcpy(return_value, version_ptr, version_length);
      return_value[version_length] = '\0';

      InternalFreeCarmaVersion(version_ptr, version_length);
      return return_value;
    }

    // This function is called from Fortran to populate the C++ output structure
    void TransferCarmaOutputToCpp(
        void* c_output_ptr,  // Actually a CARMAOutput* but using void* for C compatibility
        int nz,
        int ny,
        int nx,
        int nelem,
        int ngroup,
        int nbin,
        int ngas,
        int nstep,
        double current_time,
        int current_step,
        const float* lat,
        const float* lon,
        const float* zc,
        const float* zl,
        const float* pressure,
        const float* temperature,
        const float* air_density,
        const float* number_density,
        const float* surface_area,
        const float* mass_density,
        const float* effective_radius,
        const float* mass_mixing_ratio,
        const float* bin_wet_radius,
        const float* bin_number_density,
        const float* bin_mass_mixing_ratio)
    {
      CARMAOutput* output = static_cast<CARMAOutput*>(c_output_ptr);
      if (!output)
      {
        std::cerr << "Error: null output pointer in TransferCarmaOutputToCpp" << std::endl;
        return;
      }

      // Set dimensions
      output->nz = nz;
      output->ny = ny;
      output->nx = nx;
      output->nelem = nelem;
      output->ngroup = ngroup;
      output->nbin = nbin;
      output->ngas = ngas;
      output->nstep = nstep;

      // Set time information
      output->current_time = current_time;
      output->current_step = current_step;

      // Copy coordinate data
      output->lat.resize(ny);
      for (int i = 0; i < ny; ++i)
      {
        output->lat[i] = static_cast<double>(lat[i]);
      }

      output->lon.resize(nx);
      for (int i = 0; i < nx; ++i)
      {
        output->lon[i] = static_cast<double>(lon[i]);
      }

      output->zc.resize(nz);
      for (int i = 0; i < nz; ++i)
      {
        output->zc[i] = static_cast<double>(zc[i]);
      }

      output->zl.resize(nz + 1);
      for (int i = 0; i < nz + 1; ++i)
      {
        output->zl[i] = static_cast<double>(zl[i]);
      }

      // Copy atmospheric state variables
      output->pressure.resize(nz);
      for (int i = 0; i < nz; ++i)
      {
        output->pressure[i] = static_cast<double>(pressure[i]);
      }

      output->temperature.resize(nz);
      for (int i = 0; i < nz; ++i)
      {
        output->temperature[i] = static_cast<double>(temperature[i]);
      }

      output->air_density.resize(nz);
      for (int i = 0; i < nz; ++i)
      {
        output->air_density[i] = static_cast<double>(air_density[i]);
      }

      // Copy group-integrated variables (2D arrays)
      output->number_density.resize(nz);
      for (int iz = 0; iz < nz; ++iz)
      {
        output->number_density[iz].resize(ngroup);
        for (int ig = 0; ig < ngroup; ++ig)
        {
          output->number_density[iz][ig] = static_cast<double>(number_density[iz * ngroup + ig]);
        }
      }

      output->surface_area.resize(nz);
      for (int iz = 0; iz < nz; ++iz)
      {
        output->surface_area[iz].resize(ngroup);
        for (int ig = 0; ig < ngroup; ++ig)
        {
          output->surface_area[iz][ig] = static_cast<double>(surface_area[iz * ngroup + ig]);
        }
      }

      output->mass_density.resize(nz);
      for (int iz = 0; iz < nz; ++iz)
      {
        output->mass_density[iz].resize(ngroup);
        for (int ig = 0; ig < ngroup; ++ig)
        {
          output->mass_density[iz][ig] = static_cast<double>(mass_density[iz * ngroup + ig]);
        }
      }

      output->effective_radius.resize(nz);
      for (int iz = 0; iz < nz; ++iz)
      {
        output->effective_radius[iz].resize(ngroup);
        for (int ig = 0; ig < ngroup; ++ig)
        {
          output->effective_radius[iz][ig] = static_cast<double>(effective_radius[iz * ngroup + ig]);
        }
      }

      output->mass_mixing_ratio.resize(nz);
      for (int iz = 0; iz < nz; ++iz)
      {
        output->mass_mixing_ratio[iz].resize(ngroup);
        for (int ig = 0; ig < ngroup; ++ig)
        {
          output->mass_mixing_ratio[iz][ig] = static_cast<double>(mass_mixing_ratio[iz * ngroup + ig]);
        }
      }

      // Copy bin-resolved variables (3D arrays)
      output->bin_wet_radius.resize(nz);
      for (int iz = 0; iz < nz; ++iz)
      {
        output->bin_wet_radius[iz].resize(ngroup);
        for (int ig = 0; ig < ngroup; ++ig)
        {
          output->bin_wet_radius[iz][ig].resize(nbin);
          for (int ib = 0; ib < nbin; ++ib)
          {
            int idx = iz * ngroup * nbin + ig * nbin + ib;
            output->bin_wet_radius[iz][ig][ib] = static_cast<double>(bin_wet_radius[idx]);
          }
        }
      }

      output->bin_number_density.resize(nz);
      for (int iz = 0; iz < nz; ++iz)
      {
        output->bin_number_density[iz].resize(ngroup);
        for (int ig = 0; ig < ngroup; ++ig)
        {
          output->bin_number_density[iz][ig].resize(nbin);
          for (int ib = 0; ib < nbin; ++ib)
          {
            int idx = iz * ngroup * nbin + ig * nbin + ib;
            output->bin_number_density[iz][ig][ib] = static_cast<double>(bin_number_density[idx]);
          }
        }
      }

      output->bin_mass_mixing_ratio.resize(nz);
      for (int iz = 0; iz < nz; ++iz)
      {
        output->bin_mass_mixing_ratio[iz].resize(ngroup);
        for (int ig = 0; ig < ngroup; ++ig)
        {
          output->bin_mass_mixing_ratio[iz][ig].resize(nbin);
          for (int ib = 0; ib < nbin; ++ib)
          {
            int idx = iz * ngroup * nbin + ig * nbin + ib;
            output->bin_mass_mixing_ratio[iz][ig][ib] = static_cast<double>(bin_mass_mixing_ratio[idx]);
          }
        }
      }

      // Set default names for groups and elements
      output->element_names.resize(nelem);
      for (int i = 0; i < nelem; ++i)
      {
        output->element_names[i] = "Element_" + std::to_string(i + 1);
      }

      output->group_names.resize(ngroup);
      for (int i = 0; i < ngroup; ++i)
      {
        output->group_names[i] = "Group_" + std::to_string(i + 1);
      }

      output->gas_names.resize(ngas);
      for (int i = 0; i < ngas; ++i)
      {
        output->gas_names[i] = "Gas_" + std::to_string(i + 1);
      }

      std::cout << "Successfully transferred CARMA output data to C++ structure." << std::endl;
      std::cout << "Dimensions: nz=" << nz << ", ngroup=" << ngroup << ", nbin=" << nbin << std::endl;
    }

#ifdef __cplusplus
  }  // extern "C"
#endif
}  // namespace musica