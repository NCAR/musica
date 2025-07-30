#include <musica/carma/carma.hpp>
#include <musica/carma/carma_c_interface.hpp>

#include <gtest/gtest.h>

using namespace musica;

class CarmaCApiTest : public ::testing::Test
{
 protected:
  void SetUp() override
  {
    // Code to set up the test environment
  }

  void TearDown() override
  {
    // Code to clean up the test environment
  }
};

TEST_F(CarmaCApiTest, GetCarmaVersion)
{
  std::string version = CARMA::GetVersion();
  ASSERT_FALSE(version.empty());

  char *version_ptr = GetCarmaVersion();
  ASSERT_NE(version_ptr, nullptr);

  ASSERT_STREQ(version_ptr, version.c_str());
  delete[] version_ptr;  // Free the memory allocated by GetCarmaVersion
}

TEST_F(CarmaCApiTest, RunCarmaWithDefaultParameters)
{
  CARMAParameters default_params;
  CARMA carma{ default_params };

  // Test that we can run CARMA with default parameters without throwing
  ASSERT_NO_THROW(carma.Run());
}

TEST_F(CarmaCApiTest, RunCarmaWithAllComponents)
{
  CARMAParameters params;
  params.nz = 2;
  params.nbin = 3;
  params.dtime = 900.0;
  params.deltaz = 500.0;
  params.zmin = 1000.0;

  // Set up wavelength bins
  params.wavelength_bins = {
    { 550e-9, 50e-9, true },  // 550 nm ± 25 nm
    { 850e-9, 100e-9, true }  // 850 nm ± 50 nm
  };
  params.number_of_refractive_indices = 2;

  // Group 1: Aluminum particles (sphere)
  CARMAGroupConfig aluminum_group;
  aluminum_group.name = "aluminum";
  aluminum_group.shortname = "ALUM";
  aluminum_group.rmin = 1e-8;
  aluminum_group.rmrat = 2.0;
  aluminum_group.ishape = ParticleShape::SPHERE;
  aluminum_group.eshape = 1.0;
  aluminum_group.is_fractal = false;
  aluminum_group.do_vtran = true;
  aluminum_group.do_drydep = true;
  aluminum_group.df = { 1.8, 1.8, 1.8 };  // fractal dimension per bin
  params.groups.push_back(aluminum_group);

  // Group 2: Sulfate particles (sphere, with swelling)
  CARMAGroupConfig sulfate_group;
  sulfate_group.name = "sulfate";
  sulfate_group.shortname = "SULF";
  sulfate_group.rmin = 5e-9;
  sulfate_group.rmrat = 2.5;
  sulfate_group.ishape = ParticleShape::SPHERE;
  sulfate_group.eshape = 1.0;
  sulfate_group.swelling_approach.algorithm = ParticleSwellingAlgorithm::FITZGERALD;
  sulfate_group.swelling_approach.composition = ParticleSwellingComposition::AMMONIUM_SULFATE;
  sulfate_group.is_sulfate = true;
  sulfate_group.do_wetdep = true;
  sulfate_group.do_vtran = true;
  sulfate_group.solfac = 0.8;
  sulfate_group.df = { 2.0, 2.0, 2.0 };
  params.groups.push_back(sulfate_group);

  // Group 3: Ice particles (hexagon)
  CARMAGroupConfig ice_group;
  ice_group.name = "ice";
  ice_group.shortname = "ICE";
  ice_group.rmin = 2e-8;
  ice_group.rmrat = 3.0;
  ice_group.ishape = ParticleShape::HEXAGON;
  ice_group.eshape = 2.0;  // aspect ratio
  ice_group.is_ice = true;
  ice_group.is_cloud = true;
  ice_group.do_vtran = true;
  ice_group.df = { 1.5, 1.5, 1.5 };
  params.groups.push_back(ice_group);

  // Element 1: Aluminum core (Group 1)
  CARMAElementConfig aluminum_element;
  aluminum_element.igroup = 1;
  aluminum_element.name = "Aluminum";
  aluminum_element.shortname = "AL";
  aluminum_element.rho = 2.70;  // g/cm³
  aluminum_element.itype = ParticleType::INVOLATILE;
  aluminum_element.icomposition = ParticleComposition::ALUMINUM;
  aluminum_element.kappa = 0.0;
  aluminum_element.isShell = false;  // core
  params.elements.push_back(aluminum_element);

  // Element 2: Sulfate (Group 2)
  CARMAElementConfig sulfate_element;
  sulfate_element.igroup = 2;
  sulfate_element.isolute = 1;  // linked to first solute
  sulfate_element.name = "Sulfate";
  sulfate_element.shortname = "SO4";
  sulfate_element.rho = 1.84;  // g/cm³
  sulfate_element.itype = ParticleType::VOLATILE;
  sulfate_element.icomposition = ParticleComposition::H2SO4;
  sulfate_element.isolute = 1;   // linked to first solute
  sulfate_element.kappa = 0.61;  // hygroscopicity
  sulfate_element.isShell = true;
  params.elements.push_back(sulfate_element);

  // Element 3: Water on sulfate (Group 2)
  CARMAElementConfig water_element;
  water_element.igroup = 2;
  water_element.name = "Water";
  water_element.shortname = "H2O";
  water_element.rho = 1.0;  // g/cm³
  water_element.itype = ParticleType::COREMASS;
  water_element.icomposition = ParticleComposition::H2O;
  water_element.kappa = 0.0;
  water_element.isShell = true;
  params.elements.push_back(water_element);

  // Element 4: Ice (Group 3)
  CARMAElementConfig ice_element;
  ice_element.igroup = 3;
  ice_element.name = "Ice";
  ice_element.shortname = "ICE";
  ice_element.rho = 0.92;  // g/cm³
  ice_element.itype = ParticleType::INVOLATILE;
  ice_element.icomposition = ParticleComposition::ICE;
  ice_element.kappa = 0.0;
  ice_element.isShell = false;
  params.elements.push_back(ice_element);

  // Solute 1: Sulfate
  CARMASoluteConfig sulfate_solute;
  sulfate_solute.name = "Sulfate";
  sulfate_solute.shortname = "NH4SO4";
  sulfate_solute.ions = 3;           // (NH4)2SO4 dissociates into 3 ions
  sulfate_solute.wtmol = 132.14e-3;  // kg/mol
  sulfate_solute.rho = 1769.0;       // kg/m³
  params.solutes.push_back(sulfate_solute);

  // Gas 1: Sulfuric acid vapor
  CARMAGasConfig h2so4_gas;
  h2so4_gas.name = "Sulfuric Acid";
  h2so4_gas.shortname = "H2SO4V";
  h2so4_gas.wtmol = 98.08e-3;  // kg/mol
  h2so4_gas.ivaprtn = VaporizationAlgorithm::H2O_BUCK_1981;
  h2so4_gas.icomposition = GasComposition::H2SO4;
  h2so4_gas.dgc_threshold = 1e-8;
  h2so4_gas.ds_threshold = 1e-6;
  params.gases.push_back(h2so4_gas);

  // Coagulation configuration
  CARMACoagulationConfig coagulation_config;
  coagulation_config.igroup1 = 2;  // Sulfate group
  coagulation_config.igroup2 = 2;  // Sulfate group
  coagulation_config.igroup3 = 2;  // Resulting particles in sulfate group
  coagulation_config.algorithm = ParticleCollectionAlgorithm::CONSTANT;
  coagulation_config.ck0 = 0.5;           // Collection efficiency coefficient
  coagulation_config.grav_e_coll0 = 0.1;  // Gravitational collection efficiency coefficient
  coagulation_config.use_ccd = true;      // Use constant collection efficiency data
  params.coagulations.push_back(coagulation_config);

  // Growth configuration
  CARMAGrowthConfig growth_config;
  growth_config.ielem = 2;  // Sulfate element
  growth_config.igas = 1;   // Sulfuric acid gas
  params.growths.push_back(growth_config);

  // Nucleation configuration
  CARMANucleationConfig nucleation_config;
  nucleation_config.ielemfrom = 2;  // From sulfate element
  nucleation_config.ielemto = 2;    // To sulfate element
  nucleation_config.algorithm = ParticleNucleationAlgorithm::HOMOGENEOUS_NUCLEATION;
  nucleation_config.rlh_nuc = 1.0e6;  // Latent heat of nucleation [m² s⁻²]
  nucleation_config.igas = 1;         // Sulfuric acid gas
  params.nucleations.push_back(nucleation_config);

  // Initialization configuration
  params.initialization.do_thermo = true;  // Enable thermodynamic processes
  params.initialization.do_vdiff = true;   // Enable Brownian diffusion

  // Create CARMA instance and run
  CARMA carma{ params };
  CARMAOutput output;
  ASSERT_NO_THROW(output = carma.Run());

  // Verify dimensions match parameters
  EXPECT_EQ(output.lat.size(), 1);
  EXPECT_EQ(output.lon.size(), 1);
  EXPECT_EQ(output.vertical_center.size(), params.nz);
  EXPECT_EQ(output.pressure.size(), params.nz);
  EXPECT_EQ(output.temperature.size(), params.nz);
  EXPECT_EQ(output.air_density.size(), params.nz);

  // Verify 3D particle arrays (nz x nbin x nelem)
  EXPECT_EQ(output.particle_concentration.size(), params.nz);
  EXPECT_EQ(output.mass_mixing_ratio.size(), params.nz);
  if (!output.particle_concentration.empty())
  {
    EXPECT_EQ(output.particle_concentration[0].size(), params.nbin);
    if (!output.particle_concentration[0].empty())
    {
      EXPECT_EQ(output.particle_concentration[0][0].size(), params.elements.size());
    }
  }

  // Verify 3D group arrays (nz x nbin x ngroup)
  EXPECT_EQ(output.wet_radius.size(), params.nz);
  EXPECT_EQ(output.wet_density.size(), params.nz);
  EXPECT_EQ(output.fall_velocity.size(), params.nz + 1);
  EXPECT_EQ(output.nucleation_rate.size(), params.nz);
  EXPECT_EQ(output.deposition_velocity.size(), params.nz);

  // Verify 2D arrays (nbin x ngroup)
  EXPECT_EQ(output.dry_radius.size(), params.nbin);
  EXPECT_EQ(output.mass_per_bin.size(), params.nbin);
  if (!output.dry_radius.empty())
  {
    EXPECT_EQ(output.dry_radius[0].size(), params.groups.size());
  }

  // Verify 1D group arrays
  EXPECT_EQ(output.group_particle_number_concentration.size(), params.groups.size());
  EXPECT_EQ(output.constituent_type.size(), params.groups.size());
  EXPECT_EQ(output.max_prognostic_bin.size(), params.groups.size());
}

