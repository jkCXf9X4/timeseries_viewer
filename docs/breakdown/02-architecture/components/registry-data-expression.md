---
id: INFO-025
type: info
title: Registry, expression, and persistence components
summary: Variable registry maps fully qualified names; expression engine evaluates derived series with interpolation; persistence stores and restores state
date: 2026-11-11
status: current
---

# Registry, expression, and persistence components

The registry, expression engine, and persistence layer handle data identity, derivation, and storage.

- **Variable registry:** maps fully qualified names to source-local series objects and preserves dotted hierarchy for the browser tree.
- **Expression engine:** evaluates derived series from named inputs and produces a normal plottable result.
- **Expression stack:** derived series use embedded Lua with `sol2` bindings for expressions.
- **Alignment:** cross-series comparison aligns to the left-hand time grid using interpolation.
- **Persistence layer:** serializes and restores the workspace, source selections, plot layout, and expressions.
- **Output control:** expression evaluation goes through a controlled interface so results stay deterministic and explainable.