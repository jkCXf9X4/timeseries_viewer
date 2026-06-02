# IMP-001 Richer Plot Editing Controls

## Problem

The application can add series to tabs, but existing plot content is not yet editable enough for production analysis workflows.

## Why It Matters

Users need to inspect, reorganize, hide, and tune plotted series without rebuilding a tab from scratch.

## Scope

- Remove a series from a tab
- Toggle series visibility
- Rebind a series to a different source or variable
- Edit axis range and autoscale settings
- Edit color or other per-series presentation settings

## Out of Scope

- Replacing the plot engine
- Adding new file formats
- Changing the project file schema beyond what the plot editor needs

## Progress State

`Backlog`

## Acceptance Criteria

- A tab can remove an existing series.
- A tab can hide or show an existing series.
- A tab can update per-series presentation settings without recreating the plot.
- The state survives save and reload.

## Traceability

- Product need: [01-product/use-cases](../../01-product/use-cases.md)
- Architecture: [02-architecture](../../02-architecture/layer.md)
- Verification: [04-verification](../../04-verification/layer.md)
