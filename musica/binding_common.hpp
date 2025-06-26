#pragma once

#include <musica/util.hpp>

#include <pybind11/pybind11.h>

#include <type_traits>

namespace py = pybind11;

// Helper template to handle C-style errors and convert them to Python exceptions
// This centralizes error handling for all Python bindings in MUSICA
template<typename Func>
auto HandleMusicaErrors(Func func) -> decltype(func(std::declval<musica::Error *>()))
{
  musica::Error error;
  auto result = func(&error);
  if (!musica::IsSuccess(error))
  {
    std::string error_msg = "MUSICA error";
    if (error.message_.value_ != nullptr)
    {
      error_msg = std::string(error.message_.value_);
    }
    musica::DeleteError(&error);
    throw py::value_error(error_msg);
  }
  return result;
}

// Overload for void functions (that don't return anything)
template<typename Func>
void HandleMusicaErrorsVoid(Func func)
{
  musica::Error error;
  func(&error);
  if (!musica::IsSuccess(error))
  {
    std::string error_msg = "MUSICA error";
    if (error.message_.value_ != nullptr)
    {
      error_msg = std::string(error.message_.value_);
    }
    musica::DeleteError(&error);
    throw py::value_error(error_msg);
  }
}

void bind_all(py::module_ &m);