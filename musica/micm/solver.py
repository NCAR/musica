# Copyright (C) 2023-2025 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0

from .. import backend

_backend = backend.get_backend()

SolverType = _backend._micm._SolverType
