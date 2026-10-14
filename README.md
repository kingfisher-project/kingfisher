[![Docs](https://github.com/kingfisher-project/kingfisher/actions/workflows/docs.yml/badge.svg)](https://github.com/kingfisher-project/kingfisher/actions/workflows/docs.yml)

# Kingfisher: Spectral Element Radiation Transport Code
Kingfisher is a deterministic spectral element radiation transport code built on [mfem](https://github.com/mfem/mfem/).
This project is currently being developed by Liam Pohlmann at the University of Illinois Urbana-Champaign.

## Documentation
The documentation is at <https://kingfisher-project.github.io/kingfisher/>.

To build it locally, you need a C++ compiler plus the following packages:

| Package | Version |
|---|---|
| [CMake](https://cmake.org) | ≥ 3.25 |
| [Ninja](https://ninja-build.org) | ≥ 1.13.2 |
| [Doxygen](https://www.doxygen.nl) | ≥ 1.18.0 |
| [Graphviz](https://graphviz.org) | ≥ 14.1.2 |
| [Python](https://www.python.org) | ≥ 3.11 |
| [Sphinx](https://www.sphinx-doc.org) | ≥ 8.2.3, < 9 |
| [myst-parser](https://myst-parser.readthedocs.io) | ≥ 5.1.0, < 6 |
| [breathe](https://breathe.readthedocs.io) | ≥ 5.1.0, < 6 |
| [sphinxcontrib-bibtex](https://sphinxcontrib-bibtex.readthedocs.io) | ≥ 2.7.0, < 3 |
| [sphinx-copybutton](https://sphinx-copybutton.readthedocs.io) | ≥ 0.5.2, < 0.6 |
| [sphinx-design](https://sphinx-design.readthedocs.io) | ≥ 0.7.0, < 0.8 |
| [sphinx-autobuild](https://github.com/sphinx-doc/sphinx-autobuild) | ≥ 2025.8.25 |
| [furo](https://pradyunsg.me/furo/) | ≥ 2025.12.19 |

All of them are in the conda environment `docs/environment.yml`. The Python packages alone are also in `docs/requirements.txt` for pip. Then build the site:

```console
$ conda env create -f docs/environment.yml
$ conda activate kingfisher-docs
$ cmake --workflow --preset docs
```

and open `build/docs/docs/html/index.html`.

## Installation
See the documentation's [Getting Started](https://kingfisher-project.github.io/kingfisher/getting-started/index.html) page.

## License
Kingfisher is distributed under the BSD-3-Clause license.

