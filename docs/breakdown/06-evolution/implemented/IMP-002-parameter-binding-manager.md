---
id: IMP-002
type: imp
title: Dedicated parameter binding manager
summary: Make source-to-parameter bindings explicit and rebindable in one binding view
date: 2026-11-11
status: implemented
---

# Dedicated parameter binding manager

- **Question:** how does a user inspect and change the source binding of each selected parameter when the same parameter exists in several files or databases?
- **Evidence:** the parameter-selection panel behaves mostly as an add-from-tree workflow instead of a true binding manager (`UC-004`, `INFO-024`).
- **Change:** one binding view shows all open sources and their selectable parameters, the current source binding per selected parameter, and rebinding without reopening the file, with filtering and grouping by source.
- **Risk:** automated source matching and network-backed binding resolution are out of scope.
- **Acceptance:** each selected parameter shows its source binding, the binding changes without reopening the source, and the UI distinguishes selected, available, and missing parameters.