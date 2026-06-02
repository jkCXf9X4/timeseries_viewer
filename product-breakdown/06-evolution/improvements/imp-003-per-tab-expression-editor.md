# IMP-003 Per-Tab Expression Editor State

## Problem

The expression input is currently shared across the application, which is awkward once users work in several tabs or windows at the same time.

## Why It Matters

Derived series are part of the analysis context for a specific plot, not a global app-wide setting.

## Scope

- Store expression draft text per tab
- Keep derived-series editing local to the owning tab
- Restore per-tab drafts from saved project state if needed

## Out of Scope

- New expression languages
- Complex formula libraries

## Progress State

`Backlog`

## Acceptance Criteria

- Switching tabs does not overwrite another tab’s in-progress expression.
- Derived series can be edited from the tab that owns them.
- Saved workspace state preserves tab-local expression context when required.

## Traceability

- Product need: [01-product/use-cases](../../01-product/use-cases.md)
- Architecture: [02-architecture](../../02-architecture/layer.md)
- Verification: [04-verification](../../04-verification/layer.md)
