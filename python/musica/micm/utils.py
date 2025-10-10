# Copyright (C) 2023-2025 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0

import numpy as np

from .. import backend

_backend = backend.get_backend()

species_ordering = _backend._micm._species_ordering
user_defined_rate_parameters_ordering = _backend._micm._user_defined_rate_parameters_ordering


def is_scalar_number(x):
    return (
        isinstance(x, (int, float, np.number))
        and not isinstance(x, bool)
    )
