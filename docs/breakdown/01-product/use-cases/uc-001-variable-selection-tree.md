---
id: UC-001
type: use-case
title: Variable selection tree
summary: Browse variables through an expandable tree where `.` in a name means hierarchy, and check leaves to plot them
date: 2026-11-11
status: current
---

# Variable selection tree

"Let me browse variables through an expandable tree and plot the ones I check, without scanning a flat list."

- **Actor:** an engineer inspecting a large source with long variable names.
- **Goal:** find a variable quickly and add or remove it from the active plot.
- **Request:** "show my variables grouped by the `.` in their names, and add or remove the ones I check."
- **Behavior:** parent nodes expand and collapse but are not selectable as series; leaf nodes use tickboxes that add or remove a variable from the active plot.
- **Outcome:** the checked variable appears in the active plot, the same logical name maps to the same fully qualified variable, and the tree stays usable with many variables present.