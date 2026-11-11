---
id: INFO-010
type: info
title: Visual layout
summary: A three-region desktop workspace with a left parameter browser, central analysis workspace, and right plot inspector
date: 2026-11-11
status: current
---

# Visual layout

The interface is a three-region desktop workspace.

- **Left sidebar:** the parameter browser and source-control area, anchored to the left edge, full viewport height, user-resizable width, fixed position.
- **Center workspace:** the active analysis windows, occupying the space between the sidebars, able to show multiple windows and multiple tabs with one plot per tab.
- **Right sidebar:** the plot inspector, anchored to the right edge, full viewport height, user-resizable width, fixed position.
- **Design constraint:** sidebars are fixed to their sides, span the full viewport height, and have adjustable widths.
- **Design constraint:** the central workspace stays usable as the viewport changes size.
- **Design constraint:** the inspector stays tied to the active plot, not a static window position.