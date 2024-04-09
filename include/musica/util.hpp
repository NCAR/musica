/// Copyright (C) 2023-2024 National Center for Atmospheric Research
/// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <cstddef>

#ifdef __cplusplus
extern "C"
{
#endif

struct ConstString
{
  const char* value_;
  const size_t size_;
};

struct String
{
  char* value_;
  size_t size_;
};

struct Mapping
{
    char name[256];
    size_t index;
    size_t string_length;
};

String ToString(char* value);
ConstString ToConstString(const char* value);

#ifdef __cplusplus
}
#endif