---
id: INFO-017
type: info
title: Architecture structure
summary: Separates source ingestion, normalized time-series state, expression evaluation, view state, and UI rendering
date: 2026-11-11
status: current
---

# Architecture structure

The architecture separates concerns into source ingestion, normalized time-series state, expression evaluation, view state, and UI rendering.

- **Separation:** source ingestion, normalized time-series state, expression evaluation, view state, and UI rendering are distinct concerns.
- **In-memory model:** selected series load into memory for medium-file operation.
- **Normalization:** source adapters feed a shared normalized representation so plotting and expressions do not care about the original source format.
- **Isolation:** multi-window layout state management stays isolated from the underlying series model.