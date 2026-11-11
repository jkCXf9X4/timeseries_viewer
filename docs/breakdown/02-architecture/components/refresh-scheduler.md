---
id: INFO-026
type: info
title: Refresh scheduler component
summary: Polls sources for changes and invalidates or reloads affected series when live mode is enabled
date: 2026-11-11
status: current
---

# Refresh scheduler component

The refresh scheduler keeps live mode up to date.

- **Polling:** polls sources for changes.
- **Reload:** invalidates or reloads affected series when live mode is enabled.
- **Choice:** live updates use polling rather than platform file watchers.