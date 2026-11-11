---
id: INFO-018
type: info
title: Architecture boundaries
summary: The major module boundaries that partition UI, windows, binding, sources, registry, cache, expressions, persistence, and refresh
date: 2026-11-11
status: current
---

# Architecture boundaries

The architecture is partitioned into nine major boundaries.

- **UI shell:** UI shell and docking layout.
- **Window manager:** window manager and tabbed plot containers.
- **Parameter binding:** parameter binding and source-to-variable selection.
- **Source adapters:** source adapters for CSV and SQLite.
- **Variable registry:** the canonical variable registry.
- **Cache:** the time-series cache and derived-series model.
- **Expression engine:** expression evaluation.
- **Persistence:** project persistence.
- **Refresh:** background refresh and invalidation.