---
id: UC-005
type: use-case
title: Multi-window tabbed plot workspace
summary: Keep separate analysis contexts visible at once, each with tabs and one plot per tab, with fixed side panels following the active plot
date: 2026-11-11
status: current
---

# Multi-window tabbed plot workspace

"Let me keep several analysis contexts visible at once, each with its own tabs and plots."

- **Actor:** a user who wants one window per scenario or comparison task.
- **Goal:** view multiple windows simultaneously, each with multiple tabs and one plot per tab with its own parameter selection.
- **Request:** "open more windows and tabs, and let each tab have its own selection."
- **Behavior:** more than one top-level window is visible at once, each window has multiple tabs, each tab owns one plot, and a plot does not inherit another tab's selection.
- **Outcome:** the user compares plots by moving between tabs or viewing windows at once, the parameter browser stays fixed on the left and the plot inspector on the right (widths resizable), and clicking a window or tab makes that plot the active one for both side panels.