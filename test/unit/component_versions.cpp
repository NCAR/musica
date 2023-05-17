#include <gtest/gtest.h>

#include <iostream>

#include <musica/component_versions.h>

TEST(Musica, Version) {
  char* versions = getAllComponentVersions();

  ASSERT_NE(versions, nullptr);

  std::cout << versions << std::endl;
  free(versions);
}