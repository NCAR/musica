// Copyright (C) 2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0

#include "synthetic_nc.hpp"

#include <exception>
#include <iostream>
#include <string>
#include <vector>

int main(int argc, char** argv)
{
  if (argc != 2)
  {
    std::cerr << "usage: generate_miem_fortran_fixture OUTPUT.nc\n";
    return 2;
  }

  try
  {
    constexpr int n_cells = 6;
    miem_test::UptempoNcOptions options;
    options.write_grid_metadata = true;
    miem_test::CreateUptempoTestNetCDF(
        argv[1],
        2,
        n_cells,
        { "1970-01-01_00:00:00", "1970-01-01_01:00:00" },
        { "ground_nox", "stack_nox" },
        {
          std::vector<double>(2 * n_cells, 2.0e-9),
          std::vector<double>(2 * n_cells, 6.0e-9),
        },
        options);
  }
  catch (const std::exception& error)
  {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
