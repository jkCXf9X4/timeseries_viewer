# IMP-018 Extract ImGui Backend from main.cpp

## Problem

`main.cpp` (452 lines) contains the entire `ImGuiBackend` struct (~320 lines) alongside the main entry point and event loop. The ImGuiBackend is the concrete `Ui` type that bridges the templated UI functions to Dear ImGui API calls. Keeping it in `main.cpp` prevents reuse and makes the file a dumping ground.

## Why It Matters

Separates the platform-specific backend (GLFW + ImGui + ImPlot) from the application entry point. Makes the `Ui` interface reusable and keeps `main.cpp` focused on initialization and the frame loop.

## Scope

- `src/ui/backend/imgui_backend.hpp` — ImGuiBackend struct declaration extracted from `main.cpp`.
- `src/ui/backend/imgui_backend.cpp` — ImGuiBackend struct implementation extracted from `main.cpp`.
- `main.cpp` is shortened to initialization + frame loop only (target: under 150 lines).

## Out of Scope

- UI panel rendering or template functions (tracked in IMP-017).
- Application model changes (tracked in IMP-016).
- Source adapter, expression engine, or persistence layer changes.
- Changing the template interface contract between the UI backend and the rendering templates.

## Constraints and Dependencies

- May depend on Dear ImGui, ImPlot, GLFW, glad, nfd.
- Must NOT depend on `src/model/`, `src/io/`, `src/expr/`, or `src/persist/`.
- The template interface contract (methods called by `app_ui.hpp` templates) must remain stable.
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

- Application compiles and runs identically after extraction.
- `main.cpp` is under 150 lines.
- ImGuiBackend compiles independently of `src/app/`, `src/model/`, `src/io/`, `src/expr/`, and `src/persist/`.
- No ImGui, GLFW, or platform headers are included from `main.cpp` after extraction.

## Traceability

- Architecture: [02-architecture/layer.md](../../02-architecture/layer.md) (Component: UI Shell)
- Implementation: [03-implementation/layer.md](../../03-implementation/layer.md) (Proposed structure: `src/ui/`)