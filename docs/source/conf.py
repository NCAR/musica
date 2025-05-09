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
sys.path.insert(0, os.path.abspath('.'))

# -- Project information -----------------------------------------------------

project = 'MUSICA'
copyright = f'2024-{datetime.datetime.now().year}, NSF NCAR'
author = 'NSF NCAR'

suffix = ''
# the suffix is required. This is controlled by the dockerfile that builds
# the docs

regex = r'project\(.*VERSION\s+(\d+\.\d+\.\d+)\)'
release = f'0.0.0'
# read the version from the cmake files
with open(f'../../CMakeLists.txt', 'r') as f:
    for line in f:
        match = re.match(regex, line)
        if match:
            release = match.group(1)

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
]

breathe_default_project = "musica"

bibtex_bibfiles = ['references.bib']
suppress_warnings = ["bibtex.missing_field"]

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = []

autosummary_generate = True

# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = 'pydata_sphinx_theme'

html_theme_options = {
    "external_links": [],
    "github_url": "https://github.com/NCAR/musica",
    "navbar_end": ["version-switcher", "navbar-icon-links"],
    "switcher": {
        "json_url": "https://ncar.github.io/musica/_static/switcher.json",
        "version_match": release,  # assumes `release` is defined
    },
    "pygments_light_style": "tango",
    "pygments_dark_style": "monokai",
}

html_css_files = [
    'custom.css'
]

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ['_static']

html_favicon = '_static/favicon/favicon.ico'
