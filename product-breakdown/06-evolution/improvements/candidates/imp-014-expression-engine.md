# IMP-014 Extract Expression Engine

## Problem

The Lua-based expression engine (`evaluate_expression`, `sol::state` setup, Lua usertype bindings for SeriesData, series arithmetic functions, interpolation) lives in `core.hpp` alongside model types and IO code. Expression evaluation is a domain with its own dependencies (sol2, Lua) and its own risk profile (script sandboxing, error handling).

## Why It Matters

Isolate the Lua/sol2 dependency so model, IO, and persistence code do not need to link against Lua. Enables testing the expression engine independently and contains script sandboxing to a single module.

## Scope

- `src/expr/evaluate.hpp` — `evaluate_expression`, Lua state setup, usertype bindings.
- `src/expr/arithmetic.hpp` — Series arithmetic functions (`series_add`, `series_mul`, etc.) and interpolation (`interpolate_value`, `remap_to_grid`).
- `src/expr/registry.hpp` — Narrow API: accepts named series references, returns derived series. No access to model internals.

## Out of Scope

- Source adapter code (CSV, SQLite — tracked in IMP-012, IMP-013).
- JSON serialization or project persistence.
- UI rendering or ImGui backend.
- Changing the Lua sandboxing model or expression language semantics.

## Constraints and Dependencies

- May depend on `src/model/` types (SeriesData, SeriesRegistry) and `sol2`/Lua.
- Must NOT depend on `src/io/`, `src/persist/`, `src/app/`, or Dear ImGui.
- Public API must be a narrow interface: input = `SeriesRegistry` + expression string, output = `SeriesData`.
- No direct access to `AppState` or any UI type.
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

- `evaluate_expression` compiles and works without CSV, SQLite, JSON, or ImGui headers.
- Lua binding tests pass in isolation.
- The expression engine's public API accepts only `SeriesRegistry + string` and returns `SeriesData`.
- No `#include` of non-model headers from any file under `src/expr/`.

## Traceability

- Architecture: [02-architecture/layer.md](../../02-architecture/layer.md) (Component: Expression Engine)
- Implementation: [03-implementation/layer.md](../../03-implementation/layer.md) (Proposed structure: `src/expr/`)