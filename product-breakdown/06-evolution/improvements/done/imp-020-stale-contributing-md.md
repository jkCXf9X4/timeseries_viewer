# IMP-020 Update docs/contributing.md for new module structure

## Problem

`docs/contributing.md` describes a monolithic project layout (`src/timeseries_viewer/core.hpp` as the core data model, only `core_tests.cpp`). After IMP-011 through IMP-018, the codebase is restructured into eight module directories under `src/` and three test files. New contributors reading the contributing guide will look for files in the wrong places.

## Why It Matters

The contributing guide is the first document a new contributor reads. Stale file paths and missing module directories create confusion, slow onboarding, and undermine confidence in the documentation.

## Scope

- Edit `docs/contributing.md`, specifically the "Project Layout" section.
- Replace the old flat layout listing with an accurate description of the current module structure:
  - `src/model/` — Core data structures and config types
  - `src/io/csv/` — CSV source adapter
  - `src/io/sqlite/` — SQLite source adapter
  - `src/expr/` — Expression evaluation engine
  - `src/persist/` — Project persistence layer
  - `src/ui/` — Rendering and GUI abstractions
  - `src/ui/backend/` — ImGui backend glue
  - `src/app/` — Application entry point and shell
- List all three test files: `core_tests.cpp`, `app_model_tests.cpp`, `gui_render_tests.cpp`.

## Out of Scope

- Do not change other sections of `docs/contributing.md`.
- Do not modify other documentation files.
- Do not change code or test files.

## Constraints and Dependencies

- Follow KM-005: no parallel structures — the description must match the actual current code layout exactly.
- Must check actual `src/` directory structure to ensure correctness.

## Prevention Rules

KM-005, KM-007, PAT-001

## Progress State

`Backlog`

## Acceptance Criteria

- `docs/contributing.md` accurately describes the current `src/` directory layout.
- All eight module directories are mentioned (`model`, `io/csv`, `io/sqlite`, `expr`, `persist`, `ui`, `ui/backend`, `app`).
- All three test files are listed.
- `core.hpp` is described as a forwarding header (not the monolithic core).
- No stale file paths remain in the "Project Layout" section.

## Traceability

- Source: `docs/contributing.md`
- Architecture: `product-breakdown/03-implementation/layer.md` (Proposed code structure now matches actual layout)
- Evidence: `product-breakdown/06-evolution/reviews/doc-review-2026-06-04.md` (Finding 2, HIGH)