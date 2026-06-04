# IMP-016 Restructure App Model into Domain Sub-modules

## Problem

`src/app/app_model.cpp` (1189 lines) is a monolithic implementation file containing workspace management, cache logic, source lifecycle, parameter binding, derived series, project load/save interaction, and live reload — all in a single translation unit. The architecture plan specifies `src/app/windows/` and `src/app/selection/` as separate sub-modules.

## Why It Matters

The app model has several distinct responsibilities (window/tab management, parameter binding, cache management, source lifecycle, live reload) that are currently tangled. Splitting them improves testability and makes each sub-domain's invariants explicit.

## Scope

- `src/app/windows/workspace.hpp/.cpp` — `ensure_workspace_defaults`, `add_window`, `remove_window`, `add_tab`, `select_series`, `remove_series`, `active_window`, `active_tab`.
- `src/app/selection/binding.hpp/.cpp` — `list_bindable_parameters`, `bindable_parameter_tree`, `parameter_is_selected`, `set_parameter_selected`, `remove_parameter_series`, series matching helpers.
- `src/app/cache.hpp/.cpp` — `rebuild_cache`, `rebuild_cache_metadata`, `ensure_tab_data`, raw series cache logic.
- `src/app/source.hpp/.cpp` — `open_source`, `reload_sources`, `poll_live_reload`, source stamp tracking.
- `src/app/project.hpp/.cpp` — `save_project_file`, `load_project_file`, `populate_legacy_bindings`.
- `src/app/app_model.hpp` becomes a unified public header that includes sub-module interfaces.

## Out of Scope

- UI rendering or ImGui backend (tracked in IMP-017, IMP-018).
- Source adapter internals (tracked in IMP-012, IMP-013).
- Expression engine internals (tracked in IMP-014).
- Persistence layer internals (tracked in IMP-015).

## Constraints and Dependencies

- Sub-modules may depend on `src/model/`, `src/io/`, `src/expr/`, `src/persist/`.
- Must NOT depend on `src/ui/` or Dear ImGui.
- Shared types (AppState, OpenSource, BindableParameter, SeriesBindingKey) must remain in a single header (`app_model.hpp` or `src/app/state.hpp`).
- Anonymous namespace helpers in `app_model.cpp` must be evaluated per sub-module — some belong to specific sub-domains, some are shared utilities.
- All include paths must be updated (KM-007).
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

- `app_model.cpp` is removed (or reduced to sub-module forwarding includes).
- All tests pass with no change in behavior.
- Window/tab management, parameter binding, cache, source lifecycle, and project persistence each compile and can be unit tested independently.
- No ImGui or UI headers are reachable from any file under `src/app/`.

## Traceability

- Architecture: [02-architecture/layer.md](../../02-architecture/layer.md) (Components: Window Manager, Parameter Binding Menu, Refresh Scheduler)
- Implementation: [03-implementation/layer.md](../../03-implementation/layer.md) (Proposed structure: `src/app/windows/`, `src/app/selection/`)