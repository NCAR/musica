// Copyright (C) 2023-2024 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#include <musica/util.hpp>

#include <cstddef>
#include <cstring>

namespace musica
{

  String CreateString(const char* value)
  {
    String str;
    str.size_ = std::strlen(value);
    str.value_ = new char[str.size_ + 1];
    std::strcpy(str.value_, value);
    return str;
  }

  void DeleteString(String* str)
  {
    if (str->value_ != nullptr)
      delete[] str->value_;
    str->value_ = nullptr;
    str->size_ = 0;
  }

  Error NoError()
  {
    return ToError("", 0, "Success");
  }

  Error ToError(const char* category, int code)
  {
    return ToError(category, code, "");
  }

  Error ToError(const char* category, int code, const char* message)
  {
    Error error;
    error.code_ = code;
    error.category_ = CreateString(category);
    error.message_ = CreateString(message);
    return error;
  }

  Error ToError(const std::system_error& e)
  {
    return ToError(e.code().category().name(), e.code().value(), e.what());
  }

  bool IsSuccess(const Error& error)
  {
    return error.code_ == 0;
  }

  bool IsError(const Error& error, const char* category, int code)
  {
    return error.code_ == code && (error.category_.value_ == nullptr && category == nullptr) ||
           std::strcmp(error.category_.value_, category) == 0;
  }

  void DeleteError(Error* error)
  {
    DeleteString(&(error->category_));
    DeleteString(&(error->message_));
  }

  bool operator==(const Error& lhs, const Error& rhs)
  {
    if (lhs.code_ == 0 && rhs.code_ == 0)
    {
      return true;
    }
    return lhs.code_ == rhs.code_ && (lhs.category_.value_ == nullptr && rhs.category_.value_ == nullptr) ||
           std::strcmp(lhs.category_.value_, rhs.category_.value_) == 0;
  }

  bool operator!=(const Error& lhs, const Error& rhs)
  {
    return !(lhs == rhs);
  }

  Mapping ToMapping(const char* name, std::size_t index)
  {
    Mapping mapping;
    mapping.name_ = CreateString(name);
    mapping.index_ = index;
    return mapping;
  }

  void DeleteMapping(Mapping* mapping)
  {
    DeleteString(&(mapping->name_));
  }

  void DeleteMappings(Mapping* mappings, std::size_t size)
  {
    for (std::size_t i = 0; i < size; i++)
    {
      DeleteMapping(&(mappings[i]));
    }
    delete[] mappings;
  }

}  // namespace musica