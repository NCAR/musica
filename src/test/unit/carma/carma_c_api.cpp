#include <musica/carma/carma.hpp>
#include <musica/carma/carma_c_interface.hpp>
#include <musica/carma/carma_state.hpp>

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

TEST_F(CarmaCApiTest, CreateWithParams)
{
  CARMAParameters params;
  params.nz = 2;
  params.nbin = 3;
  params.dtime = 900.0;

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
}

TEST_F(CarmaCApiTest, CreateWithAluminumTestParams)
{
  CARMAParameters params = CARMA::CreateAluminumTestParams();
  CARMA carma{ params };

  // Verify the aluminum test parameters are set correctly
  EXPECT_EQ(params.nbin, 5);
  EXPECT_EQ(params.dtime, 1800.0);
  EXPECT_EQ(params.wavelength_bins.size(), 5);
  EXPECT_EQ(params.number_of_refractive_indices, 1);
}

TEST_F(CarmaCApiTest, CanSetBinValues)
{
  CARMAParameters params = CARMA::CreateAluminumTestParams();

  // Add a gas to the parameters
  CARMAGasConfig gas_config;
  gas_config.name = "TestGas";
  gas_config.shortname = "TG";
  gas_config.wtmol = 0.018;  // Water vapor
  gas_config.ivaprtn = VaporizationAlgorithm::H2O_BUCK_1981;
  gas_config.icomposition = GasComposition::H2O;
  gas_config.dgc_threshold = 1e-8;
  gas_config.ds_threshold = 1e-6;
  params.gases.push_back(gas_config);

  CARMA carma{ params };
  CARMAStateParameters state_params;
  state_params.longitude = 0.0;
  state_params.latitude = 0.0;
  state_params.temperature = std::vector<double>(params.nz, 273.15);
  state_params.pressure = std::vector<double>(params.nz, 101325.0);
  state_params.pressure_levels = std::vector<double>(params.nz + 1, 101325.0);
  state_params.vertical_levels = std::vector<double>(params.nz + 1, 1.0);
  state_params.vertical_center = std::vector<double>(params.nz, 16500.0);
  state_params.coordinates = CarmaCoordinates::CARTESIAN;

  CARMAState state = CARMAState(carma, state_params);
  ASSERT_NO_THROW(state.SetBin(1, 1, std::vector<double>{ 1.0 }, 0.0001));
  ASSERT_NO_THROW(state.SetDetrain(1, 1, std::vector<double>{ 1.0 }));
  ASSERT_NO_THROW(state.SetGas(
      1,
      std::vector<double>(params.nz, 1.4e-3),
      std::vector<double>(params.nz, 2.3e-4),
      std::vector<double>(params.nz, 0.3),
      std::vector<double>(params.nz, 0.5)));
  ASSERT_NO_THROW(state.SetTemperature(std::vector<double>(params.nz, 273.15)));
  ASSERT_NO_THROW(state.SetAirDensity(std::vector<double>(params.nz, 1.225)));

  CARMAStateStepConfig step_config;
  step_config.cloud_fraction = std::vector<double>(params.nz, 0.5);
  step_config.critical_relative_humidity = std::vector<double>(params.nz, 0.8);
  step_config.land.surface_friction_velocity = 0.1;
  step_config.land.aerodynamic_resistance = 100.0;
  step_config.land.area_fraction = 0.5;
  ASSERT_NO_THROW(state.Step(step_config));

  CARMAGroupProperties group_props = carma.GetGroupProperties(1);
  CARMAElementProperties element_props = carma.GetElementProperties(1);
}