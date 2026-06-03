# Improvement Backlog

This file tracks the remaining product improvements that are not required for the current core release but are still important for a production-quality tool.

## Status Legend

- `Backlog`: identified and not yet started
- `In Progress`: active implementation or design work
- `Blocked`: waiting on a dependency or decision
- `Done`: implemented and verified

## Backlog Items

| ID | Item | Status | Primary Gap | Trace |
| --- | --- | --- | --- | --- |
| IMP-001 | Richer plot editing controls | Done | No remove/rebind/visibility/axis/color editor for existing series | [done](improvements/done/imp-001-rich-plot-editing.md) |
| IMP-002 | Dedicated parameter binding manager | Done | Parameter selection is explicit but still mostly add-from-tree | [done](improvements/done/imp-002-parameter-binding-manager.md) |
| IMP-003 | Per-tab expression editor state | Done | Expression input is global instead of owned by each tab | [done](improvements/done/imp-003-per-tab-expression-editor.md) |
| IMP-004 | Native top-level windows | Backlog | Multiple analysis windows are currently ImGui windows inside one OS window | [candidate](improvements/candidates/imp-004-native-top-level-windows.md) |
| IMP-005 | GUI integration verification | Done | No automated end-to-end GUI interaction coverage | [done](improvements/done/imp-005-gui-integration-verification.md) |
| IMP-006 | Large-data scaling strategy | Done | Medium-file in-memory loading is the current ceiling | [done](improvements/done/imp-006-large-data-scaling.md) |
| IMP-007 | Rendered GUI workflow verification | Done | The harness is state-level only and does not exercise the shipped ImGui UI | [done](improvements/done/imp-007-rendered-gui-workflow-verification.md) |
| IMP-008 | Variable browser workflow fidelity | Done | Dotted-name tree construction is tested, but the browse-and-select workflow is not | [done](improvements/done/imp-008-variable-browser-workflow-fidelity.md) |
| IMP-009 | Parameter menu discoverability | Done | Source/parameter binding exists in state, but the explicit menu or panel behavior is not verified | [done](improvements/done/imp-009-parameter-menu-discoverability.md) |
| IMP-010 | Comparison label disambiguation | Done | Mixed-source comparisons are not asserted for clear, distinct labels | [done](improvements/done/imp-010-comparison-label-disambiguation.md) |

## Notes

- These items are intentionally separated from the core product layers so the release boundary stays clear.
- Each candidate file should be updated as design work progresses and should be linked from the relevant product, architecture, verification, or implementation artifacts when it starts affecting them.
- The current implementation now includes a scripted render-path harness, so the GUI workflow verification is covered at the shared render layer as well as at the state layer.
