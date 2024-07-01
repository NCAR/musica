#include <gtest/gtest.h>

#include <micm/version.hpp>

TEST(ConnectToMICM, Version) {
  std::string version = micm::getMicmVersion();
}