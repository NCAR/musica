#include <gtest/gtest.h>
#include <iostream>

#include <musica/component_versions.hpp>

TEST(ConnectToMICM, Version) {
  char* versions = musica::getAllComponentVersions();
  std::cout << versions << std::endl;
  free(versions);
}