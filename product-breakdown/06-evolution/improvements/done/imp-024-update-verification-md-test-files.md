# IMP-024 Update docs/verification.md for test files

## Problem

`docs/verification.md` does not mention `app_model_tests.cpp` or `gui_render_tests.cpp`. The test suite has three files, but the document only references `tests/fixtures/`. A reader cannot determine from the verification doc that there are separate test files for app model behavior and GUI rendering.

## Why It Matters

The verification document should accurately describe how the product is tested. Omitting two of the three test files gives an incomplete picture and misleads readers about the scope of verification.

## Scope

- Edit `docs/verification.md` only.
- Add a "Test Files" section that lists all three test files and their focus areas.
- Mention the `GuiHarness` support class used in `app_model_tests.cpp`.

## Out of Scope

- Do not modify actual test files (`tests/core_tests.cpp`, `tests/app_model_tests.cpp`, `tests/gui_render_tests.cpp`).
- Do not change other sections of `docs/verification.md`.
- Do not modify other documentation files.

## Constraints and Dependencies

- Follow KM-005: no parallel structures — descriptions must match actual test file names and focus areas.
- Verify actual test file content briefly to ensure accurate descriptions.

## Prevention Rules

KM-005, KM-007, PAT-001

## Progress State

`Backlog`

## Acceptance Criteria

- `docs/verification.md` lists all three test files.
- Each test file has a brief description of its focus area.
- `GuiHarness` is mentioned as a test support class.
- Existing content in `docs/verification.md` is preserved.

## Traceability

- Source: `docs/verification.md`
- Reference: `tests/core_tests.cpp`, `tests/app_model_tests.cpp`, `tests/gui_render_tests.cpp`
- Evidence: `product-breakdown/06-evolution/reviews/doc-review-2026-06-04.md` (Finding 6, MEDIUM)