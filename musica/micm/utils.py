# Copyright (C) 2023-2025 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0
#
# This file is part of the musica Python package.
# For more information, see the LICENSE file in the top-level directory of this distribution.
from __future__ import annotations
import numpy as np

# Import backend symbols from the backend module
from .. import backend

# Get all the backend symbols we need
_backend = backend.get_backend()

species_ordering = _backend._micm._species_ordering
user_defined_rate_parameters_ordering = _backend._micm._user_defined_rate_parameters_ordering


def is_scalar_number(x):
    return (
        isinstance(x, (int, float, np.number))
        and not isinstance(x, bool)
    )
