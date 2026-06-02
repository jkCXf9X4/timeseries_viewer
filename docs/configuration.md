# Configuration

## Runtime Behavior

The application currently has a small fixed configuration surface:

- no runtime configuration file
- no command-line options
- live polling enabled from the UI only
- project state stored in JSON files

## Live Refresh

- Polling interval is currently fixed at roughly one second.
- File change detection is based on file metadata changes.
- Source reloads happen when the watched file changes.

## Paths

- Source files are referenced by local filesystem path.
- Project files store source paths and may use relative paths when saved.
- Moving a project file to another machine may require manual path repair.

## Environment

- `VCPKG_ROOT` must point at the local vcpkg checkout when configuring the build.
- The Linux desktop build still depends on the system GUI libraries required by GLFW.

## Product Context

The stable product decisions that shape configuration are in:

- [02 architecture](../product-breakdown/02-architecture/layer.md)
- [05 operation](../product-breakdown/05-operation/layer.md)
