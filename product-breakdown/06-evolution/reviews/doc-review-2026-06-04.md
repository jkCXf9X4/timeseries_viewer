# Documentation Review - 2026-06-04

## Summary

- Files reviewed: 35 (13 product-breakdown, 12 docs, 10 code headers/sources)
- Findings: 11 (4 high, 4 medium, 3 low)
- Accuracy issues: 3
- Stale references: 3
- Completeness gaps: 4
- Consistency issues: 1

## Findings

### [HIGH] IMP-011 through IMP-018 listed as Backlog in improvements/README.md but are actually Done

- **File**: `product-breakdown/06-evolution/improvements/README.md`
- **Type**: stale-ref
- **Evidence**: Lines 19-26 list IMP-011 through IMP-018 under "Backlog":
  ```
  ## Backlog
  - [IMP-011 Extract canonical model types](candidates/imp-011-model-types.md)
  - [IMP-012 Extract CSV source adapter](candidates/imp-012-csv-adapter.md)
  - ...
  - [IMP-018 Extract ImGui backend from main.cpp](candidates/imp-018-imgui-backend.md)
  ```
  However, all eight files exist at `product-breakdown/06-evolution/improvements/done/imp-01[1-8]-*.md`, not in `candidates/`.
- **Issue**: The index file directs readers to `candidates/` paths that do not exist. The actual files are in `done/`. This is a stale reference that will mislead anyone looking for these improvement records.
- **Recommendation**: Move the eight IMP entries from the "Backlog" section to the "Done" section in `improvements/README.md`, updating the paths from `candidates/` to `done/`.

### [HIGH] Stale file paths in docs/contributing.md

- **File**: `docs/contributing.md`
- **Type**: stale-ref
- **Evidence**: Lines 12-15:
  ```
  ## Project Layout
  - `src/timeseries_viewer/core.hpp`: core data model, import, expression, and persistence helpers
  - `src/app/main.cpp`: GUI shell and application flow
  - `src/third_party/imgui/backends/`: vendored Dear ImGui backend glue
  - `tests/core_tests.cpp`: Catch2 coverage for the verification plan
  ```
- **Issue**: After IMP-011 through IMP-018, the code was restructured into `src/model/`, `src/io/`, `src/expr/`, `src/persist/`, `src/ui/`, `src/app/`. The `core.hpp` is now a thin forwarding header (13 lines). The description "core data model, import, expression, and persistence helpers" no longer matches reality. The test suite now has three files (`core_tests.cpp`, `app_model_tests.cpp`, `gui_render_tests.cpp`), not just `core_tests.cpp`. The `src/third_party/imgui/backends/` path is correct but incomplete — the new `src/ui/backend/` directory is not mentioned.
- **Recommendation**: Update the project layout section to reflect the current module structure: `src/model/`, `src/io/csv/`, `src/io/sqlite/`, `src/expr/`, `src/persist/`, `src/ui/`, `src/app/`, and list all three test files.

### [HIGH] Missing PB-012 in decision-log.md

- **File**: `product-breakdown/decision-log.md`
- **Type**: completeness
- **Evidence**: The decision log table (lines 6-18) lists PB-001 through PB-011. However, `product-breakdown/02-architecture/layer.md` line 87 defines PB-012:
  ```
  - PB-012: The parameter browser is fixed to the left side of the app, the plot inspector is fixed to the right side, and both panels follow the active plot.
  ```
- **Issue**: PB-012 is recorded in the architecture layer but missing from the centralized decision log. This breaks the traceability contract that the decision log is the "root index of stable decisions."
- **Recommendation**: Add PB-012 to the decision log table with layer "02-architecture" and status "Recorded".

### [HIGH] Missing docstrings on all public API headers

- **File**: Multiple header files
- **Type**: completeness
- **Evidence**: The following public API headers contain zero docstrings on their types, functions, or parameters:
  - `src/model/types.hpp` (74 lines, 10+ public types)
  - `src/model/config.hpp` (55 lines, 6 public types)
  - `src/io/csv/csv.hpp` (122 lines, 3 public functions)
  - `src/io/sqlite/sqlite.hpp` (220 lines, 6 public functions)
  - `src/expr/expr.hpp` (183 lines, 10+ public functions)
  - `src/persist/persist.hpp` (165 lines, 8 public functions)
  - `src/ui/render.hpp` (24 lines, 1 public function template)
  - `src/ui/backend/imgui_backend.hpp` (91 lines, 1 struct with 30+ methods)
  - `src/timeseries_viewer/app_model.hpp` (106 lines, 2 structs, 1 class, 30+ function declarations)
- **Issue**: None of the public API surfaces have any documentation comments. A new contributor cannot understand the purpose, preconditions, or postconditions of any function without reading the implementation. This is a significant onboarding and maintenance gap.
- **Recommendation**: Add docstrings to all public types and function declarations. At minimum, document: purpose, parameters, return value, and any preconditions or side effects.

### [MEDIUM] improvement-backlog.md does not reflect IMP-011 through IMP-018 as Done

- **File**: `product-breakdown/06-evolution/improvement-backlog.md`
- **Type**: completeness
- **Evidence**: The backlog table (lines 14-25) lists IMP-001 through IMP-010. IMP-011 through IMP-018 are not present at all, even though they have been implemented and their records exist in `improvements/done/`.
- **Issue**: The improvement backlog is the canonical tracker for all product improvements. Omitting eight completed improvements means the backlog does not provide a complete picture of what has been done.
- **Recommendation**: Add IMP-011 through IMP-018 to the table with status "Done" and links to their done files.

### [MEDIUM] docs/verification.md does not mention app_model_tests or gui_render_tests

