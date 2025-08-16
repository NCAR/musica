.. _tutorials page:

##########
Tutorials
##########

MUSICA offers a series of tutorial snippets in Fortran as well as tutorial notebooks in Python that guide users from simple workflows to more advanced simulations, both available here.

Fortran
========
1. :ref:`installing MUSICA <chapter0>`
2. :ref:`first Fortran MUSICA program <chapter1>`
3. :ref:`box model example <chapter2>`

.. toctree::
   :hidden:
   
   chapter0
   chapter1
   chapter2


Python
========
The Python tutorials are written in `Jupyter <https://jupyter.org>`_ Notebooks and made available in multiple formats below for ease of access.


Interactive Notebooks
----------------------
The MUSICA repository utilizes `Binder <https://mybinder.readthedocs.io/en/latest/index.html#>`_ to allow users to interact with the tutorial notebooks on a `JupyterHub <https://jupyter.org/hub>`_.
Please note that our `Binder page <https://mybinder.org/v2/gh/NCAR/musica/HEAD?filepath=tutorials>`_ uses the latest version of MUSICA pushed to main as opposed to the latest release.
Each of the links below will open a JupyterHub set up with all necessary dependencies to run each tutorial:

1. `working with multiple grid cells <https://mybinder.org/v2/gh/NCAR/musica/HEAD?filepath=tutorials/1.%20multiple_grid_cells.ipynb>`_
2. `latin hypercube sampling <https://mybinder.org/v2/gh/NCAR/musica/HEAD?filepath=tutorials/2.%20hypercube.ipynb>`_
3. `user-defined reactions <https://mybinder.org/v2/gh/NCAR/musica/HEAD?filepath=tutorials/3.%20user_defined_reactions.ipynb>`_
4. `local paralellization <https://mybinder.org/v2/gh/NCAR/musica/HEAD?filepath=tutorials/4.%20local_parallelization.ipynb>`_
5. `HPC parallelization <https://mybinder.org/v2/gh/NCAR/musica/HEAD?filepath=tutorials/5.%20hpc_parallelization.ipynb>`_
6. `using GPU solvers <https://mybinder.org/v2/gh/NCAR/musica/HEAD?filepath=tutorials/6.%20gpu_solver.ipynb>`_
7. `using CARMA <https://mybinder.org/v2/gh/NCAR/musica/HEAD?filepath=tutorials/7.%20carma.ipynb>`_

GitHub
--------
For users that wish to directly download local copies of the tutorial notebooks, they are each made available on our GitHub within the `tutorials <https://github.com/NCAR/musica/tree/main/tutorials>`_ folder. Each notebook is also linked below:

1. `multiple grid cells notebook <https://github.com/NCAR/musica/blob/main/tutorials/1.%20multiple_grid_cells.ipynb>`_
2. `hypercube sampling notebook <https://github.com/NCAR/musica/blob/main/tutorials/2.%20hypercube.ipynb>`_
3. `user-defined reactions notebook <https://github.com/NCAR/musica/blob/main/tutorials/3.%20user_defined_reactions.ipynb>`_
4. `local parallelization notebook <https://github.com/NCAR/musica/blob/main/tutorials/4.%20local_parallelization.ipynb>`_
5. `HPC parallelization notebook <https://github.com/NCAR/musica/blob/main/tutorials/5.%20hpc_parallelization.ipynb>`_
6. `GPU solvers notebook <https://github.com/NCAR/musica/blob/main/tutorials/6.%20gpu_solver.ipynb>`_
7. `CARMA notebook <https://github.com/NCAR/musica/blob/main/tutorials/7.%20carma.ipynb>`_


Web View
---------
The tutorial notebooks are also included here in the documentation for convenient online browsing.

.. toctree::
   :maxdepth: 1

   1. multiple_grid_cells
   2. hypercube
   3. user_defined_reactions
   4. local_parallelization
   5. hpc_parallelization
   6. gpu_solver
   7. carma