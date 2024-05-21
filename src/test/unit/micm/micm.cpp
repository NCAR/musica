#include <micm/version.hpp>

#include <gtest/gtest.h>

#include <iostream>

TEST(ConnectToMICM, Version)
{
  std::string version = micm::getMicmVersion();
  std::cout << version << std::endl;
}