---
id: INFO-023
type: info
title: Window manager and plot workspace components
summary: Window manager owns top-level windows and tab sets; plot workspace owns overlays, axes, visibility, and per-plot selection
date: 2026-11-11
status: current
---

# Window manager and plot workspace components

The window manager and plot workspace own the multi-window, multi-tab layout.

- **Window manager:** owns top-level windows, each window's tab set, and the one-plot-per-tab rule.
- **Plot workspace:** owns overlays, axes, visibility, styling, and one plot per tab.
- **Per-plot state:** each plot keeps its own parameter-selection state.
- **Layout isolation:** the multi-window layout adds state-management complexity that must stay isolated from the underlying series model.