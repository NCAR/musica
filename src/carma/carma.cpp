// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file contains the implementation of the CARMA class, which creates C connections
// to the CARMA aerosol model, allowing it to be used in a C++ context.
#include <musica/carma/carma.hpp>
#include <musica/carma/carma_c_interface.hpp>

#include <cstring>
#include <stdexcept>

namespace musica
{
  CARMA::CARMA(const CARMAParameters& params)
  {
    int return_code = 0;
    carma_parameters_ = ToCCompatible(params);
    f_carma_type_ = InternalCreateCarma(*carma_parameters_, &return_code);
    if (return_code != 0)
    {
      throw std::runtime_error("Failed to create CARMA instance with return code: " + std::to_string(return_code));
    }
  }

  CARMA::~CARMA()
  {
    int return_code = 0;
    FreeCCompatible(carma_parameters_);
    InternalDestroyCarma(f_carma_type_, &return_code);
    f_carma_type_ = nullptr;  // Clear the pointer to avoid dangling pointer
  }

  std::string CARMA::GetVersion()
  {
    char* version_ptr = nullptr;
    int version_length = 0;

    InternalGetCarmaVersion(&version_ptr, &version_length);

    std::string version(version_ptr, version_length);

    // Free the memory allocated by Fortran
    InternalFreeCarmaVersion(version_ptr, version_length);

    return version;
  }

  CARMAOutput CARMA::Run()
  {
    int return_code = 0;
    CARMAOutput output;

    InternalRunCarma(*carma_parameters_, f_carma_type_, static_cast<void*>(&output), &return_code);

    if (return_code != 0)
    {
      throw std::runtime_error("CARMA simulation failed with return code: " + std::to_string(return_code));
    }

    return output;
  }

