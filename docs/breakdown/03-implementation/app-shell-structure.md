---
id: INFO-028
type: info
title: App and UI code structure
summary: Source layout for the app shell, window and tab ownership, parameter selection, and the fixed-panel UI
date: 2026-11-11
status: current
---

# App and UI code structure

The app and UI code is split into focused source directories.

- **`src/app/`:** application startup, main loop, and routing between UI panels.
- **`src/app/windows/`:** top-level window and tab ownership.
- **`src/app/selection/`:** parameter binding menu and source-to-parameter mapping.
- **`src/ui/`:** fixed parameter sidebar, plot inspector, plot workspace, expression editor, and project dialogs.