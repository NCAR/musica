// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <musica/util.hpp>

#include <pybind11/pybind11.h>

#include <string>

namespace py = pybind11;

/// @brief Check MUSICA error and handle appropriately based on severity
/// @param error The MUSICA error object to check (will be cleaned up and reset)
/// @param context_message Additional context to prepend to error message
/// @throws py::value_error for MUSICA_SEVERITY_ERROR
/// @throws std::runtime_error for MUSICA_SEVERITY_CRITICAL
/// @warning Issues Python warning for MUSICA_SEVERITY_WARNING
/// @note This function calls DeleteError to reset error to clean state
void handle_error(musica::Error& error, const std::string& context_message = "");
