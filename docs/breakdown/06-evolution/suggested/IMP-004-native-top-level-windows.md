---
id: IMP-004
type: imp
title: Native top-level windows
summary: Create separate OS-level windows for analysis contexts synchronized with the same workspace model
date: 2026-11-11
status: suggested
---

# Native top-level windows

- **Question:** how do analysis windows become separate OS-level windows rather than ImGui windows inside one native window?
- **Evidence:** multiple analysis windows currently live inside one native app window, which blocks viewing windows on different monitors or arranging them independently in the desktop (`UC-005`, `INFO-023`).
- **Change:** analysis contexts create separate native windows, synchronized with the same workspace model, preserving tab and series state across window creation and closure.
- **Risk:** remote collaboration and cross-process window sharing are out of scope; multi-window state management complexity must stay isolated from the series model.
- **Acceptance:** a user opens more than one native analysis window, each hosts tabs with one plot per tab, and closing one window does not discard unrelated workspace state.