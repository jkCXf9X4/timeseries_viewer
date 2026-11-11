---
title: Implemented
summary: Improvements closed as done
---

# Implemented

## Contents

<!-- pb:index:start -->
<!-- GENERATED FILE — do not edit. Regenerate with `pb registers`. -->
- **IMP-001** [Richer plot editing controls](IMP-001-rich-plot-editing.md) — Add remove, rebind, visibility, axis, and color editing for existing plot series
- **IMP-002** [Dedicated parameter binding manager](IMP-002-parameter-binding-manager.md) — Make source-to-parameter bindings explicit and rebindable in one binding view
- **IMP-003** [Per-tab expression editor state](IMP-003-per-tab-expression-editor.md) — Scope expression draft text and derived-series editing to the owning tab
- **IMP-005** [GUI integration verification](IMP-005-gui-integration-verification.md) — Exercise the user interface workflow end to end with repeatable integration tests
- **IMP-006** [Large-data scaling strategy](IMP-006-large-data-scaling.md) — Define the dataset size envelope and the implementation path for the in-memory model
- **IMP-007** [Rendered GUI workflow verification](IMP-007-rendered-gui-workflow-verification.md) — Drive the real GUI frame loop with a deterministic C++ harness against the shipped UI path
- **IMP-008** [Variable browser workflow fidelity](IMP-008-variable-browser-workflow-fidelity.md) — Verify the full browse-and-select workflow, not just dotted-name tree construction
- **IMP-009** [Parameter menu discoverability](IMP-009-parameter-menu-discoverability.md) — Verify the explicit user-facing menu or panel that exposes source bindings
- **IMP-010** [Comparison label disambiguation](IMP-010-comparison-label-disambiguation.md) — Keep series labels distinct in mixed-source comparisons across save and reload
- **IMP-011** [Extract canonical model types](IMP-011-model-types.md) — Split the monolithic core header into a pure model layer with no I/O, expression, or persistence dependencies
- **IMP-012** [Extract CSV source adapter](IMP-012-csv-adapter.md) — Isolate CSV parsing behind its own adapter so format changes cannot affect other code
- **IMP-013** [Extract SQLite source adapter](IMP-013-sqlite-adapter.md) — Encapsulate SQLite database handling, including connection and statement lifetime, behind a clean adapter boundary
- **IMP-014** [Extract expression engine](IMP-014-expression-engine.md) — Contain the Lua and sol2 dependency in one module with a narrow named-series-to-derived-series API
- **IMP-015** [Extract persistence layer](IMP-015-persistence-layer.md) — Keep JSON serialization and project schema logic in one place with backward-readable schema evolution
- **IMP-016** [Restructure app model into domain submodules](IMP-016-app-model-submodules.md) — Split the monolithic app model into window, selection, cache, source, and project submodules
- **IMP-017** [Isolate UI rendering from app model](IMP-017-ui-rendering.md) — Organize rendering by panel and keep rendering functions free of business logic
- **IMP-018** [Extract ImGui backend from main.cpp](IMP-018-imgui-backend.md) — Move the concrete backend bridging to ImGui, GLFW, and ImPlot out of the entry point
- **IMP-019** [Fix stale IMP index](IMP-019-stale-imp-index.md) — Correct the improvements index so completed IMPs are listed as done with working paths
- **IMP-020** [Update contributing guide for module structure](IMP-020-stale-contributing-md.md) — Replace the monolithic project layout description with the current module layout and test files
- **IMP-021** [Add PB-012 to the decision log](IMP-021-missing-pb-012-decision-log.md) — Record the sidebar-layout decision in the centralized decision log
- **IMP-022** [Add docstrings to public API headers](IMP-022-missing-docstrings-public-api.md) — Document purpose, parameters, return values, preconditions, and side effects on all public API surfaces
- **IMP-023** [Update improvement backlog](IMP-023-update-improvement-backlog.md) — Reflect all completed improvements in the tracker
- **IMP-024** [Update verification doc for test files](IMP-024-update-verification-md-test-files.md) — List all three test files and their focus areas in the verification documentation
- **IMP-025** [Expand changelog](IMP-025-expand-changelog.md) — Record the implemented improvements in the changelog
- **IMP-026** [Create undiscovered-improvements placeholder](IMP-026-create-undiscovered-improvements.md) — Provide a designated empty-state file for ideas not yet ready to become IMP candidates
- **IMP-027** [Add module structure to the application plan](IMP-027-application-plan-md.md) — Connect the plan's architecture to the actual module layout in the codebase
- **IMP-028** [Add PB-012 to the traceability map](IMP-028-traceability-map-pb-012.md) — Trace the sidebar-layout decision through the cross-layer map
<!-- pb:index:end -->
