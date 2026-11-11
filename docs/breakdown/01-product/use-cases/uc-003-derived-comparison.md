---
id: UC-003
type: use-case
title: Derived comparison
summary: Plot a simple derived relationship such as `a - b` alongside the raw series without exporting to another tool
date: 2026-11-11
status: current
---

# Derived comparison

"Let me plot a derived relationship like `a - b` directly, without exporting to another tool."

- **Actor:** an analyst comparing two signals directly.
- **Goal:** plot a simple derived relationship such as `a - b`.
- **Request:** "compute this expression and show it next to the raw series."
- **Behavior:** derived series are treated like ordinary plot series once created.
- **Outcome:** the raw traces and the derived result appear together in the same plot, using a system simple enough for analysis during use.