#include <gtest/gtest.h>
#include <iostream>

#include <micm/version.hpp>

TEST(ConnectToMICM, Version) {
  std::string version = micm::getMicmVersion();
  std::cout << version << std::endl;
}