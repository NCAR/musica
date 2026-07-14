#include <musica/configuration/emissions.hpp>
#include <musica/configuration/read_mechanism.hpp>
#include <musica/utils/error_code.hpp>

#include <mechanism_configuration/mechanism.hpp>

#include <gtest/gtest.h>

#include <fstream>
#include <string>
#include <utility>
#include <vector>

namespace mc_types = mechanism_configuration::types;

namespace
{
  template<typename Func>
  void ExpectMiemError(Func&& func, musica::MiemErrorCode expected_code)
  {
    try
    {
      func();
      FAIL() << "Expected musica::Exception to be thrown";
    }
    catch (const musica::Exception& e)
    {
      EXPECT_EQ(e.code_, static_cast<int>(expected_code));
      EXPECT_STREQ(e.category_, MUSICA_MIEM_ERROR_CATEGORY);
    }
  }

  mechanism_configuration::Mechanism MakeMinimalMechanism()
  {
    mechanism_configuration::Mechanism mechanism{};

    mc_types::Inventory inventory{};
    inventory.name = "test inventory";
    inventory.directory = "some/dir";
    inventory.file_pattern = "FILE_{YYYY}{MM}.nc";
    inventory.convention = "eccad";

    mc_types::SpeciesMapping mapping{};
    mapping.inventory_species = "inv_species";
    mapping.mechanism_species = "mech_species";
    mapping.scaling_factor = 1.0;

    mc_types::SpeciesMap species_map{};
    species_map.name = "test species map";
    species_map.mappings = { mapping };

    mc_types::SourceDescriptor source{};
    source.name = "test source";
    source.inventory = "test inventory";
    source.species_map = "test species map";

    mc_types::EmissionsConfig config{};
    config.inventories = { inventory };
    config.species_maps = { species_map };
    config.sources = { source };

    mechanism.emissions = config;
    return mechanism;
  }
}  // namespace

TEST(MiemParser, RoundTripFromStringYaml)
{
  musica::Emissions emissions = musica::ConvertEmissions(musica::ReadMechanism("configs/v1/emissions/config.yaml"));

  ASSERT_EQ(emissions.sources.size(), 2);

  const miem::Source& anthro = emissions.sources[0];
  EXPECT_EQ(anthro.name_, "cams anthro source");
  EXPECT_EQ(anthro.mode_, miem::SourceMode::Offline);
  EXPECT_EQ(anthro.type_, miem::SourceType::Anthropogenic);
  EXPECT_EQ(anthro.file_pattern_, "cams/v6.2/CAMS-GLOB-ANT_v6.2_{YYYY}-{MM}.nc");
  EXPECT_EQ(anthro.convention_, "eccad");
  EXPECT_EQ(anthro.temporal_interpolation_, miem::TemporalInterpolation::Linear);
  EXPECT_EQ(anthro.vertical_injection_, miem::VerticalInjection::Surface);
  EXPECT_EQ(anthro.category_, 0);
  EXPECT_EQ(anthro.hierarchy_, 1);
  EXPECT_EQ(anthro.scaling_factor_, 1.0);
  EXPECT_EQ(anthro.sector_, "anthropogenic");

  ASSERT_EQ(anthro.species_map_.Mappings().size(), 3);
  EXPECT_EQ(anthro.species_map_.Mappings()[0].inventory_name_, "NOx");
  EXPECT_EQ(anthro.species_map_.Mappings()[0].mechanism_name_, "NO");
  EXPECT_DOUBLE_EQ(anthro.species_map_.Mappings()[0].scaling_factor_, 0.9);

  const miem::Source& fire = emissions.sources[1];
  EXPECT_EQ(fire.name_, "gfed fire source");
  EXPECT_EQ(fire.type_, miem::SourceType::Fire);
  EXPECT_EQ(fire.file_pattern_, "gfed/v4/GFED4_{YYYY}{MM}.nc");
  EXPECT_EQ(fire.temporal_interpolation_, miem::TemporalInterpolation::Nearest);
  EXPECT_EQ(fire.category_, 1);

  EXPECT_EQ(emissions.regridding.type_, miem::RegriddingType::None);
}

TEST(MiemParser, RoundTripJsonAndYamlMatch)
{
  musica::Emissions yaml_emissions = musica::ConvertEmissions(musica::ReadMechanism("configs/v1/emissions/config.yaml"));
  musica::Emissions json_emissions = musica::ConvertEmissions(musica::ReadMechanism("configs/v1/emissions/config.json"));

  ASSERT_EQ(yaml_emissions.sources.size(), json_emissions.sources.size());
  for (size_t i = 0; i < yaml_emissions.sources.size(); ++i)
  {
    EXPECT_EQ(yaml_emissions.sources[i].name_, json_emissions.sources[i].name_);
    EXPECT_EQ(yaml_emissions.sources[i].file_pattern_, json_emissions.sources[i].file_pattern_);
  }
}

