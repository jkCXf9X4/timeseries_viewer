# IMP-011 Extract Canonical Model Types

## Problem

`src/timeseries_viewer/core.hpp` is a 1270-line monolithic header. The first ~500 lines contain all data-structure definitions (SeriesData, ColumnInfo, SourceCatalog, TreeNode, SeriesRegistry, config structs, equality operators, utility functions like `trim`, `split_csv_line`, `parse_double`, etc.). These model types are consumed by every other module but have no dependency on source format, Lua, or JSON. Keeping them in a single giant header forces unnecessary recompilation and obscures the type hierarchy.

## Why It Matters

Extract the pure data-structure layer so that model types can be included without pulling in CSV, SQLite, Lua, or JSON dependencies. Reduces compilation coupling and makes the type hierarchy explicit.

## Scope

- `src/model/types.hpp` — Core data structures (SeriesData, ColumnInfo, SourceCatalog, TreeNode, etc.)
- `src/model/registry.hpp` — SeriesRegistry (key-value container)
- `src/model/config.hpp` — Config types (PlotSeriesConfig, PlotTabConfig, WorkspaceConfig, etc.)
- `src/model/equality.hpp` — Equality operators
- `src/model/utility.hpp` — Pure utility functions (trim, split_csv_line, parse_double, parse_scalar, etc.)
- `core.hpp` becomes a forwarding header or is removed entirely.

## Out of Scope

- Source-format-specific code (CSV, SQLite parsing stays out).
- Lua/sol2 expression evaluation code.
- JSON serialization and project persistence.

## Constraints and Dependencies

- No dependency on `nlohmann_json`, `SQLite3`, `sol2`, or Lua headers.
- Must remain C++23 standard library only.
- Header-only types (no .cpp needed for the model layer).
- Must include `#pragma once`.
- All existing `#include` paths from other files must be updated.
- Must respect component boundaries defined in `product-breakdown/02-architecture/layer.md`.

## Prevention Rules

- KM-002: Do not ignore architecture constraints.
- KM-004: No parallel structures — consolidate, do not duplicate.
- KM-007: Clean up all stale references to the old include paths.
- KM-008: No orphaned artifacts — remove or redirect old files.
- PAT-001: Surgical changes — extract without altering behavior or public interfaces.

## Progress State

`Backlog`

## Acceptance Criteria

- All model types compile without third-party library headers.
- `timeseries_core` tests pass after include-path migration.
- No CSV, SQLite, Lua, JSON, or ImGui headers are reachable from any file under `src/model/`.

## Traceability

- Architecture: [02-architecture/layer.md](../../02-architecture/layer.md) (Component: Variable Registry, Model)
- Implementation: [03-implementation/layer.md](../../03-implementation/layer.md) (Proposed structure: `src/model/`)