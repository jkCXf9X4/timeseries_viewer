---
id: INFO-024
type: info
title: Parameter binding and source adapter components
summary: Explicit source-to-parameter mapping plus CSV and SQLite adapters that normalize inputs into a common representation
date: 2026-11-11
status: current
---

# Parameter binding and source adapter components

The binding menu and source adapters keep source-to-parameter mapping explicit and format-agnostic.

- **Binding menu:** presents available parameters grouped by open source and maintains the explicit mapping between displayed series and the file or database they come from.
- **Explicit binding:** parameter selection is explicit and bound to the open source or database that provides the series.
- **Source adapters:** normalize CSV and SQLite inputs into a common internal representation.
- **Format-agnostic:** plotting and expressions do not care about the original source format.