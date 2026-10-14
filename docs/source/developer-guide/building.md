# Building from Source

Kingfisher's build configurations are CMake presets, defined in `CMakePresets.json`. Each one builds in its own directory, `build/<preset>/`. The dependencies are listed in `cmake/Dependencies.cmake` and fetched with [CPM.cmake](https://github.com/cpm-cmake/CPM.cmake); see [](../getting-started/install.md) for the prerequisites.

## Workflows

A workflow preset runs configure, build, and (where relevant) test in one command:

```console
$ cmake --workflow --preset debug
```

| Workflow | Build type | Builds | Runs tests |
|---|---|---|---|
| `debug` | `Debug` | everything | yes |
| `dev` | `RelWithDebInfo` | everything | yes |
| `release` | `Release` | the `kingfisher` executable | no |
| `docs` | — | the documentation (see [](documentation.md)) | no |

Use `debug` when stepping through code in a debugger, and `dev` for day-to-day work, since it is optimized but still has debug info.

## Individual steps

The workflow steps can also be run on their own, which is faster while editing because configure only needs to run once:

```console
$ cmake --preset dev            # configure
$ cmake --build --preset dev    # build
$ ctest --preset dev            # test
```

Every preset name (`debug`, `dev`, `release`, `docs`) works for `cmake --preset` and `cmake --build --preset`. `debug` and `dev` also have test presets.

## Options

| Option | Default | Meaning |
|---|---|---|
| `KINGFISHER_BUILD_TESTS` | `ON` (`OFF` in `release`) | Build the unit tests |
| `KINGFISHER_BUILD_DOCS` | `OFF` (`ON` in `docs`) | Build only the documentation |
| `KINGFISHER_USE_MPI` | `OFF` | Reserved for the parallel build; not implemented yet |

Pass an option when configuring, for example `cmake --preset dev -DKINGFISHER_BUILD_TESTS=OFF`.

## Parallel builds (planned)

For now, Kingfisher and MFEM are built in serial. Supporting MPI involves:

1. **MFEM.** In `cmake/Dependencies.cmake`, set `MFEM_USE_MPI` to `ON` and provide hypre and METIS, either with MFEM's own `MFEM_FETCH_HYPRE`/`MFEM_FETCH_METIS` options or as CPM packages.
2. **Kingfisher.** In `src/CMakeLists.txt`, find MPI and link `MPI::MPI_CXX`.
3. **Tests.** In `tests/CMakeLists.txt`, register multi-rank tests through `mpiexec`.
4. **Presets.** Add `mpi-*` configure presets that inherit from `base` and set `KINGFISHER_USE_MPI=ON`, then remove the error in the top-level `CMakeLists.txt` that rejects it.

Each of these places has a `NOTE(parallel)` comment with the details.
