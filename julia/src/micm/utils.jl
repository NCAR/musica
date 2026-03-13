# Copyright (C) 2023-2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0

"""
    flat_index(i_cell, i_item, n_items, vector_size) -> Int

Calculate the 0-based flat index into the ordered concentration or rate parameter vector.

Uses the same vector-ordering formula as the Python interface:
  `idx = (group_index * n_items + i_item) * vector_size + row_in_group`

# Arguments
- `i_cell::Int`: 1-based grid cell index
- `i_item::Int`: 0-based species or rate parameter index (from C++ ordering map)
- `n_items::Int`: Total number of species or rate parameters
- `vector_size::Int`: Vector dimension for vector-ordered solvers (1 for standard-order)

# Returns
0-based index suitable for passing to C++ accessor functions.
"""
function flat_index(i_cell::Int, i_item::Int, n_items::Int, vector_size::Int)
    cell_0 = i_cell - 1  # convert to 0-based
    group_index = cell_0 ÷ vector_size
    row_in_group = cell_0 % vector_size
    return (group_index * n_items + i_item) * vector_size + row_in_group
end
