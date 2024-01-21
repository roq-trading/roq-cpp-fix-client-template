A template project for creating your own FIX client.

## Prerequisites

> Use `stable` for (the approx. monthly) release build.
> Use `unstable` for the more regularly updated development builds.

### Initialize sub-modules

```bash
git submodule update --init --recursive
```

### Create environment (Mambaforge)

```bash
scripts/create_conda_env.sh unstable debug
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

## Building your own conda package

```bash
scripts/build_conda_package.sh stable
```