TEST(MiemParser, ReadFromStringMatchesReadFromFile)
{
  std::ifstream file("configs/v1/emissions/config.yaml");
  std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

  musica::Emissions from_string = musica::ConvertEmissions(musica::ReadMechanismFromString(contents));
  musica::Emissions from_file = musica::ConvertEmissions(musica::ReadMechanism("configs/v1/emissions/config.yaml"));

  ASSERT_EQ(from_string.sources.size(), from_file.sources.size());
  EXPECT_EQ(from_string.sources[0].name_, from_file.sources[0].name_);
}

TEST(MiemParser, BadConfigurationFilePath)
{
  EXPECT_THROW(musica::ConvertEmissions(musica::ReadMechanism("bad config path")), musica::Exception);
}

TEST(MiemParser, MissingEmissionsSectionIsNotAnError)
{
  mechanism_configuration::Mechanism mechanism{};
  ASSERT_FALSE(mechanism.emissions.has_value());

  musica::Emissions emissions = musica::ConvertEmissions(mechanism);
  EXPECT_TRUE(emissions.sources.empty());
  EXPECT_EQ(emissions.regridding.type_, miem::RegriddingType::None);
}

TEST(MiemParser, UnresolvedInventoryReferenceThrows)
{
  mechanism_configuration::Mechanism mechanism = MakeMinimalMechanism();
  mechanism.emissions->sources[0].inventory = "does not exist";

  ExpectMiemError([&]() { musica::ConvertEmissions(mechanism); }, musica::MiemErrorCode::UnresolvedReference);
}

TEST(MiemParser, UnresolvedSpeciesMapReferenceThrows)
{
  mechanism_configuration::Mechanism mechanism = MakeMinimalMechanism();
  mechanism.emissions->sources[0].species_map = "does not exist";

  ExpectMiemError([&]() { musica::ConvertEmissions(mechanism); }, musica::MiemErrorCode::UnresolvedReference);
}

TEST(MiemParser, SourceTypeEnumMapping)
{
  const std::vector<std::pair<mc_types::SourceType, miem::SourceType>> cases = {
    { mc_types::SourceType::Anthropogenic, miem::SourceType::Anthropogenic },
    { mc_types::SourceType::Fire, miem::SourceType::Fire },
    { mc_types::SourceType::Biogenic, miem::SourceType::Biogenic },
    { mc_types::SourceType::Dust, miem::SourceType::Dust },
    { mc_types::SourceType::SeaSalt, miem::SourceType::SeaSalt },
    { mc_types::SourceType::Lightning, miem::SourceType::Lightning },
  };

  for (const auto& [mc_type, expected] : cases)
  {
    mechanism_configuration::Mechanism mechanism = MakeMinimalMechanism();
    mechanism.emissions->sources[0].type = mc_type;

    musica::Emissions emissions = musica::ConvertEmissions(mechanism);
    ASSERT_EQ(emissions.sources.size(), 1);
    EXPECT_EQ(emissions.sources[0].type_, expected);
  }
}

TEST(MiemParser, TemporalInterpolationEnumMapping)
{
  const std::vector<std::pair<mc_types::TemporalInterpolation, miem::TemporalInterpolation>> cases = {
    { mc_types::TemporalInterpolation::Linear, miem::TemporalInterpolation::Linear },
    { mc_types::TemporalInterpolation::Nearest, miem::TemporalInterpolation::Nearest },
    { mc_types::TemporalInterpolation::None, miem::TemporalInterpolation::None },
  };

  for (const auto& [mc_interp, expected] : cases)
  {
    mechanism_configuration::Mechanism mechanism = MakeMinimalMechanism();
    mechanism.emissions->sources[0].temporal_interpolation = mc_interp;

    musica::Emissions emissions = musica::ConvertEmissions(mechanism);
    ASSERT_EQ(emissions.sources.size(), 1);
    EXPECT_EQ(emissions.sources[0].temporal_interpolation_, expected);
  }
}

TEST(MiemParser, FilePatternJoinsDirectoryAndPattern)
{
  mechanism_configuration::Mechanism mechanism = MakeMinimalMechanism();
  musica::Emissions emissions = musica::ConvertEmissions(mechanism);

  ASSERT_EQ(emissions.sources.size(), 1);
  EXPECT_EQ(emissions.sources[0].file_pattern_, "some/dir/FILE_{YYYY}{MM}.nc");
}
