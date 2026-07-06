# Copyright (C) 2023-2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0

"""
    SolverType

Enum representing the type of MICM solver to use.

Values match the C++ `musica::MICMSolver` enum.
"""
@enum SolverType begin
    Rosenbrock = 1
    RosenbrockStandardOrder = 2
    BackwardEuler = 3
    BackwardEulerStandardOrder = 4
    CudaRosenbrock = 5
end
