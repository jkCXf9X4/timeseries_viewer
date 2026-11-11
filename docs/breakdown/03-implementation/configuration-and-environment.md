---
id: INFO-032
type: info
title: Configuration and environment
summary: C++23, CMake, vcpkg as the dependency source of record, and a local vcpkg toolchain on a desktop graphics stack
date: 2026-11-11
status: current
---

# Configuration and environment

The implementation targets a fixed toolchain.

- **Language:** C++23 is the target language level.
- **Build system:** CMake is the build system.
- **Dependencies:** vcpkg is the dependency source of record.
- **Environment:** the Linux build environment assumes a local vcpkg toolchain and a desktop graphics stack.