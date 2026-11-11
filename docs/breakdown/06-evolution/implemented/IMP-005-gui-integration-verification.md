---
id: IMP-005
type: imp
title: GUI integration verification
summary: Exercise the user interface workflow end to end with repeatable integration tests
date: 2026-11-11
status: implemented
---

# GUI integration verification

- **Question:** how does the test suite exercise the most fragile user-facing behavior in the UI end to end?
- **Evidence:** the core data behavior is validated but source selection, parameter binding, tab management, and plot ownership are not (`INFO-035`).
- **Change:** automated GUI-level checks cover source opening, parameter selection, tab creation, and derived-series creation through the user interface, and verify the workspace survives save and load through UI-driven flows.
- **Risk:** screenshot-heavy visual tests and cross-platform UI automation are out of scope for the Linux-first focus.
- **Acceptance:** primary user flows are covered by repeatable integration tests that fail when binding or workspace layout regresses, with the strategy documented in the verification layer (`INFO-037`).