"""Sphinx configuration for the Kingfisher documentation."""

import re
import sys
from datetime import date
from pathlib import Path

DOCS_DIR = Path(__file__).resolve().parent.parent
REPO_DIR = DOCS_DIR.parent

# Local Sphinx extensions.
sys.path.insert(0, str(Path(__file__).resolve().parent / "_ext"))

# -- Project information -----------------------------------------------------

# The version is set once, in the project() call of the top-level CMakeLists.txt.
_cmakelists = (REPO_DIR / "CMakeLists.txt").read_text()
_version = re.search(
    r"project\s*\([^)]*\bVERSION\s+([0-9][0-9.]*)", _cmakelists)

project = "Kingfisher"
author = "Liam Pohlmann"
copyright = f"{date.today().year}, {author}"
release = _version.group(1)
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
    "sphinx_licenseinfo",
    "schema_doc",
    "sphinx.ext.autosectionlabel",
]

autosectionlabel_prefix_document = True

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

# -- Input file schema --------------------------------------------------------

# The JSON Schema that the {schema-block} directive (docs/source/_ext/schema_doc.py)
# builds the input file tables from.
schema_path = str(REPO_DIR / "python" / "src" /
                  "kingfisher" / "schemas" / "input-v0.schema.json")

# -- Breathe (C++ API from Doxygen XML) --------------------------------------

# Written by the docs-doxygen CMake target; see docs/CMakeLists.txt, which
# passes the real path with -D. The default matches the "docs" preset.
breathe_projects = {"kingfisher": str(
    REPO_DIR / "build" / "docs" / "docs" / "doxygen-xml")}
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
