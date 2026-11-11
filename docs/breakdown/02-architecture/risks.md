---
id: INFO-021
type: info
title: Architecture risks
summary: In-memory scale limits, polling latency, expression determinism, and multi-window state complexity
date: 2026-11-11
status: current
---

# Architecture risks

The architecture carries four main risks.

- **Scale:** in-memory loading may not scale to very large sources without later redesign.
- **Immediacy:** polling is simple but less immediate than file notifications.
- **Expression determinism:** expression evaluation needs a controlled interface so results stay deterministic and explainable.
- **Layout complexity:** a multi-window layout adds state-management complexity that must stay isolated from the underlying series model.