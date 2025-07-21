Multiple Grid Cell Calculations
================================
Thus far, this user guide has focused on box model examples in MUSICA, where the solver treats a single, well-mixed air mass.
However, the `Solver` object in MUSICA includes an important attribute: `number_of_grid_cells`.
This attribute specifies the number of independent, well-mixed air masses (grid cells) whose chemical systems will be solved simultaneously by the same numerical solver.

Here, we will demonstrate how to set up and solve a multi-grid-cell chemical system in MUSICA, building on the concepts introduced in the earlier box model examples.
The following libraries will be used::

    import musica
    import musica.mechanism_configuration as mc
    import matplotlib.pyplot as plt
    import pandas as pd
    import numpy as np

.. toctree::
   :maxdepth: 1
   :caption: Contents:

   two-grid-cell/index
   hypercube/index
   parallel/index