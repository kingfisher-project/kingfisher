# Testing

Unit tests use [GoogleTest](https://google.github.io/googletest/), which CPM downloads along with the other dependencies.

## Running the tests

Build and run every test with either development workflow:

```console
$ cmake --workflow --preset debug
$ cmake --workflow --preset dev
```

After the first build, rebuild and rerun without reconfiguring:

```console
$ cmake --build --preset dev && ctest --preset dev
```

To run only some tests, filter by name with a regular expression:

```console
$ ctest --preset dev -R AlwaysTrue
```

## What must be tested

Every pull request that changes behavior must include tests. Reviewers, including the automatic Claude review, treat missing tests as a blocking issue.

- **New code:** every new public function, class, and method has GoogleTest coverage in `tests/`. Name the file after the code it tests, for example `tests/sweep_operator_test.cc` for `src/sweep_operator.cc`. New test files are picked up automatically.
- **Bug fixes:** add a regression test that fails without the fix.
- **Numerical methods:** add verification tests against an analytic or manufactured solution. Where theory gives a convergence rate, check the observed rate over a sequence of refinements, not just the error on one mesh.
- **Edge cases and errors:** test boundary cases (void regions, vacuum and reflective boundaries, a single group or element) and check that invalid input fails with a clear error.

Tests should be able to fail. Avoid tests that only check that code runs, and choose tolerances tight enough to catch a real error. Read test data through the `TEST_DIR` definition rather than a hard-coded path, and don't rely on global state, so that tests keep working once they run under MPI.
