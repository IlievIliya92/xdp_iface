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
# sys.path.insert(0, os.path.abspath('.'))

import os
import sys
from xml.dom import minidom

# -- Project information -----------------------------------------------------

project = 'XDP Interface'
copyright = '2023, Iliya Iliev'
author = 'Iliya Iliev'

# -- Constants ---------------------------------------------------------------
PROJECT_FILE = 'project.xml'
if os.path.exists(f"../../{PROJECT_FILE}"):
    PROJECT_FILE_PATH = f"../../{PROJECT_FILE}"
elif os.path.exists(f"./{PROJECT_FILE}"):
    PROJECT_FILE_PATH = f"./{PROJECT_FILE}"
else:
    sys.exit(0)

# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
    "sphinx.ext.autodoc",
    "sphinx.ext.extlinks",
    "sphinx.ext.intersphinx",
    "sphinx.ext.mathjax",
    "sphinx.ext.todo",
    'sphinx_rtd_theme',
    'sphinxcontrib.autoprogram',
    "myst_parser",
    "sphinx_git",
    "sphinx_copybutton",
    "sphinx_design",
    "sphinx_inline_tabs",
    'sphinx_c_autodoc',
    'sphinx_c_autodoc.napoleon',
    'sphinx_c_autodoc.viewcode',
    'sphinx.ext.todo',
    'sphinx.ext.coverage',
]

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = []

# -- API C files -------------------------------------------------------------
c_autodoc_roots = ['../../src',
                   '../../include']

# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = "sphinx_rtd_theme"

autosectionlabel_prefix_document = True
html_theme_options = {
    'logo_only': False,
    'display_version': True,
    'prev_next_buttons_location': 'bottom',
    'style_external_links': True,
    'vcs_pageview_mode': '',
    'style_nav_header_background': 'rgba(52,49,49,1) 100%;',
    'collapse_navigation': True,
    'sticky_navigation': True,
    'navigation_depth': 2,
    'includehidden': True,
    'titles_only': True,
    'sticky_navigation': True
}

xml_file = minidom.parse(PROJECT_FILE_PATH)
project_version = xml_file.getElementsByTagName('version')
version = f"v.{project_version[0].attributes['major'].value}.\
{project_version[0].attributes['minor'].value}.{project_version[0].attributes['patch'].value}"

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_show_sourcelink = False
html_static_path = ['_static']
html_title = "XDP Interface"
#html_logo = "_static/logo.png"
