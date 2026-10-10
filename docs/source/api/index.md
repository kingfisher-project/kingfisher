# C++ API

The API reference comes in two forms, both generated from the Doxygen comments in the source:

- The <a href="../doxygen/index.html"><strong>full Doxygen reference</strong></a> lists every class, function, and file, with inheritance and include graphs and a source browser.
- The **pages in this section** present the main classes by topic, alongside the rest of this documentation.

:::{note}
Kingfisher has no public classes yet, so this section has no pages. Add one per module as the code lands.
:::

## Adding an API page

Create a Markdown file in `docs/source/api/`, add it to a `toctree` on this page, and pull in each class with a Breathe directive:

````markdown
# Mesh

```{doxygenclass} kingfisher::Mesh
```
````

Other pages can then link to the class with `` {cpp:class}`kingfisher::Mesh` ``. The [Breathe directive reference](https://breathe.readthedocs.io/en/latest/directives.html) lists the directives for functions, structs, enums, and namespaces.
