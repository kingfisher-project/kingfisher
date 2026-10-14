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
