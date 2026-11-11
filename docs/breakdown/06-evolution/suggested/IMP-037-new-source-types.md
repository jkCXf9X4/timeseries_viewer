---
id: IMP-037
type: imp
title: New source types beyond CSV and SQLite
summary: Add additional file and metadata source types while preserving the adapter boundary
date: 2026-11-11
status: suggested
---

# New source types beyond CSV and SQLite

- **Question:** how does the viewer extend to source types beyond CSV and SQLite?
- **Evidence:** CSV and SQLite are the initial supported source types, and other engineering formats may be needed (`INFO-007`, `INFO-024`).
- **Change:** additional source types are added behind the source-adapter boundary, feeding the same normalized representation.
- **Risk:** each new source adds surface area; feature expansion must not erode the low-dependency goal.
- **Acceptance:** a new source type opens and normalizes through its own adapter without changes to plotting or expressions.