Scale solver construction to large explicit mechanisms

Several superlinear time and memory costs made building solvers for large
explicit mechanisms (thousands of species, e.g. GECKO-A) intractable. This
reworks ProcessSet construction, the Markowitz reorder, the Doolittle symbolic
LU factorization, and per-reaction phase storage so setup scales to ~7,500
species in seconds instead of tens of minutes. Behavior is unchanged on
existing mechanisms (verified bit-for-bit ordering quality and passing unit
tests).

- ProcessSet Jacobian-info construction scanned every process for every
  independent variable -- O(species x reactions). Replaced with a single pass
  that collects (independent_id, process_id) jobs and stable-sorts them by
  independent_id, giving identical output ordering in O(reactions x reactants).

- DiagonalMarkowitzReorder operated on a dense order x order pattern matrix --
  O(order^3) time. Reimplemented on sparse adjacency sets, producing the same
  minimum-degree ordering in ~O(order^2 + fill). For a ~7,500-species mechanism
  the dense reorder alone took tens of minutes; the sparse version produces the
  same low-fill ordering (which keeps the subsequent LU factorization small) in
  seconds.

- ChemicalReactionBuilder::SetPhase stored a copy of the entire Phase --
  including every species in that phase -- on each reaction. Nothing in the
  solver-building path reads a reaction's phase species (the System's phase is
  what's used); only the phase NAME is ever read. For a ~7,500-species GECKO-A
  mechanism with ~42,000 reactions this copied ~114 GB of species (reactions x
  species x sizeof(Species)) before the solver was even built. Store only the
  phase name instead.

- The Doolittle symbolic factorization (GetLUMatrices and Initialize) walked the
  full dense (i, k, j) index grid -- an O(n^3) loop -- and probed element
  presence up to O(n^3) times via std::set::find (O(log n)) or
  SparseMatrix::IsZero (linear). Factored the symbolic factorization into a
  shared ComputeFillPattern() that walks only the non-zero structure, maintaining
  sorted adjacency for L rows, U rows and L columns in increasing-row order:
    U[i][k] fills iff A[i][k] != 0, k == i, or some j<i has L[i][j] and U[j][k]
    L[k][i] fills iff A[k][i] != 0, or some j<i has L[k][j] and U[j][i]
  This is O(non-zeros in the factors) plus an O(n^2) scan of the sparse input A,
  rather than O(n^3). Initialize rebuilds the same index arrays in identical
  order using binary search on the sorted rows for membership tests, so the
  produced factorization is unchanged. The Doolittle and linear-solver unit tests
  (including random-fill matrices and the Markowitz-reordered path) pass
  unchanged.

Also: comment and naming cleanups, removal of a redundant check, and an updated
Xcode version.

----

Co-authored-by: Claude Opus 4.8 (1M context) <noreply@anthropic.com>
