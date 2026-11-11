---
id: INFO-042
type: info
title: Plot and expression acceptance criteria
summary: Multi-series comparison, isolated per-tab selection, deterministic derived expressions, and non-corrupting live refresh
date: 2026-11-11
status: current
---

# Plot and expression acceptance criteria

The viewer is accepted when plotting, derivation, and refresh meet these checks.

- **Comparison:** a comparison plot can show multiple series together.
- **Isolation:** each plot tab keeps an isolated parameter selection.
- **Expressions:** derived expressions evaluate deterministically and render like normal series.
- **Live refresh:** live refresh updates visible data without corrupting the workspace.