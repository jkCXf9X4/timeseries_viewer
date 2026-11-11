---
id: INFO-019
type: info
title: Data flow
summary: How data moves from opening sources through adapters, the registry, binding, plotting, refresh, and persistence
date: 2026-11-11
status: current
---

# Data flow

Data flows from source opening through normalization, selection, plotting, refresh, and persistence.

- **Open:** the user opens one or more sources.
- **Normalize:** source adapters extract columns or tables and normalize them.
- **Expose:** the variable registry exposes selectable series in a tree.
- **Bind:** the parameter binding menu maps displayed parameters to one or more open sources.
- **Compose:** the user adds series or expressions to one or more tabs across one or more windows.
- **Render:** the plot workspace renders raw and derived series in the selected tab.
- **Refresh:** the refresh scheduler reloads changed sources and updates dependent plots.
- **Persist:** project persistence stores and restores the current state.