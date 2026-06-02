# Install and Build

## Prerequisites

- Linux desktop environment
- CMake 3.22 or newer
- Ninja
- C++ compiler with C++23 support
- `vcpkg` checkout available through `VCPKG_ROOT`
- System development packages needed by GLFW on Linux

On Ubuntu-like systems, install the native GUI build prerequisites first:

```sh
sudo apt-get install libxinerama-dev libxcursor-dev xorg-dev libglu1-mesa-dev pkg-config
```

## Configure

From the repository root:

```sh
cmake --preset=vcpkg
```

The preset expects `VCPKG_ROOT` to point at the local vcpkg checkout.

## Build

```sh
cmake --build build
```

This builds both the desktop application and the Catch2 test target.

## Test

```sh
ctest --test-dir build --output-on-failure
```

## Output

- Application binary: `build/timeseries_viewer`
- Test binary: `build/timeseries_viewer_tests`

## Product Context

Build-time decisions and implementation structure are recorded in:

- [03 implementation](../product-breakdown/03-implementation/layer.md)
- [02 architecture](../product-breakdown/02-architecture/layer.md)
