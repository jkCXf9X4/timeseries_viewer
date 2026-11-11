---
id: INFO-048
type: info
title: Support scenarios
summary: Moved sources, changed SQLite schemas, disappearing variables, and cross-machine project paths
date: 2026-11-11
status: current
---

# Support scenarios

The product must cope with four common failure scenarios.

- **Moved source:** a source file is moved or renamed.
- **Schema change:** a SQLite table schema changes between sessions.
- **Live loss:** a variable disappears from a live source.
- **Cross-machine:** a project file is opened on another machine with partial path differences.