  CCARMAParameters* CARMA::ToCCompatible(const CARMAParameters& params)
  {
    CCARMAParameters* c_params = new CCARMAParameters();

    // Copy simple scalar values
    c_params->nz = params.nz;
    c_params->ny = params.ny;
    c_params->nx = params.nx;
    c_params->nbin = params.nbin;
    c_params->dtime = params.dtime;
    c_params->nstep = params.nstep;
    c_params->deltaz = params.deltaz;
    c_params->zmin = params.zmin;

    // Handle wavelength grid
    if (!params.wavelength_bins.empty())
    {
      c_params->wavelength_bin_size = static_cast<int>(params.wavelength_bins.size());
      c_params->wavelength_bins = new CARMAWavelengthBinC[c_params->wavelength_bin_size];
      for (int i = 0; i < c_params->wavelength_bin_size; ++i)
      {
        c_params->wavelength_bins[i].center = params.wavelength_bins[i].center;
        c_params->wavelength_bins[i].width = params.wavelength_bins[i].width;
        c_params->wavelength_bins[i].do_emission = params.wavelength_bins[i].do_emission;
      }
    }
    else
    {
      c_params->wavelength_bins = nullptr;
      c_params->wavelength_bin_size = 0;
    }

    // Handle number of refractive indices
    c_params->number_of_refractive_indices = params.number_of_refractive_indices;

    // Handle groups array
    if (!params.groups.empty())
    {
      c_params->groups_size = static_cast<int>(params.groups.size());
      c_params->groups = new CARMAGroupConfigC[c_params->groups_size];

      for (int i = 0; i < c_params->groups_size; ++i)
      {
        const auto& group = params.groups[i];
        auto& c_group = c_params->groups[i];

        c_group.name_length = std::min(static_cast<int>(group.name.length()), 255);
        std::strncpy(c_group.name, group.name.c_str(), 255);
        c_group.name[255] = '\0';

        c_group.shortname_length = std::min(static_cast<int>(group.shortname.length()), 6);
        std::strncpy(c_group.shortname, group.shortname.c_str(), 6);
        c_group.shortname[6] = '\0';

        c_group.rmin = group.rmin;
        c_group.rmrat = group.rmrat;
        c_group.rmassmin = group.rmassmin;
        c_group.ishape = static_cast<int>(group.ishape);
        c_group.eshape = group.eshape;
        c_group.swelling_algorithm = static_cast<int>(group.swelling_approach.algorithm);
        c_group.swelling_composition = static_cast<int>(group.swelling_approach.composition);
        c_group.fall_velocity_routine = static_cast<int>(group.fall_velocity_routine);
        c_group.mie_calculation_algorithm = static_cast<int>(group.mie_calculation_algorithm);
        c_group.optics_algorithm = static_cast<int>(group.optics_algorithm);
        c_group.is_ice = group.is_ice;
        c_group.is_fractal = group.is_fractal;
        c_group.is_cloud = group.is_cloud;
        c_group.is_sulfate = group.is_sulfate;
        c_group.do_wetdep = group.do_wetdep;
        c_group.do_drydep = group.do_drydep;
        c_group.do_vtran = group.do_vtran;
        c_group.solfac = group.solfac;
        c_group.scavcoef = group.scavcoef;
        c_group.dpc_threshold = group.dpc_threshold;
        c_group.rmon = group.rmon;
        c_group.falpha = group.falpha;
        c_group.neutral_volfrc = group.neutral_volfrc;

        // Handle df array
        if (!group.df.empty())
        {
          c_group.df_size = static_cast<int>(group.df.size());
          c_group.df = new double[c_group.df_size];
          std::memcpy(c_group.df, group.df.data(), c_group.df_size * sizeof(double));
        }
        else
        {
          c_group.df = nullptr;
          c_group.df_size = 0;
        }
      }
    }
    else
    {
      c_params->groups = nullptr;
      c_params->groups_size = 0;
    }

    // Handle elements array
    if (!params.elements.empty())
    {
      c_params->elements_size = static_cast<int>(params.elements.size());
      c_params->elements = new CARMAElementConfigC[c_params->elements_size];

      for (int i = 0; i < c_params->elements_size; ++i)
      {
        const auto& element = params.elements[i];
        auto& c_element = c_params->elements[i];

        c_element.igroup = element.igroup;
        c_element.isolute = element.isolute;

        c_element.name_length = std::min(static_cast<int>(element.name.length()), 255);
        std::strncpy(c_element.name, element.name.c_str(), 255);
        c_element.name[255] = '\0';

        c_element.shortname_length = std::min(static_cast<int>(element.shortname.length()), 6);
        std::strncpy(c_element.shortname, element.shortname.c_str(), 6);
        c_element.shortname[6] = '\0';

        c_element.itype = static_cast<int>(element.itype);
        c_element.icomposition = static_cast<int>(element.icomposition);
        c_element.isShell = element.isShell;
        c_element.rho = element.rho;
        c_element.kappa = element.kappa;

        // Handle rhobin array
        if (!element.rhobin.empty())
        {
          c_element.rhobin_size = static_cast<int>(element.rhobin.size());
          c_element.rhobin = new double[c_element.rhobin_size];
          std::memcpy(c_element.rhobin, element.rhobin.data(), c_element.rhobin_size * sizeof(double));
        }
        else
        {
          c_element.rhobin = nullptr;
          c_element.rhobin_size = 0;
        }

        // Handle arat array
        if (!element.arat.empty())
        {
          c_element.arat_size = static_cast<int>(element.arat.size());
          c_element.arat = new double[c_element.arat_size];
          std::memcpy(c_element.arat, element.arat.data(), c_element.arat_size * sizeof(double));
        }
        else
        {
          c_element.arat = nullptr;
          c_element.arat_size = 0;
        }

        // Handle refractive indices
        if (!element.refidx.empty())
        {
          c_element.refidx_dim_1_size = static_cast<int>(element.refidx.size());
          c_element.refidx_dim_2_size = static_cast<int>(element.refidx[0].size());
          c_element.refidx = new CARMAComplexC[c_element.refidx_dim_1_size * c_element.refidx_dim_2_size];
          for (int j = 0; j < c_element.refidx_dim_1_size; ++j)
          {
            for (int k = 0; k < c_element.refidx_dim_2_size; ++k)
            {
              c_element.refidx[j * c_element.refidx_dim_2_size + k].real = element.refidx[j][k].real;
              c_element.refidx[j * c_element.refidx_dim_2_size + k].imaginary = element.refidx[j][k].imaginary;
            }
          }
        }
        else
        {
          c_element.refidx = nullptr;
          c_element.refidx_dim_1_size = 0;
          c_element.refidx_dim_2_size = 0;
        }
      }
    }
    else
    {
      c_params->elements = nullptr;
      c_params->elements_size = 0;
    }

    // Handle solutes array
    if (!params.solutes.empty())
    {
      c_params->solutes_size = static_cast<int>(params.solutes.size());
      c_params->solutes = new CARMASoluteConfigC[c_params->solutes_size];

      for (int i = 0; i < c_params->solutes_size; ++i)
      {
        const auto& solute = params.solutes[i];
        auto& c_solute = c_params->solutes[i];

        c_solute.name_length = std::min(static_cast<int>(solute.name.length()), 255);
        std::strncpy(c_solute.name, solute.name.c_str(), c_solute.name_length);
        c_solute.name[c_solute.name_length] = '\0';

        c_solute.shortname_length = std::min(static_cast<int>(solute.shortname.length()), 6);
        std::strncpy(c_solute.shortname, solute.shortname.c_str(), 6);
        c_solute.shortname[6] = '\0';

        c_solute.ions = solute.ions;
        c_solute.wtmol = solute.wtmol;
        c_solute.rho = solute.rho;
      }
    }
    else
    {
      c_params->solutes = nullptr;
      c_params->solutes_size = 0;
    }

    // Handle gases array
    if (!params.gases.empty())
    {
      c_params->gases_size = static_cast<int>(params.gases.size());
      c_params->gases = new CARMAGasConfigC[c_params->gases_size];

      for (int i = 0; i < c_params->gases_size; ++i)
      {
        const auto& gas = params.gases[i];
        auto& c_gas = c_params->gases[i];

        c_gas.name_length = std::min(static_cast<int>(gas.name.length()), 255);
        std::strncpy(c_gas.name, gas.name.c_str(), 255);
        c_gas.name[255] = '\0';

        c_gas.shortname_length = std::min(static_cast<int>(gas.shortname.length()), 6);
        std::strncpy(c_gas.shortname, gas.shortname.c_str(), 6);
        c_gas.shortname[6] = '\0';

        c_gas.wtmol = gas.wtmol;
        c_gas.ivaprtn = static_cast<int>(gas.ivaprtn);
        c_gas.icomposition = static_cast<int>(gas.icomposition);
        c_gas.dgc_threshold = gas.dgc_threshold;
        c_gas.ds_threshold = gas.ds_threshold;

        // Handle refractive indices
        if (!gas.refidx.empty())
        {
          c_gas.refidx_dim_1_size = static_cast<int>(gas.refidx.size());
          c_gas.refidx_dim_2_size = static_cast<int>(gas.refidx[0].size());
          c_gas.refidx = new CARMAComplexC[c_gas.refidx_dim_1_size * c_gas.refidx_dim_2_size];
          for (int j = 0; j < c_gas.refidx_dim_1_size; ++j)
          {
            for (int k = 0; k < c_gas.refidx_dim_2_size; ++k)
            {
              c_gas.refidx[j * c_gas.refidx_dim_2_size + k].real = gas.refidx[j][k].real;
              c_gas.refidx[j * c_gas.refidx_dim_2_size + k].imaginary = gas.refidx[j][k].imaginary;
            }
          }
        }
        else
        {
          c_gas.refidx = nullptr;
          c_gas.refidx_dim_1_size = 0;
          c_gas.refidx_dim_2_size = 0;
        }
      }
    }
    else
    {
      c_params->gases = nullptr;
      c_params->gases_size = 0;
    }

    // Handle coagulations array
    if (!params.coagulations.empty())
    {
      c_params->coagulations_size = static_cast<int>(params.coagulations.size());
      c_params->coagulations = new CARMACoagulationConfigC[c_params->coagulations_size];
      for (int i = 0; i < c_params->coagulations_size; ++i)
      {
        const auto& coagulation = params.coagulations[i];
        auto& c_coagulation = c_params->coagulations[i];

        c_coagulation.igroup1 = coagulation.igroup1;
        c_coagulation.igroup2 = coagulation.igroup2;
        c_coagulation.igroup3 = coagulation.igroup3;
        c_coagulation.algorithm = static_cast<int>(coagulation.algorithm);
        c_coagulation.ck0 = coagulation.ck0;
        c_coagulation.grav_e_coll0 = coagulation.grav_e_coll0;
        c_coagulation.use_ccd = coagulation.use_ccd;
      }
    }
    else
    {
      c_params->coagulations = nullptr;
      c_params->coagulations_size = 0;
    }

    // Handle growths array
    if (!params.growths.empty())
    {
      c_params->growths_size = static_cast<int>(params.growths.size());
      c_params->growths = new CARMAGrowthConfigC[c_params->growths_size];
      for (int i = 0; i < c_params->growths_size; ++i)
      {
        const auto& growth = params.growths[i];
        auto& c_growth = c_params->growths[i];

        c_growth.ielem = growth.ielem;
        c_growth.igas = growth.igas;
      }
    }
    else
    {
      c_params->growths = nullptr;
      c_params->growths_size = 0;
    }

    // Handle nucleations array
    if (!params.nucleations.empty())
    {
      c_params->nucleations_size = static_cast<int>(params.nucleations.size());
      c_params->nucleations = new CARMANucleationConfigC[c_params->nucleations_size];
      for (int i = 0; i < c_params->nucleations_size; ++i)
      {
        const auto& nucleation = params.nucleations[i];
        auto& c_nucleation = c_params->nucleations[i];

        c_nucleation.ielemfrom = nucleation.ielemfrom;
        c_nucleation.ielemto = nucleation.ielemto;
        c_nucleation.algorithm = static_cast<int>(nucleation.algorithm);
        c_nucleation.rlh_nuc = nucleation.rlh_nuc;
        c_nucleation.igas = nucleation.igas;
        c_nucleation.ievp2elem = nucleation.ievp2elem;
      }
    }
    else
    {
      c_params->nucleations = nullptr;
      c_params->nucleations_size = 0;
    }

    return c_params;
  }

