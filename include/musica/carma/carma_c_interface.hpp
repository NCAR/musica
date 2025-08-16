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
      double center;     // Center of the wavelength bin [m]
      double width;      // Width of the wavelength bin [m]
      bool do_emission;  // Flag to indicate if emission is considered for this bin
    };

    struct CARMAComplexC
    {
      double real;       // Real part
      double imaginary;  // Imaginary part
    };

    struct CARMAGroupConfigC
    {
      int name_length;                // length of name string
      char name[256];                 // 255 chars + null terminator
      int shortname_length;           // length of shortname string
      char shortname[7];              // 6 chars + null terminator
      double rmin;                    // minimum radius [m]
      double rmrat;                   // volume ratio between bins
      double rmassmin;                // minimum mass [kg] (When rmassmin > 0, rmin is ignored)
      int ishape;                     // Particle shape (enum value)
      double eshape;                  // aspect ratio
      int swelling_algorithm;         // Swelling algorithm (enum value)
      int swelling_composition;       // Composition for swelling (enum value)
      int fall_velocity_routine;      // Fall velocity algorithm (enum value)
      int mie_calculation_algorithm;  // Mie calculation algorithm (enum value)
      int optics_algorithm;           // Optics algorithm (enum value)
      bool is_ice;
      bool is_fractal;
      bool is_cloud;
      bool is_sulfate;
      bool do_wetdep;
      bool do_drydep;
      bool do_vtran;
      double solfac;          // Solubility factor for wet deposition
      double scavcoef;        // Scavenging coefficient for wet deposition
      double dpc_threshold;   // convergence criteria for particle concentration [fraction]
      double rmon;            // monomer radius [m]
      double* df;             // fractal dimension per bin (allocated separately)
      int df_size;            // size of df array
      double falpha;          // fractal packing coefficient
      double neutral_volfrc;  // neutral volume fraction for fractal particles
    };

    struct CARMAElementConfigC
    {
      int igroup;
      int isolute;
      int name_length;       // length of name string
      char name[256];        // 255 chars + null terminator
      int shortname_length;  // length of shortname string
      char shortname[7];     // 6 chars + null terminator
      int itype;
      int icomposition;
      bool isShell;
      double rho;
      double* rhobin;         // density per bin (allocated separately)
      int rhobin_size;        // size of rhobin array
      double* arat;           // area ratio per bin (allocated separately)
      int arat_size;          // size of arat array
      double kappa;           // hygroscopicity parameter
      CARMAComplexC* refidx;  // pointer to refractive indices array
      int refidx_dim_1_size;  // size of refractive indices array first dimension
      int refidx_dim_2_size;  // size of refractive indices array second dimension
    };

    // C-Compatible structure for CARMA solute configuration
    struct CARMASoluteConfigC
    {
      int name_length;       // length of name string
      char name[256];        // 255 chars + null terminator
      int shortname_length;  // length of shortname string
      char shortname[7];     // 6 chars + null terminator
      int ions;              // number of ions the solute dissociates into
      double wtmol;          // molar mass of the solute [kg/mol]
      double rho;            // mass density of the solute [kg/m3]
    };

    // C-Compatible structure for CARMA gas species configuration
    struct CARMAGasConfigC
    {
      int name_length;       // length of name string
      char name[256];        // 255 chars + null terminator
      int shortname_length;  // length of shortname string
      char shortname[7];     // 6 chars + null terminator
      double wtmol;          // molar mass of the gas [kg/mol]
      int ivaprtn;           // vaporization routine (enum value)
      int icomposition;      // composition of the gas (enum value)
      double dgc_threshold;  // convergence criteria for gas concentration [0 : off; > 0 : fraction]
      double
          ds_threshold;  // convergence criteria for gas saturation [0 : off; > 0 : fraction; < 0 : amount past 0 crossing]
      CARMAComplexC* refidx;  // pointer to wavelength-resolved refractive indices (allocated separately)
      int refidx_dim_1_size;  // size of first dimension
      int refidx_dim_2_size;  // size of second dimension
    };

    // C-Compatible structure for CARMA coagulation configuration
    struct CARMACoagulationConfigC
    {
      int igroup1;          // first group index (first group to coagulate)
      int igroup2;          // second group index (second group to coagulate)
      int igroup3;          // third group index (coagulated particles)
      int algorithm;        // collection algorithm (enum value)
      double ck0;           // collection efficiency constant (0.0 = off)
      double grav_e_coll0;  // gravitational collection efficiency constant (0.0 = off)
      bool use_ccd;         // use constant collection efficiency data
    };

    // C-Compatible structure for CARMA growth configuration
    struct CARMAGrowthConfigC
    {
      int ielem;  // element index to grow
      int igas;   // gas index to grow from
    };

    // C-Compatible structure for CARMA nucleation configuration
    struct CARMANucleationConfigC
    {
      int ielemfrom;   // element index to nucleate from
      int ielemto;     // element index to nucleate to
      int algorithm;   // nucleation algorithm (enum value)
      double rlh_nuc;  // latent heat of nucleation [m2 s-2]
      int igas;        // gas index to nucleate from
      int ievp2elem;   // element index to evaporate to (if applicable)
    };

    // C-Compatible structure for CARMA initialization configuration
    struct CARMAInitializationConfigC
    {
      bool do_cnst_rlh;       // use constant values for latent heats
      bool do_detrain;        // do detrainment
      bool do_fixedinit;      // use fixed initialization from reference atmosphere
      bool do_incloud;        // do in-cloud processes (growth, coagulation)
      bool do_explised;       // do sedimentation with substepping
      bool do_substep;        // do substepping
      bool do_thermo;         // do thermodynamic processes
      bool do_vdiff;          // do Brownian diffusion
      bool do_vtran;          // do sedimentation
      bool do_drydep;         // do dry deposition
      bool do_pheat;          // do particle heating
      bool do_pheatatm;       // do particle heating of atmosphere
      bool do_clearsky;       // do clear sky growth and coagulation
      bool do_partialinit;    // do initialization of coagulation from reference atmosphere (requires do_fixedinit)
      bool do_coremasscheck;  // check core mass for particles
      int sulfnucl_method;    // method for sulfate nucleation (enum value)
      double vf_const;        // constant fall velocity [m/s] (0: off)
      int minsubsteps;        // minimum number of substeps
      int maxsubsteps;        // maximum number of substeps
      int maxretries;         // maximum number of retries
      double conmax;          // minimum relative concentration to consider
      double dt_threshold;    // convergence criteria for temperature [fraction] (0: off)
      double cstick;          // accommodation coefficient for coagulation
      double gsticki;         // accommodation coefficient for growth of ice
      double gstickl;         // accommodation coefficient for growth of liquid
      double tstick;          // accommodation coefficient temperature
    };

    // C-compatible structure for CARMA element properties
    struct CARMAElementPropertiesC
    {
      int group_index;        // Group index
      int solute_index;       // Solute index
      int composition;        // Composition enum
      int type;               // Type enum
      bool is_shell;          // Is shell flag
      double kappa;           // Hygroscopicity parameter
      double* rho;            // Mass density of the particle element [kg/m³] (allocated separately)
      int rho_size;           // Size of rho array
      CARMAComplex* refidx;   // Refractive indices array [real, imaginary] (allocated separately)
      int refidx_dim_1_size;  // Size of first dimension of refractive indices array (indices)
      int refidx_dim_2_size;  // Size of second dimension of refractive indices array (wavelengths)
    };

    // C-compatible structure for CARMA parameters
    // MUST match the exact order and types of the Fortran carma_parameters_t struct
    struct CCARMAParameters
    {
      // Model dimensions
      int nbin;
      int nz;

      // Time stepping parameters
      double dtime;

      // Wavelength grid
      CARMAWavelengthBinC* wavelength_bins;  // Pointer to wavelength bins array
      int wavelength_bin_size;               // Size of wavelength bin arrays
      int number_of_refractive_indices;      // Number of refractive indices per wavelength

      // Component configurations
      CARMAGroupConfigC* groups;      // Pointer to groups array
      int groups_size;                // Number of groups
      CARMAElementConfigC* elements;  // Pointer to elements array
      int elements_size;              // Number of elements
      CARMASoluteConfigC* solutes;    // Pointer to solutes array
      int solutes_size;               // Number of solutes
      CARMAGasConfigC* gases;         // Pointer to gases array
      int gases_size;                 // Number of gases

      // Process configurations
      CARMACoagulationConfigC* coagulations;  // Pointer to coagulations array
      int coagulations_size;                  // Number of coagulations
      CARMAGrowthConfigC* growths;            // Pointer to growths array
      int growths_size;                       // Number of growths
      CARMANucleationConfigC* nucleations;    // Pointer to nucleations array
      int nucleations_size;                   // Number of nucleations

      // Initialization configuration
      CARMAInitializationConfigC initialization;  // Initialization configuration
    };

    struct CARMAOutputDataC
    {
      void* c_output_ptr;

      // Grid and atmospheric data
      const double* lat;
      const double* lon;
      const double* vertical_center;
      const double* vertical_levels;
      const double* pressure;
      const double* temperature;
      const double* air_density;

      // Fundamental particle state data [nz, nbin, nelem]
      const double* particle_concentration;  // number density [#/cm³]
      const double* mass_mixing_ratio;       // mass mixing ratio [kg/kg]

      // Bin-level particle properties [nz, nbin, ngroup]
      const double* wet_radius;           // wet particle radius [cm]
      const double* wet_density;          // wet particle density [g/cm³]
      const double* fall_velocity;        // fall velocity [cm/s] (nz+1, nbin, ngroup)
      const double* nucleation_rate;      // nucleation rate [1/cm³/s]
      const double* deposition_velocity;  // deposition velocity [cm/s]

      // Group configuration data [nbin, ngroup]
      const double* dry_radius;    // dry particle radius [cm]
      const double* mass_per_bin;  // particle mass [g]
      const double* radius_ratio;  // radius ratio
      const double* area_ratio;    // area ratio

      // Group mapping and properties (integer data stored as doubles)
      const int* group_particle_number_concentration;  // concentration element per group [ngroup]
      const int* constituent_type;                     // constituent type per group [ngroup]
      const int* max_prognostic_bin;                   // max prognostic bin per group [ngroup]
    };

    struct CARMAStateParametersC
    {
      double time;                         // Current time [s]
      double time_step;                    // Time step [s]
      double longitude;                    // Longitude [degrees]
      double latitude;                     // Latitude [degrees]
      int coordinates;                     // Coordinate system
      const double* vertical_center;       // Vertical center heights [m]
      int vertical_center_size;            // Size of vertical center array
      const double* vertical_levels;       // Vertical levels heights [m]
      int vertical_levels_size;            // Size of vertical levels array
      const double* temperature;           // Temperature profile [K]
      int temperature_size;                // Size of temperature array
      const double* pressure;              // Pressure profile [Pa]
      int pressure_size;                   // Size of pressure array
      const double* pressure_levels;       // Pressure levels [Pa]
      int pressure_levels_size;            // Size of pressure levels array
      const double* specific_humidity;     // Specific humidity profile [kg/kg]
      int specific_humidity_size;          // Size of specific humidity array
      const double* relative_humidity;     // Relative humidity profile [fraction]
      int relative_humidity_size;          // Size of relative humidity array
      const double* original_temperature;  // Original temperature profile [K]
      int original_temperature_size;       // Size of original temperature array
      const double* radiative_intensity;   // Radiative intensity [W/m²/sr/m]
      int radiative_intensity_dim_1_size;  // Size of radiative intensity array
      int radiative_intensity_dim_2_size;  // Size of radiative intensity array
    };

    struct CARMASurfacePropertiesC
    {
      double surface_friction_velocity;  // Surface friction velocity [m/s]
      double aerodynamic_resistance;     // Aerodynamic resistance [s/m]
      double area_fraction;              // Area fraction [fraction]
    };

    struct CARMAStateStepConfigC
    {
      const double* cloud_fraction;              // Cloud fraction at vertical centers [fraction]
      int cloud_fraction_size;                   // Size of cloud fraction array
      const double* critical_relative_humidity;  // Critical relative humidity for liquid clouds [fraction]
      int critical_relative_humidity_size;       // Size of critical relative humidity array
      CARMASurfacePropertiesC land;              // Surface properties for land
      CARMASurfacePropertiesC ocean;             // Surface properties for ocean
      CARMASurfacePropertiesC ice;               // Surface properties for ice
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

    // CARMA State management functions
    void* InternalCreateCarmaState(
        void* carma_instance,
        const CCARMAParameters& carma_params,
        const CARMAStateParametersC& state_params,
        int* rc);
    void InternalDestroyCarmaState(void* carma_state_instance, int* rc);

    void InternalSetBin(
        void* carma_state_instance,
        int bin_index,
        int element_index,
        const double* values,
        int values_size,
        double surface_mass,
        int* rc);
    void InternalSetDetrain(
        void* carma_state_instance,
        int bin_index,
        int element_index,
        const double* values,
        int values_size,
        int* rc);
    void InternalSetGas(
        void* carma_state_instance,
        int gas_index,
        const double* values,
        int values_size,
        const double* old_mmr,
        int old_mmr_size,
        const double* gas_saturation_wrt_ice,
        int gas_saturation_wrt_ice_size,
        const double* gas_saturation_wrt_liquid,
        int gas_saturation_wrt_liquid_size,
        int* rc);

    void InternalGetStepStatistics(
        void* carma_state_instance,
        int* max_number_of_substeps,
        double* max_number_of_retries,
        double* total_number_of_steps,
        int* total_number_of_substeps,
        double* total_number_of_retries,
        double* xc,
        double* yc,
        double* z_substeps,
        int nz,
        int* rc);

    void InternalGetBin(
        void* carma_state_instance,
        int bin_index,
        int element_index,
        int nz,
        double* mass_mixing_ratio,
        double* number_mixing_ratio,
        double* number_density,
        double* nucleation_rate,
        double* wet_particle_radius,
        double* wet_particle_density,
        double* dry_particle_density,
        double* particle_mass_on_surface,
        double* sedimentation_flux,
        double* fall_velocity,
        double* deposition_velocity,
        double* delta_particle_temperature,
        double* kappa,
        double* total_mass_mixing_ratio,
        int* rc);

    void InternalGetDetrain(
        void* carma_state_instance,
        int bin_index,
        int element_index,
        int nz,
        double* mass_mixing_ratio,
        double* number_mixing_ratio,
        double* number_density,
        double* wet_particle_radius,
        double* wet_particle_density,
        int* rc);

    void InternalGetGas(
        void* carma_state_instance,
        int gas_index,
        int nz,
        double* mass_mixing_ratio,
        double* gas_saturation_wrt_ice,
        double* gas_saturation_wrt_liquid,
        double* gas_vapor_pressure_wrt_ice,
        double* gas_vapor_pressure_wrt_liquid,
        double* weight_pct_aerosol_composition,
        int* rc);

    void InternalGetEnvironmentalValues(
        void* carma_state_instance,
        int nz,
        double* temperature,
        double* pressure,
        double* air_density,
        double* latent_heat,
        int* rc);

    void InternalSetTemperature(void* carma_state_instance, const double* temperature, int temperature_size, int* rc);

    void InternalSetAirDensity(void* carma_state_instance, const double* air_density, int air_density_size, int* rc);

    void InternalStepCarmaState(void* carma_state_instance, const CARMAStateStepConfigC step_config, int* rc);

    void InternalGetGroupProperties(
        void* carma_instance,
        int group_index,
        int nbin,
        int nwave,
        int nelem,
        double* bin_radius,
        double* bin_radius_lower_bound,
        double* bin_radius_upper_bound,
        double* bin_width,
        double* bin_mass,
        double* bin_width_mass,
        double* bin_volume,
        double* projected_area_ratio,
        double* radius_ratio,
        double* porosity_ratio,
        double* extinction_coefficient,
        double* single_scattering_albedo,
        double* asymmetry_factor,
        int* particle_number_element_for_group,
        int* number_of_core_mass_elements_for_group,
        int* element_index_of_core_mass_elements,
        int* last_prognostic_bin,
        double* numbers_of_monomers_per_bin,
        int* rc);

    void InternalGetElementProperties(
        void* carma_instance,
        int element_index,
        CARMAElementPropertiesC* element_properties,
        int* rc);

#ifdef __cplusplus
  }  // extern "C"
#endif
}  // namespace musica