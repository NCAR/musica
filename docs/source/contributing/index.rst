.. _contributing:

Contributing
============

The code for MUSICA is hosted on `GitHub <https://github.com/NCAR/musica>`_. For any proposed changes (bug fixes, new
features, documentation updates, etc.), please start by opening an `issue <https://github.com/NCAR/musica/issues/new/choose>`_
describing your request or planned contribution.

Creating a development environment
-----------------------------------
To contribute to MUSICA and test changes locally, we recommend making a `conda <https://www.anaconda.com/docs/getting-started/miniconda/main>`_ environment:

.. code-block:: console

    $ git clone https://github.com/NCAR/musica.git
    $ cd musica
    $ conda create --name musica python=<minimum-3.9> --yes
    $ conda activate musica

After the GitHub repository is cloned and the virtual environment made, the MUSICA project can be built as follows:

.. code-block:: console

    $ mkdir build && cd build
    $ ccmake ..
    $ make

and `pip` can be used for an editable installation:

.. code-block:: console

    $ pip install -e .

For detailed developer options and dependency management, see our `Software Development Plan <https://github.com/NCAR/musica/blob/main/docs/Software%20Development%20Plan.pdf>`_.

Types of contributions
-----------------------
We appreciate all types of contributions:

Code Contributions
^^^^^^^^^^^^^^^^^^^
- Bug fixes
- New features
- Performance improvements
- Test coverage improvements

Documentation
^^^^^^^^^^^^^^^
- README improvements
- Code documentation
- Tutorial and example updates
- Wiki contributions

Testing and Validation
^^^^^^^^^^^^^^^^^^^^^^^
- Bug reports with reproducible examples
- Testing on different platforms
- Validation against known results

Scientific Contributions
^^^^^^^^^^^^^^^^^^^^^^^^^^
- New chemical mechanisms
- Algorithm improvements
- Performance optimizations

Contribution process
---------------------
1. **Fork the repository** and create a feature branch
2. **Make your changes** following our coding standards
3. **Add tests** for new functionality
4. **Update documentation** as needed
5. **Submit a pull request** with a clear description

Development guidelines
-----------------------
This section outlines best practices and requirements for contributing code, managing dependencies, and ensuring compatibility across platforms.

Code standards
^^^^^^^^^^^^^^
- Follow existing code style and conventions. We primarily follow the `Google C++ Style Guide <https://google.github.io/styleguide/cppguide.html>`_ and `PEP 8 <https://peps.python.org/pep-0008/>`_ for Python.
- Include appropriate tests for new functionality
- Document new features and APIs

Dependency management
^^^^^^^^^^^^^^^^^^^^^^
We use CMake for C++ dependencies and pip for Python dependencies. See our `README <https://github.com/NCAR/musica?tab=readme-ov-file#developer-options>`_ for information about specifying dependency versions during development.

GPU support
^^^^^^^^^^^^
If contributing GPU-related code, please test on appropriate hardware and
follow our GPU build guidelines.

Documentation
-------------
All of our docs are stored in the ``docs`` directory and built using `Sphinx <https://www.sphinx-doc.org/en/master/>`_. 
There are several Python dependencies that are necessary to build the documentation locally. These dependencies can be installed by 
running the following from your cloned ``music-box`` directory:

.. code-block:: console

    $ cd docs
    $ pip install -r requirements.txt

For contributors wanting to visualize changes to the C++ API Reference, a separate xml folder must be generated in the ``/build`` folder with the following command run from the ``docs`` directory:

.. code-block:: console

    $ doxygen Doxyfile.in

To build the documentation locally after edits:

- On macOS/Linux: ``make html``
- On Windows (cmd or PowerShell): ``.\make.bat html``

Recognition policy
------------------
We believe in recognizing all contributors appropriately:

Core Development Team
^^^^^^^^^^^^^^^^^^^^^^^
Contributors who make substantial, ongoing contributions to the codebase, architecture, or project direction will be listed as authors in:

- `pyproject.toml` (for Python package metadata)
- `.zenodo.json` (as "creators" for software citations)
- `AUTHORS.md` (as core developers)

Additional Contributors  
^^^^^^^^^^^^^^^^^^^^^^^^^
Contributors who make valuable but smaller contributions will be acknowledged in:

- `.zenodo.json` (as "contributors" with appropriate type)
- `AUTHORS.md` (in the Additional Contributors section)
- GitHub's contributor list (automatic)

Questions?
-----------
- Check our `documentation <https://ncar.github.io/musica/index.html>`_
- Read our `Software Development Plan <https://github.com/NCAR/musica/blob/main/docs/Software%20Development%20Plan.pdf>`_
- Contact the maintainers at musica-support@ucar.edu

Thank you for your interest in contributing to MUSICA!