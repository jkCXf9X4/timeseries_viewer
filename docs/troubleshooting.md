# Troubleshooting

## Build Fails On GLFW

If `cmake --preset=vcpkg` fails while building `glfw3`, install the Linux desktop development packages required by GLFW:

```sh
sudo apt-get install libxinerama-dev libxcursor-dev xorg-dev libglu1-mesa-dev pkg-config
```

## Build Fails To Find Vcpkg

Check that `VCPKG_ROOT` points to the local vcpkg checkout before configuring.

## Source Does Not Plot

- Confirm the selected column is numeric.
- Confirm the time column was inferred or selected correctly.
- Confirm the source file still exists and has not been renamed.

## Project Does Not Restore Cleanly

- Check whether source paths changed between machines.
- Reopen the project and repair missing paths if the filesystem layout differs.
- Confirm the source table still exists in the SQLite database.

## Live Data Does Not Refresh

- Check that `Live` is enabled.
- Confirm the source file timestamp is changing.
- Wait for the next polling interval.

## Product Context

Failure modes and operational risks are documented in:

- [05 operation](../product-breakdown/05-operation/layer.md)
