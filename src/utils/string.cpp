// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#include <musica/utils/string.hpp>

#include <cstring>

namespace musica
{

  void CreateString(const char* value, String* str)
  {
    str->size_ = std::strlen(value);
    str->value_ = new char[str->size_ + 1];
    std::memcpy(str->value_, value, str->size_ + 1);
  }

  void DeleteString(String* str)
  {
    if (str->value_ != nullptr)
      delete[] str->value_;
    str->value_ = nullptr;
    str->size_ = 0;
  }

}  // namespace musica
