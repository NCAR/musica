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
    void TransferCarmaOutputToCpp(const CARMAOutputDataC* output_data)
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

      // Set dimensions
      output->nz = output_data->nz;
      output->ny = output_data->ny;
      output->nx = output_data->nx;
      output->nelem = output_data->nelem;
      output->ngroup = output_data->ngroup;
      output->nbin = output_data->nbin;
      output->ngas = output_data->ngas;
      output->nstep = output_data->nstep;

      // Copy coordinate data
      output->lat.resize(output_data->ny);
      for (int i = 0; i < output_data->ny; ++i)
      {
        output->lat[i] = output_data->lat[i];
      }

      output->lon.resize(output_data->nx);
      for (int i = 0; i < output_data->nx; ++i)
      {
        output->lon[i] = output_data->lon[i];
      }

      output->vertical_center.resize(output_data->nz);
      for (int i = 0; i < output_data->nz; ++i)
      {
        output->vertical_center[i] = output_data->vertical_center[i];
      }

      output->vertical_levels.resize(output_data->nz + 1);
      for (int i = 0; i < output_data->nz + 1; ++i)
      {
        output->vertical_levels[i] = output_data->vertical_levels[i];
      }

      // Copy atmospheric state variables
      output->pressure.resize(output_data->nz);
      for (int i = 0; i < output_data->nz; ++i)
      {
        output->pressure[i] = output_data->pressure[i];
      }

      output->temperature.resize(output_data->nz);
      for (int i = 0; i < output_data->nz; ++i)
      {
        output->temperature[i] = output_data->temperature[i];
      }

      output->air_density.resize(output_data->nz);
      for (int i = 0; i < output_data->nz; ++i)
      {
        output->air_density[i] = output_data->air_density[i];
      }

      output->radiative_heating.resize(output_data->nz);
      for (int i = 0; i < output_data->nz; ++i)
      {
        output->radiative_heating[i] = output_data->radiative_heating[i];
      }

      output->delta_temperature.resize(output_data->nz);
      for (int i = 0; i < output_data->nz; ++i)
      {
        output->delta_temperature[i] = output_data->delta_temperature[i];
      }

      // Copy gas variables (2D arrays: nz x ngas)
      if (output_data->ngas > 0)
      {
        output->gas_mmr.resize(output_data->nz);
        output->gas_saturation_liquid.resize(output_data->nz);
        output->gas_saturation_ice.resize(output_data->nz);
        output->gas_vapor_pressure_ice.resize(output_data->nz);
        output->gas_vapor_pressure_liquid.resize(output_data->nz);
        output->gas_weight_percent.resize(output_data->nz);

        for (int iz = 0; iz < output_data->nz; ++iz)
        {
          output->gas_mmr[iz].resize(output_data->ngas);
          output->gas_saturation_liquid[iz].resize(output_data->ngas);
          output->gas_saturation_ice[iz].resize(output_data->ngas);
          output->gas_vapor_pressure_ice[iz].resize(output_data->ngas);
          output->gas_vapor_pressure_liquid[iz].resize(output_data->ngas);
          output->gas_weight_percent[iz].resize(output_data->ngas);

          for (int ig = 0; ig < output_data->ngas; ++ig)
          {
            // Fortran uses column-major ordering: arr(nz, ngas)
            // C index = iz + ig * nz
            int idx = iz + ig * output_data->nz;
            output->gas_mmr[iz][ig] = output_data->gas_mmr[idx];
            output->gas_saturation_liquid[iz][ig] = output_data->gas_saturation_liquid[idx];
            output->gas_saturation_ice[iz][ig] = output_data->gas_saturation_ice[idx];
            output->gas_vapor_pressure_ice[iz][ig] = output_data->gas_vapor_pressure_ice[idx];
            output->gas_vapor_pressure_liquid[iz][ig] = output_data->gas_vapor_pressure_liquid[idx];
            output->gas_weight_percent[iz][ig] = output_data->gas_weight_percent[idx];
          }
        }
      }

      // Copy group-integrated variables (2D arrays)
      output->number_density.resize(output_data->nz);
      output->surface_area.resize(output_data->nz);
      output->mass_density.resize(output_data->nz);
      output->effective_radius.resize(output_data->nz);
      output->effective_radius_wet.resize(output_data->nz);
      output->mean_radius.resize(output_data->nz);
      output->nucleation_rate.resize(output_data->nz);
      output->mass_mixing_ratio.resize(output_data->nz);
      output->projected_area.resize(output_data->nz);
      output->aspect_ratio.resize(output_data->nz);
      output->vertical_mass_flux.resize(output_data->nz);
      output->extinction.resize(output_data->nz);
      output->optical_depth.resize(output_data->nz);

      for (int iz = 0; iz < output_data->nz; ++iz)
      {
        output->number_density[iz].resize(output_data->ngroup);
        output->surface_area[iz].resize(output_data->ngroup);
        output->mass_density[iz].resize(output_data->ngroup);
        output->effective_radius[iz].resize(output_data->ngroup);
        output->effective_radius_wet[iz].resize(output_data->ngroup);
        output->mean_radius[iz].resize(output_data->ngroup);
        output->nucleation_rate[iz].resize(output_data->ngroup);
        output->mass_mixing_ratio[iz].resize(output_data->ngroup);
        output->projected_area[iz].resize(output_data->ngroup);
        output->aspect_ratio[iz].resize(output_data->ngroup);
        output->vertical_mass_flux[iz].resize(output_data->ngroup);
        output->extinction[iz].resize(output_data->ngroup);
        output->optical_depth[iz].resize(output_data->ngroup);

        for (int ig = 0; ig < output_data->ngroup; ++ig)
        {
          // Fortran uses column-major ordering: arr(nz, ngroup)
          // C index = iz + ig * nz
          int idx = iz + ig * output_data->nz;
          output->number_density[iz][ig] = output_data->number_density[idx];
          output->surface_area[iz][ig] = output_data->surface_area[idx];
          output->mass_density[iz][ig] = output_data->mass_density[idx];
          output->effective_radius[iz][ig] = output_data->effective_radius[idx];
          output->effective_radius_wet[iz][ig] = output_data->effective_radius_wet[idx];
          output->mean_radius[iz][ig] = output_data->mean_radius[idx];
          output->nucleation_rate[iz][ig] = output_data->nucleation_rate[idx];
          output->mass_mixing_ratio[iz][ig] = output_data->mass_mixing_ratio[idx];
          output->projected_area[iz][ig] = output_data->projected_area[idx];
          output->aspect_ratio[iz][ig] = output_data->aspect_ratio[idx];
          output->vertical_mass_flux[iz][ig] = output_data->vertical_mass_flux[idx];
          output->extinction[iz][ig] = output_data->extinction[idx];
          output->optical_depth[iz][ig] = output_data->optical_depth[idx];
        }
      }

      // Copy bin-resolved variables (3D arrays)
      output->bin_wet_radius.resize(output_data->nz);
      output->bin_number_density.resize(output_data->nz);
      output->bin_density.resize(output_data->nz);
      output->bin_mass_mixing_ratio.resize(output_data->nz);
      output->bin_deposition_velocity.resize(output_data->nz);

      for (int iz = 0; iz < output_data->nz; ++iz)
      {
        output->bin_wet_radius[iz].resize(output_data->ngroup);
        output->bin_number_density[iz].resize(output_data->ngroup);
        output->bin_density[iz].resize(output_data->ngroup);
        output->bin_mass_mixing_ratio[iz].resize(output_data->ngroup);
        output->bin_deposition_velocity[iz].resize(output_data->ngroup);

        for (int ig = 0; ig < output_data->ngroup; ++ig)
        {
          output->bin_wet_radius[iz][ig].resize(output_data->nbin);
          output->bin_number_density[iz][ig].resize(output_data->nbin);
          output->bin_density[iz][ig].resize(output_data->nbin);
          output->bin_mass_mixing_ratio[iz][ig].resize(output_data->nbin);
          output->bin_deposition_velocity[iz][ig].resize(output_data->nbin);

          for (int ib = 0; ib < output_data->nbin; ++ib)
          {
            int idx = ib;
            output->bin_wet_radius[iz][ig][ib] = output_data->bin_wet_radius[idx];
            output->bin_number_density[iz][ig][ib] = output_data->bin_number_density[idx];
            output->bin_density[iz][ig][ib] = output_data->bin_density[idx];
            output->bin_mass_mixing_ratio[iz][ig][ib] = output_data->bin_mass_mixing_ratio[idx];
            output->bin_deposition_velocity[iz][ig][ib] = output_data->bin_deposition_velocity[idx];
          }
        }
      }

      // Copy group properties (2D arrays: nbin x ngroup)
      output->group_radius.resize(output_data->nbin);
      output->group_mass.resize(output_data->nbin);
      output->group_volume.resize(output_data->nbin);
      output->group_radius_ratio.resize(output_data->nbin);
      output->group_aspect_ratio.resize(output_data->nbin);
      output->group_fractal_dimension.resize(output_data->nbin);

      for (int ib = 0; ib < output_data->nbin; ++ib)
      {
        output->group_radius[ib].resize(output_data->ngroup);
        output->group_mass[ib].resize(output_data->ngroup);
        output->group_volume[ib].resize(output_data->ngroup);
        output->group_radius_ratio[ib].resize(output_data->ngroup);
        output->group_aspect_ratio[ib].resize(output_data->ngroup);
        output->group_fractal_dimension[ib].resize(output_data->ngroup);

        for (int ig = 0; ig < output_data->ngroup; ++ig)
        {
          // Fortran uses column-major ordering: arr(nbin, ngroup)
          // C index = ib + ig * nbin
          int idx = ib + ig * output_data->nbin;
          output->group_radius[ib][ig] = output_data->group_radius[idx];
          output->group_mass[ib][ig] = output_data->group_mass[idx];
          output->group_volume[ib][ig] = output_data->group_volume[idx];
          output->group_radius_ratio[ib][ig] = output_data->group_radius_ratio[idx];
          output->group_aspect_ratio[ib][ig] = output_data->group_aspect_ratio[idx];
          output->group_fractal_dimension[ib][ig] = output_data->group_fractal_dimension[idx];
        }
      }

      // Set default names for groups and elements
      output->element_names.resize(output_data->nelem);
      for (int i = 0; i < output_data->nelem; ++i)
      {
        output->element_names[i] = "Element_" + std::to_string(i + 1);
      }

      output->group_names.resize(output_data->ngroup);
      for (int i = 0; i < output_data->ngroup; ++i)
      {
        output->group_names[i] = "Group_" + std::to_string(i + 1);
      }

      output->gas_names.resize(output_data->ngas);
      for (int i = 0; i < output_data->ngas; ++i)
      {
        output->gas_names[i] = "Gas_" + std::to_string(i + 1);
      }
    }

#ifdef __cplusplus
  }  // extern "C"
#endif
}  // namespace musica