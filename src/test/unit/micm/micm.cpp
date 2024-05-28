#include <musica/component_versions.hpp>

#include <gtest/gtest.h>

#include <iostream>

TEST(ConnectToMICM, Version)
{
  char* versions = musica::GetAllComponentVersions();
  std::cout << versions << std::endl;
  free(versions);
}