---
id: INFO-047
type: info
title: Operational behavior
summary: Session-only unsaved state, in-UI error context, predictable polling refresh, and local project files
date: 2026-11-11
status: current
---

# Operational behavior

The product behaves predictably during normal operation.

- **Unsaved state:** preserve unsaved view state only within the current session.
- **Errors:** surface import and expression errors in the UI with enough context to fix them.
- **Polling:** keep polling behavior predictable so users know when live data will refresh.
- **Projects:** keep saved project files tied to the user's local file layout, not to remote state.