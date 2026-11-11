---
id: INFO-011
type: info
title: Left sidebar behavior
summary: Parameter browser and source control that opens sources, reloads, toggles live mode, saves projects, and ticks variables to plot
date: 2026-11-11
status: current
---

# Left sidebar behavior

The left sidebar is the parameter browser and source-control area.

- **Actions:** open CSV files and SQLite databases, reload sources, toggle live refresh, and open and save project files.
- **Variable add:** add and remove variables from the active plot using tickboxes.
- **Source tree:** show source trees built from dotted variable names.
- **Source browser:** sources group by open file or database, parent nodes are navigation only, and leaf nodes are bindable variables with tickboxes.
- **Clarity:** the browser makes the source of a variable clear enough to avoid ambiguous selection.
- **Active plot coupling:** the parameter browser always applies to the currently active plot context and does not duplicate the full plot summary shown in the right sidebar.