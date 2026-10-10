# Writing Documentation

This site is built with [Sphinx](https://www.sphinx-doc.org) from the Markdown files in `docs/source/`, and the API reference is built with [Doxygen](https://www.doxygen.nl) from comments in the C++ source. Both are rebuilt and published on every push to `main`.

## Building locally

```console
$ pixi run docs
```

This runs Doxygen and then Sphinx, and writes the site to `docs/_build/html/`. Sphinx warnings are treated as errors, as they are in continuous integration.

While writing, use the live preview instead. It serves the site locally and rebuilds a page when its source file is saved:

```console
$ pixi run docs-serve
```

`docs-serve` runs Doxygen once at startup. After changing Doxygen comments, run `pixi run docs-doxygen` to refresh the API reference.

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

The site's colors are set in `docs/source/_static/css/kingfisher.css`, and the Doxygen reference's in `docs/doxygen/customization.css`. Both take their palette from the logo; change a color in both files to keep the two in step.
