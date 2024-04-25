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
    Error error;
    error.code_ = 0;
    error.category_ = ToConstString("");
    error.message_ = ToConstString("Success");
    return error;
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
    Error error;
    error.code_ = e.code().value();
    error.category_ = ToConstString(e.code().category().name());
    error.message_ = ToConstString(e.what());
    return error;
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