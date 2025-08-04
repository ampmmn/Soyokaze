# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

version = '0.42.0'

from configparser import ConfigParser
config = ConfigParser()
config.read('conf.ini', encoding='utf-8')

project = config.get('config', 'project', fallback='Soyokaze')
author = config.get('config', 'author', fallback='ymgw')
author = config.get('config', 'copyright', fallback='2023, ymgw')

html_title = f'{project} {version}'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
        'myst_parser',
        'sphinxcontrib.plantuml',
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

language = 'ja'

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'furo'
html_static_path = ['_static']

from sphinx.application import Sphinx

def setup(app: Sphinx):
    if project == "Soyokaze":
        app.tags.add("soyokaze")

