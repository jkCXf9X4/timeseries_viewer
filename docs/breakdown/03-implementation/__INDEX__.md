---
title: Implementation
summary: Where the code lives and what is in it
---

# Implementation

## Contents

<!-- pb:index:start -->
<!-- GENERATED FILE — do not edit. Regenerate with `pb registers`. -->
- **INFO-027** [Build approach](build-approach.md) — The codebase is organized around the same separation as the architecture: shell, import, data, expressions, persistence, tests
- **INFO-028** [App and UI code structure](app-shell-structure.md) — Source layout for the app shell, window and tab ownership, parameter selection, and the fixed-panel UI
- **INFO-029** [Data and I/O code structure](data-and-io-structure.md) — Source layout for CSV and SQLite import, the canonical model, expressions, persistence, and tests
- **INFO-030** [App and UI interfaces](app-and-ui-interfaces.md) — One-plot-per-tab ownership, fixed side panels tied to the active plot, and ready-to-render series consumption
- **INFO-031** [Data, expression, and persistence interfaces](data-and-expression-interfaces.md) — Normalized import results, stable lookup, serializable bindings, a narrow expression API, and model round-tripping
- **INFO-032** [Configuration and environment](configuration-and-environment.md) — C++23, CMake, vcpkg as the dependency source of record, and a local vcpkg toolchain on a desktop graphics stack
- **INFO-033** [Implementation constraints](constraints.md) — Stay focused on the product scope, avoid unneeded abstractions, and keep source logic behind adapters
- **INFO-034** [Implementation risks](risks.md) — Model-layer sprawl, expression helper coupling, and backward-readable project schemas
<!-- pb:index:end -->