TEST_F(CarmaCApiTest, RunCarmaWithAluminumTestParams)
{
  CARMAParameters params = CARMA::CreateAluminumTestParams();
  CARMA carma{ params };

  // Verify the aluminum test parameters are set correctly
  EXPECT_EQ(params.nz, 1);
  EXPECT_EQ(params.nbin, 5);
  EXPECT_EQ(params.dtime, 1800.0);
  EXPECT_EQ(params.deltaz, 1000.0);
  EXPECT_EQ(params.zmin, 16500.0);
  EXPECT_EQ(params.wavelength_bins.size(), 5);
  EXPECT_EQ(params.number_of_refractive_indices, 1);

  // Run CARMA and get the output
  CARMAOutput output;
  ASSERT_NO_THROW(output = carma.Run());

  // Verify that the basic dimensions are correct
  EXPECT_EQ(output.lat.size(), 1);
  EXPECT_EQ(output.lon.size(), 1);
  EXPECT_EQ(output.vertical_center.size(), params.nz);
  EXPECT_EQ(output.pressure.size(), params.nz);
  EXPECT_EQ(output.temperature.size(), params.nz);
  EXPECT_EQ(output.air_density.size(), params.nz);

  // Verify that the new nucleation_rate field is properly sized (3D: nz x nbin x ngroup)
  EXPECT_EQ(output.nucleation_rate.size(), params.nz);
  if (!output.nucleation_rate.empty())
  {
    EXPECT_EQ(output.nucleation_rate[0].size(), params.nbin);
    if (!output.nucleation_rate[0].empty())
    {
      EXPECT_EQ(output.nucleation_rate[0][0].size(), params.groups.size());
    }
  }

  // Verify that the new deposition_velocity field is properly sized (3D: nz x nbin x ngroup)
  EXPECT_EQ(output.deposition_velocity.size(), params.nz);
  if (!output.deposition_velocity.empty())
  {
    EXPECT_EQ(output.deposition_velocity[0].size(), params.nbin);
    if (!output.deposition_velocity[0].empty())
    {
      EXPECT_EQ(output.deposition_velocity[0][0].size(), params.groups.size());
    }
  }

  // Verify that the new 1D group mapping arrays are properly sized
  EXPECT_EQ(output.group_particle_number_concentration.size(), params.groups.size());
  EXPECT_EQ(output.constituent_type.size(), params.groups.size());
  EXPECT_EQ(output.max_prognostic_bin.size(), params.groups.size());
}