/// Copyright (C) 2023-2024 National Center for Atmospheric Research
/// SPDX-License-Identifier: Apache-2.0
#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

/// @brief Default error handler
void DefaultErrorHandler(const int code, const char* message);

/// @brief Error handler function pointer
typedef void (*ErrorHandler)(const int code, const char* message);

/// @brief Set the error handler
void SetErrorHandler(ErrorHandler handler);

/// @brief Error function
void Error(const int code, const char* message);

#ifdef __cplusplus
}
#endif
