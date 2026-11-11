---
id: IMP-017
type: imp
title: Isolate UI rendering from app model
summary: Organize rendering by panel and keep rendering functions free of business logic
date: 2026-11-11
status: implemented
---

# Isolate UI rendering from app model

- **Question:** how does UI rendering separate from application logic while keeping the mockable backend pattern?
- **Evidence:** the UI rendering header directly includes and mutates the app model through rendering functions, mixing rendering with application logic (`INFO-030`).
- **Change:** rendering organizes by panel in `src/ui/panels/`, `src/ui/plot/`, and `src/ui/render.hpp`, preserving the `Ui` template backend injection and calling app functions rather than implementing business logic.
- **Risk:** the change must not create a parallel rendering abstraction or leak business logic into rendering functions.
- **Acceptance:** the UI renders identically after reorganization, the old rendering header is removed or reduced to forwarding includes, each panel compiles with its minimal include set, and no business logic leaks into rendering functions.