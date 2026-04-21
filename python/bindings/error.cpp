// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0

#include "error.hpp"

#include <musica/error.hpp>
#include <stdexcept>

void handle_error(musica::Error& error, const std::string& context_message)
{
  if (musica::IsSuccess(error))
  {
    musica::DeleteError(&error);
    return;
  }

  std::string full_message = context_message;
  if (!context_message.empty())
  {
    full_message += ": ";
  }
  full_message += std::string(error.message_.value_);

  int severity = error.severity_;

  // Clean up error memory (DeleteError now also resets code_ and severity_)
  musica::DeleteError(&error);

  switch (severity)
  {
    case MUSICA_SEVERITY_INFO:
      return;
    case MUSICA_SEVERITY_WARNING:
      // Issue warnings.warn() but don't throw
      // If PyErr_WarnEx returns -1 and sets an exception
      if (PyErr_WarnEx(PyExc_UserWarning, full_message.c_str(), 1) == -1) 
      { 
        throw py::error_already_set();
      }
      return;
    case MUSICA_SEVERITY_ERROR:
      throw py::value_error(full_message);
    case MUSICA_SEVERITY_CRITICAL:
      // Using the pybind11 wrapper for RuntimeError
      // py::runtime_error() is not supported in 
      throw py::runtime_error(full_message);
    default:
      // Unknown severity, treat as error
      throw py::value_error("Unknown severity: " + full_message);
  }
}
