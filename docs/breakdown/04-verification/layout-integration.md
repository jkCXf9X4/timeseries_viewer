---
id: INFO-038
type: info
title: Layout integration coverage
summary: Multi-window, multi-tab one-plot-per-tab layout, fixed side panels, and active-plot tracking on focus change
date: 2026-11-11
status: current
---

# Layout integration coverage

Integration tests exercise the window, tab, and side-panel layout.

- **Windows and tabs:** open multiple windows, create multiple tabs in a window, and verify each tab owns exactly one plot.
- **Side panels:** verify the parameter browser stays fixed on the left with resizable width and the plot inspector stays fixed on the right with resizable width.
- **Active plot:** verify both side panels update to the active plot when the focused tab changes.
- **Focus:** verify clicking a window or tab updates the active plot shown in both side panels.