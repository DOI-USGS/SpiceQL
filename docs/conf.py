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
# import os
# import sys
# sys.path.insert(0, os.path.abspath('.'))

import subprocess, os

breathe_projects = {}

# Check if we're running on Read the Docs' servers
read_the_docs_build = os.environ.get('READTHEDOCS', None) == 'True'
build_started = os.path.isfile("CMakeCache.txt")

# go into main build directory 
# os.chdir('../build')

# Print some debug stuff
print("READ THE DOCS SERVER FLAG: ", read_the_docs_build)
print("BUILD FLAG: ", build_started)
print("WORKING DIR: ", os.getcwd())

output_dir = 'docs'
# subprocess.call('doxygen', shell=True)
breathe_projects['SpiceQL'] = output_dir + '/xml'

# -- Project information -----------------------------------------------------

project = 'SpiceQL'
copyright = '2021, USGS'
author = 'USGS Astrogeology'

# The full version, including alpha/beta/rc tags
release = '0.1'


# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [ "breathe", "m2r2", "sphinx_material", "sphinxcontrib.gist"]

# Breathe Configuration
breathe_default_project = "SpiceQL"

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']


# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#

html_theme = 'sphinx_material'
html_favicon  = "favicon.ico"

# Material theme options (see theme.conf for more information)
html_theme_options = {

    # Set the name of the project to appear in the navigation.
    'nav_title': 'Sugar Spice',

    # Set you GA account ID to enable tracking
    # 'google_analytics_account': 'UA-XXXXX',

    # Specify a base_url used to generate sitemap.xml. If not
    # specified, then no sitemap will be built.
    # 'base_url': 'https://github.com/USGS-Astrogeology/SpiceQL',

    # Set the color and the accent color
    'color_primary': '#000000',
    'color_accent': 'light-blue',

    'logo_icon': '&#xe87a',

    # Set the repo location to get a badge with stats
    'repo_url': 'https://github.com/USGS-Astrogeology/SpiceQL',
    'repo_name': 'Project',

    # Visible levels of the global TOC; -1 means unlimited
    'globaltoc_depth': 3,
    # If False, expand all TOC entries
    'globaltoc_collapse': False,
    # If True, show hidden TOC entries
    'globaltoc_includehidden': False,
}

html_sidebars = {
    "**": ["logo-text.html", "globaltoc.html", "localtoc.html", "searchbox.html"]
}

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
# html_static_path = ['_static']

if read_the_docs_build and not build_started:
        subprocess.call('cmake ..', shell=True) 
        subprocess.call('make Sphinx', shell=True)
        os.environ["SPICEQLDOCSBUILD"] = "True"
