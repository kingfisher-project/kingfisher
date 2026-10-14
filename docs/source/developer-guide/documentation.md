# Writing Documentation

This site is built with [Sphinx](https://www.sphinx-doc.org) from the Markdown files in `docs/source/`, and the API reference is built with [Doxygen](https://www.doxygen.nl) from comments in the C++ source. Both are rebuilt and published on every push to `main`.

## Building locally

Building the docs is optional and needs a few tools that the code build doesn't:

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

A C++ compiler is also needed, because CMake checks for one when configuring. The Doxygen theme, [doxygen-awesome-css](https://github.com/jothepro/doxygen-awesome-css), is downloaded by CPM.

The easiest way to get all of these is the conda environment in `docs/environment.yml`:

```console
$ conda env create -f docs/environment.yml
$ conda activate kingfisher-docs
```

Alternatively, install the Python packages with pip, into a virtual environment for example, and the rest with your system package manager:

```console
$ pip install -r docs/requirements.txt
$ sudo apt install doxygen graphviz   # or: brew install doxygen graphviz
```

Then build the site:

```console
$ cmake --workflow --preset docs
```

This runs Doxygen and then Sphinx, and writes the site to `build/docs/docs/html/`. Sphinx warnings are treated as errors, as they are in continuous integration. The `docs` preset doesn't compile Kingfisher or download its libraries.

While writing, use the live preview instead. It serves the site locally and rebuilds a page when its source file is saved:

```console
$ cmake --build --preset docs --target docs-serve
```

`docs-serve` runs Doxygen once at startup. After changing Doxygen comments, run `cmake --build --preset docs --target docs-doxygen` to refresh the API reference.

## Where things go

| Content | Location |
|---|---|
| What a class or function does, its parameters, and its return value | Doxygen comments in the source |
| Why the code works the way it does, and how to use it | Pages in `docs/source/` |
| References | `docs/source/references.bib` |
| Figures | Next to the page that uses them |

Each sidebar section is a directory with an `index.md` that lists its pages in a `toctree`. A new page appears in the sidebar once it is added to one.

## Markup

Pages are written in [MyST Markdown](https://myst-parser.readthedocs.io), which is Markdown with Sphinx's directives and roles.

Math uses LaTeX, inline as `$...$` and displayed as `$$...$$`. A label after a displayed equation numbers it and makes it referable:

```markdown
$$
\vOmega \cdot \nabla \angflux + \sigt \angflux = q
$$ (eq-example)

See {eq}`eq-example`.
```

Recurring symbols are defined once as MathJax macros in `mathjax3_config` in `docs/source/conf.py`, so that every page writes them the same way:

| Macro | Symbol | Meaning |
|---|---|---|
| `\vr` | $\vr$ | position |
| `\vOmega` | $\vOmega$ | direction |
| `\angflux` | $\angflux$ | angular flux |
| `\sclflux` | $\sclflux$ | scalar flux |
| `\sigt` | $\sigt$ | total cross section |
| `\sigs` | $\sigs$ | scattering cross section |

Cite a reference from `references.bib` with `` {cite}`key` ``, and link to a class in the API reference with `` {cpp:class}`kingfisher::ClassName` `` once it has a page under [](../api/index.md).

## Appearance

The site uses the [Furo](https://pradyunsg.me/furo/) theme, which follows the reader's system light or dark setting and has a toggle at the top of each page. Its light and dark palettes are set in `html_theme_options` in `docs/source/conf.py`, with a few additions in `docs/source/_static/css/kingfisher.css`. The Doxygen reference is styled separately in `docs/doxygen/customization.css`. All three take their colors from the logo; change a color in each to keep them in step.
