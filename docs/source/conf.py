# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
import os
import sys
import datetime
import re

conf_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, conf_dir)

# -- Project information -----------------------------------------------------

project = 'MUSICA'
copyright = f'2024-{datetime.datetime.now().year}, NSF-NCAR/ACOM'
author = 'NSF-NCAR/ACOM'

regex = r'project\(.*VERSION\s+(\d+\.\d+\.\d+)\)'
version = '0.0.0'
# Read the version from the cmake files (use absolute path from conf.py location)
cmake_file = os.path.join(conf_dir, '..', '..', 'CMakeLists.txt')
with open(cmake_file, 'r') as f:
    for line in f:
        match = re.match(regex, line)
        if match:
            version = match.group(1)
release = version

# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
    'breathe',
    'sphinx_copybutton',
    'sphinx_design',
    'sphinxcontrib.bibtex',
    'sphinx.ext.autodoc',
    'sphinx.ext.napoleon',
    'sphinx.ext.autosummary',
    'sphinx.ext.intersphinx',
    'nbsphinx'
]

# -- Breathe configuration ---------------------------------------------------

this_dir = os.path.dirname(os.path.abspath(__file__))
breathe_projects_dir = os.path.abspath(
    os.path.join(this_dir, "..", "..", "build", "docs", "doxygen", "xml")
)
breathe_projects = {"musica": breathe_projects_dir}
breathe_default_project = "musica"

highlight_language = 'python'

bibtex_bibfiles = ['references.bib']
suppress_warnings = ["bibtex.missing_field"]

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = []

autosummary_generate = True

# -- Intersphinx mappings -------------
intersphinx_mapping = {
    'micm': ('https://micm.readthedocs.io', None),
    'tuv-x': ('https://tuv-x.readthedocs.io', None),
    'mc': ('https://MechanismConfiguration.readthedocs.io', None),
    'mb': ('https://music-box.readthedocs.io', None)
}
# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = 'pydata_sphinx_theme'

html_theme_options = {
    "external_links": [],
    "github_url": "https://github.com/NCAR/musica",
    "navbar_end": ["navbar-icon-links"],
    "pygments_light_style": "tango",
    "pygments_dark_style": "monokai",
}

html_css_files = ['custom.css']

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ['_static']

html_favicon = '_static/favicon/favicon.ico'
