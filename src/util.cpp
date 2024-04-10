/// Copyright (C) 2023-2024 National Center for Atmospheric Research
/// SPDX-License-Identifier: Apache-2.0

#include <cstring>
#include <musica/util.hpp>

String ToString(char* value)
{
    String str;
    str.value_ = value;
    str.size_ = std::strlen(value);
    return str;
}

void DeleteString(String str)
{
    delete[] str.value_;
}