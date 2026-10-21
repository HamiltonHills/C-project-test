# C-project-test

A small C project for testing
[pipeline-actions](https://github.com/HamiltonHills/pipeline-actions)'
`c-pipeline` against a real SonarQube and JFrog Artifactory.

`kvget` reads a `key = value` config file:

```console
$ kvget app.conf port
8080
$ kvget app.conf        # number of keys
2
```

Exit codes: 0 found, 1 key missing, 2 usage or parse error.

## Build

```bash
cmake -S . -B build -G Ninja && cmake --build build && ctest --test-dir build
```

## CI

See the header of [`.github/workflows/ci.yml`](.github/workflows/ci.yml) for
the secrets and variables it needs. Push a `v*` tag to cut a release.
