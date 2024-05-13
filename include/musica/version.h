/* Copyright (C) 2023-2024 National Center for Atmospheric Research
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#ifdef __cplusplus
namespace musica {
extern "C" {
#endif

const char* getMusicaVersion();
unsigned getMusicaVersionMajor();
unsigned getMusicaVersionMinor();
unsigned getMusicaVersionPatch();
unsigned getMusicaVersionTweak();

#ifdef __cplusplus
}
}
#endif
