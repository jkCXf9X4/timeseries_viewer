---
id: INFO-030
type: info
title: App and UI interfaces
summary: One-plot-per-tab ownership, fixed side panels tied to the active plot, and ready-to-render series consumption
date: 2026-11-11
status: current
---

# App and UI interfaces

The app and UI layers hold explicit interface contracts.

- **Window and tab layer:** owns exactly one plot per tab and keeps per-tab parameter state isolated.
- **UI shell:** pins the parameter browser to the left and the plot inspector to the right while keeping both panels tied to the active plot.
- **Plot layer:** consumes ready-to-render series objects rather than raw files.