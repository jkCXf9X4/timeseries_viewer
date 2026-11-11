---
id: INFO-039
type: info
title: Workflow coverage
summary: Live reload on source change, clear reporting of missing variables, and preserved layout on reload
date: 2026-11-11
status: current
---

# Workflow coverage

Workflow tests cover live mode and recovery behavior.

- **Live reload:** live mode detects a changed source and reloads the affected series.
- **Missing variable:** a missing or renamed variable is reported clearly.
- **Preservation:** reloaded data keeps the view layout and selected plots intact where possible.