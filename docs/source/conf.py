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

html_theme = "sphinx_rtd_theme"
html_title = f"Kingfisher {release} documentation"
html_logo = "../logo/logo_kingfisher.svg"
html_favicon = "../logo/logo_kingfisher.svg"
html_static_path = ["_static"]
html_css_files = ["css/kingfisher.css"]
html_show_sphinx = False

html_theme_options = {
    "logo_only": False,
    # The logo has black linework, so it sits on a white header.
    "style_nav_header_background": "#FFFFFF",
    "collapse_navigation": False,
    "navigation_depth": 3,
    "prev_next_buttons_location": "bottom",
    "style_external_links": True,
}

# "Edit on GitHub" link in the top right of every page.
html_context = {
    "display_github": True,
    "github_user": "kingfisher-project",
    "github_repo": "kingfisher",
    "github_version": "main",
    "conf_py_path": "/docs/source/",
}
