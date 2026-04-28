// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#include <musica/utils/error_handler.hpp>

#include <cstring>

namespace musica
{
  void DeleteError(Error* error)
  {
    if (error == nullptr)
      return;
    error->code_ = MUSICA_STATUS_SUCCESS;
    error->severity_ = MUSICA_SEVERITY_INFO;
    DeleteString(&(error->category_));
    DeleteString(&(error->message_));
  }

  void NoError(Error* error)
  {
    DeleteError(error);
    CreateString("", &error->category_);
    CreateString("Success", &error->message_);
  }

  void ToError(const char* category, int code, const char* message, int severity, Error* error)
  {
    DeleteError(error);
    error->code_ = code;
    error->severity_ = severity;
    CreateString(category, &error->category_);
    CreateString(message, &error->message_);
  }

  void ToError(const std::system_error& e, int severity, Error* error)
  {
    ToError(e.code().category().name(), e.code().value(), e.what(), severity, error);
  }

#ifdef MUSICA_USE_MICM
  void ToError(const micm::MicmException& e, Error* error)
  {
    ToError(e.category_, e.code_, e.what(), MUSICA_SEVERITY_ERROR, error);
  }
#endif

  bool IsSuccess(const Error& error)
  {
    return error.code_ == MUSICA_STATUS_SUCCESS;
  }

  bool operator==(const Error& lhs, const Error& rhs)
  {
    if (lhs.code_ != rhs.code_)
      return false;
    if (lhs.code_ == MUSICA_STATUS_SUCCESS)
      return true;  // both success, category irrelevant
    if (lhs.category_.value_ == nullptr || rhs.category_.value_ == nullptr)
      return lhs.category_.value_ == rhs.category_.value_;
    return std::strcmp(lhs.category_.value_, rhs.category_.value_) == 0;
  }

  bool operator!=(const Error& lhs, const Error& rhs)
  {
    return !(lhs == rhs);
  }

}  // namespace musica
