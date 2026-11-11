---
id: IMP-031
type: imp
title: More expression helpers
summary: Extend the expression language with additional helpers once basic arithmetic is stable
date: 2026-11-11
status: suggested
---

# More expression helpers

- **Question:** how do users express richer derived relationships than basic arithmetic?
- **Evidence:** the expression system supports simple arithmetic such as `a - b`, but users may need more helpers for real analysis (`UC-003`, `INFO-025`).
- **Change:** additional expression helpers are added once basic arithmetic is stable, keeping the narrow named-series API.
- **Risk:** helper growth must not erode determinism or complicate the controlled evaluation interface.
- **Acceptance:** new helpers evaluate deterministically through the existing narrow API and render like normal series.