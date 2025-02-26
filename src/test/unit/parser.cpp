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
  EXPECT_EQ(parsed.mechanism->version.major, 0);
  EXPECT_EQ(parsed.mechanism->version.minor, 0);
  EXPECT_EQ(parsed.mechanism->version.patch, 0);
}

TEST(Parser, Version1Configuration)
{
}