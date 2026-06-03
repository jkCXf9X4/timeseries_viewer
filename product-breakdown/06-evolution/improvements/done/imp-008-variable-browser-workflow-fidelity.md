# IMP-008 Variable Browser Workflow Fidelity

## Problem

The suite verifies dotted-name tree construction, but it does not verify the full browsing workflow that a user follows to select variables from a large source.

## Why It Matters

The variable browser is the main discovery surface for UC-01. If selection, expand/collapse behavior, or scale behavior regresses, the app can remain technically correct while becoming hard to use.

## Scope

- Verify that leaf nodes, not parent nodes, drive series selection
- Verify that a selected leaf adds the expected variable to the active plot
- Add coverage for large variable sets so the browser remains usable at scale

## Out of Scope

- Redesigning the browser widget
- Adding search or filtering unless required by the browser implementation
- Source-format changes

## Progress State

`Done`

## Acceptance Criteria

- A test proves that selecting a dotted-name leaf results in the correct fully qualified series
- Parent nodes remain navigation-only
- The browser workflow remains stable with many variables present

## Traceability

- Product need: [01-product/use-cases](../../01-product/use-cases.md)
- Verification: [04-verification](../../04-verification/layer.md)
- Architecture: [02-architecture](../../02-architecture/layer.md)

## Implementation Notes

- The browser now renders a dotted-name tree built from bindable parameters rather than a flat column list.
- The render-path tests verify that parent nodes are navigation-only and that leaf selection adds the expected series even with large parameter sets.
