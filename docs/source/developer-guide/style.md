# Code Style

Kingfisher's C++ follows the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html). This page summarizes the parts that come up most often, and adds Kingfisher's rules for data layout and for Python.

## Formatting

Formatting is defined by the `.clang-format` file at the root of the repository, which uses the Google base style. CI checks every pull request with clang-format 18. Format your changes before committing:

```console
$ find src include tests -name '*.cc' -o -name '*.h' | xargs clang-format -i
```

Most editors can run clang-format on save using the same `.clang-format` file.

## Linting

The `.clang-tidy` file at the root of the repository enforces the [naming](#naming) rules below and catches common bugs and performance problems, such as a loop variable or parameter that is copied when a reference would do. CI runs clang-tidy 18 on the changed files of every pull request. To run it locally, install clang-tidy 18 (for example `sudo apt install clang-tidy-18`) and point it at the build directory of any preset you have configured, for example `build/dev`:

```console
$ clang-tidy -p build/dev src/transport/sweep.cc
```

clang-tidy can't tell an accessor from any other method, so it accepts `snake_case` for all methods. Only use `snake_case` for accessors and mutators, as in the table below.

If a warning is wrong for a particular line, silence that check alone and say why:

```cpp
double* data = buffer.GetData();  // NOLINT(bugprone-...): reason
```

## Naming

| Kind | Convention | Example |
|---|---|---|
| Types (classes, structs, enums, type aliases) | `PascalCase` | `AngularQuadrature` |
| Functions and methods | `PascalCase` | `ComputeScalarFlux()` |
| Accessors and mutators (optional) | `snake_case` | `num_groups()`, `set_num_groups()` |
| Variables and parameters | `snake_case` | `num_groups` |
| Class data members | `snake_case_` | `angular_flux_` |
| Struct data members | `snake_case` | `weights` |
| Constants (`constexpr`, including local ones, and namespace- or class-scope `const`) | `kPascalCase` | `kMaxOrder` |
| Enumerators | `PascalCase` | `BoundaryCondition::Vacuum` |
| Local non-`constexpr` `const` variables | `snake_case` | `const int num_dofs = ...` |
| Namespaces | `snake_case` | `kingfisher` |
| Macros | `KINGFISHER_UPPER_CASE` | `KINGFISHER_USE_MPI` |
| Files | `snake_case.cc`, `snake_case.h` | `sweep_operator.cc` |
| Test files | `snake_case_test.cc` | `sweep_operator_test.cc` |

Always declare enums as `enum class`, never as a plain `enum`. A scoped enum's enumerators don't leak into the enclosing scope and don't convert implicitly to `int`. Because every enumerator is written after its type, as in `BoundaryCondition::Vacuum`, it doesn't need Google's `k` prefix. This rule covers the enums Kingfisher declares. Using plain enums from MFEM or other libraries is fine. clang-tidy 18 can't check this rule, so reviewers do.

## Files and headers

Code is grouped into one subdirectory per subsystem, for example `angular/`, `spatial/`, `transport/`, `materials/`, and `io/`. Public headers, sources, and tests use the same subdirectories:

```text
include/kingfisher/angular/quadrature.h
src/angular/quadrature.cc
tests/angular/quadrature_test.cc
```

- Put a subsystem's data structs in the same directory as the free functions that work on them. Don't divide code by kind, such as a `data/` directory and a `kernels/` directory, and don't add a catch-all `utils/` directory.
- Create a subdirectory when its first file is added, not ahead of time. `src/main.cc` stays at the top of `src/`.
- Sources are `.cc` files in `src/`. Public headers are `.h` files in `include/kingfisher/`, and are included with their full path from there, as in `#include "kingfisher/angular/quadrature.h"`.
- Header guards are `KINGFISHER_<PATH>_<FILE>_H_`, where `<PATH>` is the path under `include/kingfisher/`. For example, `include/kingfisher/angular/quadrature.h` uses `KINGFISHER_ANGULAR_QUADRATURE_H_`.
- Includes are ordered as in the Google guide: the related header, C system headers, the C++ standard library, other libraries (MFEM, fmt, rapidyaml), then Kingfisher headers. clang-format sorts within each group.

## Namespaces

All Kingfisher code is in the single namespace `kingfisher`, as all of MFEM is in `mfem`. Subdirectories organize files and are not mirrored by sub-namespaces.

- Put helpers that are used in only one `.cc` file in an anonymous namespace in that file.
- Add a sub-namespace only when a group of free functions has generic names, such as `Initialize()` or `Apply()`, that would otherwise need a prefix to be clear. Functions that take different struct types can share a name through overloading and don't need one.

## Data-oriented design

Kingfisher's hot loops (angular sweeps, element assembly, source and group iterations) touch large amounts of data on every pass, so how the data is laid out in memory matters more than how it is wrapped in classes. Design the data first, then write the functions that transform it.

**Store hot data as structure of arrays.** Prefer one contiguous buffer per quantity over a container of objects:

```cpp
// Avoid: each ordinate is a separate object, and a loop over one field
// strides over the others.
struct Ordinate {
  double mu, eta, xi, weight;
};
std::vector<Ordinate> ordinates;

// Prefer: each field is contiguous, and loops read only what they need.
struct AngularQuadrature {
  mfem::Vector mu, eta, xi, weights;  // one entry per ordinate
};
```

Flat buffers indexed by group, angle, and element (with the layout documented next to the buffer) are preferred over nested containers. Containers of pointers, such as `std::vector<std::unique_ptr<T>>`, are for things that are few and rarely iterated, not for per-element or per-angle data.

**Keep hot loops free of overhead.** Inside angular sweeps, assembly loops, and iteration loops:

- Don't allocate. Allocate workspaces (including temporary `mfem::Vector` and `mfem::DenseMatrix` objects) once, outside the loop, and reuse them.
- Don't make virtual calls or `std::function` calls per element or per angle. Choose the implementation once, outside the loop.
- Don't copy large objects by accident. Pass by reference or `std::span`.

**Follow memory order.** The innermost loop should run over the fastest-varying index of the data it reads. Batch work, for example all elements for one ordinate, instead of processing one item at a time through an object interface.

**Separate data from behavior.** Plain structs and free functions that act on them are preferred over deep class hierarchies when the data is processed in bulk.

**Stay device-ready.** Use MFEM's memory model (`Read()`, `Write()`, `ReadWrite()`, `mfem::forall`) for bulk data, so that GPU execution remains possible later.

## Documentation comments

Public classes, functions, and their parameters have Doxygen comments, which become the [C++ API](../api/index.md) reference. Use `///` comments:

```cpp
/// Computes the scalar flux by integrating the angular flux over all
/// ordinates.
///
/// @param quadrature The angular quadrature set.
/// @param angular_flux Angular flux, indexed by ordinate then degree of freedom.
/// @param[out] scalar_flux Scalar flux, one entry per degree of freedom.
void ComputeScalarFlux(const AngularQuadrature& quadrature,
                       const mfem::Vector& angular_flux,
                       mfem::Vector& scalar_flux);
```

Cite the method's source with a reference to `docs/source/references.bib` in the theory pages, and in a comment next to the implementation.

## Python

Kingfisher will have a Python API for generating YAML input files. Python code follows [PEP 8](https://peps.python.org/pep-0008/), and in addition:

- Public functions and classes have [numpydoc](https://numpydoc.readthedocs.io/en/latest/format.html) docstrings and type hints.
- Filesystem paths use `pathlib.Path`.
- Setters validate their inputs and raise errors with clear messages.
- YAML is written through a YAML library, never by string formatting.
- New functions and classes have [pytest](https://docs.pytest.org) tests.

A formatter and linter (such as [ruff](https://docs.astral.sh/ruff/)) will be added to CI along with the Python code.
