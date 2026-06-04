# IMP-017 Isolate UI Rendering from App Model

## Problem

`src/timeseries_viewer/app_ui.hpp` (360 lines) is a header providing template functions for UI rendering. It directly includes and depends on `app_model.hpp` and mutates `AppState` through rendering functions like `render_parameter_panel` and `render_plot_inspector`. The architecture specifies `src/ui/` as the UI layer.

## Why It Matters

Separates rendering concerns from application logic. Enables the UI layer to be tested with a mock backend and makes the interface between app state and rendering explicit.

## Scope

- `src/ui/panels/parameter.hpp` — Parameter panel rendering.
- `src/ui/panels/inspector.hpp` — Plot inspector rendering.
- `src/ui/panels/status.hpp` — Status bar rendering.
- `src/ui/plot/workspace.hpp` — Analysis window and plot rendering.
- `src/ui/render.hpp` — Top-level `render_app` function.
- The `Ui` template parameter pattern is preserved (ImGui backend injection) but organized by panel.

## Out of Scope

- Business logic or data manipulation — rendering functions call `tsv::app::` functions but do not implement workspace or binding logic.
- ImGui backend extraction (tracked in IMP-018).
- Source adapter, expression engine, or persistence layer changes.
- Creating a new rendering abstraction that competes with the existing `app_ui.hpp` — extend from it.

## Constraints and Dependencies

- May depend on `src/app/` (AppState, OpenSource, etc.) and the `Ui` backend interface.
- Must NOT depend on `src/io/`, `src/expr/`, or `src/persist/` directly.
- No data manipulation — rendering functions call `tsv::app::` functions, they do not implement business logic.
- Must not create a parallel rendering abstraction that competes with the existing `app_ui.hpp` — extend from it.
- Must respect component boundaries defined in `product-breakdown/02-architecture/layer.md`.

## Prevention Rules

- KM-002: Do not ignore architecture constraints.
- KM-004: No parallel structures — consolidate, do not duplicate.
- KM-005: Do not introduce unnecessary ownership changes — rendering only reads and displays state.
- KM-007: Clean up all stale references to the old include paths.
- KM-008: No orphaned artifacts — remove or redirect old files.
- PAT-001: Surgical changes — extract without altering behavior or public interfaces.

## Progress State

`Backlog`

## Acceptance Criteria

- UI renders identically after reorganization.
- `app_ui.hpp` is removed (or reduced to forwarding includes).
- Each panel file compiles independently with its minimal set of includes.
- No business logic leaks into rendering functions.

## Traceability

- Architecture: [02-architecture/layer.md](../../02-architecture/layer.md) (Components: UI Shell, Plot Workspace)
- Implementation: [03-implementation/layer.md](../../03-implementation/layer.md) (Proposed structure: `src/ui/`)