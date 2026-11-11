---
id: UC-004
type: use-case
title: Parameter binding menu
summary: Choose which parameters display and bind each to an open file or database through an explicit menu
date: 2026-11-11
status: current
---

# Parameter binding menu

"Let me say which parameter comes from which source through an explicit menu, without guessing how the app interpreted my names."

- **Actor:** a user with several open files or databases who wants control over source mapping.
- **Goal:** choose which parameters display and bind them to one or more open sources.
- **Request:** "show this parameter from that source."
- **Behavior:** a menu or panel shows available parameters and their source binding, with tickboxes to assign parameters to the current plot; the binding can change without reopening the file.
- **Outcome:** the plot shows exactly the chosen source and parameter combination, the source of each parameter is visible, and the selection state stays explicit and stable across the session.