# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

version = '0.50.0'

from configparser import ConfigParser
config = ConfigParser()
config.read('conf.ini', encoding='utf-8')

project = config.get('config', 'project', fallback='Soyokaze')
author = config.get('config', 'author', fallback='ymgw')
copyright = config.get('config', 'copyright', fallback='2023, ymgw')

html_title = f'{project} {version}'

html_show_sourcelink = False
html_show_sphinx = False

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
        'myst_parser',
        'sphinxcontrib.plantuml',
        'sphinx_rtd_theme',
        'sphinx_fontawesome',
]

myst_enable_extensions = [
    "colon_fence",
    "attrs_block",
    "substitution"
]

myst_substitutions = {
    "project": f"{project}",
    "project_lower": config.get("config", "project_lower", fallback=f"soyokaze"),
    "version": f"{version}",
    "distribution_url": config.get("config", "distribution_url", fallback=f""),
}

templates_path = ['_templates']
exclude_patterns = []

language = 'en'

gettext_compact = False

locale_dirs = ['locale/']

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

#html_theme = 'furo'
html_theme = 'sphinx_rtd_theme'
html_static_path = ['_static']

from sphinx.application import Sphinx

def setup(app: Sphinx):
    if project == "Soyokaze":
        app.tags.add("soyokaze")

