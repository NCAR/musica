/// Copyright (C) 2023-2024 National Center for Atmospheric Research
/// SPDX-License-Identifier: Apache-2.0

#include <musica/error.hpp>
#include <iostream>

ErrorHandler MusicaErrorHandler = DefaultErrorHandler;

void DefaultErrorHandler(const int code, const char* message)
{
    std::cerr << "Error " << code << ": " << message << std::endl;
    exit(code);
}

void SetErrorHandler(ErrorHandler handler)
{
    MusicaErrorHandler = handler;
}

void Error(const int code, const char* message)
{
    MusicaErrorHandler(code, message);
}