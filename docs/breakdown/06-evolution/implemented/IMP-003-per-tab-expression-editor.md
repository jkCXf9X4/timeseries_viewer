---
id: IMP-003
type: imp
title: Per-tab expression editor state
summary: Scope expression draft text and derived-series editing to the owning tab
date: 2026-11-11
status: implemented
---

# Per-tab expression editor state

- **Question:** how does a user keep an in-progress derived expression while working in several tabs or windows at once?
- **Evidence:** the expression input is shared across the application, which is awkward once a user works in several tabs or windows (`UC-003`, `INFO-013`).
- **Change:** expression draft text is stored per tab, derived-series editing stays local to the owning tab, and per-tab drafts restore from saved project state when needed.
- **Risk:** new expression languages and complex formula libraries are out of scope.
- **Acceptance:** switching tabs does not overwrite another tab's in-progress expression, derived series are edited from the owning tab, and saved workspace state preserves tab-local expression context.