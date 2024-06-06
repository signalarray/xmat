# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'xmat'
copyright = '2024, Signal Array'
author = 'Signal Array'
release = '0.1'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

import sphinx_rtd_theme

html_theme = "sphinx_rtd_theme"

html_theme_path = [sphinx_rtd_theme.get_html_theme_path()]


def setup(app):
	app.add_css_file("main_stylesheet.css")


extensions = ['sphinx_rtd_theme']


# templates_path = ['_templates']

exclude_patterns = []

html_static_path = ['_static']
