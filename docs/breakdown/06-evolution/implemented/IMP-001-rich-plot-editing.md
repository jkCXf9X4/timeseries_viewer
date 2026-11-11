---
id: IMP-001
type: imp
title: Richer plot editing controls
summary: Add remove, rebind, visibility, axis, and color editing for existing plot series
date: 2026-11-11
status: implemented
---

# Richer plot editing controls

- **Question:** how can a user inspect, reorganize, hide, and tune plotted series without rebuilding a tab from scratch?
- **Evidence:** a tab can add series but cannot edit existing plot content, which blocks production analysis workflows (`UC-003`, `CP-002`).
- **Change:** a tab can remove a series, toggle visibility, rebind to another source or variable, edit axis range and autoscale, and edit per-series presentation settings.
- **Risk:** scope pressure to replace the plot engine or add file formats; out of scope, kept to the plot editor.
- **Acceptance:** a tab removes and hides or shows an existing series, updates per-series presentation without recreating the plot, and the state survives save and reload.