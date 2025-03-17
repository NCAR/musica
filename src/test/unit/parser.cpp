#include <gtest/gtest.h>
#include <mechanism_configuration/parser.hpp>

TEST(Parser, BadConfigurationFilePath)
{
  mechanism_configuration::UniversalParser parser;
  auto parsed = parser.Parse("bad config path");
  EXPECT_FALSE(parsed);
}

TEST(Parser, Version0Configuration)
{
  mechanism_configuration::UniversalParser parser;
  auto parsed = parser.Parse("configs/chapman");
  EXPECT_TRUE(parsed);

  using V0 = mechanism_configuration::v0::types::Mechanism;
  V0* v0_mechanism = dynamic_cast<V0*>(parsed.mechanism.get());

  EXPECT_EQ(v0_mechanism->name, "Chapman");
  EXPECT_EQ(v0_mechanism->version.major, 0);
  EXPECT_EQ(v0_mechanism->version.minor, 0);
  EXPECT_EQ(v0_mechanism->version.patch, 0);
  EXPECT_EQ(v0_mechanism->reactions.arrhenius.size(), 4);
  EXPECT_EQ(v0_mechanism->reactions.user_defined.size(), 3);
  EXPECT_EQ(v0_mechanism->species.size(), 5);
}

TEST(Parser, Version1Configuration)
{
}