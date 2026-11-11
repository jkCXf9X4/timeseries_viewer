---
id: INFO-013
type: info
title: Right sidebar behavior
summary: Plot inspector showing the active plot summary, plot and series settings, and derived-series controls
date: 2026-11-11
status: current
---

# Right sidebar behavior

The right sidebar is the plot inspector for the active plot.

- **Responsibilities:** show the active plot summary, edit tab-level plot settings, edit series settings, add derived series to the active tab, and show source and binding information for the selected series.
- **Active plot summary:** window title, tab title, and the selected series name; the right sidebar is the only place this summary is shown.
- **Plot settings:** tab title, autoscale toggles for X and Y axes, manual X and Y range entry when autoscale is off, and a point budget for large series.
- **Series list:** lists the series attached to the active tab, each selectable for editing, showable or hidden, recolorable, and removable.
- **Series detail:** raw series expose source alias and binding details; derived series expose their expression.
- **Selected series editor:** edits the series name, visibility, color, and for derived series the expression, and allows removal.
- **Derived controls:** an expression draft for the active tab creates a new derived series, scoped to the active tab rather than shared across the app.