# IMP-005 GUI Integration Verification

## Problem

The current test suite validates core data behavior, but it does not exercise the GUI workflow end to end.

## Why It Matters

The most fragile user-facing behavior lives in the UI: source selection, parameter binding, tab management, and plot ownership.

## Scope

- Add automated GUI-level checks where practical
- Verify source opening, parameter selection, tab creation, and derived series creation through the user interface
- Check that the workspace survives save/load through UI-driven flows

## Out of Scope

- Screenshot-heavy visual design tests
- Cross-platform UI automation that would distort the Linux-first focus

## Progress State

`Backlog`

## Acceptance Criteria

- The primary user flows are covered by repeatable integration tests.
- The tests fail when the binding or workspace layout regresses.
- The test strategy is documented in the verification layer.

## Traceability

- Product need: [01-product/use-cases](../../01-product/use-cases.md)
- Verification layer: [04-verification](../../04-verification/layer.md)
