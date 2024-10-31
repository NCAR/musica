// Copyright (C) 2023-2024 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file contains hepler functions for the internal TUV-x interface.
#include "interface_util.hpp"

namespace musica
{
  String InternalCreateString(const char *str)
  {
    String string;
    string.size_ = strlen(str);
    string.value_ = new char[string.size_ + 1];
    strcpy(string.value_, str);
    return string;
  }

  Mapping *InternalAllocateMappings(const std::size_t size)
  {
    return new Mapping[size];
  }
}