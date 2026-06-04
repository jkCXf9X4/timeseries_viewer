# IMP-013 Extract SQLite Source Adapter

## Problem

SQLite logic (`load_sqlite_catalog`, `load_sqlite_series_targeted`, `fetch_sqlite_tables/columns/rows`) lives in `core.hpp`. Like CSV, it is interleaved with unrelated code including model types, expressions, and JSON helpers.

## Why It Matters

Isolate SQLite-specific database handling behind a clean adapter boundary. SQLite has different error handling (sqlite3_open_v2 lifetime, statement finalization) that should be encapsulated and not leak into other modules.

## Scope

- `src/io/sqlite/catalog.hpp` — `load_sqlite_catalog`, `fetch_sqlite_tables`, `fetch_sqlite_columns`, `read_sqlite_rows`.
- `src/io/sqlite/load.hpp` — `load_sqlite_series_targeted`, `sqlite_quote_identifier`.
- Public API accepts file path and table metadata, returns normalized series data.

## Out of Scope

- CSV adapter code (tracked in IMP-012).
- Expression engine code (tracked in IMP-014).
- JSON serialization or project persistence.
- UI rendering or ImGui backend.

## Constraints and Dependencies

- May depend on `src/model/` types and the `SQLite3` library.
- Must NOT depend on `src/io/csv/`, `src/expr/`, `src/persist/`, or Dear ImGui.
- Database connection lifetime must be encapsulated within the module.
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

- All SQLite loading functions compile and tests pass without CSV, Lua, or JSON headers.
- No `#include` of non-model headers from any file under `src/io/sqlite/`.
- Database connection and statement lifetime is managed entirely within the module.

## Traceability

- Architecture: [02-architecture/layer.md](../../02-architecture/layer.md) (Component: Source Adapters)
- Implementation: [03-implementation/layer.md](../../03-implementation/layer.md) (Proposed structure: `src/io/sqlite/`)