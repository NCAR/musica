/**
 * This file contains the version information for the project
 * Copyright (C) 2023-2024 National Center for Atmospheric Research,
 * 
 * SPDX-License-Identifier: Apache-2.0* creating solvers, and solving the model.
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

const char* getMusicaVersion();
unsigned getMusicaVersionMajor();
unsigned getMusicaVersionMinor();
unsigned getMusicaVersionPatch();
unsigned getMusicaVersionTweak();

#ifdef __cplusplus
}
#endif
