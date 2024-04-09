/// Copyright (C) 2023-2024 National Center for Atmospheric Research
/// SPDX-License-Identifier: Apache-2.0

#include <cstring>
#include <musica/util.hpp>

String ToString(char* value)
{
    String str(value, std::strlen(value));
    return str;
}

ConstString ToConstString(const char* value)
{
    ConstString str(value, std::strlen(value));
    return str;
}