# IMP-022 Add docstrings to public API headers

## Problem

All nine public API headers in the codebase contain zero docstrings on their types, functions, or parameters. A new contributor cannot understand the purpose, preconditions, or side effects of any public function without reading the implementation. This is a significant onboarding and maintenance gap.

## Why It Matters

Public API surfaces should be self-documenting. The absence of docstrings forces every contributor to read implementation code to understand the API contract, slowing development and increasing the risk of misuse.

## Scope

- Add `///` docstrings to ALL public types and function declarations across these nine headers:
  - `src/model/types.hpp`
  - `src/model/config.hpp`
  - `src/io/csv/csv.hpp`
  - `src/io/sqlite/sqlite.hpp`
  - `src/expr/expr.hpp`
  - `src/persist/persist.hpp`
  - `src/ui/render.hpp`
  - `src/ui/backend/imgui_backend.hpp`
  - `src/timeseries_viewer/app_model.hpp`

## Out of Scope

- Do not change function signatures or implementations.
- Do not add docstrings to private/internal helpers or implementation files.
- Do not add docstrings to third-party or vendored code.
- Do not add docstrings to test files.
- Do not add docstrings to CMakeLists.txt or other build files.

## Constraints and Dependencies

- Follow PAT-001: surgical changes — docstrings must be added without altering behavior, signatures, or interfaces.
- Docstrings should cover: purpose, parameters (for functions), return value, preconditions or side effects.
- Use `///` (triple-slash) format to avoid breaking any existing comment conventions.
- Code must compile without warnings after changes.

## Prevention Rules

PAT-001, KM-005

## Progress State

`Backlog`

## Acceptance Criteria

- Every public type in the nine headers has a docstring describing its purpose.
- Every public function declaration has a docstring describing its purpose, parameters, and return value.
- No function signature is changed.
- Code compiles without warnings or errors.
- No new warnings from documentation linters (if active).

## Traceability

- Headers: `src/model/types.hpp`, `src/model/config.hpp`, `src/io/csv/csv.hpp`, `src/io/sqlite/sqlite.hpp`, `src/expr/expr.hpp`, `src/persist/persist.hpp`, `src/ui/render.hpp`, `src/ui/backend/imgui_backend.hpp`, `src/timeseries_viewer/app_model.hpp`
- Evidence: `product-breakdown/06-evolution/reviews/doc-review-2026-06-04.md` (Finding 4, HIGH)