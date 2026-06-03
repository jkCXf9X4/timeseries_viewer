# IMP-009 Parameter Menu Discoverability

## Problem

The state model can bind a series to a source, but the user-facing menu or panel behavior that exposes those bindings is not yet verified.

## Why It Matters

UC-04 requires an explicit selection surface where users can see which source each parameter comes from and change it without reopening files. If that affordance is weak or ambiguous, the workflow becomes guesswork.

## Scope

- Verify the parameter-selection panel or menu in the real UI
- Show source association explicitly for each selectable parameter
- Support filtering or grouping by open source in a user-visible way

## Out of Scope

- Replacing the underlying binding model
- Adding automatic source inference rules
- Introducing hidden or implicit selection behavior

## Progress State

`Done`

## Acceptance Criteria

- The user can inspect which open source owns a parameter
- The user can rebind a parameter through an explicit menu or panel
- The UI makes source/parameter associations visible rather than implicit

## Traceability

- Product need: [01-product/use-cases](../../01-product/use-cases.md)
- Verification: [04-verification](../../04-verification/layer.md)
- Implementation: [03-implementation](../../03-implementation/layer.md)

## Implementation Notes

- The parameter browser groups bindable parameters under each open source and shows the backing source path in the panel.
- The UI keeps source bindings explicit and supports rebinding the currently selected series from the tree.
