A template project to help you get started with creating your own FIX client using Roq's FIX C++ API.

## Prerequisites

> Use `stable` for (the approx. monthly) release build.
> Use `unstable` for the more regularly updated development builds.

### Initialize sub-modules

```bash
git submodule update --init --recursive
```

### Create development environment

```bash
scripts/create_conda_env unstable debug
```

### Activate environment

```bash
source opt/conda/bin/activate dev
```

## Build the project

> Sometimes you may have to delete CMakeCache.txt if CMake has already cached an incorrect configuration.

```bash
cmake . && make -j4
```
