# IMP-002 Dedicated Parameter Binding Manager

## Problem

The parameter-selection panel exists, but it behaves mostly as an add-from-tree workflow instead of a true binding manager.

## Why It Matters

Users need to inspect and change bindings explicitly when the same parameter exists in multiple files or databases.

## Scope

- Show all open sources and their selectable parameters in one binding view
- Show the current source binding for each selected parameter
- Rebind an existing plot series to another source or table
- Support filtering and grouping by source

## Out of Scope

- Automated source matching
- Server-side or network-backed binding resolution

## Progress State

`Backlog`

## Acceptance Criteria

- Each selected parameter shows its source binding.
- The binding can be changed without reopening the source file.
- The UI clearly distinguishes selected, available, and missing parameters.

## Traceability

- Product need: [01-product/use-cases](../../01-product/use-cases.md)
- Architecture: [02-architecture](../../02-architecture/layer.md)
- Verification: [04-verification](../../04-verification/layer.md)
