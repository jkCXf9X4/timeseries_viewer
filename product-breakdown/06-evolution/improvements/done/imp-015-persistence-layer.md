# IMP-015 Extract Persistence Layer

## Problem

JSON serialization (`to_json`/`from_json` for all config types, `save_project`/`load_project`, path helpers) lives in `core.hpp`. Project persistence has its own serialization concerns (schema evolution, backward compatibility, path resolution) that are orthogonal to model types and IO.

## Why It Matters

Isolate the `nlohmann_json` dependency and project schema logic. Keeps schema versioning and backward-compatibility logic in one place so changes to the project format do not risk affecting model definitions or source adapters.

## Scope

- `src/persist/save.hpp` — `save_project`, `project_relative_path`.
- `src/persist/load.hpp` — `load_project`, `resolve_project_path`, legacy format migration.
- `src/persist/json_helpers.hpp` — JSON conversion functions for config types.

## Out of Scope

- Source adapter code (CSV, SQLite — tracked in IMP-012, IMP-013).
- Expression engine code (tracked in IMP-014).
- UI rendering or ImGui backend.
- Changing the project JSON schema or adding new schema versions.

## Constraints and Dependencies

- May depend on `src/model/` types and `nlohmann_json`.
- Must NOT depend on `src/io/`, `src/expr/`, or Dear ImGui.
- Schema changes must remain backward-readable.
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

- Project round-trip (save + load) works without CSV, SQLite, Lua, or ImGui headers.
- Legacy project format (pre-schema, flat views) still loads correctly.
- No `#include` of non-model headers from any file under `src/persist/`.
- All `to_json`/`from_json` specializations for config types are contained in this module.

## Traceability

- Architecture: [02-architecture/layer.md](../../02-architecture/layer.md) (Component: Persistence Layer)
- Implementation: [03-implementation/layer.md](../../03-implementation/layer.md) (Proposed structure: `src/persist/`)