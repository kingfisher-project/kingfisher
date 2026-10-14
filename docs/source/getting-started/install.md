# Installation

Kingfisher is built with CMake. Its libraries ([MFEM](https://mfem.org), [fmt](https://fmt.dev), [rapidyaml](https://github.com/biojppm/rapidyaml), and [GoogleTest](https://google.github.io/googletest/) for the tests) are downloaded and built automatically by [CPM.cmake](https://github.com/cpm-cmake/CPM.cmake) the first time the project is configured.

## Prerequisites

- CMake 3.25 or newer
- [Ninja](https://ninja-build.org)
- A C++20 compiler (GCC 11+, Clang 14+, or Apple Clang 15+)
- Git, which CPM uses to download the dependencies

## Building

From the root of the repository:

```console
$ cmake --workflow --preset release
```

This configures and builds an optimized `kingfisher` executable at `build/release/kingfisher`. The first configure downloads and compiles MFEM, so it takes a few minutes. Later builds reuse it.

See [](../developer-guide/building.md) for the other presets, including debug builds and running the tests.

## Reusing downloads

By default each build directory keeps its own copy of the downloaded sources. To share one copy between build directories, and between checkouts, set `CPM_SOURCE_CACHE` before configuring:

```console
$ export CPM_SOURCE_CACHE=$HOME/.cache/CPM
```

## Using an existing MFEM

To build against an MFEM source tree you already have, for example one with local changes, point CPM at it:

```console
$ cmake --preset release -DCPM_mfem_SOURCE=/path/to/mfem
$ cmake --build --preset release
```

To use an installed MFEM instead (one that provides `MFEMConfig.cmake`), pass `-DCPM_USE_LOCAL_PACKAGES=ON`, which makes CPM try `find_package` before downloading. This applies to every dependency, not just MFEM. The installed MFEM must be a serial build.
