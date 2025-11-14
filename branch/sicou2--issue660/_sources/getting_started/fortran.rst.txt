Fortran
============

1. Installing MUSICA
------------------------
The MUSICA library and the MUSICA-Fortran interface can be installed
together or separately. The MUSICA library is required to build the
MUSICA-Fortran interface.

The MUSICA library and the MUSICA-Fortran interface can be installed
using the CMake build system.

1a. Local installation
~~~~~~~~~~~~~~~~~~~~~~~~

First, ensure that you have the required dependencies installed. On
Fedora, you can install the required dependencies with the following:

.. code-block:: bash

  sudo dnf install -y cmake git gcc-c++ gcc-gfortran netcdf-devel netcdf-fortran-devel

On other distributions or operating systems, you may need to install the dependencies
using the package manager for your system.

Next, clone the MUSICA repository from GitHub:

.. code-block:: bash

  git clone https://github.com/NCAR/musica.git

Next, create a build directory and run CMake:

.. code-block:: bash

  mkdir build
  cd build
  cmake -D CMAKE_INSTALL_PREFIX=<INSTALL_DIR>  -D MUSICA_BUILD_FORTRAN_INTERFACE=ON ../musica

where ``<INSTALL_DIR>`` is the directory where you want to install MUSICA.
We use the ``MUSICA_BUILD_FORTRAN_INTERFACE`` option to build the MUSICA-Fortran
interface, which is not built by default.

Finally, build and install MUSICA:

.. code-block:: bash

  make
  make install

1b. Docker Installation
~~~~~~~~~~~~~~~~~~~~~~~~~

Alternatively, you can build and install MUSICA using Docker. First, ensure
that you have
`Docker Desktop <https://www.docker.com/products/docker-desktop/>`_
installed and running on your system.

Then, clone the MUSICA GitHub repository and use the provided Dockerfile to
build the MUSICA Docker image:

.. code-block:: bash

  git clone https://github.com/NCAR/musica.git
  cd musica
  docker build -t musica-fortran -f docker/Dockerfile.fortran-gcc .
  docker run -it musica-fortran bash

You can then perform the remainder of the tutorial inside the Docker container.
The ``<MUSICA_DIR>`` directory used throughout the tutorial will be located at ``/musica/build``

Once you are finished, you can exit the container by typing ``exit``. (Note that
the container will be deleted along with any files you created or modified when
you exit.)

2. An MICM Box Model Example in Fortran
-----------------------------------------

In this next MUSICA Fortran example, we will setup a MICM solver, starting with a set of MICM configuration files,
and run the solver for a single integration time step.

The following three configuration files (``config.json``, ``species.json``, and ``reactions.json``)
should be saved in a subdirectory named ``configs/analytical`` relative directory
from which you plan to call the box model executable.

(You can find a copy of these files in the MUSICA repository at ``configs/analytical``.)

The top-level MICM configuration ``config.json`` file
simply lists the configuration files to parse. In this case, these are the 
chemical species configuration file ``species.json`` and
the reactions configuration file ``reactions.json``.

The contents of the ``config.json`` file for this example are:

  .. literalinclude:: ../../../configs/v0/analytical/config.json
    :language: json

For this example, we will have a system of three chemical species
`A`, `B`, and `C`, defined in the JSON file ``species.json`` as follows:

  .. literalinclude:: ../../../configs/v0/analytical/species.json
    :language: json

The ``reactions.json`` specifies a mechanism, or a set of reactions for the system.
Here, we will introduce two Arrhenius type reactions, the first
with `B` evolving to `C`, and specifying all five reaction parameters,
and the second reaction with `A` evolving to `B` and using only two reaction parameters.

We also include two reactions with rate constants that are provided by the host
application at runtime. These types of reactions are useful when outside calculations
are needed to determine the rate constants, such as in the case of photolysis reactions.

The ``reactions.json`` file for this example should look like this:

  .. literalinclude:: ../../../configs/v0/analytical/reactions.json
    :language: json

More information on MICM configurations and reactions can be found in the `MICM documentation
<https://ncar.github.io/micm/user_guide/>`_

To create a simple box model, save the following Fortran code to a file named ``micm_box_model.F90``: 

  .. literalinclude:: ../../../fortran/test/fetch_content_integration/test_micm_box_model.F90
    :language: f90

From the ``musica_util`` module we need the Fortran types
``error_t``, ``string_t``, and ``mapping_t``.
A pointer to a ``musica_micm::micm_t`` will serve as the interface to the MICM solver
(in the example the pointer name is ``micm``).
Note that the ``config_path`` in the code sample has been set to ``configs/analytical``,
so that subdirectory should be created relative to the main program and contain
the MICM JSON configuration files,
otherwise the ``config_path`` should be modified appropriately.
The initial species concentrations are initialized in the ``concentrations`` array,
which is an argument to the MICM solver.

Finally, a single time step solution is obtained through a call to ``micm%solve``,
after which the updated concentrations may be displayed.

To build the example, follow the instructions in :ref:`chapter1`.

Assuming you name the executable ``micm_box_model``, you can run the program as follows:

.. code-block:: bash

  $ ./micm_box_model
 Creating MICM solver...
 Species Name:A, Index:           1
 Species Name:B, Index:           2
 Species Name:C, Index:           5
 Species Name:D, Index:           3
 Species Name:E, Index:           4
 Species Name:F, Index:           6
 Solving starts...
 After solving, concentrations  0.38236272259073301        1.4676807523204496       0.67030703490468713        1.1155750798779909        1.1499565250888166        1.2141178852173222
  $