  void CARMA::FreeCCompatible(CCARMAParameters* c_params)
  {
    // Free wavelength bin centers array
    delete[] c_params->wavelength_bins;
    c_params->wavelength_bins = nullptr;

    // Free groups array and its nested arrays
    if (c_params->groups != nullptr)
    {
      for (int i = 0; i < c_params->groups_size; ++i)
      {
        delete[] c_params->groups[i].df;
      }
      delete[] c_params->groups;
      c_params->groups = nullptr;
    }

    // Free elements array and its nested arrays
    if (c_params->elements != nullptr)
    {
      for (int i = 0; i < c_params->elements_size; ++i)
      {
        delete[] c_params->elements[i].rhobin;
        delete[] c_params->elements[i].arat;
      }
      delete[] c_params->elements;
      c_params->elements = nullptr;
    }

    // Free solutes array
    if (c_params->solutes != nullptr)
    {
      delete[] c_params->solutes;
      c_params->solutes = nullptr;
    }

    // Free gases array and its nested arrays
    if (c_params->gases != nullptr)
    {
      for (int i = 0; i < c_params->gases_size; ++i)
      {
        delete[] c_params->gases[i].refidx;
      }
      delete[] c_params->gases;
      c_params->gases = nullptr;
    }

    // Free coagulations array
    if (c_params->coagulations != nullptr)
    {
      delete[] c_params->coagulations;
      c_params->coagulations = nullptr;
    }

    // Free growths array
    if (c_params->growths != nullptr)
    {
      delete[] c_params->growths;
      c_params->growths = nullptr;
    }

    // Free nucleations array
    if (c_params->nucleations != nullptr)
    {
      delete[] c_params->nucleations;
      c_params->nucleations = nullptr;
    }

    // Reset sizes
    c_params->wavelength_bin_size = 0;
    c_params->groups_size = 0;
    c_params->elements_size = 0;
    c_params->solutes_size = 0;
    c_params->gases_size = 0;
    c_params->coagulations_size = 0;
    c_params->growths_size = 0;
    c_params->nucleations_size = 0;

    // Delete the C-compatible parameters structure itself
    delete c_params;
  }

