#include <gtest/gtest.h>

#include <iostream>

#include <musica/component_versions.hpp>

using namespace musica;

TEST(Musica, Version) {
  char* versions = getAllComponentVersions();

  ASSERT_NE(versions, nullptr);

  std::cout << versions << std::endl;
  free(versions);
}

TEST(Musica, VersionIsNullTerminated) {
  char* versions = getAllComponentVersions();

  ASSERT_NE(versions, nullptr);

  size_t length = strlen(versions);
  ASSERT_EQ(versions[length], '\0');  // Check if the last character is the null terminator

  free(versions);
}