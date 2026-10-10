"""Sphinx configuration for the Kingfisher documentation."""

import tomllib
from datetime import date
from pathlib import Path

DOCS_DIR = Path(__file__).resolve().parent.parent
REPO_DIR = DOCS_DIR.parent

# -- Project information -----------------------------------------------------

with open(REPO_DIR / "pixi.toml", "rb") as f:
    _workspace = tomllib.load(f)["workspace"]

project = "Kingfisher"
author = "Liam Pohlmann"
copyright = f"{date.today().year}, {author}"
release = _workspace["version"]
version = ".".join(release.split(".")[:2])

# -- General configuration ---------------------------------------------------

extensions = [
    "myst_parser",
    "breathe",
    "sphinx.ext.mathjax",
    "sphinx.ext.githubpages",
    "sphinxcontrib.bibtex",
    "sphinx_copybutton",
    "sphinx_design",
]

source_suffix = {".md": "markdown", ".rst": "restructuredtext"}
root_doc = "index"
exclude_patterns = []
numfig = True

# -- MyST --------------------------------------------------------------------

myst_enable_extensions = [
    "amsmath",
    "colon_fence",
    "deflist",
    "dollarmath",
]
myst_heading_anchors = 3

# -- Math --------------------------------------------------------------------

# Shared notation, so every page writes the same symbol the same way.
mathjax3_config = {
    "tex": {
        "macros": {
            "vr": r"\mathbf{r}",
            "vOmega": r"\boldsymbol{\Omega}",
            "angflux": r"\psi",
            "sclflux": r"\phi",
            "sigt": r"\Sigma_t",
            "sigs": r"\Sigma_s",
        }
    }
}

# -- Breathe (C++ API from Doxygen XML) --------------------------------------

# Written by `pixi run docs-doxygen`; see docs/Doxyfile.
breathe_projects = {"kingfisher": str(DOCS_DIR / "_build" / "doxygen-xml")}
breathe_default_project = "kingfisher"
breathe_default_members = ("members", "undoc-members")

# -- Bibliography ------------------------------------------------------------

bibtex_bibfiles = ["references.bib"]
bibtex_default_style = "unsrt"

# -- HTML output -------------------------------------------------------------

html_theme = "furo"
html_title = f"Kingfisher {release} documentation"
html_logo = "../logo/logo_kingfisher.svg"
html_favicon = "../logo/logo_kingfisher.svg"
html_static_path = ["_static"]
html_css_files = ["css/kingfisher.css"]
html_show_sphinx = False

# Palette taken from docs/logo/logo_kingfisher.svg. Furo follows the reader's
# system light/dark setting and offers a toggle; these are its two palettes.
# docs/doxygen/customization.css uses the same colors for the Doxygen pages.
_TEAL = "#005D82"
_NAVY = "#04364A"
_ORANGE = "#D34305"

html_theme_options = {
    "light_css_variables": {
        "color-brand-primary": _TEAL,
        "color-brand-content": _TEAL,
        "color-brand-visited": _TEAL,
        "color-foreground-primary": "#252525",
        "color-foreground-secondary": "#5F6B70",
        "color-background-primary": "#FFFFFF",
        "color-background-secondary": "#F4F8FA",
        "color-background-hover": "#E6F0F4",
        "color-background-border": "#D5DDE0",
        "color-link--hover": _ORANGE,
        "color-link-underline--hover": _ORANGE,
        "color-inline-code-background": "#F4F8FA",
        "kf-heading": _NAVY,
        "kf-accent": _ORANGE,
        "kf-table-head": _NAVY,
    },
    "dark_css_variables": {
        "color-brand-primary": "#5BB8DA",
        "color-brand-content": "#5BB8DA",
        "color-brand-visited": "#5BB8DA",
        "color-foreground-primary": "#E8EEF0",
        "color-foreground-secondary": "#A9B8BE",
        "color-background-primary": "#0B1A21",
        "color-background-secondary": "#06222D",
        "color-background-hover": "#0F3442",
        "color-background-border": "#23404C",
        "color-link--hover": "#F0703A",
        "color-link-underline--hover": "#F0703A",
        "color-inline-code-background": "#022633",
        "kf-heading": "#E8EEF0",
        "kf-accent": "#F0703A",
        "kf-table-head": _NAVY,
    },
    # "Edit this page" link on every page.
    "source_repository": "https://github.com/kingfisher-project/kingfisher/",
    "source_branch": "main",
    "source_directory": "docs/source/",
}
