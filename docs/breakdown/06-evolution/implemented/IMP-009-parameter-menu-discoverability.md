---
id: IMP-009
type: imp
title: Parameter menu discoverability
summary: Verify the explicit user-facing menu or panel that exposes source bindings
date: 2026-11-11
status: implemented
---

# Parameter menu discoverability

- **Question:** how does the user-facing menu or panel expose and change source bindings, rather than leaving them implicit?
- **Evidence:** the state model can bind a series to a source, but the user-facing menu or panel behavior that exposes those bindings is not verified (`UC-004`, `INFO-024`).
- **Change:** the parameter-selection panel or menu is verified in the real UI, shows the source association for each selectable parameter, and supports filtering or grouping by open source.
- **Risk:** replacing the binding model or adding automatic source-inference rules is out of scope.
- **Acceptance:** the user can inspect which open source owns a parameter, rebind through an explicit menu or panel, and the UI makes source and parameter associations visible rather than implicit.