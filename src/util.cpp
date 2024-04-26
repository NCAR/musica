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

ConstString ToConstString(const char* value)
{
    ConstString str;
    str.value_ = value;
    str.size_ = std::strlen(value);
    return str;
}

void DeleteString(String str)
{
    delete[] str.value_;
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
    error.category_ = ToConstString(category);
    error.message_ = ToConstString(message);
    return error;
}

Error ToError(const std::system_error& e)
{
    return ToError(e.code().category().name(), e.code().value(), e.what());
}

bool operator==(const Error& lhs, const Error& rhs)
{
    if (lhs.code_ == 0 && rhs.code_ == 0)
    {
        return true;
    }
    return lhs.code_ == rhs.code_ &&
           std::strcmp(lhs.category_.value_, rhs.category_.value_) == 0;
}

bool operator!=(const Error& lhs, const Error& rhs)
{
    return !(lhs == rhs);
}