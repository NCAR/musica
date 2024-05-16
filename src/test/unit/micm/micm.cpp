#include <gtest/gtest.h>
#include <iostream>

#include <micm/version.hpp>
#include <musica/component_versions.hpp>

TEST(ConnectToMICM, Version) {
  std::string version = micm::getMicmVersion();
  std::cout << version << std::endl;
  char* versions = musica::getAllComponentVersions();
}