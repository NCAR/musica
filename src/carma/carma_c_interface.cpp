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
        const CARMAOutputDataC* output_data,
        int nz,
        int ny,
        int nx,
        int nbin,
        int nelem,
        int ngroup,
        int nwave)
    {
      if (!output_data)
      {
        std::cerr << "Error: null output_data pointer in TransferCarmaOutputToCpp" << std::endl;
        return;
      }

      CARMAOutput* output = static_cast<CARMAOutput*>(output_data->c_output_ptr);
      if (!output)
      {
        std::cerr << "Error: null output pointer in TransferCarmaOutputToCpp" << std::endl;
        return;
      }

      // Copy coordinate data
      output->lat.resize(ny);
      for (int i = 0; i < ny; ++i)
      {
        output->lat[i] = output_data->lat[i];
      }

      output->lon.resize(nx);
      for (int i = 0; i < nx; ++i)
      {
        output->lon[i] = output_data->lon[i];
      }

      output->vertical_center.resize(nz);
      for (int i = 0; i < nz; ++i)
      {
        output->vertical_center[i] = output_data->vertical_center[i];
      }

      output->vertical_levels.resize(nz + 1);
      for (int i = 0; i < nz + 1; ++i)
      {
        output->vertical_levels[i] = output_data->vertical_levels[i];
      }

      // Copy atmospheric state variables
      output->pressure.resize(nz);
      for (int i = 0; i < nz; ++i)
      {
        output->pressure[i] = output_data->pressure[i];
      }

      output->temperature.resize(nz);
      for (int i = 0; i < nz; ++i)
      {
        output->temperature[i] = output_data->temperature[i];
      }

      output->air_density.resize(nz);
      for (int i = 0; i < nz; ++i)
      {
        output->air_density[i] = output_data->air_density[i];
      }

      output->particle_concentration.resize(nz);
      for (int iz = 0; iz < nz; ++iz)
      {
        output->particle_concentration[iz].resize(nbin);
        for (int ib = 0; ib < nbin; ++ib)
        {
          output->particle_concentration[iz][ib].resize(nelem);
          for (int ie = 0; ie < nelem; ++ie)
          {
            // Fortran: pc(nz, nbin, nelem) -> C index = iz + ib*nz + ie*nz*nbin
            int idx = iz + ib * nz + ie * nz * nbin;
            output->particle_concentration[iz][ib][ie] = output_data->particle_concentration[idx];
          }
        }
      }

      output->mass_mixing_ratio.resize(nz);
      for (int iz = 0; iz < nz; ++iz)
      {
        output->mass_mixing_ratio[iz].resize(nbin);
        for (int ib = 0; ib < nbin; ++ib)
        {
          output->mass_mixing_ratio[iz][ib].resize(nelem);
          for (int ie = 0; ie < nelem; ++ie)
          {
            int idx = iz + ib * nz + ie * nz * nbin;
            output->mass_mixing_ratio[iz][ib][ie] = output_data->mass_mixing_ratio[idx];
          }
        }
      }

      output->wet_radius.resize(nz);
      for (int iz = 0; iz < nz; ++iz)
      {
        output->wet_radius[iz].resize(nbin);
        for (int ib = 0; ib < nbin; ++ib)
        {
          output->wet_radius[iz][ib].resize(ngroup);
          for (int ig = 0; ig < ngroup; ++ig)
          {
            // Fortran: r_wet(nz, nbin, ngroup) -> C index = iz + ib*nz + ig*nz*nbin
            int idx = iz + ib * nz + ig * nz * nbin;
            output->wet_radius[iz][ib][ig] = output_data->wet_radius[idx];
          }
        }
      }

      output->wet_density.resize(nz);
      for (int iz = 0; iz < nz; ++iz)
      {
        output->wet_density[iz].resize(nbin);
        for (int ib = 0; ib < nbin; ++ib)
        {
          output->wet_density[iz][ib].resize(ngroup);
          for (int ig = 0; ig < ngroup; ++ig)
          {
            int idx = iz + ib * nz + ig * nz * nbin;
            output->wet_density[iz][ib][ig] = output_data->wet_density[idx];
          }
        }
      }

      output->fall_velocity.resize(nz + 1);
      for (int iz = 0; iz < nz + 1; ++iz)
      {
        output->fall_velocity[iz].resize(nbin);
        for (int ib = 0; ib < nbin; ++ib)
        {
          output->fall_velocity[iz][ib].resize(ngroup);
          for (int ig = 0; ig < ngroup; ++ig)
          {
            // Fortran: vf(nz+1, nbin, ngroup) -> C index = iz + ib*(nz+1) + ig*(nz+1)*nbin
            int idx = iz + ib * (nz + 1) + ig * (nz + 1) * nbin;
            output->fall_velocity[iz][ib][ig] = output_data->fall_velocity[idx];
          }
        }
      }

      // Nucleation rate (3D: nz x nbin x ngroup)
      output->nucleation_rate.resize(nz);
      for (int iz = 0; iz < nz; ++iz)
      {
        output->nucleation_rate[iz].resize(nbin);
        for (int ib = 0; ib < nbin; ++ib)
        {
          output->nucleation_rate[iz][ib].resize(ngroup);
          for (int ig = 0; ig < ngroup; ++ig)
          {
            int idx = iz + ib * nz + ig * nz * nbin;
            output->nucleation_rate[iz][ib][ig] = output_data->nucleation_rate[idx];
          }
        }
      }

      // Deposition velocity (3D: nz x nbin x ngroup)
      output->deposition_velocity.resize(nz);
      for (int iz = 0; iz < nz; ++iz)
      {
        output->deposition_velocity[iz].resize(nbin);
        for (int ib = 0; ib < nbin; ++ib)
        {
          output->deposition_velocity[iz][ib].resize(ngroup);
          for (int ig = 0; ig < ngroup; ++ig)
          {
            int idx = iz + ib * nz + ig * nz * nbin;
            output->deposition_velocity[iz][ib][ig] = output_data->deposition_velocity[idx];
          }
        }
      }

      // Dry radius
      output->dry_radius.resize(nbin);
      for (int ib = 0; ib < nbin; ++ib)
      {
        output->dry_radius[ib].resize(ngroup);
        for (int ig = 0; ig < ngroup; ++ig)
        {
          // Fortran: r_dry(nbin, ngroup) -> C index = ib + ig * nbin
          int idx = ib + ig * nbin;
          output->dry_radius[ib][ig] = output_data->dry_radius[idx];
        }
      }

      // Mass per bin
      output->mass_per_bin.resize(nbin);
      for (int ib = 0; ib < nbin; ++ib)
      {
        output->mass_per_bin[ib].resize(ngroup);
        for (int ig = 0; ig < ngroup; ++ig)
        {
          int idx = ib + ig * nbin;
          output->mass_per_bin[ib][ig] = output_data->mass_per_bin[idx];
        }
      }

      // Radius ratio
      output->radius_ratio.resize(nbin);
      for (int ib = 0; ib < nbin; ++ib)
      {
        output->radius_ratio[ib].resize(ngroup);
        for (int ig = 0; ig < ngroup; ++ig)
        {
          int idx = ib + ig * nbin;
          output->radius_ratio[ib][ig] = output_data->radius_ratio[idx];
        }
      }

      // Area ratio
      output->aspect_ratio.resize(nbin);
      for (int ib = 0; ib < nbin; ++ib)
      {
        output->aspect_ratio[ib].resize(ngroup);
        for (int ig = 0; ig < ngroup; ++ig)
        {
          int idx = ib + ig * nbin;
          output->aspect_ratio[ib][ig] = output_data->area_ratio[idx];
        }
      }

      // Group mapping and properties (1D arrays)
      output->group_particle_number_concentration.resize(ngroup);
      for (int ig = 0; ig < ngroup; ++ig)
      {
        output->group_particle_number_concentration[ig] = output_data->group_particle_number_concentration[ig];
      }

      output->constituent_type.resize(ngroup);
      for (int ig = 0; ig < ngroup; ++ig)
      {
        output->constituent_type[ig] = output_data->constituent_type[ig];
      }

      output->max_prognostic_bin.resize(ngroup);
      for (int ig = 0; ig < ngroup; ++ig)
      {
        output->max_prognostic_bin[ig] = output_data->max_prognostic_bin[ig];
      }

      output->extinction.resize(nwave);
      for (int iw = 0; iw < nwave; ++iw)
      {
        output->extinction[iw].resize(nbin);
        for (int ib = 0; ib < nbin; ++ib)
        {
          output->extinction[iw][ib].resize(ngroup);
          for (int ig = 0; ig < ngroup; ++ig)
          {
            // Fortran: qext(nwave, nbin, ngroup) -> C index = iw + ib*nwave + ig*nwave*nbin
            int idx = iw + ib * nwave + ig * nwave * nbin;
            output->extinction[iw][ib][ig] = output_data->extinction_efficiency[idx];
          }
        }
      }
    }

#ifdef __cplusplus
  }  // extern "C"
#endif
}  // namespace musica