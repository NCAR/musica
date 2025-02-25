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
}

TEST(Parser, Version1Configuration)
{
}