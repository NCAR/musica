// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <musica/carma/carma.hpp>

namespace musica
{
#ifdef __cplusplus
  extern "C"
  {
#endif

    struct CARMAWavelengthBinC
    {
      double center;    // Center of the wavelength bin [m]
      double width;     // Width of the wavelength bin [m]
      bool do_emission; // Flag to indicate if emission is considered for this bin
    };

    struct CARMAGroupConfigC
    {
      int name_length;       // length of name string
      char name[256];        // 255 chars + null terminator
      int shortname_length;  // length of shortname string
      char shortname[7];     // 6 chars + null terminator
      double rmin;           // minimum radius [m]
      double rmrat;          // volume ratio between bins
      double rmassmin;       // minimum mass [kg] (When rmassmin > 0, rmin is ignored)
      int ishape;            // Particle shape (enum value)
      double eshape;         // aspect ratio
      int swelling_algorithm; // Swelling algorithm (enum value)
      int swelling_composition; // Composition for swelling (enum value)
      int fall_velocity_routine; // Fall velocity algorithm (enum value)
      int mie_calculation_algorithm; // Mie calculation algorithm (enum value)
      int optics_algorithm;  // Optics algorithm (enum value)
      bool is_ice;
      bool is_fractal;
      bool is_cloud;
      bool is_sulfate;
      bool do_wetdep;
      bool do_drydep;
      bool do_vtran;
      double solfac; // Solubility factor for wet deposition
      double scavcoef; // Scavenging coefficient for wet deposition
      double dpc_threshold; // convergence criteria for particle concentration [fraction]
      double rmon; // monomer radius [m]
      double* df;   // fractal dimension per bin (allocated separately)
      int df_size;  // size of df array
      double falpha; // fractal packing coefficient
      double neutral_volfrc; // neutral volume fraction for fractal particles
    };

    struct CARMAElementConfigC
    {
      int id;
      int igroup;
      int name_length;       // length of name string
      char name[256];        // 255 chars + null terminator
      int shortname_length;  // length of shortname string
      char shortname[7];     // 6 chars + null terminator
      double rho;
      int itype;
      int icomposition;
      int isolute;
      double* rhobin;   // density per bin (allocated separately)
      int rhobin_size;  // size of rhobin array
      double* arat;     // area ratio per bin (allocated separately)
      int arat_size;    // size of arat array
      double kappa;
      bool isShell;
    };

    // C-compatible structure for CARMA parameters
    // MUST match the exact order and types of the Fortran carma_parameters_t struct
    struct CCARMAParameters
    {
      // Model dimensions
      int nz;
      int ny;
      int nx;
      int nbin;
      int nsolute;
      int ngas;
      int idx_wave;

      // Time stepping parameters
      double dtime;
      int nstep;

      // Spatial parameters
      double deltaz;
      double zmin;

      // Wavelength grid
      CARMAWavelengthBinC* wavelength_bins;   // Pointer to wavelength bins array
      int wavelength_bin_size;                // Size of wavelength bin arrays
      int number_of_refractive_indices;       // Number of refractive indices per wavelength

      // Optical parameters
      double* extinction_coefficient;   // Pointer to extinction coefficient array
      int extinction_coefficient_size;  // Size of extinction coefficient array

      // Group and element configurations
      CARMAGroupConfigC* groups;      // Pointer to groups array
      int groups_size;                // Number of groups
      CARMAElementConfigC* elements;  // Pointer to elements array
      int elements_size;              // Number of elements
    };

    struct CARMAOutputDataC
    {
      void* c_output_ptr;
      int nz;
      int ny;
      int nx;
      int nelem;
      int ngroup;
      int nbin;
      int ngas;
      int nstep;
      const double* lat;
      const double* lon;
      const double* vertical_center;
      const double* vertical_levels;
      const double* pressure;
      const double* temperature;
      const double* air_density;
      const double* radiative_heating;
      const double* delta_temperature;
      const double* gas_mmr;
      const double* gas_saturation_liquid;
      const double* gas_saturation_ice;
      const double* gas_vapor_pressure_ice;
      const double* gas_vapor_pressure_liquid;
      const double* gas_weight_percent;
      const double* number_density;
      const double* surface_area;
      const double* mass_density;
      const double* effective_radius;
      const double* effective_radius_wet;
      const double* mean_radius;
      const double* nucleation_rate;
      const double* mass_mixing_ratio;
      const double* projected_area;
      const double* aspect_ratio;
      const double* vertical_mass_flux;
      const double* extinction;
      const double* optical_depth;
      const double* bin_wet_radius;
      const double* bin_number_density;
      const double* bin_density;
      const double* bin_mass_mixing_ratio;
      const double* bin_deposition_velocity;
      const double* group_radius;
      const double* group_mass;
      const double* group_volume;
      const double* group_radius_ratio;
      const double* group_aspect_ratio;
      const double* group_fractal_dimension;
    };

    // The external C API for CARMA
    // callable by wrappers in other languages

    char* GetCarmaVersion();

    // for use by musica internally.
    void InternalGetCarmaVersion(char** version_ptr, int* version_length);
    void InternalFreeCarmaVersion(char* version_ptr, int version_length);

    // CARMA instance management functions
    void* InternalCreateCarma(const CCARMAParameters& params, int* rc);
    void InternalDestroyCarma(void* carma_instance, int* rc);

    // CARMA driver interface functions
    void InternalRunCarma(const CCARMAParameters& params, void* carma_instance, void* output, int* rc);

    // Transfer function called from Fortran
    void TransferCarmaOutputToCpp(const CARMAOutputDataC* output_data);

#ifdef __cplusplus
  }  // extern "C"
#endif
}  // namespace musica