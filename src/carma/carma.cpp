// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file contains the implementation of the CARMA class, which creates C connections
// to the CARMA aerosol model, allowing it to be used in a C++ context.
#include <musica/carma/carma.hpp>
#include <musica/carma/carma_c_interface.hpp>

#include <cstring>
#include <stdexcept>

namespace musica
{
  CARMA::CARMA()
  {
  }

  CARMA::~CARMA()
  {
  }

  std::string CARMA::GetVersion()
  {
    char* version_ptr = nullptr;
    int version_length = 0;

    InternalGetCarmaVersion(&version_ptr, &version_length);

    std::string version(version_ptr, version_length);

    // Free the memory allocated by Fortran
    InternalFreeCarmaVersion(version_ptr, version_length);

    return version;
  }

  CARMAOutput CARMA::Run(const CARMAParameters& params)
  {
    int return_code = 0;
    CARMAOutput output;
    InternalRunCarma(params, static_cast<void*>(&output), &return_code);

    if (return_code != 0)
    {
      throw std::runtime_error("CARMA simulation failed with return code: " + std::to_string(return_code));
    }

    return output;
  }

}  // namespace musica