  CARMAParameters CARMA::CreateAluminumTestParams()
  {
    CARMAParameters params;

    // Set default values for the aluminum test case
    params.nz = 1;
    params.ny = 1;
    params.nx = 1;
    params.nbin = 5;
    params.dtime = 1800.0;   // 30 minutes
    params.deltaz = 1000.0;  // 1 km
    params.zmin = 16500.0;   // 16.5 km

    // Wavelength grid
    params.wavelength_bins = {
      { 0.55e-6, 0.01e-6, true },  // Example wavelength bin at 550 nm with 10 nm width
      { 0.65e-6, 0.01e-6, true },  // Example wavelength bin at 650 nm with 10 nm width
      { 0.75e-6, 0.01e-6, true },  // Example wavelength bin at 750 nm with 10 nm width
      { 0.85e-6, 0.01e-6, true },  // Example wavelength bin at 850 nm with 10 nm width
      { 0.95e-6, 0.01e-6, true }   // Example wavelength bin at 950 nm with 10 nm width
    };
    params.number_of_refractive_indices = 1;  // Assume one refractive index per wavelength

    // Create a default group
    CARMAGroupConfig group;
    group.name = "aluminum";
    group.shortname = "PRALUM";
    group.rmin = 21.5e-8;  // minimum radius [m]
    group.rmrat = 2.0;     // volume ratio between bins
    group.ishape = ParticleShape::SPHERE;
    group.eshape = 1.0;  // aspect ratio
    group.is_ice = false;
    group.is_fractal = true;
    group.mie_calculation_algorithm = MieCalculationAlgorithm::TOON_1981;  // Toon & Ackerman 1981
    group.do_wetdep = false;
    group.do_drydep = true;
    group.do_vtran = true;
    group.solfac = 0.0;                      // no solvation
    group.scavcoef = 0.0;                    // no scavenging
    group.rmon = 21.5e-8;                    // monomer radius [m]
    group.df = { 1.6, 1.6, 1.6, 1.6, 1.6 };  // fractal dimension per bin
    group.falpha = 1.0;                      // fractal packing coefficient

    // Add the group to the parameters
    params.groups.push_back(group);

    // Create a default element
    CARMAElementConfig element;
    element.igroup = 1;  // belongs to the first group
    element.name = "Aluminum";
    element.shortname = "ALUM";
    element.rho = 2700.0;  // bulk density [kg/m3]
    element.itype = ParticleType::INVOLATILE;
    element.icomposition = ParticleComposition::ALUMINUM;
    element.isolute = 0;                         // no solute
    element.arat = { 1.0, 1.0, 1.0, 1.0, 1.0 };  // no area ratio per bin
    element.kappa = 0.0;                         // no hygroscopicity
    element.isShell = true;                      // part of the shell

    // Add the element to the parameters
    params.elements.push_back(element);

    // Add coagulation
    CARMACoagulationConfig coagulation;
    coagulation.igroup1 = 1;  // coagulation within the first group
    coagulation.igroup2 = 1;  // coagulation within the first group
    coagulation.igroup3 = 1;  // add coagulated particles back to the first group
    coagulation.algorithm = ParticleCollectionAlgorithm::FUCHS;

    // Add the coagulation to the parameters
    params.coagulations.push_back(coagulation);


    return params;
  }

}  // namespace musica