# IMP-012 Extract CSV Source Adapter

## Problem

CSV parsing logic (`load_csv_catalog`, `load_csv_series_streaming`, `read_csv_rows`, column inference helpers) lives in `core.hpp` interleaved with SQLite, JSON, and expression code. The architecture specifies that source adapters should be separate and format-agnostic at their public interface.

## Why It Matters

Isolate CSV-specific parsing so that format changes (new CSV features, edge cases) do not risk affecting other code. Creates a clear adapter boundary that can be tested separately.

## Scope

- `src/io/csv/catalog.hpp` — `load_csv_catalog` and column inference.
- `src/io/csv/load.hpp` — `load_csv_series_streaming`, `read_csv_rows`, `downsample_series`.
- Public API accepts file path and column metadata, returns normalized series data.

## Out of Scope

- SQLite adapter code (tracked in IMP-013).
- Expression engine code (tracked in IMP-014).
- JSON serialization or project persistence.
- UI rendering or ImGui backend.

## Constraints and Dependencies

- May depend on `src/model/` types (SeriesRequest, ColumnInfo, LoadOutcome, etc.).
- Must NOT depend on `src/io/sqlite/`, `src/expr/`, `src/persist/`, or Dear ImGui.
- The core catalog loading function may remain inline or move to a `.cpp` file.
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

- `load_csv_catalog` and `load_csv_series_streaming` compile and pass without SQLite, Lua, or JSON headers in scope.
- CSV tests run independently of SQLite/Lua test suites.
- No `#include` of non-model headers from any file under `src/io/csv/`.

## Traceability

- Architecture: [02-architecture/layer.md](../../02-architecture/layer.md) (Component: Source Adapters)
- Implementation: [03-implementation/layer.md](../../03-implementation/layer.md) (Proposed structure: `src/io/csv/`)