- **File**: `docs/verification.md`
- **Type**: completeness
- **Evidence**: Lines 13-19 list coverage areas but do not mention the test files. The test suite now has three files (`tests/core_tests.cpp`, `tests/app_model_tests.cpp`, `tests/gui_render_tests.cpp`), but the doc only references `tests/fixtures/` (lines 34-35).
- **Issue**: A reader cannot tell from the verification doc that there are separate test files for app model behavior and GUI rendering. The doc also does not mention the `GuiHarness` test support class used in `app_model_tests.cpp`.
- **Recommendation**: Add a "Test Files" section listing all three test files and their focus areas. Mention the GuiHarness support class.

### [MEDIUM] docs/changelog.md is too sparse

- **File**: `docs/changelog.md`
- **Type**: completeness
- **Evidence**: The entire changelog (lines 3-11) is a single "Unreleased" section with a bullet list of initial features. None of the 18 IMP improvements are mentioned.
- **Issue**: There is no record of what changed between versions or which improvements were implemented. This makes it impossible to track the evolution of the product from the changelog alone.
- **Recommendation**: Either expand the changelog to list each IMP as a separate entry, or add a reference to `product-breakdown/06-evolution/improvements/` as the authoritative change record.

### [MEDIUM] Missing file: product-breakdown/06-evolution/undiscovered-improvements.md

- **File**: `product-breakdown/06-evolution/undiscovered-improvements.md`
- **Type**: completeness
- **Evidence**: The file does not exist (confirmed by glob search). It was listed in the review task as a file to read.
- **Issue**: If this file is intended to exist, it is missing. If it is intentionally absent, there should be no reference to it anywhere. (No reference was found in the reviewed files, so this may simply be a planned file that was never created.)
- **Recommendation**: Either create the file with initial content (e.g., a placeholder noting that no undiscovered improvements have been identified yet) or confirm it is intentionally absent and remove it from any future review checklists.

### [LOW] docs/application-plan.md does not reference the new module structure

- **File**: `docs/application-plan.md`
- **Type**: stale-ref
- **Evidence**: The plan document describes the intended architecture and implementation approach. It references `src/timeseries_viewer/core.hpp` implicitly through the architectural description but does not mention the `src/model/`, `src/io/`, `src/expr/`, `src/persist/`, `src/ui/` module structure that was implemented by IMP-011 through IMP-018.
- **Issue**: As a plan document, this is lower severity. However, it is the "source plan" referenced by `product-breakdown/README.md` (line 26) and should reflect the actual implementation structure to remain useful as a reference.
- **Recommendation**: Add a note or section referencing the current module structure, or update the "Architectural Decisions" section to mention the module layout.

### [LOW] traceability-map.md does not include PB-012

- **File**: `product-breakdown/traceability-map.md`
- **Type**: consistency
- **Evidence**: The traceability map (lines 5-15) traces product intent through all layers. PB-012 (sidebar layout decision from 02-architecture) is not represented in any row.
- **Issue**: The sidebar layout (parameter browser left, plot inspector right, both following active plot) is a significant architectural decision that should be traceable through the map.
- **Recommendation**: Add a row for the sidebar layout decision, tracing from product behavior (UI description) through architecture (PB-012) to implementation (UI panel rendering) and verification (sidebar layout tests).

### [LOW] README.md references docs/example.png (file exists, no issue)

- **File**: `README.md`
- **Type**: accuracy
- **Evidence**: Line 13 references `docs/example.png`. The file exists at the expected path.
- **Issue**: No issue — the file exists. This is noted for completeness only.
- **Recommendation**: None.

## Summary by Severity

| Severity | Count | Key Issues |
|----------|-------|------------|
| HIGH     | 4     | Stale IMP index in improvements/README.md, stale paths in contributing.md, missing PB-012 in decision log, missing docstrings on all public API headers |
| MEDIUM   | 4     | improvement-backlog.md missing IMP-011..018, verification.md missing test file references, sparse changelog, missing undiscovered-improvements.md |
| LOW      | 3     | application-plan.md not updated for module structure, traceability-map missing PB-012, README example.png verified OK |

## Files with No Issues Found

- `product-breakdown/README.md` — Accurate, complete, consistent.
- `product-breakdown/00-intent/layer.md` — Stable, no stale references.
- `product-breakdown/01-product/layer.md` — Accurate, matches current scope.
- `product-breakdown/01-product/use-cases.md` — Complete, well-linked.
- `product-breakdown/01-product/ui-description.md` — Detailed, accurate.
- `product-breakdown/02-architecture/layer.md` — Accurate, includes PB-012.
- `product-breakdown/03-implementation/layer.md` — Proposed code structure matches current layout.
- `product-breakdown/04-verification/layer.md` — Accurate verification expectations.
- `product-breakdown/05-operation/layer.md` — Accurate operational requirements.
- `product-breakdown/06-evolution/layer.md` — Accurate evolution direction.
- `docs/README.md` — Clean index, no issues.
- `docs/install-build.md` — Accurate build instructions.
- `docs/usage.md` — Accurate usage guidance.
- `docs/configuration.md` — Accurate configuration description.
- `docs/data-formats.md` — Accurate format descriptions.
- `docs/troubleshooting.md` — Accurate troubleshooting steps.
- `docs/operations.md` — Accurate operational guidance.
- `docs/security.md` — Accurate security notes.
- `CMakeLists.txt` — Accurate build configuration.
- `src/timeseries_viewer/core.hpp` — Correct forwarding header.
- `src/timeseries_viewer/app_model.hpp` — Correct declarations (but missing docstrings — see HIGH finding).
- `src/app/main.cpp` — Correct entry point.
