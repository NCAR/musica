#if 0
// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#endif
#pragma once

#define MUSICA_CARMA_ERROR_CODE_MEMORY_ALLOCATION       97
#define MUSICA_CARMA_ERROR_CODE_DIMENSION_MISMATCH      98
#define MUSICA_CARMA_ERROR_CODE_CREATION_FAILED         99
#define MUSICA_CARMA_ERROR_CODE_UNASSOCIATED_POINTER    100
#define MUSICA_CARMA_ERROR_CODE_DESTROY_FAILED          101
#define MUSICA_CARMA_ERROR_CODE_ADD_CARMA_OBJECT_FAILED 102
#define MUSICA_CARMA_ERROR_CODE_INITIALIZATION_FAILED   103
#define MUSICA_CARMA_ERROR_CODE_SET_FAILED              104
#define MUSICA_CARMA_ERROR_CODE_STEP_FAILED             105
#define MUSICA_CARMA_ERROR_CODE_GET_FAILED              106

#ifdef __cplusplus
  #include <string>

namespace musica
{
  inline std::string CarmaErrorCodeToMessage(int error_code)
  {
    switch (error_code)
    {
      case MUSICA_CARMA_ERROR_CODE_MEMORY_ALLOCATION: return "Memory allocation failed in CARMA.";
      case MUSICA_CARMA_ERROR_CODE_DIMENSION_MISMATCH: return "Dimension mismatch in CARMA parameters.";
      case MUSICA_CARMA_ERROR_CODE_CREATION_FAILED: return "Failed to create CARMA instance.";
      case MUSICA_CARMA_ERROR_CODE_UNASSOCIATED_POINTER: return "Unassociated pointer in CARMA state.";
      case MUSICA_CARMA_ERROR_CODE_DESTROY_FAILED: return "Failed to destroy CARMA instance.";
      case MUSICA_CARMA_ERROR_CODE_ADD_CARMA_OBJECT_FAILED: return "Failed to add CARMA object.";
      case MUSICA_CARMA_ERROR_CODE_INITIALIZATION_FAILED: return "CARMA initialization failed.";
      case MUSICA_CARMA_ERROR_CODE_SET_FAILED: return "Failed to set values in CARMA state.";
      case MUSICA_CARMA_ERROR_CODE_STEP_FAILED: return "Failed to step CARMA state.";
      case MUSICA_CARMA_ERROR_CODE_GET_FAILED: return "Failed to get values from CARMA state.";
      default: return "Unknown CARMA error code: " + std::to_string(error_code);
    }
  }
}  // namespace musica
#endif