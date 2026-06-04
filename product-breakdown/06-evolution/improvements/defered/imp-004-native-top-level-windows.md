# IMP-004 Native Top-Level Windows

## Problem

The current implementation supports multiple analysis windows inside one native app window, not separate OS-level windows.

## Why It Matters

Some analysis workflows benefit from viewing windows on different monitors or independently arranging them in the desktop environment.

## Scope

- Create separate native windows for analysis contexts
- Keep each native window synchronized with the same workspace model
- Preserve tab and series state across window creation and closure

## Out of Scope

- Remote collaboration
- Cross-process window sharing

## Progress State

`Backlog`

## Acceptance Criteria

- A user can open more than one native analysis window.
- Each native window can host tabs and one plot per tab.
- Closing one window does not discard unrelated workspace state.

## Traceability

- Product need: [01-product/use-cases](../../01-product/use-cases.md)
- Architecture: [02-architecture](../../02-architecture/layer.md)
- Verification: [04-verification](../../04-verification/layer.md)
